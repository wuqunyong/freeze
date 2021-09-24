#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <variant>
#include <utility>
#include <set>

#include "apie/mysql_driver/mysql_field.h"

#include "boost/pfr.hpp"


template<typename T, std::size_t... Idx>
void to_layout_offset(std::vector<uint32_t>& vec, T& t, std::index_sequence<Idx...> /*unused*/)
{
	(vec.push_back(reinterpret_cast<char*>(&boost::pfr::get<Idx>(t)) - reinterpret_cast<char*>(&boost::pfr::get<0>(t))), ...);
}


template<typename T, std::size_t... Idx>
void to_layout_type(std::vector<std::set<MysqlField::DB_FIELD_TYPE>>& vec, T& t, std::index_sequence<Idx...> /*unused*/)
{
	(vec.push_back(get_field_type(boost::pfr::get<Idx>(t))), ...);
}


#define DAO_DEFINE_TYPE_INTRUSIVE(Type)       \
	public:                                   \
		Type fields;                          \
                                              \
	virtual void* layoutAddress() override    \
	{                                         \
		return &fields;                       \
	}                                         \
                                              \
	virtual std::vector<uint32_t> layoutOffset() override \
	{                                                     \
		std::vector<uint32_t> layout;                     \
		to_layout_offset(layout, fields, std::make_index_sequence<boost::pfr::tuple_size_v<Type>>{});  \
		return layout;                                                                                 \
	}                                                                                                  \
                                                                                                       \
	virtual std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layoutType() override                     \
	{                                                                                                  \
		std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layout;                                       \
		to_layout_type(layout, fields, std::make_index_sequence<boost::pfr::tuple_size_v<Type>>{});    \
		return layout;                                                                                 \
	}                                                                                                  \