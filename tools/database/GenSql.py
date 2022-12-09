import os
import sys
import logging
import time
import pymysql
import yaml
from pathlib import Path
import traceback
import hashlib
import subprocess

sDate = time.strftime("%Y%m%d_%H%M%S", time.localtime())
sLogName = "GenSql_" + sDate + ".log"
logging.basicConfig(level=logging.INFO, filename=sLogName, filemode="w", format="%(asctime)s | %(levelname)s | %(message)s")

sSQLPattern = "SELECT DATA_TYPE, COLUMN_NAME, COLUMN_DEFAULT, COLUMN_KEY, ORDINAL_POSITION, COLUMN_TYPE, TABLE_SCHEMA, TABLE_NAME " \
              "FROM INFORMATION_SCHEMA.columns WHERE table_schema = '{}' and table_name = '{}'"
sSchemaSQL = "SELECT table_name, table_type FROM information_schema.tables WHERE table_schema = '{}'"

class TableMeta:
   def __init__(self, sSchema, sTable, lField):
        self.sSchema = sSchema
        self.sTable = sTable
        self.lField = lField


def readConfig(path):
    if not os.path.isabs(path):
        path = os.path.abspath(path)
    with open(path, 'r', encoding="utf-8") as file:
        config = yaml.safe_load(file)
    return config

def IsUnsigned(sName) :
    sLittleName = sName.lower()
    iPos = sLittleName.find("unsigned")
    if iPos == -1:
        return False
    return True

def GenStructDefine(sName, lField) :
    sDefine = ""
    sDefine += "struct {} {{\n".format(sName)
    for elems in lField:
        if elems[2] is None:
            sField = "\t" + elems[0] + " " + elems[1] + ";\n"
        else:
            if elems[0].lower() == "std::string":
                if elems[2].upper() == "CURRENT_TIMESTAMP":
                    sField = "\t" + elems[0] + " " + elems[1] + " = " + elems[2] + "();\n"
                else:
                    sField = "\t" + elems[0] + " " + elems[1] + " = \"" + elems[2] + "\";\n"
            else:
                sField = "\t" + elems[0] + " " + elems[1] + " = " + elems[2] + ";\n"
        sDefine += sField
    sDefine += "};\n"

    return sDefine

def GenEnumDefine(lField) :
    sDefine = ""
    sDefine += "enum Fields {\n"
    for elems in lField:
        sField = "\t" + elems[1] + " = " + str(elems[4]-1) + ",\n"
        sDefine += sField
    sDefine += "};\n"

    return sDefine;

def GenConstructor(sName, lField, sBindType):
    sConstructor = ""
    sParameters = ""
    sStatements = ""

    for elems in lField:
        if elems[3].upper() != "PRI":
            continue
        sValue1 = "\t" + elems[0] + " " + elems[1] + ","
        sParameters += sValue1

        sValue2 = "\tthis->fields." + elems[1] + " = " + elems[1] + ";\n"
        sStatements += sValue2

    iLen = len(sParameters)
    sValue = sParameters[0:iLen-1]

    sTemplate = """{}_AutoGen({})
    		{{
    			{}
    			this->bindTable({}, getFactoryName());
    		}}
"""

    sConstructor += sTemplate.format(sName, sValue, sStatements, sBindType)
    return sConstructor


def GenCreate(sName, lField):
    sCreate = ""
    sParameters = ""
    sStatements = ""

    for elems in lField:
        if elems[3].upper() != "PRI":
            continue
        sValue1 = "\t" + elems[0] + " " + elems[1] + ","
        sParameters += sValue1

        sValue2 = "\t" + elems[1] + ","
        sStatements += sValue2

    iLen = len(sParameters)
    sValue = sParameters[0:iLen-1]

    iLen1 = len(sStatements)
    sArgs = sStatements[0:iLen1-1]

    sTemplate = """static std::shared_ptr <{}_AutoGen> Create({})
    {{
    return std::shared_ptr <{}_AutoGen> (new {}_AutoGen({}));
    }}
"""

    sCreate += sTemplate.format(sName, sValue, sName, sName, sArgs)
    return sCreate

def GenGetFieldName(sTable, lField):
    sCreate = ""
    sParameters = ""
    sStatements = """
    static std::map<uint32_t, std::string> fields = 
    {{
        {}
    }};
    """

    for elems in lField:
        sClassName = "{}_AutoGen".format(sTable)
        sParameters += "{{ {}::{}, \"{}\" }},".format(sClassName, elems[1], elems[1])

    sParameters = sParameters[0:len(sParameters) - 1]

    sStatements = sStatements.format(sParameters)


    sTemplate = """virtual std::string getFieldName(uint32_t iIndex) override
    {{
        {}
        return fields[iIndex];
    }}
"""

    sCreate += sTemplate.format(sStatements)
    return sCreate

def GenProperty(sTable, lField):
    sSetTemplate = """{} set_{}({})
        		{{
        			{}
        		}}
"""
    sGetTemplate = """{} get_{}({})
            		{{
            			{}
            		}}
"""

    sProperty = ""
    for elems in lField:
        sReturn = elems[0]
        sName = elems[1]
        sParameters = elems[0] + " " + elems[1]
        sBody = ""
        sBody += "this->fields.{} = {};\n".format(elems[1], elems[1])
        sClassName = "{}_AutoGen".format(sTable)
        sBody += "this->markDirty({{ {}::{} }});\n".format(sClassName, elems[1])

        sSetFunc = sSetTemplate.format("void", sName, sParameters, sBody)
        sProperty += sSetFunc + "\n"

        sBody = ""
        sBody += "return this->fields.{};\n".format(elems[1])
        sGetFunc = sGetTemplate.format(sReturn, sName, "", sBody)
        sProperty += sGetFunc + "\n"

    return sProperty


def GenFile(sSChema, sName, sStruct, sEnum, sConstructor, sCreate, sProperty, sBindType, sBindSpace, sGetField) :
    sTemplate = """// ---------------------------------------------------------------------
// THIS FILE IS AUTO-GENERATED BY SCRIPT, SO PLEASE DON'T MODIFY IT BY YOURSELF!
// Source: {}.{}
// ---------------------------------------------------------------------

#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"


namespace apie {{
namespace {} {{

	class {}_AutoGen : public DeclarativeBase,
						public std::enable_shared_from_this<{}_AutoGen> {{
	private:
		{}
		{}
		{}
		
	public:
		{}
		{}
		{}
		
		DAO_DEFINE_TYPE_INTRUSIVE_MACRO({}_AutoGen, db_fields, {});
	}};
	
}}

APIE_REGISTER_TABLE({}, apie::{}::{}_AutoGen, {})

}}
"""
    sDefine = sTemplate.format(sSChema, sName,
                               sBindSpace,
                               sName, sName,
                               sStruct, sEnum, sGetField,
                               sCreate, sConstructor, sProperty,
                               sName, sName,
                               sBindType, sBindSpace, sName, sName)
    return sDefine

def TypeNameConvert(sName, sSign):
    bUnsigned = IsUnsigned(sSign)
    if bUnsigned:
        dMap = {
            "tinyint": "uint8_t",
            "smallint": "uint16_t",
            "int": "uint32_t",
            "bigint": "uint64_t",
            "char": "std::string",
            "varchar": "std::string",
            "tinyblob": "std::string",
            "blob": "std::string",
            "mediumblob": "std::string",
            "longblob": "std::string",
            "tinytext": "std::string",
            "text": "std::string",
            "mediumtext": "std::string",
            "longtext": "std::string",
            "date": "std::string",
            "datetime": "std::string",
            "varbinary": "std::string",
            "float": "float",
            "double": "double"
        }

        if sName in dMap:
            return dMap[sName]
    else:
        dMap = {
            "tinyint": "int8_t",
            "smallint": "int16_t",
            "int": "int32_t",
            "bigint": "int64_t",
            "char": "std::string",
            "varchar": "std::string",
            "tinyblob": "std::string",
            "blob": "std::string",
            "mediumblob": "std::string",
            "longblob": "std::string",
            "tinytext": "std::string",
            "text": "std::string",
            "mediumtext": "std::string",
            "longtext": "std::string",
            "date": "std::string",
            "datetime": "std::string",
            "varbinary": "std::string",
            "float": "float",
            "double": "double"
        }

        if sName in dMap:
            return dMap[sName]
    return sName + "_undefined"

def ConvertMetadata(lData):
    lResult = []
    for elems in lData:
        lElems = list(elems)
        lElems[0] = TypeNameConvert(lElems[0], lElems[5])
        lResult.append(lElems)
    return lResult

def ConnectMysql(dConfig):
    connection = pymysql.connect(host=dConfig["host"], port=dConfig["port"], user=dConfig["user"],
                                     password=dConfig["passwd"], database=dConfig["database"], charset=dConfig["charset"])
    return connection

def ExecuteSql(connection, sSQL):
    connection.ping()

    tResult = None

    # with connection:
    with connection.cursor() as cursor:
        logging.info("execute | sql:{}".format(sSQL))
        cursor.execute(sSQL)
        tResult = cursor.fetchall()
        logging.info("result:{}".format(tResult))

    # connection is not autocommit by default. So you must commit to save
    # your changes.
    connection.commit()

    return tResult

def main(argc, argv ):
    if argc < 2:
        logging.error("invalid argc:{}".format(argc))
        raise Exception("invalid argc:{}".format(argc))

    fileList = []
    sConfigPath = argv[1]
    if not os.path.isabs(sConfigPath):
        sConfigPath = os.path.abspath(sConfigPath)

    sDir = os.path.dirname(sConfigPath)

    dConfig = readConfig(sConfigPath)

    for items in dConfig["database"]:
        sSChema = items["database"]
        sBindType = items["bind_type"]
        bindList = sBindType.split("::")
        if len(bindList) < 1:
            raise Exception("invalid bindType:{}".format(sBindType))
        sTemp = bindList[-1]
        sBindSpace = sTemp.lower()
        connection = ConnectMysql(items)

        sSChemaSQL = sSchemaSQL.format(sSChema)
        tSChemaResult = ExecuteSql(connection, sSChemaSQL)

        for tItem in tSChemaResult:
            sTable = tItem[0]

            sSQL = sSQLPattern.format(sSChema, sTable)
            tResult = ExecuteSql(connection, sSQL)
            lResult = ConvertMetadata(tResult)

            logging.info("lResult:{}".format(lResult))

            sStruct = GenStructDefine("db_fields", lResult)
            sEnum = GenEnumDefine(lResult)
            sConstructor = GenConstructor(sTable, lResult, sBindType)
            sCreate = GenCreate(sTable, lResult)
            sGetField = GenGetFieldName(sTable, lResult)
            sProperty = GenProperty(sTable, lResult)

            sFile = GenFile(sSChema, sTable, sStruct, sEnum, sConstructor, sCreate, sProperty, sBindType, sBindSpace, sGetField)

            logging.info("lResult:{}".format(lResult))

            sCreateDir = sDir + "/dao" + "/" + sBindSpace
            os.makedirs(sCreateDir, exist_ok=True)
            sMergeName = sCreateDir + "/" + sTable + "_AutoGen" + ".h"

            with open(sMergeName, "w") as fObj:
                sData = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
                fObj.write(sFile)

if __name__ == "__main__":
    try:
        print("生成DAO开始\n")
        print("args:{}\n".format(sys.argv))

        logging.info("生成DAO开始")
        logging.info("args:{}".format(sys.argv))
        main(len(sys.argv), sys.argv)
    except:
        tException = sys.exc_info()
        logging.error("Exception | {}".format(tException))

        sTrace = traceback.format_exc()
        logging.error("traceback | {}".format(sTrace))

        logging.error("生成DAO失败, Failure")
        print("生成DAO失败, Failure, logFile:{}\n".format(sLogName))
        os._exit(1)
    else:
        logging.info("生成DAO完成, Success")
        print("生成DAO完成, Success, logFile:{}\n".format(sLogName))
        os._exit(0)

