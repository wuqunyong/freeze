..\protoc login_msg.proto --cpp_out=./
..\protoc role_server_msg.proto --cpp_out=./
..\protoc rpc_login.proto --cpp_out=./

..\protoc login_msg.proto --python_out=../../client
..\protoc rpc_login.proto --python_out=../../client

pause