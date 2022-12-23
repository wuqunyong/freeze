rem ..\protoc login_msg.proto --cpp_out=./
rem ..\protoc role_server_msg.proto --cpp_out=./
rem ..\protoc rpc_login.proto --cpp_out=./

rem ..\protoc login_msg.proto --python_out=../../client/proto
rem ..\protoc rpc_login.proto --python_out=../../client/proto

..\protoc login.proto --cpp_out=./
..\protoc userinfo.proto --cpp_out=./
..\protoc map.proto --cpp_out=./
..\protoc db.proto --cpp_out=./
..\protoc talent.proto --cpp_out=./
..\protoc decree.proto --cpp_out=./


..\protoc error_code.proto --cpp_out=./

pause