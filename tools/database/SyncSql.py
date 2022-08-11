import os
import sys
import logging
import shutil
import time
import json
import pymysql
import yaml
from pathlib import Path

sDate = time.strftime("%Y%m%d_%H%M%S", time.localtime())
sLogName = "SyncSql_" + sDate + ".log"
logging.basicConfig(level=logging.INFO, filename=sLogName, filemode="w", format="%(asctime)s | %(levelname)s | %(message)s")


sExecuteSqlRecordName = "ExecuteSqlRecord"
sCreateType = "create"
sUpdateType = "update"

def TraversalDir(listObj, path, sType, sDatabase):
    if os.path.exists(path):
        files = os.listdir(path)
        for file in files:
            m = os.path.join(path, file)
            if os.path.isdir(m):
                TraversalDir(listObj, m)
            elif os.path.isfile(m):
                remainder, filename = os.path.split(m)
                if m.endswith(".sql"):
                    tData = (sType, sDatabase, m)
                    listObj.append(tData)

def CollectSqlFile(fileList, path):
    with open(path, 'r', encoding="utf-8") as file:
        try:
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
            return True, config
        except yaml.YAMLError as exc:
            logging.error("parse yaml error | path:{}".format(path))
            return False, None
    return False, None

def sortFunc(sTable):
    sPath, sFile = os.path.split(sTable)
    fields = sFile.split("_")

    iNum = 0
    if fields[0].isdigit():
        iNum = int(fields[0]) * 10000
    if fields[1].isdigit():
        iNum += int(fields[1])
    return iNum

def ParseSql(filename):
    try:
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
    except Exception as msg:
        logging.error("ParseSql error | file:{} | error:{}".format(filename, msg))
        return None

def InExecuteSqlRecord(sAddress, fileName):
    sRecordName = sExecuteSqlRecordName + "_" + sAddress + ".txt"
    fle = Path(sRecordName)
    fle.touch(exist_ok=True)

    with open(sRecordName, "r") as fObj:
        lData = fObj.readlines()

        for name in lData:
            sName = name.strip()
            sFileName = fileName.strip()
            if sName == sFileName:
                return True
    return False

def AddExecuteSqlRecord(sAddress, fileName):
    sRecordName = sExecuteSqlRecordName + "_" + sAddress + ".txt"
    with open(sRecordName, "a") as fObj:
        sData = "{}\n".format(fileName)
        fObj.write(sData)
        logging.info("AddExecuteSqlRecord | file:{} | table:{}".format(sRecordName, fileName))

def ConnectMysql(bCreate, dConfig, sDatabase, sCharset):
    if bCreate:
        connection = pymysql.connect(host=dConfig["host"], port=dConfig["port"], user=dConfig["user"],
                                     password=dConfig["passwd"], charset=sCharset)
    else:
        connection = pymysql.connect(host=dConfig["host"], port=dConfig["port"], user=dConfig["user"],
                                     password=dConfig["passwd"], database=sDatabase, charset=sCharset)
    return connection

def ExecuteSql(sAddress, bCheck, connection, fileName, lStatement):
    logging.info("ExecuteSql | sAddress:{} | bCheck:{}| connection:{} | fileName:{} | lStatement:{}".format(
        sAddress, bCheck, connection, fileName, lStatement))

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
        AddExecuteSqlRecord(sAddress, fileName)

def main(argc, argv ):
    if argc < 2:
        logging.error("invalid argc:{}".format(argc))
        return

    fileList = []
    sConfigPath = argv[1]
    bResult, configObj = CollectSqlFile(fileList, sConfigPath)
    if not bResult:
        logging.error("CollectSqlFile error | path:{}".format(sConfigPath))
        return

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

    dSqlMap = {} # {"update": {"database": {"update_database_table":[statement1, statement2]}}}
    for sType, dDatabase in sortMap.items():
        if sType not in dSqlMap:
            dSqlMap[sType] = {}
        for sDatabase, listTable in dDatabase.items():
            if sDatabase not in dSqlMap[sType]:
                dSqlMap[sType][sDatabase] = {}

            sPrefix = sType + "|" + sDatabase + "|"
            for sFile in listTable:
                lStatement = ParseSql(sFile)
                if lStatement is None:
                    logging.error("ParseSql error| file:{}".format(sFile))
                    return
                _, sFileName = os.path.split(sFile)
                sKey = sPrefix + sFileName
                dSqlMap[sType][sDatabase][sKey] = lStatement

    logging.info("sort statement:{}".format(dSqlMap))

    if sCreateType in dSqlMap:
        for sDatabase, dTable in dSqlMap[sCreateType].items():
            for dMysql in configObj["database"]:
                logging.info("create | ConnectMysql | config:{} | database:{} | charset:{}".format(dMysql, sDatabase, configObj["charset"]))
                createConn = ConnectMysql(True, dMysql, sDatabase, configObj["charset"])

                lInit = []
                sCreateDatabase = "CREATE DATABASE IF NOT EXISTS `{}` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;".format(sDatabase)
                lInit.append(sCreateDatabase)
                sUseDatabase = "USE `{}`;".format(sDatabase)
                lInit.append(sUseDatabase)

                sAddress = "{}_{}".format(dMysql["host"], dMysql["port"])
                ExecuteSql(sAddress, False, createConn, "", lInit)

                for sTable, lSql in dTable.items():
                    logging.info("ExecuteSql | file:{}".format(sTable))
                    ExecuteSql(sAddress, True, createConn, sTable, lSql)

    if sUpdateType in dSqlMap:
        for sDatabase, dTable in dSqlMap[sUpdateType].items():
            for dMysql in configObj["database"]:
                logging.info("update | ConnectMysql | config:{} | database:{} | charset:{}".format(dMysql, sDatabase, configObj["charset"]))
                updateConn = ConnectMysql(False, dMysql, sDatabase, configObj["charset"])

                sAddress = "{}_{}".format(dMysql["host"], dMysql["port"])
                for sTable, lSql in dTable.items():
                    logging.info("ExecuteSql | file:{}".format(sTable))
                    ExecuteSql(sAddress, True, updateConn, sTable, lSql)


if __name__ == "__main__":
    logging.info("数据同步开始")
    logging.info("args:{}".format(sys.argv))
    
    main(len(sys.argv), sys.argv)
    logging.info("Success 数据同步完成")
