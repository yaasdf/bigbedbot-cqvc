/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载

#include "app/eat.h"
#include "app/pee.h"
#include "app/duel.h"
#include "app/monopoly.h"
#include "app/help.h"
#include "app/event_case.h"
#include "app/group.h"
#include "app/weather.h"

#include <curl/curl.h>

using namespace std;
int64_t QQME;

#include "app/dbconn.h"
#include <thread>
#include <chrono>
void timedCommit(SQLite& db)
{
    db.transactionStart();
    while (enabled)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1min);
        db.commit(true);
    }
    db.transactionStop();
}

std::map<int, std::map<int, std::vector<std::function<void()>>>> timedEventQueue;
void timedEventLoop()
{
    using namespace std::chrono_literals;
    int prev_hr = 0, prev_min = 0;
    while (enabled)
    {
        auto t = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN, 0);
        if (prev_min == t.tm_min)
        {
            std::this_thread::sleep_for(30s);
            continue;
        }

        for (auto& f : timedEventQueue[t.tm_hour][t.tm_min])
        {
            if (f) f();
        }

        prev_hr = t.tm_hour;
        prev_min = t.tm_min;
        std::this_thread::sleep_for(1min);
    }
}


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
    if (enabled)
    {
        for (auto& [group, cfg] : grp::groups)
        {
            if (cfg.flipcoin_running)
                duel::flipcoin::roundCancel(group);
            if (cfg.roulette_running)
                duel::roulette::roundCancel(group);
        }
        pee::db.transactionStop();
        enabled = false;
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
	enabled = true;
    QQME = CQ_getLoginQQ(ac);
    eat::foodCreateTable();
	eat::drinkCreateTable();
    //eat::foodLoadListFromDb();
    eat::updateSteamGameList();
    pee::peeCreateTable();
    pee::peeLoadFromDb();
	grp::CreateTable();
	grp::LoadListFromDb();

    for (auto& g : grp::groups)
        g.second.updateMembers();

    timedEventQueue.clear();
    std::thread(timedEventLoop).detach();

    std::thread(timedCommit, std::ref(eat::db)).detach();
    std::thread(timedCommit, std::ref(pee::db)).detach();

    mnp::calc_event_max();

    {
        pee::daily_refresh_time = time(nullptr) - 60 * 60 * 24; // yesterday
        pee::daily_refresh_tm_auto = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
    }

    timedEventQueue[0][0].push_back([&]() {
        for (auto& g : grp::groups)
            g.second.updateMembers();
    });

    timedEventQueue[0][0].push_back([&]() {
        eat::updateSteamGameList();
    });

    timedEventQueue[pee::NEW_DAY_TIME_HOUR][pee::NEW_DAY_TIME_MIN].push_back([&]() {
        pee::flushDailyTimep(true);
    });

    // 照顾美国人
    timedEventQueue[4][0].push_back([&]() {
        event_case::startEvent();
    });
    timedEventQueue[5][0].push_back([&]() {
        event_case::stopEvent();
    });

    timedEventQueue[event_case::EVENT_CASE_TIME_HOUR_START][event_case::EVENT_CASE_TIME_MIN_START].push_back([&]() {
        event_case::startEvent();
    });
    timedEventQueue[event_case::EVENT_CASE_TIME_HOUR_END][event_case::EVENT_CASE_TIME_MIN_END].push_back([&]() {
        event_case::stopEvent();
    });

    std::string boot_info = help::boot_info();
    broadcastMsg(boot_info.c_str());

	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
    if (enabled)
    {
        for (auto& [group, cfg] : grp::groups)
        {
            if (cfg.flipcoin_running)
                duel::flipcoin::roundCancel(group);
            if (cfg.roulette_running)
                duel::roulette::roundCancel(group);
        }
        pee::db.transactionStop();
        enabled = false;
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
        CQ_sendPrivateMsg(ac, fromQQ, pee::unsmoke(fromQQ).c_str());
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

    if (!strcmp(msg, "帮助") || !strcmp(msg, "幫助"))
    {
        std::string help = help::help();
        CQ_sendGroupMsg(ac, fromGroup, help.c_str());
        return EVENT_BLOCK;
    }

    std::string buf;
	bool handled = false;

	if (strstr(msg, "!roll") == msg || strstr(msg, "/roll") == msg)
	{
		auto query = msg2args(msg);
		int val = 100;
		if (query.size() > 1)
		{
			val = atoi(query[1].c_str());
			if (val < 0) val = 100;
		}
		CQ_sendGroupMsg(ac, fromGroup, std::to_string(randInt(0, val)).c_str());
		return EVENT_BLOCK;
	}

	// 群组命令
	auto b = grp::msgDispatcher(msg);
	if (b.func)
	{
		if (grp::groups.find(fromGroup) == grp::groups.end())
		{
			grp::newGroup(fromGroup);
		}

		buf = b.func(fromGroup, fromQQ, b.args, msg);
		if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
		handled = true;
	}

	if (grp::groups.find(fromGroup) == grp::groups.end())
	{
		return EVENT_IGNORE;
	}
	auto& gr = grp::groups[fromGroup];
	using namespace grp;

    // 吃什么
	if (gr.getFlag(Group::MASK_EAT))
	{
		auto c = eat::msgDispatcher(msg);
		if (c.func)
		{
			buf = c.func(fromGroup, fromQQ, c.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 开箱
	if (gr.getFlag(Group::MASK_MONOPOLY))
	{
		auto d = pee::msgDispatcher(msg);
		if (d.func)
		{
			buf = d.func(fromGroup, fromQQ, d.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 禁烟跌坑
	if (gr.getFlag(Group::MASK_SMOKE))
	{
		auto e = pee::smokeIndicator(msg);
		if (e.func)
		{
			buf = e.func(fromGroup, fromQQ, e.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // 
	if (gr.getFlag(Group::MASK_ROULETTE))
	{
		auto f = duel::msgDispatcher(msg);
		if (f.func)
		{
			buf = f.func(fromGroup, fromQQ, f.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // fate
	if (gr.getFlag(Group::MASK_MONOPOLY))
	{
		auto g = mnp::msgDispatcher(msg);
		if (g.func)
		{
			buf = g.func(fromGroup, fromQQ, g.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // event_case
	if (gr.getFlag(Group::MASK_MONOPOLY))
	{
		auto h = event_case::msgDispatcher(msg);
		if (h.func)
		{
			buf = h.func(fromGroup, fromQQ, h.args, msg);
			if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
			handled = true;
		}
	}

    // event_case
    auto i = weather::msgDispatcher(msg);
    if (i.func)
    {
        buf = i.func(fromGroup, fromQQ, i.args, msg);
        if (!buf.empty()) CQ_sendGroupMsg(ac, fromGroup, buf.c_str());
		handled = true;
    }

    // update smoke status 
    if (fromQQ != QQME && fromQQ != 10000 && fromQQ != 1000000)
    {
        pee::prevUser[fromGroup] = fromQQ;
        if (pee::smokeGroups.find(fromQQ) != pee::smokeGroups.end())
        {
            time_t t = time(nullptr);
            std::list<int64_t> expired;
            for (auto& g : pee::smokeGroups[fromQQ])
                if (t > g.second) expired.push_back(g.first);
            for (auto& g : expired)
                pee::smokeGroups.erase(g);
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
CQEVENT(int32_t, __menuA, 0)() {
    pee::flushDailyTimep();
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
    time_t t = time(nullptr);
    for (auto& c : pee::smokeGroups)
        for (auto& g : c.second)
            if (t < g.second)
                CQ_setGroupBan(ac, g.first, c.first, 0);
	return 0;
}


GroupMemberInfo::GroupMemberInfo(const char* base64_decoded)
{
    size_t offset = 0;
    int16_t len = 0;

    group = ntohll(*(uint64_t*)(&base64_decoded[offset]));
    offset += 8;

    qqid = ntohll(*(uint64_t*)(&base64_decoded[offset]));
    offset += 8;

    len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
    offset += 2;
    nick = simple_str(len, &base64_decoded[offset]);
    offset += len;

    len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
    offset += 2;
    card = simple_str(len, &base64_decoded[offset]);
    offset += len;

    gender = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    age = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
    offset += 2;
    area = simple_str(len, &base64_decoded[offset]);
    offset += len;

    joinTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    speakTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
    offset += 2;
    level = simple_str(len, &base64_decoded[offset]);
    offset += len;

    permission = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    dummy1 = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(&base64_decoded[offset]));
    offset += 2;
    title = simple_str(len, &base64_decoded[offset]);
    offset += len;

    titleExpireTime = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;

    canModifyCard = ntohl(*(uint32_t*)(&base64_decoded[offset]));
    offset += 4;
}

std::string getCardFromGroupInfoV2(const char* base64_decoded)
{
    /*
    size_t nick_offset = 8 + 8;
    size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
    size_t card_offset = nick_offset + 2 + nick_len;
    size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
    if (card_len == 0)
        return std::string(&base64_decoded[nick_offset+2], nick_len);
    else
        return std::string(&base64_decoded[card_offset+2], card_len);
        */
    return GroupMemberInfo(base64_decoded).card;
}

int getPermissionFromGroupInfoV2(const char* base64_decoded)
{
    /*
    size_t nick_offset = 8 + 8;
    size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
    size_t card_offset = nick_offset + 2 + nick_len;
    size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
    size_t area_offset = card_offset + 2 + card_len + 4 + 4;
    size_t area_len = (base64_decoded[area_offset] << 8) + base64_decoded[area_offset + 1];
    size_t glvl_offset = area_offset + 2 + area_len + 4 + 4;
    size_t glvl_len = (base64_decoded[glvl_offset] << 8) + base64_decoded[glvl_offset + 1];
    size_t perm_offset = glvl_offset + 2 + glvl_len;
    return (base64_decoded[perm_offset + 0] << (8 * 3)) +
           (base64_decoded[perm_offset + 1] << (8 * 2)) +
           (base64_decoded[perm_offset + 2] << (8 * 1)) +
           (base64_decoded[perm_offset + 3]);
           */
    return GroupMemberInfo(base64_decoded).permission;
}

#include "cpp-base64/base64.h"
std::string getCard(int64_t group, int64_t qq)
{
    if (grp::groups.find(group) != grp::groups.end())
    {
        if (grp::groups[group].haveMember(qq))
            return grp::groups[group].members[qq].card.length() != 0 ?
                grp::groups[group].members[qq].card : grp::groups[group].members[qq].nick;
        else
            return CQ_At(qq);
    }
    else
    {
        std::string qqname;
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            std::string decoded = base64_decode(std::string(cqinfo));
            if (!decoded.empty())
            {
                qqname = getCardFromGroupInfoV2(decoded.c_str());
            }
        }
        if (qqname.empty()) qqname = CQ_At(qq);
        return qqname;
    }
}

bool isGroupManager(int64_t group, int64_t qq)
{
	const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, TRUE);
	if (cqinfo && strlen(cqinfo) > 0)
	{
		std::string decoded = base64_decode(std::string(cqinfo));
		if (!decoded.empty())
		{
			return (getPermissionFromGroupInfoV2(decoded.c_str()) >= 2);
		}
	}
	return false;
}

bool isGroupOwner(int64_t group, int64_t qq)
{
	const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, TRUE);
	if (cqinfo && strlen(cqinfo) > 0)
	{
		std::string decoded = base64_decode(std::string(cqinfo));
		if (!decoded.empty())
		{
			return (getPermissionFromGroupInfoV2(decoded.c_str()) >= 3);
		}
	}
	return false;
}