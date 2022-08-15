import os
import sys
import logging
import time
import pymysql
import yaml
from pathlib import Path
import traceback
import hashlib

sDate = time.strftime("%Y%m%d_%H%M%S", time.localtime())
sLogName = "SyncSql_" + sDate + ".log"
logging.basicConfig(level=logging.INFO, filename=sLogName, filemode="w", format="%(asctime)s | %(levelname)s | %(message)s")


sExecuteSqlRecordName = "ExecuteSqlRecord"
sCreateType = "base"
sUpdateType = "update"

def FileMd5(fileName):
    md5_hash = hashlib.md5()
    with open(fileName, "rb") as f:
        # Read and update hash in chunks of 4K
        for byte_block in iter(lambda: f.read(4096), b""):
            md5_hash.update(byte_block)
        return md5_hash.hexdigest()

def TraversalDir(listObj, path, sType, sDatabase):
    if os.path.exists(path):
        files = os.listdir(path)
        for file in files:
            m = os.path.join(path, file)
            if os.path.isdir(m):
                continue
            elif os.path.isfile(m):
                # remainder, filename = os.path.split(m)
                if file.endswith(".sql"):
                    tData = (sType, sDatabase, m)
                    listObj.append(tData)

def CollectSqlFile(fileList, path):
    if not os.path.isabs(path):
        path = os.path.abspath(path)
    with open(path, 'r', encoding="utf-8") as file:
        config = yaml.safe_load(file)
        sPath, _ = os.path.split(path)
        logging.info("sql_path:{}".format(sPath))

        sCreateDir = os.path.join(sPath, sCreateType)
        if os.path.exists(sCreateDir):
            files = os.listdir(sCreateDir)
            for file in files:
                if file.startswith("."):
                    continue
                dbDirName = os.path.join(sCreateDir, file)
                if os.path.isdir(dbDirName):
                    sDbDir = os.path.join(sCreateDir, dbDirName)
                    TraversalDir(fileList, sDbDir, sCreateType, file)

        sUpdateDir = os.path.join(sPath, sUpdateType)
        if os.path.exists(sUpdateDir):
            files = os.listdir(sUpdateDir)
            for file in files:
                if file.startswith("."):
                    continue
                dbDirName = os.path.join(sUpdateDir, file)
                if os.path.isdir(dbDirName):
                    sDbDir = os.path.join(sUpdateDir, dbDirName)
                    TraversalDir(fileList, sDbDir, sUpdateType, file)
        return config

def sortFunc(sTable):
    sPath, sFile = os.path.split(sTable)
    fields = sFile.split("_")

    iLen = len(fields)
    iNum = 0
    if iLen > 0 and fields[0].isdigit():
        iNum = int(fields[0]) * 10000
    if iLen > 1 and fields[1].isdigit():
        iNum += int(fields[1])
    tData = (iNum, sTable)
    return tData

def ParseSql(filename):
    fd = open(filename, 'r', encoding='utf-8')  # 以只读的方式打开sql文件
    data = fd.readlines()
    fd.close()

    stmts = []
    DELIMITER = ';'
    stmt = ''

    for lineno, line in enumerate(data):
        if not line.strip():
            continue

        if line.startswith('--'):
            continue

        if 'DELIMITER' in line:
            DELIMITER = line.split()[1]
            continue

        stmt += line
        if DELIMITER in line:
            stmts.append(stmt.strip())
            stmt = ''

    if stmt:
        stmts.append(stmt)

    return stmts

def InExecuteSqlRecord(sAddress, fileName):
    sRecordName = sExecuteSqlRecordName + "_" + sAddress + ".txt"
    fle = Path(sRecordName)
    fle.touch(exist_ok=True)

    with open(sRecordName, "r") as fObj:
        lData = fObj.readlines()

        for name in lData:
            sName = name.strip()
            tField = sName.split()
            iLen = len(tField)
            if iLen > 2:
                sCheck = tField[2]
                sCheckName = sCheck.strip()
                sFileName = fileName.strip()
                if sCheckName == sFileName:
                    return True
    return False

def AddExecuteSqlRecord(sAddress, fileName, sMd5):
    sRecordName = sExecuteSqlRecordName + "_" + sAddress + ".txt"
    sDate = time.strftime("%Y-%m-%d_%H:%M:%S", time.localtime())
    with open(sRecordName, "a") as fObj:
        sData = "{}    {}    {}\n".format(sDate, sMd5, fileName)
        fObj.write(sData)
        logging.info("AddExecuteSqlRecord | record:{} | table:{}".format(sRecordName, fileName))

def ConnectMysql(bCreate, dConfig, sDatabase, sCharset):
    if bCreate:
        connection = pymysql.connect(host=dConfig["host"], port=dConfig["port"], user=dConfig["user"],
                                     password=dConfig["passwd"], charset=sCharset)
    else:
        connection = pymysql.connect(host=dConfig["host"], port=dConfig["port"], user=dConfig["user"],
                                     password=dConfig["passwd"], database=sDatabase, charset=sCharset)
    return connection

def ExecuteSql(sAddress, bCheck, connection, fileName, dSqlData):
    logging.info("ExecuteSql | sAddress:{} | bCheck:{} | connection:{} | fileName:{} | dSqlData:{}".format(
        sAddress, bCheck, connection, fileName, dSqlData))

    lStatement = dSqlData["lSql"]
    sMd5 = dSqlData["md5"]

    if len(lStatement) == 0:
        return

    if bCheck:
        bResult = InExecuteSqlRecord(sAddress, fileName)
        if bResult:
            return

    connection.ping()
    # with connection:
    with connection.cursor() as cursor:
        for sql in lStatement:
            logging.info("execute | sql:{}".format(sql))
            cursor.execute(sql)

    # connection is not autocommit by default. So you must commit to save
    # your changes.
    connection.commit()

    if bCheck:
        AddExecuteSqlRecord(sAddress, fileName, sMd5)

def main(argc, argv ):
    if argc < 2:
        logging.error("invalid argc:{}".format(argc))
        raise Exception("invalid argc:{}".format(argc))

    fileList = []
    sConfigPath = argv[1]
    configObj = CollectSqlFile(fileList, sConfigPath)

    sortMap = {}  # {"update": {"database": ["1_1_table", "1_2_table"]}}
    for items in fileList:
        sType, sDatabase, sTable = items
        if sType not in sortMap:
            sortMap[sType] = {}
        if sDatabase not in sortMap[sType]:
            sortMap[sType][sDatabase] = []
        sortMap[sType][sDatabase].append(sTable)

    for database in sortMap.values():
        for listTable in database.values():
            listTable.sort(reverse=False, key=sortFunc)

    logging.info("sort tables:{}".format(sortMap))


    dSqlMap = {} # {"update": {"database": {"update_database_table":{"lSql":[statement1, statement2], "fileName":name, "md5":md5}}}}
    for sType, dDatabase in sortMap.items():
        if sType not in dSqlMap:
            dSqlMap[sType] = {}
        for sDatabase, listTable in dDatabase.items():
            if sDatabase not in dSqlMap[sType]:
                dSqlMap[sType][sDatabase] = {}

            sPrefix = sType + "|" + sDatabase + "|"
            for sFile in listTable:
                lStatement = ParseSql(sFile)
                _, sFileName = os.path.split(sFile)
                sKey = sPrefix + sFileName

                dSqlData = {}
                dSqlData["lSql"] = lStatement
                dSqlData["fileName"] = sFile
                dSqlData["md5"] = FileMd5(sFile)
                dSqlMap[sType][sDatabase][sKey] = dSqlData

    logging.info("sort statement:{}".format(dSqlMap))

    if sCreateType in dSqlMap:
        for sDatabase, dTable in dSqlMap[sCreateType].items():
            for dMysql in configObj["database"]:
                logging.info("base | ConnectMysql | config:{} | database:{} | charset:{}".format(dMysql, sDatabase, configObj["charset"]))
                createConn = ConnectMysql(True, dMysql, sDatabase, configObj["charset"])

                lInit = []
                sCreateDatabase = "CREATE DATABASE IF NOT EXISTS `{}` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;".format(sDatabase)
                lInit.append(sCreateDatabase)
                sUseDatabase = "USE `{}`;".format(sDatabase)
                lInit.append(sUseDatabase)

                dInitData = {}
                dInitData["lSql"] = lInit
                dInitData["fileName"] = ""
                dInitData["md5"] = ""

                sAddress = "{}_{}".format(dMysql["host"], dMysql["port"])
                ExecuteSql(sAddress, False, createConn, "", dInitData)

                for sTable, dSqlData in dTable.items():
                    logging.info("ExecuteSql | file:{}".format(sTable))
                    ExecuteSql(sAddress, True, createConn, sTable, dSqlData)

    if sUpdateType in dSqlMap:
        for sDatabase, dTable in dSqlMap[sUpdateType].items():
            for dMysql in configObj["database"]:
                logging.info("update | ConnectMysql | config:{} | database:{} | charset:{}".format(dMysql, sDatabase, configObj["charset"]))
                updateConn = ConnectMysql(False, dMysql, sDatabase, configObj["charset"])

                sAddress = "{}_{}".format(dMysql["host"], dMysql["port"])
                for sTable, dSqlData in dTable.items():
                    logging.info("ExecuteSql | file:{}".format(sTable))
                    ExecuteSql(sAddress, True, updateConn, sTable, dSqlData)


if __name__ == "__main__":
    try:
        print("数据同步开始\n")
        print("args:{}\n".format(sys.argv))

        logging.info("数据同步开始")
        logging.info("args:{}".format(sys.argv))
        main(len(sys.argv), sys.argv)
    except:
        tException = sys.exc_info()
        logging.error("Exception | {}".format(tException))

        sTrace = traceback.format_exc()
        logging.error("traceback | {}".format(sTrace))

        logging.error("数据同步失败, Failure")
        print("数据同步失败, Failure, logFile:{}\n".format(sLogName))
        os._exit(1)
    else:
        logging.info("数据同步完成, Success")
        print("数据同步完成, Success, logFile:{}\n".format(sLogName))
        os._exit(0)
