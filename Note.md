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