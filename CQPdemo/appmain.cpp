/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载

#include "app/data/user.h"
#include "app/data/group.h"

#include "app/case.h"
#include "app/eat.h"
#include "app/event_case.h"
#include "app/gambol.h"
#include "app/help.h"
#include "app/monopoly.h"
#include "app/smoke.h"
#include "app/tools.h"
#include "app/user_op.h"
#include "app/weather.h"

#include "utils/time_evt.h"
#include "utils/time_util.h"

#include <curl/curl.h>
#include <thread>

using namespace std;
int64_t QQME;

/* 
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {
    curl_global_init(CURL_GLOBAL_ALL);
	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
    curl_global_cleanup();
    if (gBotEnabled)
    {
        for (auto& [group, cfg] : gambol::groupMap)
        {
            if (cfg.flipcoin_running)
                gambol::flipcoin::roundCancel(group);
            if (cfg.roulette_running)
				gambol::roulette::roundCancel(group);
        }
        user::db.transactionStop();
		grp::db.transactionStop();
		eat::db.transactionStop();
        gBotEnabled = false;
    }
	return 0;
}


/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	gBotEnabled = true;
    QQME = CQ_getLoginQQ(ac);

    eat::foodCreateTable();
	eat::drinkCreateTable();
    //eat::foodLoadListFromDb();
    eat::updateSteamGameList();
    user::peeCreateTable();
    user::peeLoadFromDb();
	grp::CreateTable();
	grp::LoadListFromDb();

    for (auto& [groupid, groupObj] : grp::groups)
		groupObj.updateMembers();

	user::db.startTimedCommit();
	grp::db.startTimedCommit();
	eat::db.startTimedCommit();

    mnp::calc_event_max();


	startTimedEvent();
	
	// 每天刷新群名片
	for (auto&[group, groupObj] : grp::groups)
		addTimedEvent([&]() { groupObj.updateMembers(); }, 0, 0);

	// steam game list
	addTimedEvent([&]() { eat::updateSteamGameList(); }, 0, 0);

	// 每天刷批
	user_op::daily_refresh_time = time(nullptr) - 60 * 60 * 24; // yesterday
	user_op::daily_refresh_tm_auto = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
	addTimedEvent([&]() { user_op::flushDailyTimep(true); }, user_op::NEW_DAY_TIME_HOUR, user_op::NEW_DAY_TIME_MIN);

    // 照顾美国人
	addTimedEvent([&]() { event_case::startEvent(); }, 4, 0);
	addTimedEvent([&]() { event_case::stopEvent(); }, 4, 0);

	// 下午6点活动开箱
	addTimedEvent([&]() { event_case::startEvent(); }, event_case::EVENT_CASE_TIME_HOUR_START, event_case::EVENT_CASE_TIME_MIN_START);
	addTimedEvent([&]() { event_case::stopEvent(); }, event_case::EVENT_CASE_TIME_HOUR_END, event_case::EVENT_CASE_TIME_MIN_END);

    std::string boot_info = help::boot_info();
    broadcastMsg(boot_info.c_str(), grp::Group::MASK_BOOT_ANNOUNCE);

	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
    if (gBotEnabled)
    {
        for (auto& [group, game]: gambol::groupMap)
        {
            if (game.flipcoin_running)
                gambol::flipcoin::roundCancel(group);
            if (game.roulette_running)
				gambol::roulette::roundCancel(group);
        }
        user::db.transactionStop();
		grp::db.transactionStop();
		eat::db.transactionStop();
        gBotEnabled = false;
    }
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {

    if (!strcmp(msg, "接近我") || !strcmp(msg, "解禁我"))
    {
        CQ_sendPrivateMsg(ac, fromQQ, smoke::selfUnsmoke(fromQQ).c_str());
        return EVENT_BLOCK;
    }

	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息
	return EVENT_IGNORE;
}

time_t banTime_me = 0;

/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
    
    if (time(NULL) <= banTime_me) return EVENT_IGNORE;

	bool handled = false;

	// 群组命令
	auto f = grp::msgDispatcher(msg);
	if (f.func)
	{
		std::string buf;
		if (grp::groups.find(fromGroup) == grp::groups.end())
		{
			grp::newGroup(fromGroup);
		}

		buf = f.func(fromGroup, fromQQ, f.args, msg);
		if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
		handled = true;
	}

	if (grp::groups.find(fromGroup) == grp::groups.end())
	{
		return EVENT_IGNORE;
	}
	auto& gr = grp::groups[fromGroup];
	using namespace grp;

	// 用户指令
	if (true)
	{
		auto f = user_op::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

	// tools
	if (true)
	{
		auto f = tools::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 吃什么
	if (gr.getFlag(Group::MASK_EAT))
	{
		auto f = eat::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 开箱
	if (gr.getFlag(Group::MASK_CASE))
	{
		auto f = opencase::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 禁烟跌坑
	if (gr.getFlag(Group::MASK_SMOKE))
	{
		auto f = smoke::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 
	if (gr.getFlag(Group::MASK_GAMBOL))
	{
		auto f = gambol::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // fate
	if (gr.getFlag(Group::MASK_MONOPOLY))
	{
		auto f = mnp::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // event_case
	if (gr.getFlag(Group::MASK_EVENT_CASE))
	{
		auto f = event_case::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // weather
	if (true)
	{
		auto f = weather::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

	// help
	if (true)
	{
		auto f = help::msgDispatcher(msg);
		if (f.func)
		{
			std::string buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // update smoke status 
    if (fromQQ != QQME && fromQQ != 10000 && fromQQ != 1000000)
    {
        gr.last_talk_member = fromQQ;
        if (smoke::smokeTimeInGroups.find(fromQQ) != smoke::smokeTimeInGroups.end())
        {
            time_t t = time(nullptr);
            std::list<int64_t> expired;
            for (auto& g : smoke::smokeTimeInGroups[fromQQ])
                if (t > g.second) expired.push_back(g.first);
            for (auto& g : expired)
				smoke::smokeTimeInGroups.erase(g);
        }
    }

    return handled ? EVENT_BLOCK : EVENT_IGNORE;
	//return EVENT_BLOCK; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=101 群事件-管理员变动
* subType 子类型，1/被取消管理员 2/被设置管理员
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=102 群事件-群成员减少
* subType 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
* fromQQ 操作者QQ(仅subType为2、3时存在)
* beingOperateQQ 被操作QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=103 群事件-群成员增加
* subType 子类型，1/管理员已同意 2/管理员邀请
* fromQQ 操作者QQ(即管理员QQ)
* beingOperateQQ 被操作QQ(即加群的QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* 菜单，可在 .json 文件中设置菜单数目、函数名
* 如果不使用菜单，请在 .json 及此处删除无用菜单
*/
CQEVENT(int32_t, __flushDailyP, 0)() {
    user_op::flushDailyTimep();
	return 0;
}

CQEVENT(int32_t, __unsmokeAll, 0)() {
    time_t t = time(nullptr);
    for (auto& c : smoke::smokeTimeInGroups)
        for (auto& g : c.second)
            if (t < g.second)
                CQ_setGroupBan(ac, g.first, c.first, 0);
	return 0;
}


CQEVENT(int32_t, __updateSteamGameList, 0)() {
	eat::updateSteamGameList();
	return 0;
}

