# -*- coding: utf-8 -*-

import json
import os
import logging
import traceback
import json
import importlib
import sys

from xlrd import open_workbook
from _symtable import CELL
from telnetlib import theNULL
from jinja2 import Template
from enum import Enum

logger = logging.getLogger('mylogger')
logger.setLevel(logging.DEBUG)
# 创建一个handler，用于写入日志文件
fh = logging.FileHandler('DumpInfo.log', mode='w')

# 再创建一个handler，用于输出到控制台
ch = logging.StreamHandler()

# 定义handler的输出格式formatter
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)
ch.setFormatter(formatter)

logger.addHandler(fh)
logger.addHandler(ch)

class EnumObj:
    def __init__(self):
        self.sName = ""
        self.sComment = ""
        self.lField = []

    def reset(self):
        self.sName = ""
        self.lField = []

    def getName(self):
        return self.sName
    def setName(self, name):
        self.sName = name
    def setComment(self, comment):
        self.sComment = comment
    def appendField(self, lValue):
        if len(self.sName) == 0:
            raise Exception("Enum name empty")

        self.lField.append(lValue)
    def getData(self):
        dData = {}
        dData["commnet"] = self.sComment
        dData["name"] = self.sName
        dData["field"] = self.lField
        return dData

class MetaType(Enum):
    ENUM = 1

class MetaInfo:
    def __init__(self):
        self.dData = {}

    def appendEnumType(self, name, fileName):
        dTypeData = self.dData.setdefault(MetaType.ENUM.name, {})
        sPbName = fileName + "_pb2"
        dTypeData[name] = sPbName

    def getData(self):
        return self.dData

class ExcelFieldType:
    def __init__(self):
        self.dData = {}

    def appendEnumType(self, name, fileName):
        sEnumeName = MetaType.ENUM.name + "|" + name;
        moduleObj = importlib.import_module(fileName)
        self.dData[sEnumeName] = (name, moduleObj)

    def getModule(self, name):
        if name not in self.dData:
            return None
        return self.dData[name]


metaInfoObj = MetaInfo()
excelFieldTypeObj = ExcelFieldType()

def Enum_Generator(sName, s):
    sSheetName = s.name
    lLoadedObj = []
    try:
        bLoading = False
        loadObj = EnumObj()
        for row in range(s.nrows):
            # 第一行为描述，忽略
            if 0 == row:
                continue

            lField = []
            for col in range(s.ncols):
                # 第一列为备注，忽略
                if 0 == col:
                    continue

                cellVal = s.cell(row, col).value
                strType = type(cellVal)

                if isinstance(cellVal, str):
                    sCellVal = cellVal
                    if col == 1:
                        if len(sCellVal) == 0:
                            continue

                        if bLoading:
                            lLoadedObj.append(loadObj)
                            metaInfoObj.appendEnumType(loadObj.getName(), sSheetName)

                            loadObj = EnumObj()
                            bLoading = False
                        loadObj.setName(sCellVal)

                        sComment = s.cell(row, 4).value
                        sCommentValue = "{}".format(sComment)
                        loadObj.setComment(sCommentValue)

                        bLoading = True
                        break
                    else:
                        lField.append(sCellVal)
                elif isinstance(cellVal, float):
                    sCellVal = cellVal
                    iValue = int(sCellVal)
                    if col == 3:
                        lField.append(iValue)
                    else:
                        sValue = "{}".format(iValue)
                        lField.append(sValue)
                else:
                    logger.info("row:%s:col:%s|other|" % (row, col), cellVal, type(cellVal))
                    raise Exception("InvalidCellType|row:%s:col:%s|other|" % (row, col), cellVal, type(cellVal))
            if len(lField) != 0:
                loadObj.appendField(lField)

        if bLoading:
            lLoadedObj.append(loadObj)
            metaInfoObj.appendEnumType(loadObj.getName(), sSheetName)
    except:
        sErrorInfo = traceback.format_exc()
        logger.error("CatchException->ErrorPos:File:%s,Sheet:%s|row:%s:col:%s[%c]" % (
            sName, s.name, row + 1, col + 1, chr(65 + col)))
        logger.error("ErrorInfo:%s" % (sErrorInfo,))

        input("导出出错,输入任意字符退出!")
        raise

    dEnumData = {}
    dEnumData["file"] = "{}:{}".format(sName, sSheetName)
    dEnumData["list"] = []
    for elems in lLoadedObj:
        dValue = elems.getData()
        dEnumData["list"].append(dValue)

    sTemplateName = "Template/{}Template.txt".format("Enum")
    File = open(sTemplateName, 'r')
    content = File.read()
    File.close()

    template = Template(content)
    rendered_form = template.render(dEnumData=dEnumData)

    sFileDirs = "DumpConfigs/PB"
    os.makedirs(sFileDirs, exist_ok = True)
    sFileName = "{}/{}.proto".format(sFileDirs, sSheetName)
    output = open(sFileName, 'w')
    output.write(rendered_form)
    output.close()


def dumpMetaExcel(sName):
    wb = open_workbook(sName)

    for s in wb.sheets():
        # print "*" * 10
        # print 'Sheet:',s.name
        logger.info("*" * 10)
        logger.info('File:%s,Sheet:%s' % (sName, s.name,))

        cellVal = s.cell(1, 0).value
        if cellVal == "ENUM":
            Enum_Generator(sName, s)
        else:
            raise Exception("Invalid Meta Type")


def dumpExcel(sNmae):
    wb = open_workbook(sNmae)

    for s in wb.sheets():
        # print "*" * 10
        # print 'Sheet:',s.name
        logger.info("*" * 10)
        logger.info('File:%s,Sheet:%s' % (sNmae, s.name,))

        # 列字段类型
        field_type = {}
        field_name = {}
        field_key = []

        lDocument = []

        try:
            for row in range(s.nrows):
                dElem = {}
                for col in range(s.ncols):
                    # 第一列为备注，忽略
                    if 0 == col:
                        continue

                    # 字段类型
                    if row > 4:
                        iFieldLen = len(field_type)
                        if col > iFieldLen:
                            break

                    cellVal = s.cell(row, col).value
                    strType = type(cellVal)
                    # print("type:%s", strType)

                    if isinstance(cellVal, str):
                        sCellVal = cellVal
                        # 0:注释行1, 1:表头, 2:注释行2, 3:字段描述
                        if row in (0, 1, 2, 3):
                            continue
                        # 4:字段类型, 有效的字段类型(key, int, float, string ,str), 第一个字段类型必须是key
                        elif 4 == row:
                            if sCellVal == "":
                                break
                            field_type[col] = sCellVal

                            if sCellVal in ("key",):
                                field_key.append(col)

                            if 1 == col:
                                if sCellVal not in ("key",):
                                    print("row:%s:col:%s|must be key:%s" % (row, col, sType))
                                    logger.error("ErrorInfo:row:%s:col:%s|must be key:%s" % (row, col, sType))
                                    raise Exception("InvalidType|row:%s:col:%s|must be key:%s" % (row, col, sType))

                        # 5:server字段名
                        elif 5 == row:
                            field_name[col] = sCellVal

                            if sCellVal in ("List",):
                                print("row:%s:col:%s|invalid type name:%s" % (row, col, sCellVal))
                                logger.error("ErrorInfo:row:%s:col:%s|invalid type name:%s" % (row, col, sCellVal))
                                raise Exception("InvalidType|row:%s:col:%s|invalid type name:%s" % (row, col, sCellVal))


                        # 6:client字段名
                        elif 6 == row:
                            continue
                        else:
                            sType = field_type[col]
                            sFieldName = field_name[col]
                            if sType in ("int", "key"):
                                if sCellVal == "":
                                    sCellVal = "0"
                                iValue = int(sCellVal)
                                dElem[sFieldName] = iValue
                            elif sType == "float":
                                if sCellVal == "":
                                    sCellVal = "0"
                                fValue = float(sCellVal)
                                dElem[sFieldName] = fValue
                            elif sType in ("string", "str"):
                                sValue = sCellVal
                                dElem[sFieldName] = sValue
                            elif sType == "dict":
                                if sCellVal == "":
                                    dValue = {}
                                    dElem[sFieldName] = dValue
                                else:
                                    dValue = eval(sCellVal)
                                    dElem[sFieldName] = dValue
                            elif sType == "list":
                                if sCellVal == "":
                                    lValue = []
                                    dElem[sFieldName] = lValue
                                else:
                                    lValue = eval(sCellVal)
                                    dElem[sFieldName] = lValue
                            else:
                                tModule = excelFieldTypeObj.getModule(sType)
                                if tModule is None:
                                    print("row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                                    logger.error("ErrorInfo:row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                                    raise Exception("InvalidType|row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                                else:
                                    if sCellVal == "":
                                        dElem[sFieldName] = 0
                                    else:
                                        iValue = getattr(tModule[1], tModule[0]).Value(sCellVal)
                                        dElem[sFieldName] = iValue

                    elif isinstance(cellVal, float):
                        sCellVal = cellVal
                        # 0:注释行1, 1:表头, 2:注释行2, 3:字段描述
                        if row in (0, 1, 2, 3):
                            continue
                        # 4:字段类型, 有效的字段类型(key, int, float, string ,str)
                        elif 4 == row:
                            field_type[col] = sCellVal
                        # 5:server字段名
                        elif 5 == row:
                            field_name[col] = sCellVal
                        # 6:client字段名
                        elif 6 == row:
                            continue
                        else:
                            sType = field_type[col]
                            sFieldName = field_name[col]
                            if sType in ("int", "key"):
                                if sCellVal == "":
                                    sCellVal = "0"
                                iValue = int(sCellVal)
                                dElem[sFieldName] = iValue
                            elif sType == "float":
                                if sCellVal == "":
                                    sCellVal = "0"
                                fValue = float(sCellVal)
                                dElem[sFieldName] = fValue
                            elif sType in ("string", "str"):
                                sValue = str(int(sCellVal))
                                dElem[sFieldName] = sValue
                            elif sType == "dict":
                                if sCellVal == "":
                                    dValue = {}
                                    dElem[sFieldName] = dValue
                                else:
                                    dValue = eval(sCellVal)
                                    dElem[sFieldName] = dValue
                            elif sType == "list":
                                if sCellVal == "":
                                    lValue = []
                                    dElem[sFieldName] = lValue
                                else:
                                    lValue = eval(sCellVal)
                                    dElem[sFieldName] = lValue
                            else:
                                print("row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                                logger.error("ErrorInfo:row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                                raise Exception("InvalidType|row:%s:col:%s|unicode error type:%s" % (row, col, sType))
                    else:
                        print("row:%s:col:%s|other|" % (row, col), cellVal, type(cellVal))
                        raise Exception("InvalidCellType|row:%s:col:%s|other|" % (row, col), cellVal, type(cellVal))

                if 0 != len(dElem):
                    lDocument.append(dElem)
        except:
            sErrorInfo = traceback.format_exc()
            logger.error("CatchException->ErrorPos:File:%s,Sheet:%s|row:%s:col:%s[%c]" % (
            sNmae, s.name, row + 1, col + 1, chr(65 + col)))
            logger.error("ErrorInfo:%s" % (sErrorInfo,))

            # print("CatchException->ErrorPos:File:%s,Sheet:%s|row:%s:col:%s[%c]" % (sNmae, s.name, row+1, col+1, chr(65+col)))

            input("导出出错,输入任意字符退出!")
            raise

        lTransferDocument = []
        dLayer = {}

        for elems in lDocument:

            lCheck = []
            lCheck.append(dLayer)

            lLayerData = []
            lLayerData.append(lTransferDocument)

            for iCheck in range(0, len(field_key)):
                iKeyCol = field_key[iCheck]
                sKeyName = field_name[iKeyCol]
                iKeyValue = elems[sKeyName]

                dCheckAccess = lCheck[len(lCheck) - 1]

                if iKeyValue in dCheckAccess:
                    lCheck.append(dCheckAccess[iKeyValue])

                    lData = lLayerData[len(lLayerData) - 1]
                    if iCheck < len(field_key) - 1:
                        dListData = lData[len(lData) - 1]
                        lLayerData.append(dListData["List"])
                    continue
                else:
                    iEnd = len(field_type) + 1
                    if iCheck + 1 < len(field_key):
                        iEnd = field_key[iCheck + 1]

                    dData = {}
                    for iCol in range(field_key[iCheck], iEnd):
                        sFieldName = field_name[iCol]
                        dData[sFieldName] = elems[sFieldName]
                    if iCheck < len(field_key) - 1:
                        dData["List"] = []

                    lData = lLayerData[len(lLayerData) - 1]
                    lData.append(dData)

                    if iCheck < len(field_key) - 1:
                        lLayerData.append(dData["List"])

                    dCheckAccess[iKeyValue] = {}
                    lCheck.append(dCheckAccess[iKeyValue])

        dConfigData = {}
        dConfigData["Config"] = {}
        dConfigData["Config"]["TableName"] = s.name
        dConfigData["Config"]["List"] = lTransferDocument

        sJSON = json.dumps(dConfigData, ensure_ascii=False).encode('utf8')
        # print(sJSON)

        # Open a file
        sMiddleDir = "./DumpConfigs/"
        if not os.path.exists(sMiddleDir):
            os.makedirs(sMiddleDir)

        sFileName = sMiddleDir + s.name + ".json"
        fo = open(sFileName, "wb")
        fo.write(sJSON)

        # Close opend file
        fo.close()

def main():
    for _, _, files in os.walk('./'):
        for f in files:
            if f.endswith('.xls') or f.endswith('.xlsx'):
                try:
                    dumpMetaExcel(f)
                except:
                    sErrorInfo = traceback.format_exc()
                    logger.error("dumpExcel error:File:%s," % (f))
                    logger.error("ErrorInfo:%s" % (sErrorInfo,))
                    input("导出出错,输入任意字符退出!")

    # Serializing json
    dMetaData = metaInfoObj.getData()
    jsonObj = json.dumps(dMetaData, indent=4)

    # Writing to sample.json
    with open("MetaType.json", "w") as outfile:
        outfile.write(jsonObj)

    input("导出成功,输入任意字符退出!")

def ProcessEnumRead(dData):
    for k, v in dData.items():
        excelFieldTypeObj.appendEnumType(k, v)

def read_main():
    sDirName = os.path.dirname(__file__)
    sys.path.append(sDirName + "/DumpConfigs/PB")

    # Opening JSON file
    with open('MetaType.json', 'r') as openfile:
        # Reading from json file
        dJsonData = json.load(openfile)

    for k, v in dJsonData.items():
        if k == MetaType.ENUM.name:
            ProcessEnumRead(v)

    print(dJsonData)

    for _, _, files in os.walk('./'):
        for f in files:
            if f.endswith('.xls') or f.endswith('.xlsx'):
                try:
                    dumpExcel(f)
                except:
                    sErrorInfo = traceback.format_exc()
                    logger.error("dumpExcel error:File:%s," % (f))
                    logger.error("ErrorInfo:%s" % (sErrorInfo,))
                    input("导出出错,输入任意字符退出!")
    input("导出成功,输入任意字符退出!")

if __name__ == "__main__":
    # main()
    read_main()
