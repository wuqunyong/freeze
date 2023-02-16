# -*- coding: utf-8 -*-

import json
import os
import logging
import traceback

from xlrd import open_workbook
from _symtable import CELL
from telnetlib import theNULL
from jinja2 import Template

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


def dumpExcel(sName):
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


def main():
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
    main()
