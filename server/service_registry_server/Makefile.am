AM_CXXFLAGS = -Wall -std=c++17

INCLUDES = -I /usr/local/include \
			-I /usr/include/mysql \
			-I /usr/local/protobuf/include \
			-I $(top_srcdir)/third_party/astar \
			-I $(top_srcdir)/third_party/cpp_redis/includes \
			-I $(top_srcdir)/third_party/cpp_redis/tacopie/includes \
			-I $(top_srcdir)/third_party/http-parser \
			-I $(top_srcdir)/third_party/jsoncpp/include \
			-I $(top_srcdir)/third_party/tinyxml2 \
			-I $(top_srcdir)/third_party/influxdb-cpp \
			-I $(top_srcdir)/pb_msg \
			-I $(top_srcdir)/libapie \
			-I $(top_srcdir)/common
			
apie_module_ldflags = -L/usr/lib64/mysql -L/usr/local/lib64 -L/usr/local/lib -L/usr/local/protobuf/lib -L$(top_srcdir)/libapie -Wl,-rpath,/usr/local/apie/lib -levent -levent_pthreads -lpthread -ldl -lc -lrt -lcrypto -lssl -llz4 -lcpp_redis -ltacopie -lnats

service_registry_sources = \
	$(top_srcdir)/server/service_registry_server/main.cpp \
	$(top_srcdir)/server/service_registry_server/service_init.cpp \
	$(top_srcdir)/server/service_registry_server/logic/service_registry.cpp 

bin_PROGRAMS = service_registry

service_registry_SOURCES = $(service_registry_sources)
service_registry_LDADD = -lapie -lmysqlclient -lprotobuf -lyaml-cpp
service_registry_LDFLAGS = $(apie_module_ldflags)