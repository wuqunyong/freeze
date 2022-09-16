init.h
module.h

module_init()
module_exit()


  SampleServiceImpl service;
  builder.RegisterService(&service);
  
  class SampleServiceImpl final : public SampleService::Service {
    Status SampleMethod(ServerContext* context, const SampleRequest* request, SampleResponse* response) override {
        response->set_response_sample_field("Hello " + request->request_sample_field());
        return Status::OK;
    }
};


nats pixie https://github.com/pixie-io/pixie


C++编程规范
https://caiorss.github.io/C-Cpp-Notes/cpp-reference-card.html#org4c85db7


windows性能监控
https://www.bilibili.com/video/BV1UA41157cC/?p=11&spm_id_from=pageDriver


https://github.com/vogeljo/reset-vassistx

Resets the trial period for Visual Assist X.

This script does the following:

Remove a temporary file "1489AFE4.TMP" in the user's temp directory used by VA
Remove a registry key "HKEY_CURRENT_USER\SOFTWARE\Licenses" used by VA
-- Tested on build 10.9.2114.0. Other builds may be immune to this script.


SetConsoleTitleW


xshell替代
https://github.com/Eugeny/tabby


xshell 的开源替代方案 WindTerm


pyinstaller --onefile -w simple.py

Hexagonal Grids
hex_linedraw


protobuff版本
protobuf-cpp-3.21.6.tar.gz