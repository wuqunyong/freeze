#!/bin/bash -e
#
#  http://free-random.cn
#  

echo
echo This script will install APie and all its dependencies.
echo It has been tested on CentOS 7 64位.
echo

set -e   # Exit immediately if a simple command exits with a non-zero status.
set -x   # activate debugging from here

BASE_DIR="$(cd "$(dirname -- "$0")" ; pwd)"
echo "CurDir: $BASE_DIR"

cmake:
  ./bootstrap
  make
  make install


yaml:

mkdir build
cd build

cmake ..
make

make test
make install


protobuf:
./autogen.sh

 ./configure --prefix=/usr/local/protobuf
 make
 make check
 sudo make install
 sudo ldconfig # refresh shared library cache.


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/protobuf/lib/
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/protobuf/lib/
export PATH=$PATH:/usr/local/protobuf/bin/

export PKG_CONFIG_PATH=/usr/local/protobuf/lib/pkgconfig/


1. 升级GCC

yum install centos-release-scl -y
yum install devtoolset-8 -y
scl enable devtoolset-8 bash
gcc --version


2. autoconf
#debug options support
#CFLAGS, CPPFLAGS
AC_ARG_ENABLE([debug],
 [AS_HELP_STRING([--enable-debug],[debug program(default is no)])],
 [CFLAGS="${CFLAGS} -g -O0"],
 [CFLAGS="-g -O2"]



3.redis

tar -zxvf redis-3.0.0.tar.gz 
cd /usr/local/redis-3.0.0
make
make install


strings  /etc/ld.so.cache


 readelf -d demo