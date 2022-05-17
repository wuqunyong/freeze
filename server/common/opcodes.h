#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"


namespace apie {

	enum Opcodes : uint16_t
	{
		//ҵ������룬��1000��ʼ
		OP_MSG_REQUEST_ACCOUNT_LOGIN_L = 1000,
		OP_MSG_RESPONSE_ACCOUNT_LOGIN_L = 1001,

		OP_MSG_REQUEST_CLIENT_LOGIN = 1002,
		OP_MSG_RESPONSE_CLIENT_LOGIN = 1003,

		OP_MSG_REQUEST_ECHO = 1004,
		OP_MSG_RESPONSE_ECHO = 1005,

		OP_MSG_REQUEST_HANDSHAKE_INIT = 1006,
		OP_MSG_RESPONSE_HANDSHAKE_INIT = 1007,
		OP_MSG_REQUEST_HANDSHAKE_ESTABLISHED = 1008,
		OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED = 1009,


		_MSG_CLIENT_LOGINTOL = 501,
		_MSG_CLIENT_LOGINTOG = 502,
		_MSG_GAMESERVER_LOGINRESP = 504,
		_MSG_Login_User_Cmd = 509,
		_MSG_MAP_USER_CMD = 701,
		_MSG_USER_INFO = 720,
		_MSG_TALENT_CMD = 750,

	};



	enum ReturnCode : uint32_t
	{
		RC_OK = 0,

		//ҵ�񷵻��룬��1000��ʼ
	};

	enum E_ErrorCode
	{
		E_ErrorCode_Suc = 0,  //�ɹ�

		//����ģ��Ĵ����붨��
		//������ÿ��ģ���ͨ�ô�����
		E_ErrorCode_Common_Begin = 0,
		E_ErrorCode_Common_Unknown = E_ErrorCode_Common_Begin + 1,		//δ֪����
		E_ErrorCode_Common_Gold_Not_Enough = E_ErrorCode_Common_Begin + 2,		//��Ҳ���
		E_ErrorCode_Common_Wood_Not_Enough = E_ErrorCode_Common_Begin + 3,		//ľ�Ĳ���
		E_ErrorCode_Common_Food_Not_Enough = E_ErrorCode_Common_Begin + 4,		//ʳ�ﲻ��
		E_ErrorCode_Common_Iron_Not_Enough = E_ErrorCode_Common_Begin + 5,		//������
		E_ErrorCode_Common_Stone_Not_Enough = E_ErrorCode_Common_Begin + 6,		//ʯ�ϲ���
		E_ErrorCode_Common_Copper_Not_Enough = E_ErrorCode_Common_Begin + 7,		//ͭ�Ҳ���
		E_ErrorCode_Common_CostBook_Not_Enough = E_ErrorCode_Common_Begin + 8,		//���鲻��
		E_ErrorCode_Common_Queue_Full = E_ErrorCode_Common_Begin + 9,		//��������
		E_ErrorCode_Common_PrvCondition_Not_Enough = E_ErrorCode_Common_Begin + 10,	//ǰ������������
		E_ErrorCode_Common_Queue_Not_Exists = E_ErrorCode_Common_Begin + 11,	//���в�����
		E_ErrorCode_Common_Config = E_ErrorCode_Common_Begin + 12,	//�Ҳ�������
		E_ErrorCode_Common_ItemNotEnough = E_ErrorCode_Common_Begin + 13,	//���߲���
		E_ErrorCode_Common_CivilizationNotEnough1 = E_ErrorCode_Common_Begin + 14,	//��������(��ʱû����)
		E_ErrorCode_Common_PowerNotEnough = E_ErrorCode_Common_Begin + 15,	//��������
		E_ErrorCode_Common_Newbee = E_ErrorCode_Common_Begin + 16,	//������
		E_ErrorCode_Common_Cooldown = E_ErrorCode_Common_Begin + 17,	//CD
		E_ErrorCode_Common_Captive = E_ErrorCode_Common_Begin + 18,	//����²
		E_ErrorCode_Common_DBUser = E_ErrorCode_Common_Begin + 19,	//DBUserΪ��
		E_ErrorCode_Common_AvoidBattle = E_ErrorCode_Common_Begin + 20,	//��ս״̬
		E_ErrorCode_Common_Owner = E_ErrorCode_Common_Begin + 21,	//����
		E_ErrorCode_Common_Friend = E_ErrorCode_Common_Begin + 22,	//�ѷ�
		E_ErrorCode_Common_VigorNotEnough = E_ErrorCode_Common_Begin + 23,	//��������
		E_ErrorCode_Common_NotBuyCount = E_ErrorCode_Common_Begin + 24,	//�����������
		E_ErrorCode_Common_UserNotExists = E_ErrorCode_Common_Begin + 25,	//��Ҳ�����
		E_ErrorCode_Common_Dead = E_ErrorCode_Common_Begin + 26,	//������
		E_ErrorCode_Common_ReNameCD = E_ErrorCode_Common_Begin + 27,	//����CD��
		E_ErrorCode_Common_NameExists = E_ErrorCode_Common_Begin + 28,	//�����Ѵ���
		E_ErrorCode_Common_NameSizeErr = E_ErrorCode_Common_Begin + 29,	//���ֳ��ȷǷ�
		E_ErrorCode_Common_NameErr = E_ErrorCode_Common_Begin + 30,	//���ַǷ�
		E_ErrorCode_Common_UserMemeSizeErr = E_ErrorCode_Common_Begin + 31,	//��Ҽ�鳤�ȷǷ�
		E_ErrorCode_Common_SkinConditionNotEnough = E_ErrorCode_Common_Begin + 32,	//���ǻ�������������
		E_ErrorCode_Common_Module = E_ErrorCode_Common_Begin + 33,	//ģ���쳣
		E_ErrorCode_Common_Module_Close = E_ErrorCode_Common_Begin + 34,	//ģ��ر�
		E_ErrorCode_Common_Module_NoOpen = E_ErrorCode_Common_Begin + 35,	//����δ����
		E_ErrorCode_Common_DB_Fail = E_ErrorCode_Common_Begin + 36,	//���ݷ����쳣
		E_ErrorCode_Common_Param_Error = E_ErrorCode_Common_Begin + 37,	//��������Ƿ�
		E_ErrorCode_Common_CivilizationConditionErr = E_ErrorCode_Common_Begin + 38,	//��������������
		E_ErrorCode_Common_RecruitTimeOut = E_ErrorCode_Common_Begin + 39,	//�����Ѿ�ʧЧ
		E_ErrorCode_Common_DstCaptive = E_ErrorCode_Common_Begin + 40,	//Ŀ�걻��²
		E_ErrorCode_Common_SeasonErr = E_ErrorCode_Common_Begin + 41,	//��ҵδ����
		E_ErrorCode_Common_RecruitLimit = E_ErrorCode_Common_Begin + 42,	//������ļ��������
		E_ErrorCode_Common_ResourceNotEnough = 43,								//��Դ����

		E_ErrorCode_Common_End = 99,

		E_ErrorCode_Map_Begin = 100,
		E_ErrorCode_FieldNoContinue = E_ErrorCode_Map_Begin,		//�ؿ鲻��������ռ��
		E_ErrorCode_Troop_Not_Exists = 101,	//���鲻����
		E_ErrorCode_Troop_State_Err = 102,	//����״̬�쳣
		E_ErrorCode_hero_Force_Full = 103,	//����Ӣ��Я��ʿ������
		E_ErrorCode_MarchPathError = 104,	//·������
		E_ErrorCode_Field_NotEmpty = 105,	//�ؿ��Ϸǿղ��ɽ���
		E_ErrorCode_Field_Level_Not_Enough = 106,	//�ؿ�ȼ�����
		E_ErrorCode_Building_Num_Full = 107,	//����������������
		E_ErrorCode_MoveCity_NotCity = 108,	//Ǩ��,���ǳǳ�(����/�ֳ�)
		E_ErrorCode_MoveCity_Troop = 109,	//Ǩ��,���Ӳ��ڳ���
		E_ErrorCode_MoveCity_Tech = 110,	//Ǩ��,���ڿƼ���������
		E_ErrorCode_MoveCity_FieldOwner = 111,	//Ǩ��,���ǵؿ�ӵ����
		E_ErrorCode_MoveCity_Building = 112,	//Ǩ��,�ؿ����н���
		E_ErrorCode_CloseCity_Status = 113,	//�ճ�,�Ѿ��ڱճ�(׼��/����/��ȴ)
		E_ErrorCode_CloseCity_TaskIdNotExists = 114,	//�ճ�,TaskId������
		E_ErrorCode_Building_Lv_Err = 115,	//�����ȼ���������
		E_ErrorCode_Troop_NoForceNum = 116,	//������������
		E_ErrorCode_FieldsNum_NotEnough = 117,	//�ؿ���������
		E_ErrorCode_Map_ThereIsBuilding = 118,	//�ؿ����н���
		E_ErrorCode_Troop_Tired = 119,	//����ƣ��
		E_ErrorCode_Troop_WoundHeavily = 120,	//��������
		E_ErrorCode_ResurgenceNotOpen = 121,	//����δ����
		E_ErrorCode_BirthCountryFull = 122,	//����������
		E_ErrorCode_OccupyLimit = 123,	//�����ؿ�����
		E_ErrorCode_BuildingLost = 124,	//���������� ���ܱ��� ת������
		E_ErrorCode_QueryObsFrequently = 125,	//��ѯ�赲��̫Ƶ��
		E_ErrorCode_Map_NotNewUser = 126,	//�������û�
		E_ErrorCode_Map_CountryNotExists = 127,	//�ݲ�����
		E_ErrorCode_Map_NotBirthCountry = 128,	//���ǳ�����
		E_ErrorCode_Map_CapacityNotEnough = 129,	//��������������
		E_ErrorCode_Map_TooFar = 130, //Ѱ·����̫Զ
		E_ErrorCode_Map_TargetInvalidPos = 131, //Ѱ·Ŀ���Ƿ�
		E_ErrorCode_Building_FieldType_Err = 132,	//����Ҫ��ؿ����ʹ���
		E_ErrorCode_Building_FieldLevel_Err = 133,	//����Ҫ��ؿ�ȼ�����
		E_ErrorCode_Building_FieldState_Err = 134,	//����Ҫ��ؿ�״̬����
		E_ErrorCode_Map_CountryNotOpen = 135,	//��δ����
		E_ErrorCode_Map_TransferCurBuilding = 136,	//���ܵ�����ǰ����
		E_ErrorCode_Map_TransferBuildingType = 137,	//�����������ʹ���
		E_ErrorCode_Map_TransferRelationShip = 138,	//����������ϵ����
		E_ErrorCode_MigrateMaster = 139,	//��������Ǩ��
		E_ErrorCode_BuildingType = 140,	//�������ʹ���
		E_ErrorCode_ConscriptQueueFull = 141,	//������������
		E_ErrorCode_Conscripting = 142,	//��������
		E_ErrorCode_TroopPosEmpty = 143,	//������ĳ��λ���ǿյ�
		E_PlayerModules_User_NoviceNotExist = 144,	//δ�����ֵ���ͼ
		E_RobotTroopByIdx = 145, //�����˶���δ�ҵ�
		E_ErrorCode_SoldierType = 146,	//��������(����)����

		E_ErrorCode_Map_End = E_ErrorCode_Map_Begin + 99,

		//��������������
		E_ErrorCode_Interior_Begin = 200,
		E_ErrorCode_Tech_Not_Exists = E_ErrorCode_Interior_Begin + 1,//�Ƽ�������
		E_ErrorCode_Redif_Capacity_Full = E_ErrorCode_Interior_Begin + 2,//������������
		E_ErrorCode_Solider_Not_Enough = E_ErrorCode_Interior_Begin + 3,//Ԥ��������
		E_ErrorCode_Building_Not_Exists = E_ErrorCode_Interior_Begin + 4,//����������
		E_ErrorCode_Troops_Transfer_Full = E_ErrorCode_Interior_Begin + 5,//��������������
		E_ErrorCode_FieldIsOccupy = E_ErrorCode_Interior_Begin + 6,//�ؿ��ѱ�ռ���޷����
		E_ErrorCode_HeroTraining_CostNotEnough = E_ErrorCode_Interior_Begin + 7,//������Դ����
		E_ErrorCode_HeroTraining_TechNotGet = E_ErrorCode_Interior_Begin + 8,//�����Ƽ�δ���
		E_ErrorCode_HeroTraining_Repeated = E_ErrorCode_Interior_Begin + 9,//�������ڽ��У��ظ�
		E_ErrorCode_HeroTraining_ReqError = E_ErrorCode_Interior_Begin + 10,//������������Ƿ�
		E_ErrorCode_CancelBuilding_Err = E_ErrorCode_Interior_Begin + 11,//ȡ������ʧ��

		E_ErrorCode_Interior_End = E_ErrorCode_Interior_Begin + 49,

		//����ϵͳ�����붨��
		E_ErrorCode_Quest_Begin = 250,
		E_ErrorCode_Quest_Not_Exists = E_ErrorCode_Quest_Begin + 1,//���񲻴���
		E_ErrorCode_Quest_Chapter_Not_Finish = E_ErrorCode_Quest_Begin + 2,//�½�����δ���
		E_ErrorCode_Quest_Not_Finish = E_ErrorCode_Quest_Begin + 3,//����δ���

		E_ErrorCode_Quest_End = E_ErrorCode_Quest_Begin + 19,

		//Ӣ�۴����붨��
		E_ErrorCode_Hero_Begin = 270,
		E_ErrorCode_Troops_State_Error = E_ErrorCode_Hero_Begin + 1,//����״̬����
		E_ErrorCode_Recruit_Hero_Times = E_ErrorCode_Hero_Begin + 2,//�鿨��������
		E_ErrorCode_Hero_Index_NotEmpty = E_ErrorCode_Hero_Begin + 3,//������Ӣ��λ�÷ǿ�
		E_ErrorCode_Hero_Not_Exists = E_ErrorCode_Hero_Begin + 4,//Ӣ�۲�����
		E_ErrorCode_Hero_InTroops = E_ErrorCode_Hero_Begin + 5,//Ӣ���������
		E_ErrorCode_Leader_Not_Enough = E_ErrorCode_Hero_Begin + 6,//ͳ��ֵ����
		E_ErrorCode_Troops_Not_Enough = E_ErrorCode_Hero_Begin + 7,//������������
		E_ErrorCode_Troops_Vice2_Not_Enough = E_ErrorCode_Hero_Begin + 8,//����2���ò�����
		E_ErrorCode_Hero_Bag_Full = E_ErrorCode_Hero_Begin + 9,//Ӣ�۱�������
		E_ErrorCode_Hero_RepeatUp = E_ErrorCode_Hero_Begin + 10,//Ӣ���ظ�����
		E_ErrorCode_Hero_NumZero = E_ErrorCode_Hero_Begin + 11,//Ӣ������Ϊ0
		E_ErrorCode_Hero_NotOpenSkill3 = E_ErrorCode_Hero_Begin + 12,//��3����δ����
		E_ErrorCode_Hero_OutToWar = E_ErrorCode_Hero_Begin + 13,//Ӣ���ѳ���
		E_ErrorCode_Hero_YetWakeUp = E_ErrorCode_Hero_Begin + 14,//Ӣ���Ѿ���	
		E_ErrorCode_Hero_Training = 285,							//Ӣ��������
		E_ErrorCode_Hero_ArmsConflict = E_ErrorCode_Hero_Begin + 16, // ���ֳ�ͻ���Ѵ���ͬ����

		E_ErrorCode_Hero_HonorNotOpen = E_ErrorCode_Hero_Begin + 19, //  ��ѫδ����
		E_ErrorCode_Hero_HonorMax = E_ErrorCode_Hero_Begin + 20,//��ѫ�Ѿ����ȼ�
		E_ErrorCode_Hero_Eat_None = E_ErrorCode_Hero_Begin + 21,//�����ĵ�Hero ����
		E_ErrorCode_Hero_Remodel_CD = E_ErrorCode_Hero_Begin + 22,//����CD
		E_ErrorCode_Hero_Remodel_No_Advance_Honor = E_ErrorCode_Hero_Begin + 23,//δ��ѫ��δ����
		E_ErrorCode_Hero_UsePoint_Over = E_ErrorCode_Hero_Begin + 24,//����ĵ��������ɷ���ֵ��
		E_ErrorCode_Hero_Busy = E_ErrorCode_Hero_Begin + 25, // Ӣ��æ
		E_ErrorCode_Hero_OveMax = E_ErrorCode_Hero_Begin + 26, // Ӣ�۱�����
		E_ErrorCode_Hero_CreateFail = E_ErrorCode_Hero_Begin + 27,	//  ����Ӣ��ʧ��
		E_ErrorCode_Hero_OverMaxLv = E_ErrorCode_Hero_Begin + 28,	//  �Ѿ��ﵽӢ�����ȼ�
		E_ErrorCode_Hero_OverPVEMaxLv = E_ErrorCode_Hero_Begin + 29,	//  �Ѿ��ﵽӢ��PVE���ȼ�

		E_ErrorCode_Hero_End = E_ErrorCode_Hero_Begin + 30,


		//����ϵͳ������
		E_ErrorCode_Skill_Begin = 300,
		E_ErrorCode_Skill_HeroNotExists = E_ErrorCode_Skill_Begin + 1,	//Ӣ�۲�����
		E_ErrorCode_Skill_HeroInTroop = E_ErrorCode_Skill_Begin + 2,	//Ӣ�۲�����
		E_ErrorCode_Skill_HeroAppointing = E_ErrorCode_Skill_Begin + 3,	//Ӣ��ί����
		E_ErrorCode_Skill_HeroLock = E_ErrorCode_Skill_Begin + 4,	//Ӣ������
		E_ErrorCode_Skill_HeroUpgradedOwnSkill = E_ErrorCode_Skill_Begin + 5,	//Ӣ���������Դ�����
		E_ErrorCode_Skill_HeroLearnedSkill = E_ErrorCode_Skill_Begin + 6,	//Ӣ����ѧϰ����
		E_ErrorCode_Skill_HeroNoInherit = E_ErrorCode_Skill_Begin + 7,	//Ӣ��û�д��м���
		E_ErrorCode_Skill_HeroQuality = E_ErrorCode_Skill_Begin + 8,	//Ӣ��Ʒ�ʲ�ƥ��
		E_ErrorCode_Skill_HeroCamp = E_ErrorCode_Skill_Begin + 9,	//Ӣ����Ӫ��ƥ��
		E_ErrorCode_Skill_YetExists = E_ErrorCode_Skill_Begin + 10,	//�����Ѿ�����
		E_ErrorCode_Skill_NotExists = E_ErrorCode_Skill_Begin + 11,	//���ܲ�����
		E_ErrorCode_Skill_AdvancedMax = E_ErrorCode_Skill_Begin + 12,	//��������
		E_ErrorCode_Skill_HeroNum = E_ErrorCode_Skill_Begin + 13,	//Ӣ����������
		E_ErrorCode_Skill_HeroNumZero = E_ErrorCode_Skill_Begin + 14,	//Ӣ������Ϊ0
		E_ErrorCode_Skill_HeroOutside = E_ErrorCode_Skill_Begin + 15,	//Ӣ���ڳ���
		E_ErrorCode_Skill_YetLearned = E_ErrorCode_Skill_Begin + 16,	//������ѧϰ
		E_ErrorCode_Skill_NotLearned = E_ErrorCode_Skill_Begin + 17,	//����δѧϰ
		E_ErrorCode_Skill_LevelMax = E_ErrorCode_Skill_Begin + 18,	//����������
		E_ErrorCode_Skill_LevelMin = E_ErrorCode_Skill_Begin + 19,	//����δ����
		E_ErrorCode_Skill_Slot = E_ErrorCode_Skill_Begin + 20,	//���ܲ�λ����
		E_ErrorCode_Skill_BindHeroFull = E_ErrorCode_Skill_Begin + 21,	//��Ӣ������
		E_ErrorCode_Skill_PointNotEnough = E_ErrorCode_Skill_Begin + 22,	//���ܵ㲻��
		E_ErrorCode_Skill_Repeat = E_ErrorCode_Skill_Begin + 23,	//�����ظ�
		E_ErrorCode_Skill_SlotLock = E_ErrorCode_Skill_Begin + 24,	//���ܲ�����
		E_ErrorCode_Skill_YetLearned3 = E_ErrorCode_Skill_Begin + 25,	//��ѧϰ����3
		E_ErrorCode_Skill_CanNotRecast = E_ErrorCode_Skill_Begin + 26,	//��������
		E_ErrorCode_Skill_HeroRepeat = E_ErrorCode_Skill_Begin + 27,	//Ӣ���ظ�
		E_ErrorCode_Skill_Create = 328,							//���ܴ���ʧ��
		E_ErrorCode_Skill_HeroWearEquipment = 329,							//Ӣ�۴���װ��
		E_ErrorCode_Skill_EventNotExists = 330,							//�����¼�������
		E_ErrorCode_Skill_EventNotOpen = 331,							//�����¼�δ����
		E_ErrorCode_Skill_EventCondition = 332,							//�����¼���������
		E_ErrorCode_Skill_EventConditionConfig = 333,							//�����¼��������ô���
		E_ErrorCode_Skill_EventConditionHero = 334,							//�����¼�Ӣ�۲���������
		E_ErrorCode_Skill_EventConditionUnfinish = 335,							//�����¼�����δ���
		E_ErrorCode_Skill_EventYetExchanged = 336,							//�����¼��Ѷһ�

		//����ϵͳ������
		E_ErrorCode_Guild_Begin = 350,
		E_ErrorCode_Guild_NotExists = E_ErrorCode_Guild_Begin + 1, //���˲�����
		E_ErrorCode_Guild_NoMaster = E_ErrorCode_Guild_Begin + 2, //������
		E_ErrorCode_Guild_NickNameExists = E_ErrorCode_Guild_Begin + 3, //���˼���Ѵ���
		E_ErrorCode_Guild_ExpandNameExists = E_ErrorCode_Guild_Begin + 4, //���������Ѵ���
		E_ErrorCode_Guild_NickNameIllegal = E_ErrorCode_Guild_Begin + 5, //���˼�ư����Ƿ��ַ�
		E_ErrorCode_Guild_ExpandNameIllegal = E_ErrorCode_Guild_Begin + 6, //�������ư����Ƿ��ַ�
		E_ErrorCode_Guild_NoteIllegalChar = E_ErrorCode_Guild_Begin + 7, //��������Ƿ��ַ�
		E_ErrorCode_Guild_NoAccess = E_ErrorCode_Guild_Begin + 8, //�޲���Ȩ��
		E_ErrorCode_Guild_MemberNotEmpty = E_ErrorCode_Guild_Begin + 9, //��Ա�б�ǿ�
		E_ErrorCode_Guild_MemberNotExists = E_ErrorCode_Guild_Begin + 10, //��Ա������
		E_ErrorCode_Guild_InGuild = E_ErrorCode_Guild_Begin + 11, //����Ѽ���
		E_ErrorCode_Guild_CountryId_Err = E_ErrorCode_Guild_Begin + 12, //���˳����ݹ�������
		E_ErrorCode_Guild_MemberIsFull = E_ErrorCode_Guild_Begin + 13, //��Ա����
		E_ErrorCode_Guild_ApplyLock = E_ErrorCode_Guild_Begin + 14, //��������������
		E_ErrorCode_Guild_InviteTimeOut = E_ErrorCode_Guild_Begin + 15, //���봦��ʱ
		E_ErrorCode_Guild_UserQuitCD = E_ErrorCode_Guild_Begin + 16, //�������CD��
		E_ErrorCode_GuildGroup_NameExists = E_ErrorCode_Guild_Begin + 17, //�����Ѵ���
		E_ErrorCode_Guild_GroupMemberNotEmpty = E_ErrorCode_Guild_Begin + 18, //���Ա�б�ǿ�
		E_ErrorCode_Guild_GroupMemberNotExists = E_ErrorCode_Guild_Begin + 19, //���Ա�в�����
		E_ErrorCode_GuildGroup_NotExists = E_ErrorCode_Guild_Begin + 20, //���鲻����
		E_ErrorCode_Guild_EmptyJobId = E_ErrorCode_Guild_Begin + 21, //�ǹ�Ա�û����ɵ����鳤
		E_ErrorCode_Guild_NickNameSizeErr = E_ErrorCode_Guild_Begin + 22, //���˼�Ƴ��ȷǷ�
		E_ErrorCode_Guild_ExpandNameSizeErr = E_ErrorCode_Guild_Begin + 23, //�������Ƴ��ȷǷ�
		E_ErrorCode_Guild_NoteSizeErr = E_ErrorCode_Guild_Begin + 24, //���泤�ȷǷ�
		E_ErrorCode_Guild_LinkApplyExists = E_ErrorCode_Guild_Begin + 25, //�Ѿ������������
		E_ErrorCode_Guild_SetEnemyCD = E_ErrorCode_Guild_Begin + 26, //���õж�״̬��ȴ��
		E_ErrorCode_Guild_LinkApplyNotExists = E_ErrorCode_Guild_Begin + 27, //�������󲻴���
		E_ErrorCode_Guild_LinkApplyTimeOut = E_ErrorCode_Guild_Begin + 28, //���봦��ʱ
		E_ErrorCode_Guild_EnemyFull = E_ErrorCode_Guild_Begin + 29, //�ж����������޷����
		E_ErrorCode_Guild_MyFriendFull = E_ErrorCode_Guild_Begin + 30, //�Լ��������������޷����
		E_ErrorCode_Guild_DstFriendFull = E_ErrorCode_Guild_Begin + 31, //Ŀ��������������޷����
		E_ErrorCode_Guild_DelFriendCD = E_ErrorCode_Guild_Begin + 32, //ɾ������״̬��ȴ��
		E_ErrorCode_Guild_ReleaseCaptieCD = E_ErrorCode_Guild_Begin + 33, //���������ȴ״̬��
		E_ErrorCode_Guild_ActNotExists = E_ErrorCode_Guild_Begin + 34, //�������
		E_ErrorCode_Guild_CollectFull = E_ErrorCode_Guild_Begin + 35, //�ղ���������
		E_ErrorCode_Guild_MailTemplateNotExists = E_ErrorCode_Guild_Begin + 36, //�ʼ�ģ�岻����
		E_ErrorCode_Guild_GuildLvNotEnough = E_ErrorCode_Guild_Begin + 37, //ͬ�˵ȼ�������
		E_ErrorCode_Guild_GuildMemberNotEnough = E_ErrorCode_Guild_Begin + 38, //ͬ�˳�Ա������
		E_ErrorCode_Guild_ConditionNotEnough = E_ErrorCode_Guild_Begin + 39, //Npc�ǳ�����������
		E_ErrorCode_Guild_JobIdNoOpen = E_ErrorCode_Guild_Begin + 40, //ְλδ����
		E_ErrorCode_Guild_GuildGrowNotEnough = E_ErrorCode_Guild_Begin + 41, //ͬ�˲�����

		E_ErrorCode_Guild_End = 399,

		E_ErrorCode_MonthCard_NotExist = 400,	//�¿�������
		E_ErrorCode_MonthCard_Expire = 401,	//�¿�����
		E_ErrorCode_MonthCard_YetReceived = 402,	//�¿�����ȡ

		E_ErrorCode_Chat_Type = 500,	//����Ƶ������
		E_ErrorCode_Chat_Channel = 501,	//�������ʹ���
		E_ErrorCode_Chat_BattleReportNotOwn = 502,	//ս�������Լ���
		E_ErrorCode_Chat_TextEmpty = 503,	//�����ı��ǿյ�
		E_ErrorCode_Chat_Forbid = 504,	//����
		E_ErrorCode_Chat_TextTooLong = 505,	//�����ı�̫��
		E_ErrorCode_Chat_TroopEmpty = 506,	//�����ǿյ�
		E_ErrorCode_Chat_BattleReportEmpty = 507,	//ս���ǿյ�
		E_ErrorCode_Chat_NotSameGroup = 508,	//ս����������ͬ����
		E_ErrorCode_Chat_ChineseDisable = 509,  //����������

		E_ErrorCode_Battle_Report_Begin = 600,	// ս����ʼ
		E_ErrorCode_Battle_Report_inval = E_ErrorCode_Battle_Report_Begin + 1,	// ��Чս��
		E_ErrorCode_Battle_Report_OverMax = E_ErrorCode_Battle_Report_Begin + 2,	//  �������ս��
		E_ErrorCode_Battle_Report_Favorited = E_ErrorCode_Battle_Report_Begin + 3,	//  ���ղ�

		E_ErrorCode_Item_Begin = 700,	// Item ��ʼ
		E_ErrorCode_Item_CheckFail = E_ErrorCode_Item_Begin + 1,	//  ʹ���������ʧ��(�������㣬���߲���ʹ�ã�
		E_ErrorCode_Item_NotEnough = E_ErrorCode_Item_Begin + 2,	//  ���߲���
		E_ErrorCode_Item_Invalid = E_ErrorCode_Item_Begin + 3,	//  ��Ч����
		E_ErrorCode_Item_OverMax = E_ErrorCode_Item_Begin + 4,	//  �����������ֵ
		E_ErrorCode_Item_BagCellOver = E_ErrorCode_Item_Begin + 5,	//  ����������������
		E_ErrorCode_Item_BagFullAndSendMail = E_ErrorCode_Item_Begin + 6,	//  ����������Ʒͨ���ʼ�Mail ���ͣ������
		E_ErrorCode_Item_TimeOut = E_ErrorCode_Item_Begin + 7,	//  ���߹���
		E_ErrorCode_Item_User_LineOff = E_ErrorCode_Item_Begin + 8,	//  �û�������

		E_ErrorCode_Appointment_Begin = 750,  // ί�� ��ʼ
		E_ErrorCode_Appointment_UnOpen = E_ErrorCode_Appointment_Begin + 1, //  δ����ί�ι�
		E_ErrorCode_Appointment_TradeType = E_ErrorCode_Appointment_Begin + 2, //  ���׵����� �Ƿ�
		E_ErrorCode_Appointment_UnKnow = E_ErrorCode_Appointment_Begin + 3, //  �Ƿ���, δ֪��ί��

		E_ErrorCode_Search_OverTime = 755,  // Ѱ��������ڣ�δ�ҵ���
		E_ErrorCode_Search_HeroRepeate = 756,  //  Ѱ��Hero �ظ� 
		E_ErrorCode_Search_Hero_NotEnough = 757,	//   Ӣ����������

		E_ErrorCode_Polic_Invalid_ID = 780,  // ��Ч����ID
		E_ErrorCode_Policy_HaveNotTimes = 781,	//  ���ô�������
		E_ErrorCode_Policy_NotSuperimpose = 782,  // ���ɵ���ʹ��
		E_ErrorCode_Policy_NotChargeTimes = 783,	//  ���ܴ�������
		E_ErrorCode_Policy_CheckFail = 784,	// ������������������
		E_ErrorCode_Policy_ForceTypeErr = 785,	// ���߱���������������
		E_ErrorCode_Policy_CivilizedTypeErr = 786,	// ��������������������

		E_ErrorCode_CivilizationActivity_Invalid_Param = 790,  // ��Ч����
		E_ErrorCode_CivilizationActivity_Reward_Invalid = 791,  //  ������Ч�� ������߲�����

		E_ErrorCode_Friend_Server = 800,	//FriendServer�쳣
		E_ErrorCode_Friend_IsSelf = 801,	//���ܰ����Լ�
		E_ErrorCode_Friend_SelfFriendFull = 802,	//�Լ���������
		E_ErrorCode_Friend_TargetFriendFull = 803,	//�Է���������
		E_ErrorCode_Friend_SelfBlack = 804,	//���Լ�������
		E_ErrorCode_Friend_TargetBlack = 805,	//�ڶԷ�������
		E_ErrorCode_Friend_YetFriend = 806,	//���Ǻ���
		E_ErrorCode_Friend_NotFriend = 807,	//���Ǻ���
		E_ErrorCode_Friend_YetApply = 808,	//�Ѿ�����
		E_ErrorCode_Friend_NotApply = 809,	//��δ����
		E_ErrorCode_Friend_BlackFull = 810,	//����������
		E_ErrorCode_Friend_YetBlack = 811,	//���ں�����
		E_ErrorCode_Friend_NotBlack = 812,	//���ں�����
		E_ErrorCode_Friend_GroupNameEmpty = 813,	//Ⱥ����Ϊ��
		E_ErrorCode_Friend_GroupNotExist = 814,	//Ⱥ�鲻����
		E_ErrorCode_Friend_GroupMemberFull = 815,	//Ⱥ���Ա����
		E_ErrorCode_Friend_GroupMemberRepeat = 816,	//Ⱥ���Ա�ظ�
		E_ErrorCode_Friend_GroupNumFull = 817,	//Ⱥ������
		E_ErrorCode_Friend_YetInGroup = 818,	//����Ⱥ����
		E_ErrorCode_Friend_NotInGroup = 819,	//����Ⱥ����
		E_ErrorCode_Friend_NotGroupInvite = 820,	//û��Ⱥ������
		E_ErrorCode_Friend_IsGroupCreator = 821,	//��Ⱥ�鴴����
		E_ErrorCode_Friend_NotGroupCreator = 822,	//����Ⱥ�鴴����
		E_ErrorCode_Friend_Refuse = 823,	//�ܾ�����

		E_ErrorCode_Business_Begin = 900,  // ��ҵ�
		E_ErrorCode_Business_Invalid_ID = E_ErrorCode_Business_Begin + 1,  // ��Ч�ID
		E_ErrorCode_Business_Had_Got_Reward = E_ErrorCode_Business_Begin + 2,  //  ��������ȡ
		E_ErrorCode_Business_Not_Reward = E_ErrorCode_Business_Begin + 3,  //  �޽���

		E_ErrorCode_Business_VedioNotUnlock = E_ErrorCode_Business_Begin + 4,	//  ��Ƶ����δ����

		E_ErrorCode_Arena_NoFoundTarget = 1000,	//û���ҵ�����
		E_ErrorCode_Arena_TroopInvalid = 1001,	//��ս������Ч
		E_ErrorCode_Arena_MatchCount = 1002,	//ƥ���������
		E_ErrorCode_Arena_Ranking = 1003,	//��������
		E_ErrorCode_Arena_TroopNumZero = 1004, //����������������0

		E_ErrorCode_Store_Over_Max_Refresh = 1015, // ��������ֹ�ˢ�´���
		E_ErrorCode_Store_NotExistGoods = 1016, //  �����ڵ���Ʒ
		E_ErrorCode_Store_NotShop = 1017, //  �����ڵ��̵�
		E_ErrorCode_Store_HaveGot = 1018,	//	�ѹ���

		E_ErrorCode_Euipment_Invalid = 1030, //  ��Чװ��,�Ƿ�װ��
		E_ErrorCode_Euipment_Hero_Invalid = 1031, //  ��Ч��Hero
		E_ErrorCode_Euipment_OverMax = 1032, //  װ����
		E_ErrorCode_Euipment_CreateFail = 1033, //  ����װ��ʧ��
		E_ErrorCode_Euipment_Lock = 1034, //  װ����
		E_ErrorCode_Euipment_TechLvLimit = 1035, //  װ���Ķ���ĿƼ��ȼ�����, �޷���һ������
		E_ErrorCode_Euipment_LvOverTechLv = 1036, //  ���ĵ�װ���ȼ����� �Ƽ��ȼ�����������
		E_ErrorCode_Euipment_Have_Hero = 1037,	//  װ����װ����Hero����
		E_ErrorCode_Euipment_IsMaxLv = 1038, //  �Ѿ����ȼ���
		E_ErrorCode_Euipment_HeroMinLv = 1039, //  hero�ȼ����� (δ�ﵽ ����װ������͵ȼ�)
		E_ErrorCode_Euipment_LimitTimeNotUse = 1040, //  ��ʱװ������ʹ��(ǿ�����ϣ�

		E_ErrorCode_Horse_Invalid = 1041, //  �������Ϸ�,�Ƿ���ƥ 

		E_ErrorCode_Newer7Day_Invalid = 1051, //  ����������Ч
		E_ErrorCode_Newer7Day_OverTime = 1052, // �������ڣ�������
		E_ErrorCode_Newer7Day_RewardUnGet = 1053, // ���� ���ǿ���ȡ״̬

		E_ErrorCode_Trial_IdInvalid = 1100,	//����Id��Ч
		E_ErrorCode_Trial_Guard = 1101,	//�����ؾ�����
		E_ErrorCode_Trial_PreviousFloor = 1102,	//��һ��δ���
		E_ErrorCode_Trial_NoMainHero = 1103,	//û������
		E_ErrorCode_Trial_FloorNotCompleted = 1104,	//δ��ɸò�ȫ���ؿ�
		E_ErrorCode_Trial_FloorRewardYetGained = 1105,	//����ȡ�ò㽱��
		E_ErrorCode_Trial_End = 1150,

		E_ErrorCode_Season2_InvalidTask = 1200, // ��Ч��ҵ�ID
		E_ErrorCode_Season2_NotReward = 1201, // �޽�Ʒ����ȡ

		//�츳
		E_ErrorCode_Talent_TalentModulesNull = 1300, // Talent Modulesģ��δ��
		E_ErrorCode_Talent_TalentGameModulesNull = 1301, // TalentGame Modulesģ��δ��
		E_ErrorCode_Talent_NoPoint = 1302, // û��ʣ��������
		E_ErrorCode_Talent_RoleExpConfigNull = 1303, // ͨ��Level��ѯRoleExp���ò�����
		E_ErrorCode_Talent_DropInfoConfigNull = 1304, // ͨ��Id��ѯDropInfo���ò�����
		E_ErrorCode_Talent_DoDropError = 1305, // ���ɵ���ʱ����
		E_ErrorCode_Talent_NoPendingChoose = 1306, // û��Ϊѡ�������
		E_ErrorCode_Talent_InvalidParams = 1307, // ��Ч�Ĳ���
		E_ErrorCode_Talent_AlreadyActivate = 1308, // �Ѽ���
		E_ErrorCode_Talent_LevelLimit = 1309, // ���츳������
		E_ErrorCode_Talent_TalentTypeConfigNull = 1310, // ͨ��Id��ѯTalentType���ò�����
		E_ErrorCode_Talent_SlotTypeNotExist = 1311, // ��λ���Ͳ�����
		E_ErrorCode_Talent_SlotLock = 1312, // ��λδ����
		E_ErrorCode_Talent_AlreadyUnload = 1313, // ��ж��
		E_ErrorCode_Talent_TalentConfigNull = 1314, // ͨ��Id��ѯTalent���ò�����
		E_ErrorCode_Talent_SysUnlock = 1315, // ϵͳ������δ����

		//SOPϵͳר�ô�����
		E_ErrorCode_Support_ServerId = 10000,	//ServerId����
		E_ErrorCode_Support_Parse = 10001,	//��������
		E_ErrorCode_Support_UserNotExists = 10002,	//�û�������
		E_ErrorCode_Support_ModuleError = 10003,	//Supportģ���쳣
		E_ErrorCode_Support_ChargeOrderRepeated = 10004,	//��ֵ�����ظ�
		E_ErrorCode_Support_ChargePcIdInvalid = 10005,	//��ֵpcid��Ч
		E_ErrorCode_Support_ChargeTypeError = 10006,	//��ֵ���ʹ���
		E_ErrorCode_Support_MonthCardIdError = 10007,	//�¿�Id����


		//LoginServer ר�� 10100 - 10200 
		E_ErrorCode_Login_ErrorVersion = 10101, //�汾����
		E_ErrorCode_Login_BlackList = 10102, //������
		E_ErrorCode_Login_ErrorUserIdData = 10103,//�û����ݴ���
		E_ErrorCode_Login_NoGateServer = 10104, // û���ҵ����ط�����
		E_ErrorCode_Login_GateServerFull = 10105,// ���ط���������������
		E_ErrorCode_Login_HttpAuthError = 10106, // �˺� ��֤ʧ��
		E_ErrorCode_Login_Ban = 10107, //�˺ű���
		E_ErrorCode_Login_ServerMaintance = 10108, //������ά����
		E_ErrorCode_Login_DisableIPState = 10109, //��IP���ڵز������¼
	};

}
