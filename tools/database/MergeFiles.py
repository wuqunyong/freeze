import os
import sys
import logging
import shutil
import time
import traceback
import hashlib
import re
import yaml


sTimestamp = time.strftime("%Y%m%d_%H%M%S", time.localtime())
sLogName = "MergeFiles_" + sTimestamp + ".log"
logging.basicConfig(level=logging.INFO, filename=sLogName, filemode="w", format="%(asctime)s | %(levelname)s | %(message)s")

sAutoMergeFilePrefix = "AutoMergeFiles_"
sPattern = "^AutoMergeFiles[_].*[.]sql$"

sCreateType = "base"
sUpdateType = "update"
sExecuteSqlRecordName = "ExecuteSqlRecord"


def RemoveFiles(sDir, sPattern, removeList):
    if os.path.exists(sDir):
        files = os.listdir(sDir)
        for file in files:
            m = os.path.join(sDir, file)
            if not os.path.isfile(m):
                continue
            if re.match(sPattern, file):
                os.remove(m)
                removeList.append(m)

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
        return sPath, config

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

def InExecuteSqlRecord(sAddress, fileName):
    sRecordName = sExecuteSqlRecordName + "_" + sAddress + ".txt"

    if os.path.exists(sRecordName):
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


def main(argc, argv):
    if argc < 2:
        logging.error("invalid argc:{}, argv:{}".format(argc, argv))
        raise Exception("invalid argc:{}, argv:{}".format(argc, argv))

    fileList = []
    sConfigPath = argv[1]
    sDir, configObj = CollectSqlFile(fileList, sConfigPath)

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

    removeList = []
    RemoveFiles(sDir, sPattern, removeList)
    logging.info("remove files:{}".format(removeList))

    filelist = []  #[(type, database, filePath)]
    for sType, dDatabase in sortMap.items():
        for sDatabase, lTables in dDatabase.items():
            for sTable in lTables:
                tData = (sType, sDatabase, sTable)
                filelist.append(tData)

    logging.info("files:{}".format(filelist))

    # 页眉
    sMergeHeader = """
-- ***********************MergeBegin***********************
-- dir:{}
-- date:{}
-- database:{}
-- files:
"""

    # 文件头
    sFileHead = """


-- --------------------------------------------------------
-- filename:{}
-- md5:{}
-- --------------------------------------------------------

"""

    # 页脚
    sMergeFooter = """

-- --------------------------------------------------------
-- date:{}
-- ************************MergeEnd************************
"""

    for dMysql in configObj["database"]:
        sAddress = "{}_{}".format(dMysql["host"], dMysql["port"])

        dCheckFile = {} #{key:[(type, database, filePath)]}
        for tData in filelist:
            sCheckKey = tData[0] + "." + tData[1]
            if sCheckKey not in dCheckFile:
                dCheckFile[sCheckKey] = []

            sPrefix = tData[0] + "|" + tData[1] + "|"
            _, sFileName = os.path.split(tData[2])
            sKey = sPrefix + sFileName

            bIn = InExecuteSqlRecord(sAddress, sKey)

            sFile = tData[2]
            logging.info("{} in {} : {}".format(sFile, sAddress, bIn))

            tCheck = (tData[0], tData[1], tData[2], bIn)
            dCheckFile[sCheckKey].append(tCheck)


        for sCheckDir, lCheckFile in dCheckFile.items():
            sChangeFlag = "Empty"
            for tData in lCheckFile:
                bIn = tData[3]
                if not bIn:
                    sChangeFlag = "Change"
                    break

            sMergeName = sAutoMergeFilePrefix + sAddress + "_" + sCheckDir + "_" + sTimestamp + "_" + sChangeFlag + ".sql"

            with open(sMergeName, "w") as fdst:
                sData = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
                fdst.write(sMergeHeader.format(sDir, sData, sCheckDir))


            sFileElmes = "--           {}\n"

            for tData in lCheckFile:
                sFile = tData[2]
                bIn = tData[3]
                if bIn:
                    continue

                with open(sMergeName, "a") as fdst:
                    fdst.write(sFileElmes.format(sFile))

            for tData in lCheckFile:
                sFile = tData[2]
                bIn = tData[3]
                if bIn:
                    continue

                with open(sMergeName, "a") as fdst:
                    sMd5 = FileMd5(sFile)
                    fdst.write(sFileHead.format(sFile, sMd5))

                with open(sMergeName, "ab") as fdst:
                    with open(sFile, "rb") as fsrc:
                        shutil.copyfileobj(fsrc, fdst)

            with open(sMergeName, "a") as fdst:
                sData = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
                fdst.write(sMergeFooter.format(sData))


if __name__ == "__main__":
    try:
        print("合并文件开始\n")
        print("args:{}\n".format(sys.argv))

        logging.info("合并文件开始")
        logging.info("args:{}".format(sys.argv))
        main(len(sys.argv), sys.argv)
    except:
        tException = sys.exc_info()
        logging.error("exception | {}".format(tException))

        sTrace = traceback.format_exc()
        logging.error("traceback | {}".format(sTrace))

        logging.error("合并文件失败, Failure")
        print("合并文件失败, Failure, logFile:{}\n".format(sLogName))
        os._exit(1)
    else:
        logging.info("合并文件完成, Success")
        print("合并文件完成, Success, logFile:{}\n".format(sLogName))
        os._exit(0)
