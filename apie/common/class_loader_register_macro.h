#pragma once

#include <iostream>

#include "apie/common/dao_macros.h"
#include "apie/common/enum_to_int.h"

#define CLASS_LOADER_REGISTER_CLASS_INTERNAL(DbType, TableClass, TableName, UniqueID)         \
  namespace {                                                                 \
  struct ProxyType##UniqueID {                                                \
    ProxyType##UniqueID() {                                                   \
        std::cout << "ProxyType:" << apie::toUnderlyingType(DbType) << " " << #TableClass << " " << #TableName << " " << UniqueID << std::endl;  \
        RegisterTable<TableClass>(DbType, #TableName);                        \
    }                                                                         \
  };                                                                          \
  static ProxyType##UniqueID g_register_class_##UniqueID;                     \
  }

#define CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(DbType, TableClass, TableName,UniqueID) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL(DbType, TableClass, TableName, UniqueID)

// register class macro
#define CLASS_LOADER_REGISTER_CLASS(DbType, TableClass, TableName) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(DbType, TableClass, TableName, __COUNTER__)

#define APIE_REGISTER_TABLE(DbType, TableClass, TableName) \
  CLASS_LOADER_REGISTER_CLASS(DbType, TableClass, TableName)
