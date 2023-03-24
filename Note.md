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


pyinstaller --onefile simple.py

Hexagonal Grids
hex_linedraw


protobuff版本
protobuf-cpp-3.21.6.tar.gz



C++ Insights - See your source code with the eyes of a compiler.
https://cppinsights.io/


https://www.modernescpp.com/index.php/a-generic-data-stream-with-coroutines-in-c-20


C:\Users\Administrator\AppData\Local\Temp\1489AFE4.TMP

Resets the trial period for Visual Assist X.

This script does the following:

Remove a temporary file "1489AFE4.TMP" in the user's temp directory used by VA
Remove a registry key "HKEY_CURRENT_USER\SOFTWARE\Licenses" used by VA


 SELECT COLUMN_NAME, DATA_TYPE, IS_NULLABLE, COLUMN_DEFAULT, COLUMN_KEY, COLUMN_TYPE, TABLE_SCHEMA FROM INFORMATION_SCHEMA.columns WHERE table_name = 'name'


clang format
 https://llvm.org/builds/


 nats
 https://github.com/pixie-io/pixie/blob/a2cc1718283e8bdb549de0e2bc19b2d0ec3d65e8/src/common/event/nats.h



Jinja2 Template
 https://j2live.ttl255.com/





 vultr


 jinja2

 s = "{{elements|length}}"

template = Template(s)
len = template.render(elements=["a", "b", "c"])

print(len)
OUTPUT
3



enum TaskState {
    TASK_OPEN = 0;
    TASK_IN_PROGRESS = 1;
    TASK_POST_PONED = 2;
    TASK_CLOSED = 3;
    TASK_DONE = 4;
}

import todolist_pb2 as TodoList

first_item.state = TodoList.TaskState.Value("TASK_DONE")

getattr(meta_enum,"SectionType").Value("SECTION_INDEX")

test = importlib.import_module("meta_enum_pb2")
getattr(test,"SectionType").Value("SECTION_INDEX")



https://crontab.guru/
Note: The day of a command's execution can be specified by two fields - day of month, and day of week. If both fields are restricted (ie, aren't *), the command will be run when either field matches the current time. For example,
"30 4 1,15 * 5" would cause a command to be run at 4:30 am on the 1st and 15th of each month, plus every Friday.



HEX oHex = OffsetCoord::RoffsetToCube(OffsetCoord::ODD, OffsetCoord(x, z));


弗洛伊德算法

https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/




https://github.com/MakersF/corobatch


https://zhuanlan.zhihu.com/p/82244559


动态寻路，带阻挡参数进行寻路


unsigned int num_fields;
unsigned int i;
MYSQL_FIELD *fields;

num_fields = mysql_num_fields(result);
fields = mysql_fetch_fields(result);
for(i = 0; i < num_fields; i++)
{
   printf("Field %u is %s\n", i, fields[i].name);
}