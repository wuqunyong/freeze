rem ..\protoc login_msg.proto --cpp_out=./
rem ..\protoc role_server_msg.proto --cpp_out=./
rem ..\protoc rpc_login.proto --cpp_out=./

rem ..\protoc login_msg.proto --python_out=../../client/proto
rem ..\protoc rpc_login.proto --python_out=../../client/proto

rem ..\protoc login.proto --cpp_out=./
rem ..\protoc userinfo.proto --cpp_out=./
rem ..\protoc map.proto --cpp_out=./
rem ..\protoc db.proto --cpp_out=./
rem ..\protoc talent.proto --cpp_out=./
rem ..\protoc decree.proto --cpp_out=./




rem ..\protoc addressbook.proto --python_out=./

rem ..\protoc meta_enum.proto --python_out=./


protoc -I=. -I=./core error_code.proto --cpp_out=./core
protoc -I=. -I=./core protocol.proto --cpp_out=./core

protoc -I=. -I=./rpc rpc_login.proto --cpp_out=./rpc

pause