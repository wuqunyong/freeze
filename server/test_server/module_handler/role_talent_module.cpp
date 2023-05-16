#include "role_talent_module.h"

#include "../../common/opcodes.h"
#include "../../pb_msg/init.h"

namespace apie {


void RoleTalentModule::registerModule()
{
	MockRole::addHandler(s_sName, "open", std::bind(&RoleTalentModule::onOpen, std::placeholders::_1, std::placeholders::_2));
	MockRole::addHandler(s_sName, "choose", std::bind(&RoleTalentModule::onChoose, std::placeholders::_1, std::placeholders::_2));
	MockRole::addHandler(s_sName, "activate", std::bind(&RoleTalentModule::onActivate, std::placeholders::_1, std::placeholders::_2));
}

void RoleTalentModule::onOpen(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::pb::talent::Talent_Open_Req request;
	mockRole.sendMsg(MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Open_Req), request);
}

void RoleTalentModule::onChoose(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::pb::talent::Talent_Choose_Req request;
	for (const auto& elems : msg.params())
	{
		uint32_t id = std::stoi(elems);
		request.add_id(id);
	}
	mockRole.sendMsg(MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Choose_Req), request);
}

void RoleTalentModule::onActivate(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::pb::talent::Talent_Activate_Req request;

	uint64_t id = std::stoull(msg.params(0));
	request.set_id(id);

	mockRole.sendMsg(MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Activate_Req), request);
}


}

