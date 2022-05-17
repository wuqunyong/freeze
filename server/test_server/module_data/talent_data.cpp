#include "talent_data.h"

#include "../logic/mock_role.h"

namespace apie {

	void TalentData::handleLoginNotice(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		pb::talent::Talent_Login_Notice response;
		bool bResult = response.ParseFromString(msg);
		if (!bResult)
		{
			return;
		}

		ptrRole->getRoleModuleData().m_talentData.panle = response.data();
	}

}
