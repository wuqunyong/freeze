#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace apie {

	struct APieConfig_Identify
	{
		uint32_t realm = 0;
		uint32_t type = 0;
		uint32_t id = 0;
		std::string auth;
		std::string ip;
		uint16_t port = 0;
		std::string out_ip;
		uint16_t out_port = 0;
		uint16_t codec_type = 1;
	};

	struct APieConfig_Certificate
	{
		std::string public_key;
		std::string private_key;
	};

	struct APieConfig_ListenersSocketAddress
	{
		std::string address;
		uint16_t port_value = 0;
		uint16_t type = 1;
		uint16_t mask_flag = 0;
	};

	struct APieConfig_ListenersElems
	{
		APieConfig_ListenersSocketAddress socket_address;
	};

	struct APieConfig_ClientsSocketAddress
	{
		std::string address;
		uint16_t port_value = 0;
		uint16_t type = 1;
		uint16_t mask_flag = 0;
	};

	struct APieConfig_ClientsElems
	{
		APieConfig_ClientsSocketAddress socket_address;
	};

	struct APieConfig_ServiceRegistry
	{
		std::string address;
		uint16_t port_value = 0;
		std::string auth;
		uint16_t type = 0;
	};


	struct APieConfig_Bind_Database
	{
		uint32_t type = 0;
		std::vector<std::string> table_name;
		uint32_t server_id = 0;
	};

	struct APieConfig_BindTables
	{
		std::vector<APieConfig_Bind_Database> database;
	};

	struct APieConfig_Log
	{
		bool merge = true;
		uint16_t level = 0;
		bool show_pos = true;
		uint16_t split_size = 128;
		std::string backup;
		std::string name;
		bool show_console = false;
	};

	struct APieConfig_Metrics
	{
		bool enable = false;
		std::string ip;
		uint16_t udp_port = 8089;
	};

	struct APieConfig_Mysql
	{
		bool enable = false;
		std::string host;
		uint16_t port = 3306;
		std::string user;
		std::string passwd;
		std::string db;
	};

	struct APieConfig_RedisClient
	{
		uint32_t type = 0;
		uint32_t id = 0;
		std::string host;
		uint16_t port = 6379;
		std::string passwd;
	};

	struct APieConfig_NatsSubscription
	{
		uint32_t type = 0;
		std::string nats_server;
		std::string channel_domains;
	};

	struct APieConfig_Nats
	{
		bool enable = false;
		std::vector<APieConfig_NatsSubscription> connections;
	};

	struct APieConfig_Etcd
	{
		bool enable = false;
		std::string urls;
		std::string prefix;
	};

	struct APieConfig_Limited
	{
		uint32_t requests_per_unit = 0;
		uint32_t uint = 60;
	};

	struct APieConfig_TaskCase
	{
		uint32_t case_type = 0;
		uint32_t loop_count = 1;
		uint32_t loop_interval_ms = 0;
	};

	struct APieConfig_TaskSuite
	{
		APieConfig_TaskCase task_case;
	};

	struct APieConfig_AutoTest
	{
		bool enable = false;
		uint32_t start = 0;
		uint32_t stop = 0;
		uint32_t ramp_up_interval = 1000;
		uint32_t ramp_up_nums = 100;
		uint32_t loop_count = 1;

		std::vector<APieConfig_TaskSuite> task_suite;
	};

	struct PBMagElem
	{
		int16_t type;
		int16_t cmd;
		std::string pb_name;
	};

	struct APieConfig
	{
		APieConfig_Identify identify;
		uint16_t io_threads = 2;
		bool daemon = true;
		uint32_t service_timeout = 180;
		uint32_t service_learning_duration = 60;
		APieConfig_Certificate certificate;
		std::vector<APieConfig_ListenersElems> listeners;
		APieConfig_ClientsElems clients;
		APieConfig_ServiceRegistry service_registry;
		APieConfig_BindTables bind_tables;
		APieConfig_Log log;
		APieConfig_Metrics metrics;
		APieConfig_Mysql mysql;
		std::vector<APieConfig_RedisClient> redis_clients;
		APieConfig_Nats nats;
		APieConfig_Etcd etcd;
		APieConfig_Limited limited;
		APieConfig_ClientsSocketAddress login_server;
		std::vector<PBMagElem> pb_map_vec;
		APieConfig_AutoTest auto_test;
	};


	// NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT
	class Mysql_BindElem
	{
	public:
		std::string ip;
		uint32_t port;
		uint32_t type;
		uint32_t mask_flag;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_BindElem, ip, port, type, mask_flag);
	};

	class Mysql_ListenersConfig
	{
	public:
		std::vector<Mysql_BindElem> bind;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_ListenersConfig, bind);


		bool isValid(std::string& errInfo)
		{
			return true;
		}
	};

	class Mysql_MysqlConfig
	{
	public:
		std::string host;
		uint32_t port;
		std::string user;
		std::string passwd;
		std::string db;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_MysqlConfig, host, port, user, passwd, db);


		bool isValid(std::string& errInfo)
		{
			return true;
		}
	};

	class Mysql_NatsConnectionElem
	{
	public:
		uint32_t type;
		std::string nats_server;
		std::string channel_domains;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_NatsConnectionElem, type, nats_server, channel_domains);
	};

	class Mysql_NatsConfig
	{
	public:
		std::vector<Mysql_NatsConnectionElem> connections;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_NatsConfig, connections);


		bool isValid(std::string& errInfo)
		{
			return true;
		}
	};

	class Mysql_RedisClient
	{
	public:
		uint32_t type;
		uint32_t id;
		std::string host;
		uint32_t port;
		std::string passwd;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_RedisClient, type, id, host, port, passwd);
	};

	class Mysql_RedisConfig
	{
	public:
		std::vector<Mysql_RedisClient> clients;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mysql_RedisConfig, clients);


		bool isValid(std::string& errInfo)
		{
			return true;
		}
	};

} 
