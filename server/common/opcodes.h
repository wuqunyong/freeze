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
		//业务操作码，从1000开始
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

		//业务返回码，从1000开始
	};

	enum E_ErrorCode
	{
		E_ErrorCode_Suc = 0,  //成功

		//各个模块的错误码定义
		//适用于每个模块的通用错误码
		E_ErrorCode_Common_Begin = 0,
		E_ErrorCode_Common_Unknown = E_ErrorCode_Common_Begin + 1,		//未知错误
		E_ErrorCode_Common_Gold_Not_Enough = E_ErrorCode_Common_Begin + 2,		//金币不足
		E_ErrorCode_Common_Wood_Not_Enough = E_ErrorCode_Common_Begin + 3,		//木材不足
		E_ErrorCode_Common_Food_Not_Enough = E_ErrorCode_Common_Begin + 4,		//食物不足
		E_ErrorCode_Common_Iron_Not_Enough = E_ErrorCode_Common_Begin + 5,		//铁矿不足
		E_ErrorCode_Common_Stone_Not_Enough = E_ErrorCode_Common_Begin + 6,		//石料不足
		E_ErrorCode_Common_Copper_Not_Enough = E_ErrorCode_Common_Begin + 7,		//铜币不足
		E_ErrorCode_Common_CostBook_Not_Enough = E_ErrorCode_Common_Begin + 8,		//策书不足
		E_ErrorCode_Common_Queue_Full = E_ErrorCode_Common_Begin + 9,		//队列已满
		E_ErrorCode_Common_PrvCondition_Not_Enough = E_ErrorCode_Common_Begin + 10,	//前置条件不满足
		E_ErrorCode_Common_Queue_Not_Exists = E_ErrorCode_Common_Begin + 11,	//队列不存在
		E_ErrorCode_Common_Config = E_ErrorCode_Common_Begin + 12,	//找不到配置
		E_ErrorCode_Common_ItemNotEnough = E_ErrorCode_Common_Begin + 13,	//道具不足
		E_ErrorCode_Common_CivilizationNotEnough1 = E_ErrorCode_Common_Begin + 14,	//名声不足(暂时没用了)
		E_ErrorCode_Common_PowerNotEnough = E_ErrorCode_Common_Begin + 15,	//势力不足
		E_ErrorCode_Common_Newbee = E_ErrorCode_Common_Begin + 16,	//新手期
		E_ErrorCode_Common_Cooldown = E_ErrorCode_Common_Begin + 17,	//CD
		E_ErrorCode_Common_Captive = E_ErrorCode_Common_Begin + 18,	//被俘虏
		E_ErrorCode_Common_DBUser = E_ErrorCode_Common_Begin + 19,	//DBUser为空
		E_ErrorCode_Common_AvoidBattle = E_ErrorCode_Common_Begin + 20,	//免战状态
		E_ErrorCode_Common_Owner = E_ErrorCode_Common_Begin + 21,	//己方
		E_ErrorCode_Common_Friend = E_ErrorCode_Common_Begin + 22,	//友方
		E_ErrorCode_Common_VigorNotEnough = E_ErrorCode_Common_Begin + 23,	//体力不足
		E_ErrorCode_Common_NotBuyCount = E_ErrorCode_Common_Begin + 24,	//购买次数不足
		E_ErrorCode_Common_UserNotExists = E_ErrorCode_Common_Begin + 25,	//玩家不存在
		E_ErrorCode_Common_Dead = E_ErrorCode_Common_Begin + 26,	//已死亡
		E_ErrorCode_Common_ReNameCD = E_ErrorCode_Common_Begin + 27,	//更名CD中
		E_ErrorCode_Common_NameExists = E_ErrorCode_Common_Begin + 28,	//名字已存在
		E_ErrorCode_Common_NameSizeErr = E_ErrorCode_Common_Begin + 29,	//名字长度非法
		E_ErrorCode_Common_NameErr = E_ErrorCode_Common_Begin + 30,	//名字非法
		E_ErrorCode_Common_UserMemeSizeErr = E_ErrorCode_Common_Begin + 31,	//玩家简介长度非法
		E_ErrorCode_Common_SkinConditionNotEnough = E_ErrorCode_Common_Begin + 32,	//主城换肤条件不满足
		E_ErrorCode_Common_Module = E_ErrorCode_Common_Begin + 33,	//模块异常
		E_ErrorCode_Common_Module_Close = E_ErrorCode_Common_Begin + 34,	//模块关闭
		E_ErrorCode_Common_Module_NoOpen = E_ErrorCode_Common_Begin + 35,	//功能未开启
		E_ErrorCode_Common_DB_Fail = E_ErrorCode_Common_Begin + 36,	//数据服务异常
		E_ErrorCode_Common_Param_Error = E_ErrorCode_Common_Begin + 37,	//请求参数非法
		E_ErrorCode_Common_CivilizationConditionErr = E_ErrorCode_Common_Begin + 38,	//文明条件不满足
		E_ErrorCode_Common_RecruitTimeOut = E_ErrorCode_Common_Begin + 39,	//卡包已经失效
		E_ErrorCode_Common_DstCaptive = E_ErrorCode_Common_Begin + 40,	//目标被俘虏
		E_ErrorCode_Common_SeasonErr = E_ErrorCode_Common_Begin + 41,	//霸业未开启
		E_ErrorCode_Common_RecruitLimit = E_ErrorCode_Common_Begin + 42,	//卡包招募次数限制
		E_ErrorCode_Common_ResourceNotEnough = 43,								//资源不足

		E_ErrorCode_Common_End = 99,

		E_ErrorCode_Map_Begin = 100,
		E_ErrorCode_FieldNoContinue = E_ErrorCode_Map_Begin,		//地块不连续不能占领
		E_ErrorCode_Troop_Not_Exists = 101,	//队伍不存在
		E_ErrorCode_Troop_State_Err = 102,	//队伍状态异常
		E_ErrorCode_hero_Force_Full = 103,	//超过英雄携带士兵上限
		E_ErrorCode_MarchPathError = 104,	//路径出错
		E_ErrorCode_Field_NotEmpty = 105,	//地块上非空不可建造
		E_ErrorCode_Field_Level_Not_Enough = 106,	//地块等级不足
		E_ErrorCode_Building_Num_Full = 107,	//建筑数量超过上限
		E_ErrorCode_MoveCity_NotCity = 108,	//迁城,不是城池(主城/分城)
		E_ErrorCode_MoveCity_Troop = 109,	//迁城,部队不在城内
		E_ErrorCode_MoveCity_Tech = 110,	//迁城,城内科技正在升级
		E_ErrorCode_MoveCity_FieldOwner = 111,	//迁城,不是地块拥有者
		E_ErrorCode_MoveCity_Building = 112,	//迁城,地块上有建筑
		E_ErrorCode_CloseCity_Status = 113,	//闭城,已经在闭城(准备/持续/冷却)
		E_ErrorCode_CloseCity_TaskIdNotExists = 114,	//闭城,TaskId不存在
		E_ErrorCode_Building_Lv_Err = 115,	//建筑等级超过上限
		E_ErrorCode_Troop_NoForceNum = 116,	//主将兵力不足
		E_ErrorCode_FieldsNum_NotEnough = 117,	//地块数量不足
		E_ErrorCode_Map_ThereIsBuilding = 118,	//地块上有建筑
		E_ErrorCode_Troop_Tired = 119,	//部队疲劳
		E_ErrorCode_Troop_WoundHeavily = 120,	//部队重伤
		E_ErrorCode_ResurgenceNotOpen = 121,	//再起未开启
		E_ErrorCode_BirthCountryFull = 122,	//出生州满了
		E_ErrorCode_OccupyLimit = 123,	//超出地块上限
		E_ErrorCode_BuildingLost = 124,	//建筑不见了 可能被拆 转移走了
		E_ErrorCode_QueryObsFrequently = 125,	//查询阻挡点太频繁
		E_ErrorCode_Map_NotNewUser = 126,	//不是新用户
		E_ErrorCode_Map_CountryNotExists = 127,	//州不存在
		E_ErrorCode_Map_NotBirthCountry = 128,	//不是出生州
		E_ErrorCode_Map_CapacityNotEnough = 129,	//出生州容量不足
		E_ErrorCode_Map_TooFar = 130, //寻路距离太远
		E_ErrorCode_Map_TargetInvalidPos = 131, //寻路目标点非法
		E_ErrorCode_Building_FieldType_Err = 132,	//建筑要求地块类型错误
		E_ErrorCode_Building_FieldLevel_Err = 133,	//建筑要求地块等级错误
		E_ErrorCode_Building_FieldState_Err = 134,	//建筑要求地块状态错误
		E_ErrorCode_Map_CountryNotOpen = 135,	//州未开启
		E_ErrorCode_Map_TransferCurBuilding = 136,	//不能调动当前建筑
		E_ErrorCode_Map_TransferBuildingType = 137,	//调动建筑类型错误
		E_ErrorCode_Map_TransferRelationShip = 138,	//调动建筑关系错误
		E_ErrorCode_MigrateMaster = 139,	//盟主不能迁徙
		E_ErrorCode_BuildingType = 140,	//建筑类型错误
		E_ErrorCode_ConscriptQueueFull = 141,	//征兵队列已满
		E_ErrorCode_Conscripting = 142,	//正在征兵
		E_ErrorCode_TroopPosEmpty = 143,	//队伍中某个位置是空的
		E_PlayerModules_User_NoviceNotExist = 144,	//未在新手岛地图
		E_RobotTroopByIdx = 145, //机器人队伍未找到
		E_ErrorCode_SoldierType = 146,	//兵种类型(大类)错误

		E_ErrorCode_Map_End = E_ErrorCode_Map_Begin + 99,

		//内政建筑错误码
		E_ErrorCode_Interior_Begin = 200,
		E_ErrorCode_Tech_Not_Exists = E_ErrorCode_Interior_Begin + 1,//科技不存在
		E_ErrorCode_Redif_Capacity_Full = E_ErrorCode_Interior_Begin + 2,//征兵容量已满
		E_ErrorCode_Solider_Not_Enough = E_ErrorCode_Interior_Begin + 3,//预备兵不足
		E_ErrorCode_Building_Not_Exists = E_ErrorCode_Interior_Begin + 4,//建筑不存在
		E_ErrorCode_Troops_Transfer_Full = E_ErrorCode_Interior_Begin + 5,//调动部队数已满
		E_ErrorCode_FieldIsOccupy = E_ErrorCode_Interior_Begin + 6,//地块已被占领无法侦查
		E_ErrorCode_HeroTraining_CostNotEnough = E_ErrorCode_Interior_Begin + 7,//练兵资源不足
		E_ErrorCode_HeroTraining_TechNotGet = E_ErrorCode_Interior_Begin + 8,//练兵科技未获得
		E_ErrorCode_HeroTraining_Repeated = E_ErrorCode_Interior_Begin + 9,//练兵正在进行，重复
		E_ErrorCode_HeroTraining_ReqError = E_ErrorCode_Interior_Begin + 10,//练兵请求参数非法
		E_ErrorCode_CancelBuilding_Err = E_ErrorCode_Interior_Begin + 11,//取消建造失败

		E_ErrorCode_Interior_End = E_ErrorCode_Interior_Begin + 49,

		//任务系统错误码定义
		E_ErrorCode_Quest_Begin = 250,
		E_ErrorCode_Quest_Not_Exists = E_ErrorCode_Quest_Begin + 1,//任务不存在
		E_ErrorCode_Quest_Chapter_Not_Finish = E_ErrorCode_Quest_Begin + 2,//章节任务未完成
		E_ErrorCode_Quest_Not_Finish = E_ErrorCode_Quest_Begin + 3,//任务未完成

		E_ErrorCode_Quest_End = E_ErrorCode_Quest_Begin + 19,

		//英雄错误码定义
		E_ErrorCode_Hero_Begin = 270,
		E_ErrorCode_Troops_State_Error = E_ErrorCode_Hero_Begin + 1,//队伍状态错误
		E_ErrorCode_Recruit_Hero_Times = E_ErrorCode_Hero_Begin + 2,//抽卡次数错误
		E_ErrorCode_Hero_Index_NotEmpty = E_ErrorCode_Hero_Begin + 3,//队伍中英雄位置非空
		E_ErrorCode_Hero_Not_Exists = E_ErrorCode_Hero_Begin + 4,//英雄不存在
		E_ErrorCode_Hero_InTroops = E_ErrorCode_Hero_Begin + 5,//英雄已上阵地
		E_ErrorCode_Leader_Not_Enough = E_ErrorCode_Hero_Begin + 6,//统御值不足
		E_ErrorCode_Troops_Not_Enough = E_ErrorCode_Hero_Begin + 7,//部队数不满足
		E_ErrorCode_Troops_Vice2_Not_Enough = E_ErrorCode_Hero_Begin + 8,//副将2配置不满足
		E_ErrorCode_Hero_Bag_Full = E_ErrorCode_Hero_Begin + 9,//英雄背包已满
		E_ErrorCode_Hero_RepeatUp = E_ErrorCode_Hero_Begin + 10,//英雄重复上阵
		E_ErrorCode_Hero_NumZero = E_ErrorCode_Hero_Begin + 11,//英雄数量为0
		E_ErrorCode_Hero_NotOpenSkill3 = E_ErrorCode_Hero_Begin + 12,//第3技能未开启
		E_ErrorCode_Hero_OutToWar = E_ErrorCode_Hero_Begin + 13,//英雄已出征
		E_ErrorCode_Hero_YetWakeUp = E_ErrorCode_Hero_Begin + 14,//英雄已觉醒	
		E_ErrorCode_Hero_Training = 285,							//英雄练兵中
		E_ErrorCode_Hero_ArmsConflict = E_ErrorCode_Hero_Begin + 16, // 兵种冲突，已存相同兵种

		E_ErrorCode_Hero_HonorNotOpen = E_ErrorCode_Hero_Begin + 19, //  授勋未解锁
		E_ErrorCode_Hero_HonorMax = E_ErrorCode_Hero_Begin + 20,//授勋已经最大等级
		E_ErrorCode_Hero_Eat_None = E_ErrorCode_Hero_Begin + 21,//可消耗的Hero 不够
		E_ErrorCode_Hero_Remodel_CD = E_ErrorCode_Hero_Begin + 22,//重塑CD
		E_ErrorCode_Hero_Remodel_No_Advance_Honor = E_ErrorCode_Hero_Begin + 23,//未授勋且未进阶
		E_ErrorCode_Hero_UsePoint_Over = E_ErrorCode_Hero_Begin + 24,//分配的点数超过可分配值。
		E_ErrorCode_Hero_Busy = E_ErrorCode_Hero_Begin + 25, // 英雄忙
		E_ErrorCode_Hero_OveMax = E_ErrorCode_Hero_Begin + 26, // 英雄背包满
		E_ErrorCode_Hero_CreateFail = E_ErrorCode_Hero_Begin + 27,	//  创建英雄失败
		E_ErrorCode_Hero_OverMaxLv = E_ErrorCode_Hero_Begin + 28,	//  已经达到英雄最大等级
		E_ErrorCode_Hero_OverPVEMaxLv = E_ErrorCode_Hero_Begin + 29,	//  已经达到英雄PVE最大等级

		E_ErrorCode_Hero_End = E_ErrorCode_Hero_Begin + 30,


		//技能系统错误定义
		E_ErrorCode_Skill_Begin = 300,
		E_ErrorCode_Skill_HeroNotExists = E_ErrorCode_Skill_Begin + 1,	//英雄不存在
		E_ErrorCode_Skill_HeroInTroop = E_ErrorCode_Skill_Begin + 2,	//英雄部队中
		E_ErrorCode_Skill_HeroAppointing = E_ErrorCode_Skill_Begin + 3,	//英雄委任中
		E_ErrorCode_Skill_HeroLock = E_ErrorCode_Skill_Begin + 4,	//英雄锁定
		E_ErrorCode_Skill_HeroUpgradedOwnSkill = E_ErrorCode_Skill_Begin + 5,	//英雄已升级自带技能
		E_ErrorCode_Skill_HeroLearnedSkill = E_ErrorCode_Skill_Begin + 6,	//英雄已学习技能
		E_ErrorCode_Skill_HeroNoInherit = E_ErrorCode_Skill_Begin + 7,	//英雄没有传承技能
		E_ErrorCode_Skill_HeroQuality = E_ErrorCode_Skill_Begin + 8,	//英雄品质不匹配
		E_ErrorCode_Skill_HeroCamp = E_ErrorCode_Skill_Begin + 9,	//英雄阵营不匹配
		E_ErrorCode_Skill_YetExists = E_ErrorCode_Skill_Begin + 10,	//技能已经存在
		E_ErrorCode_Skill_NotExists = E_ErrorCode_Skill_Begin + 11,	//技能不存在
		E_ErrorCode_Skill_AdvancedMax = E_ErrorCode_Skill_Begin + 12,	//技能满阶
		E_ErrorCode_Skill_HeroNum = E_ErrorCode_Skill_Begin + 13,	//英雄数量错误
		E_ErrorCode_Skill_HeroNumZero = E_ErrorCode_Skill_Begin + 14,	//英雄数量为0
		E_ErrorCode_Skill_HeroOutside = E_ErrorCode_Skill_Begin + 15,	//英雄在城外
		E_ErrorCode_Skill_YetLearned = E_ErrorCode_Skill_Begin + 16,	//技能已学习
		E_ErrorCode_Skill_NotLearned = E_ErrorCode_Skill_Begin + 17,	//技能未学习
		E_ErrorCode_Skill_LevelMax = E_ErrorCode_Skill_Begin + 18,	//技能已满级
		E_ErrorCode_Skill_LevelMin = E_ErrorCode_Skill_Begin + 19,	//技能未升级
		E_ErrorCode_Skill_Slot = E_ErrorCode_Skill_Begin + 20,	//技能槽位错误
		E_ErrorCode_Skill_BindHeroFull = E_ErrorCode_Skill_Begin + 21,	//绑定英雄已满
		E_ErrorCode_Skill_PointNotEnough = E_ErrorCode_Skill_Begin + 22,	//技能点不足
		E_ErrorCode_Skill_Repeat = E_ErrorCode_Skill_Begin + 23,	//技能重复
		E_ErrorCode_Skill_SlotLock = E_ErrorCode_Skill_Begin + 24,	//技能槽锁定
		E_ErrorCode_Skill_YetLearned3 = E_ErrorCode_Skill_Begin + 25,	//已学习技能3
		E_ErrorCode_Skill_CanNotRecast = E_ErrorCode_Skill_Begin + 26,	//不能重铸
		E_ErrorCode_Skill_HeroRepeat = E_ErrorCode_Skill_Begin + 27,	//英雄重复
		E_ErrorCode_Skill_Create = 328,							//技能创建失败
		E_ErrorCode_Skill_HeroWearEquipment = 329,							//英雄穿戴装备
		E_ErrorCode_Skill_EventNotExists = 330,							//技能事件不存在
		E_ErrorCode_Skill_EventNotOpen = 331,							//技能事件未开放
		E_ErrorCode_Skill_EventCondition = 332,							//技能事件条件错误
		E_ErrorCode_Skill_EventConditionConfig = 333,							//技能事件条件配置错误
		E_ErrorCode_Skill_EventConditionHero = 334,							//技能事件英雄不符合条件
		E_ErrorCode_Skill_EventConditionUnfinish = 335,							//技能事件条件未完成
		E_ErrorCode_Skill_EventYetExchanged = 336,							//技能事件已兑换

		//公会系统错误定义
		E_ErrorCode_Guild_Begin = 350,
		E_ErrorCode_Guild_NotExists = E_ErrorCode_Guild_Begin + 1, //联盟不存在
		E_ErrorCode_Guild_NoMaster = E_ErrorCode_Guild_Begin + 2, //非盟主
		E_ErrorCode_Guild_NickNameExists = E_ErrorCode_Guild_Begin + 3, //联盟简称已存在
		E_ErrorCode_Guild_ExpandNameExists = E_ErrorCode_Guild_Begin + 4, //联盟名称已存在
		E_ErrorCode_Guild_NickNameIllegal = E_ErrorCode_Guild_Begin + 5, //联盟简称包含非法字符
		E_ErrorCode_Guild_ExpandNameIllegal = E_ErrorCode_Guild_Begin + 6, //联盟名称包含非法字符
		E_ErrorCode_Guild_NoteIllegalChar = E_ErrorCode_Guild_Begin + 7, //公告包含非法字符
		E_ErrorCode_Guild_NoAccess = E_ErrorCode_Guild_Begin + 8, //无操作权限
		E_ErrorCode_Guild_MemberNotEmpty = E_ErrorCode_Guild_Begin + 9, //成员列表非空
		E_ErrorCode_Guild_MemberNotExists = E_ErrorCode_Guild_Begin + 10, //成员不存在
		E_ErrorCode_Guild_InGuild = E_ErrorCode_Guild_Begin + 11, //玩家已加盟
		E_ErrorCode_Guild_CountryId_Err = E_ErrorCode_Guild_Begin + 12, //联盟出生州规则不满足
		E_ErrorCode_Guild_MemberIsFull = E_ErrorCode_Guild_Begin + 13, //成员已满
		E_ErrorCode_Guild_ApplyLock = E_ErrorCode_Guild_Begin + 14, //联盟申请已锁定
		E_ErrorCode_Guild_InviteTimeOut = E_ErrorCode_Guild_Begin + 15, //邀请处理超时
		E_ErrorCode_Guild_UserQuitCD = E_ErrorCode_Guild_Begin + 16, //玩家退盟CD中
		E_ErrorCode_GuildGroup_NameExists = E_ErrorCode_Guild_Begin + 17, //组名已存在
		E_ErrorCode_Guild_GroupMemberNotEmpty = E_ErrorCode_Guild_Begin + 18, //组成员列表非空
		E_ErrorCode_Guild_GroupMemberNotExists = E_ErrorCode_Guild_Begin + 19, //组成员列不存在
		E_ErrorCode_GuildGroup_NotExists = E_ErrorCode_Guild_Begin + 20, //分组不存在
		E_ErrorCode_Guild_EmptyJobId = E_ErrorCode_Guild_Begin + 21, //非官员用户不可当任组长
		E_ErrorCode_Guild_NickNameSizeErr = E_ErrorCode_Guild_Begin + 22, //联盟简称长度非法
		E_ErrorCode_Guild_ExpandNameSizeErr = E_ErrorCode_Guild_Begin + 23, //联盟名称长度非法
		E_ErrorCode_Guild_NoteSizeErr = E_ErrorCode_Guild_Begin + 24, //公告长度非法
		E_ErrorCode_Guild_LinkApplyExists = E_ErrorCode_Guild_Begin + 25, //已经发起结盟请求
		E_ErrorCode_Guild_SetEnemyCD = E_ErrorCode_Guild_Begin + 26, //设置敌队状态冷却中
		E_ErrorCode_Guild_LinkApplyNotExists = E_ErrorCode_Guild_Begin + 27, //结盟请求不存在
		E_ErrorCode_Guild_LinkApplyTimeOut = E_ErrorCode_Guild_Begin + 28, //申请处理超时
		E_ErrorCode_Guild_EnemyFull = E_ErrorCode_Guild_Begin + 29, //敌对联盟已满无法添加
		E_ErrorCode_Guild_MyFriendFull = E_ErrorCode_Guild_Begin + 30, //自己好友友盟已满无法添加
		E_ErrorCode_Guild_DstFriendFull = E_ErrorCode_Guild_Begin + 31, //目标好友友盟已满无法添加
		E_ErrorCode_Guild_DelFriendCD = E_ErrorCode_Guild_Begin + 32, //删除友盟状态冷却中
		E_ErrorCode_Guild_ReleaseCaptieCD = E_ErrorCode_Guild_Begin + 33, //缴纳赎金冷却状态中
		E_ErrorCode_Guild_ActNotExists = E_ErrorCode_Guild_Begin + 34, //法令不存在
		E_ErrorCode_Guild_CollectFull = E_ErrorCode_Guild_Begin + 35, //收藏坐标已满
		E_ErrorCode_Guild_MailTemplateNotExists = E_ErrorCode_Guild_Begin + 36, //邮件模板不存在
		E_ErrorCode_Guild_GuildLvNotEnough = E_ErrorCode_Guild_Begin + 37, //同盟等级不瞒足
		E_ErrorCode_Guild_GuildMemberNotEnough = E_ErrorCode_Guild_Begin + 38, //同盟成员不满足
		E_ErrorCode_Guild_ConditionNotEnough = E_ErrorCode_Guild_Begin + 39, //Npc城池条件不满足
		E_ErrorCode_Guild_JobIdNoOpen = E_ErrorCode_Guild_Begin + 40, //职位未开启
		E_ErrorCode_Guild_GuildGrowNotEnough = E_ErrorCode_Guild_Begin + 41, //同盟不瞒足

		E_ErrorCode_Guild_End = 399,

		E_ErrorCode_MonthCard_NotExist = 400,	//月卡不存在
		E_ErrorCode_MonthCard_Expire = 401,	//月卡过期
		E_ErrorCode_MonthCard_YetReceived = 402,	//月卡已领取

		E_ErrorCode_Chat_Type = 500,	//聊天频道错误
		E_ErrorCode_Chat_Channel = 501,	//聊天类型错误
		E_ErrorCode_Chat_BattleReportNotOwn = 502,	//战报不是自己的
		E_ErrorCode_Chat_TextEmpty = 503,	//聊天文本是空的
		E_ErrorCode_Chat_Forbid = 504,	//禁言
		E_ErrorCode_Chat_TextTooLong = 505,	//聊天文本太长
		E_ErrorCode_Chat_TroopEmpty = 506,	//队伍是空的
		E_ErrorCode_Chat_BattleReportEmpty = 507,	//战报是空的
		E_ErrorCode_Chat_NotSameGroup = 508,	//战报不属于相同的组
		E_ErrorCode_Chat_ChineseDisable = 509,  //不允许中文

		E_ErrorCode_Battle_Report_Begin = 600,	// 战报开始
		E_ErrorCode_Battle_Report_inval = E_ErrorCode_Battle_Report_Begin + 1,	// 无效战报
		E_ErrorCode_Battle_Report_OverMax = E_ErrorCode_Battle_Report_Begin + 2,	//  超过最大战报
		E_ErrorCode_Battle_Report_Favorited = E_ErrorCode_Battle_Report_Begin + 3,	//  已收藏

		E_ErrorCode_Item_Begin = 700,	// Item 开始
		E_ErrorCode_Item_CheckFail = E_ErrorCode_Item_Begin + 1,	//  使用条件检测失败(数量不足，或者不可使用）
		E_ErrorCode_Item_NotEnough = E_ErrorCode_Item_Begin + 2,	//  道具不足
		E_ErrorCode_Item_Invalid = E_ErrorCode_Item_Begin + 3,	//  无效道具
		E_ErrorCode_Item_OverMax = E_ErrorCode_Item_Begin + 4,	//  超过允许最大值
		E_ErrorCode_Item_BagCellOver = E_ErrorCode_Item_Begin + 5,	//  超过背包最大格子数
		E_ErrorCode_Item_BagFullAndSendMail = E_ErrorCode_Item_Begin + 6,	//  背包满，物品通过邮件Mail 发送，请查收
		E_ErrorCode_Item_TimeOut = E_ErrorCode_Item_Begin + 7,	//  道具过期
		E_ErrorCode_Item_User_LineOff = E_ErrorCode_Item_Begin + 8,	//  用户不在线

		E_ErrorCode_Appointment_Begin = 750,  // 委任 开始
		E_ErrorCode_Appointment_UnOpen = E_ErrorCode_Appointment_Begin + 1, //  未开启委任官
		E_ErrorCode_Appointment_TradeType = E_ErrorCode_Appointment_Begin + 2, //  交易的种类 非法
		E_ErrorCode_Appointment_UnKnow = E_ErrorCode_Appointment_Begin + 3, //  非法的, 未知的委任

		E_ErrorCode_Search_OverTime = 755,  // 寻访任务过期，未找到。
		E_ErrorCode_Search_HeroRepeate = 756,  //  寻访Hero 重复 
		E_ErrorCode_Search_Hero_NotEnough = 757,	//   英雄数量不足

		E_ErrorCode_Polic_Invalid_ID = 780,  // 无效政策ID
		E_ErrorCode_Policy_HaveNotTimes = 781,	//  可用次数不够
		E_ErrorCode_Policy_NotSuperimpose = 782,  // 不可叠加使用
		E_ErrorCode_Policy_NotChargeTimes = 783,	//  充能次数不足
		E_ErrorCode_Policy_CheckFail = 784,	// 政策自身检测条件不足
		E_ErrorCode_Policy_ForceTypeErr = 785,	// 政策兵种类型条件不足
		E_ErrorCode_Policy_CivilizedTypeErr = 786,	// 政策文明类型条件不足

		E_ErrorCode_CivilizationActivity_Invalid_Param = 790,  // 无效参数
		E_ErrorCode_CivilizationActivity_Reward_Invalid = 791,  //  奖励无效， 已领或者不存在

		E_ErrorCode_Friend_Server = 800,	//FriendServer异常
		E_ErrorCode_Friend_IsSelf = 801,	//不能包含自己
		E_ErrorCode_Friend_SelfFriendFull = 802,	//自己好友满了
		E_ErrorCode_Friend_TargetFriendFull = 803,	//对方好友满了
		E_ErrorCode_Friend_SelfBlack = 804,	//在自己黑名单
		E_ErrorCode_Friend_TargetBlack = 805,	//在对方黑名单
		E_ErrorCode_Friend_YetFriend = 806,	//已是好友
		E_ErrorCode_Friend_NotFriend = 807,	//不是好友
		E_ErrorCode_Friend_YetApply = 808,	//已经申请
		E_ErrorCode_Friend_NotApply = 809,	//还未申请
		E_ErrorCode_Friend_BlackFull = 810,	//黑名单满了
		E_ErrorCode_Friend_YetBlack = 811,	//已在黑名单
		E_ErrorCode_Friend_NotBlack = 812,	//不在黑名单
		E_ErrorCode_Friend_GroupNameEmpty = 813,	//群组名为空
		E_ErrorCode_Friend_GroupNotExist = 814,	//群组不存在
		E_ErrorCode_Friend_GroupMemberFull = 815,	//群组成员满了
		E_ErrorCode_Friend_GroupMemberRepeat = 816,	//群组成员重复
		E_ErrorCode_Friend_GroupNumFull = 817,	//群组满了
		E_ErrorCode_Friend_YetInGroup = 818,	//已在群组里
		E_ErrorCode_Friend_NotInGroup = 819,	//不在群组里
		E_ErrorCode_Friend_NotGroupInvite = 820,	//没有群组邀请
		E_ErrorCode_Friend_IsGroupCreator = 821,	//是群组创建者
		E_ErrorCode_Friend_NotGroupCreator = 822,	//不是群组创建者
		E_ErrorCode_Friend_Refuse = 823,	//拒绝好友

		E_ErrorCode_Business_Begin = 900,  // 商业活动
		E_ErrorCode_Business_Invalid_ID = E_ErrorCode_Business_Begin + 1,  // 无效活动ID
		E_ErrorCode_Business_Had_Got_Reward = E_ErrorCode_Business_Begin + 2,  //  奖励已领取
		E_ErrorCode_Business_Not_Reward = E_ErrorCode_Business_Begin + 3,  //  无奖励

		E_ErrorCode_Business_VedioNotUnlock = E_ErrorCode_Business_Begin + 4,	//  视频攻略未解锁

		E_ErrorCode_Arena_NoFoundTarget = 1000,	//没有找到对手
		E_ErrorCode_Arena_TroopInvalid = 1001,	//出战队伍无效
		E_ErrorCode_Arena_MatchCount = 1002,	//匹配次数不足
		E_ErrorCode_Arena_Ranking = 1003,	//排名错误
		E_ErrorCode_Arena_TroopNumZero = 1004, //竞技场队伍数量是0

		E_ErrorCode_Store_Over_Max_Refresh = 1015, // 超过最大手工刷新次数
		E_ErrorCode_Store_NotExistGoods = 1016, //  不存在的商品
		E_ErrorCode_Store_NotShop = 1017, //  不存在的商店
		E_ErrorCode_Store_HaveGot = 1018,	//	已购买

		E_ErrorCode_Euipment_Invalid = 1030, //  无效装备,非法装备
		E_ErrorCode_Euipment_Hero_Invalid = 1031, //  无效的Hero
		E_ErrorCode_Euipment_OverMax = 1032, //  装备满
		E_ErrorCode_Euipment_CreateFail = 1033, //  创建装备失败
		E_ErrorCode_Euipment_Lock = 1034, //  装备锁
		E_ErrorCode_Euipment_TechLvLimit = 1035, //  装备的锻造的科技等级不够, 无法进一步升级
		E_ErrorCode_Euipment_LvOverTechLv = 1036, //  消耗的装备等级超过 科技等级。不能消耗
		E_ErrorCode_Euipment_Have_Hero = 1037,	//  装备已装备到Hero身上
		E_ErrorCode_Euipment_IsMaxLv = 1038, //  已经最大等级了
		E_ErrorCode_Euipment_HeroMinLv = 1039, //  hero等级不足 (未达到 穿戴装备的最低等级)
		E_ErrorCode_Euipment_LimitTimeNotUse = 1040, //  限时装备不可使用(强化材料）

		E_ErrorCode_Horse_Invalid = 1041, //  参数不合法,非法马匹 

		E_ErrorCode_Newer7Day_Invalid = 1051, //  奖励参数无效
		E_ErrorCode_Newer7Day_OverTime = 1052, // 奖励过期，不可领
		E_ErrorCode_Newer7Day_RewardUnGet = 1053, // 奖励 不是可领取状态

		E_ErrorCode_Trial_IdInvalid = 1100,	//试炼Id无效
		E_ErrorCode_Trial_Guard = 1101,	//试炼守军错误
		E_ErrorCode_Trial_PreviousFloor = 1102,	//上一层未完成
		E_ErrorCode_Trial_NoMainHero = 1103,	//没有主将
		E_ErrorCode_Trial_FloorNotCompleted = 1104,	//未完成该层全部关卡
		E_ErrorCode_Trial_FloorRewardYetGained = 1105,	//已领取该层奖励
		E_ErrorCode_Trial_End = 1150,

		E_ErrorCode_Season2_InvalidTask = 1200, // 无效霸业活动ID
		E_ErrorCode_Season2_NotReward = 1201, // 无奖品可领取

		//天赋
		E_ErrorCode_Talent_TalentModulesNull = 1300, // Talent Modules模块未空
		E_ErrorCode_Talent_TalentGameModulesNull = 1301, // TalentGame Modules模块未空
		E_ErrorCode_Talent_NoPoint = 1302, // 没有剩余的领悟点
		E_ErrorCode_Talent_RoleExpConfigNull = 1303, // 通过Level查询RoleExp配置不存在
		E_ErrorCode_Talent_DropInfoConfigNull = 1304, // 通过Id查询DropInfo配置不存在
		E_ErrorCode_Talent_DoDropError = 1305, // 生成掉落时报错
		E_ErrorCode_Talent_NoPendingChoose = 1306, // 没有为选择的领悟
		E_ErrorCode_Talent_InvalidParams = 1307, // 无效的参数
		E_ErrorCode_Talent_AlreadyActivate = 1308, // 已激活
		E_ErrorCode_Talent_LevelLimit = 1309, // 该天赋已满级
		E_ErrorCode_Talent_TalentTypeConfigNull = 1310, // 通过Id查询TalentType配置不存在
		E_ErrorCode_Talent_SlotTypeNotExist = 1311, // 槽位类型不存在
		E_ErrorCode_Talent_SlotLock = 1312, // 槽位未解锁
		E_ErrorCode_Talent_AlreadyUnload = 1313, // 已卸载
		E_ErrorCode_Talent_TalentConfigNull = 1314, // 通过Id查询Talent配置不存在
		E_ErrorCode_Talent_SysUnlock = 1315, // 系统解锁表未解锁

		//SOP系统专用错误码
		E_ErrorCode_Support_ServerId = 10000,	//ServerId错误
		E_ErrorCode_Support_Parse = 10001,	//解析错误
		E_ErrorCode_Support_UserNotExists = 10002,	//用户不存在
		E_ErrorCode_Support_ModuleError = 10003,	//Support模块异常
		E_ErrorCode_Support_ChargeOrderRepeated = 10004,	//充值订单重复
		E_ErrorCode_Support_ChargePcIdInvalid = 10005,	//充值pcid无效
		E_ErrorCode_Support_ChargeTypeError = 10006,	//充值类型错误
		E_ErrorCode_Support_MonthCardIdError = 10007,	//月卡Id错误


		//LoginServer 专用 10100 - 10200 
		E_ErrorCode_Login_ErrorVersion = 10101, //版本错误
		E_ErrorCode_Login_BlackList = 10102, //黑名单
		E_ErrorCode_Login_ErrorUserIdData = 10103,//用户数据错误
		E_ErrorCode_Login_NoGateServer = 10104, // 没有找到网关服务器
		E_ErrorCode_Login_GateServerFull = 10105,// 网关服务器连接数已满
		E_ErrorCode_Login_HttpAuthError = 10106, // 账号 验证失败
		E_ErrorCode_Login_Ban = 10107, //账号被禁
		E_ErrorCode_Login_ServerMaintance = 10108, //服务器维护中
		E_ErrorCode_Login_DisableIPState = 10109, //该IP所在地不允许登录
	};

}
