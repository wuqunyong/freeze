// ---------------------------------------------------------------------
// THIS FILE IS AUTO-GENERATED BY SCRIPT, SO PLEASE DON'T MODIFY IT BY YOURSELF!
// Source: apie.role_extra
// ---------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "apie.h"

namespace apie {
namespace dbt_role {

class role_extra_AutoGen
    : public DeclarativeBase,
      public std::enable_shared_from_this<role_extra_AutoGen> {

  using InsertCB =
      std::function<void(apie::status::Status, bool, uint64_t, uint64_t)>;
  using UpdateCB = std::function<void(apie::status::Status, bool, uint64_t)>;
  using DeleteCB = std::function<void(apie::status::Status, bool, uint64_t)>;

private:
  struct db_fields {
    uint64_t user_id;
    std::string extra_info = "";
  };

  virtual std::string getFieldName(uint32_t iIndex) override {

    static std::map<uint32_t, std::string> kFieldNameMap = {
        {role_extra_AutoGen::user_id, "user_id"},
        {role_extra_AutoGen::extra_info, "extra_info"}};

    return kFieldNameMap[iIndex];
  }

public:
  enum Fields {
    user_id = 0,
    extra_info = 1,
  };

  static std::shared_ptr<role_extra_AutoGen> Create(uint64_t user_id) {
    return std::shared_ptr<role_extra_AutoGen>(new role_extra_AutoGen(user_id));
  }

  role_extra_AutoGen(uint64_t user_id) {
    this->fields.user_id = user_id;

    this->bindTable(DeclarativeBase::DBType::DBT_Role, getFactoryName());
  }

  virtual ~role_extra_AutoGen() {}

  void set_user_id(uint64_t user_id) {
    this->fields.user_id = user_id;
    this->markDirty({role_extra_AutoGen::user_id});
  }

  uint64_t get_user_id() const { return this->fields.user_id; }

  void set_extra_info(std::string extra_info) {
    this->fields.extra_info = extra_info;
    this->markDirty({role_extra_AutoGen::extra_info});
  }

  std::string get_extra_info() const { return this->fields.extra_info; }

public:
  void SetDbProxyServer(::rpc_msg::CHANNEL server) { m_optServer = server; }

  void Insert(InsertCB cb = nullptr) {
    if (!m_optServer.has_value()) {
      ASYNC_PIE_LOG(
          PIE_ERROR,
          "DBOpreateError | Insert | isBind:{} | dbName:{} | tableName:{}",
          isBind(), getgDbName(), getTableName());
      return;
    }

    auto channel = m_optServer.value();
    InsertToDb(channel, *this, cb);
  }

  void Update(UpdateCB cb = nullptr) {
    if (!m_optServer.has_value()) {
      ASYNC_PIE_LOG(
          PIE_ERROR,
          "DBOpreateError | Update | isBind:{} | dbName:{} | tableName:{}",
          isBind(), getgDbName(), getTableName());
      return;
    }

    auto channel = m_optServer.value();
    UpdateToDb(channel, *this, cb);
  }

  void Delete(DeleteCB cb = nullptr) {
    if (!m_optServer.has_value()) {
      ASYNC_PIE_LOG(
          PIE_ERROR,
          "DBOpreateError | Delete | isBind:{} | dbName:{} | tableName:{}",
          isBind(), getgDbName(), getTableName());
      return;
    }

    auto channel = m_optServer.value();
    DeleteFromDb(channel, *this, cb);
  }

  DAO_DEFINE_TYPE_INTRUSIVE_MACRO(role_extra_AutoGen, db_fields, role_extra);
};

} // namespace dbt_role

APIE_REGISTER_TABLE(DeclarativeBase::DBType::DBT_Role,
                    apie::dbt_role::role_extra_AutoGen, role_extra)

} // namespace apie
