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
