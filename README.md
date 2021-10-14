[# Welcome to APie!](https://github.com/wuqunyong/APie)

# CentOS 7 x64安装
## 依赖

 - libevent
 - protobuf
 - yaml
 - lz4
 - [cpp_redis](https://github.com/cpp-redis/cpp_redis)
 - [nats.c](https://github.com/nats-io/nats.c)

## 安装依赖
```shell
yum install -y mysql-devel mysql-server lrzsz curl-devel openssl openssl-devel readline-devel pcre pcre-devel zlib zlib-devel libevent libevent-devel gcc gcc-c++ rpm-build automake libtool lz4-devel
```
## 安装git
```
yum install -y git
git --version
```
## clone项目
```
cd /root/
git clone https://github.com/wuqunyong/APie.git
cd /root/APie/download/
cp cmake-3.18.1-Linux-x86_64.tar.gz /root/
cp protobuf-3.11.4.zip /root/
cp yaml-cpp-master.zip /root/
cd /root/
```
### 安装cmake
```shell
tar -zxvf cmake-3.18.1-Linux-x86_64.tar.gz
cd cmake-3.18.1-Linux-x86_64
```
### 升级GCC
```shell
yum install centos-release-scl -y
yum install devtoolset-8 -y
scl enable devtoolset-8 bash
gcc --version
```
### 安装protobuf
```shell
unzip protobuf-3.11.4.zip
cd protobuf-3.11.4
./autogen.sh
./configure --prefix=/usr/local/protobuf/
make
make check
make install
ldconfig
```
### 安装yaml
```shell
unzip yaml-cpp-master.zip
cd yaml-cpp-master
mkdir build
cd build
cmake ..
make
make test
make install
```

## 安装cpp_redis
```
# Clone the project
git clone https://github.com/cpp-redis/cpp_redis.git
# Go inside the project directory
cd cpp_redis
# Get tacopie submodule
git submodule init && git submodule update
# Create a build directory and move into it
mkdir build && cd build
# Generate the Makefile using CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
# Build the library
make
# Install the library
make install
```

### 安装nats
```shell
git clone https://github.com/nats-io/nats.c.git
cd nats.c/
mkdir build && cd build
cmake .. -DNATS_BUILD_STREAMING=OFF
make
make install
```

## 编译
```shell
$ mkdir build && cd build
$ cmake ..    
$ make
$ make install
```


## CentOS7 x64安装MySQL
[# How To Install MySQL on CentOS 7](https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-centos-7)
In a web browser, visit:
```
https://dev.mysql.com/downloads/repo/yum/
```
###  Step 1 — Installing MySQL
```
wget https://dev.mysql.com/get/mysql57-community-release-el7-9.noarch.rpm
md5sum mysql57-community-release-el7-9.noarch.rpm
rpm -ivh mysql57-community-release-el7-9.noarch.rpm
yum install mysql-server
```

### Step 2 — Starting MySQL
```
systemctl start mysqld
systemctl status mysqld
grep 'temporary password' /var/log/mysqld.log
```

### Step 3 — Configuring MySQL
```
mysql_secure_installation
```
### Step 4 — Testing MySQL
```
mysqladmin -u root -p version
```

# Windows安装
安装依赖库protobuf-3.11.4.zip
直接visual studio启动，编译

# 架构图
![架构图](https://github.com/wuqunyong/freeze/blob/main/conf/topology.png)

# Reactor线程模型
 [nio reactor](http://gee.cs.oswego.edu/dl/cpjslides/nio.pdf)

# Demo
```cpp
#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../common/opcodes.h"
#include "../common/dao/model_account.h"
#include "../common/dao/model_account_name.h"
#include "../pb_msg/business/login_msg.pb.h"
#include "../pb_msg/business/rpc_login.pb.h"

apie::status::Status initHook()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Login_Server });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}
	return {apie::status::StatusCode::OK, ""};
}

apie::status::Status startHook()
{
	return {apie::status::StatusCode::OK, ""};
}

apie::status::Status handleAccount(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>& response)
{
	response->set_account_id(request->account_id());
	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status readyHook()
{
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.createService<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L, ::apie::OP_MSG_RESPONSE_ACCOUNT_LOGIN_L, ::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>(::apie::OP_MSG_REQUEST_ACCOUNT_LOGIN_L, handleAccount);

	return {apie::status::StatusCode::OK, ""};
}

apie::status::Status exitHook()
{
	return {apie::status::StatusCode::OK, ""};
}


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Init, initHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Start, startHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Ready, readyHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Exit, exitHook);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

    return 0;
}

```

