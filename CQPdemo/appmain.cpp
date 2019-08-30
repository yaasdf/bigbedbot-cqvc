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

	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
    if (enabled)
    {
        for (auto& [group, round] : duel::flipcoin::groupStat)
        {
            duel::flipcoin::roundCancel(group);
        }
        for (auto& [group, round] : duel::roulette::groupStat)
        {
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
    eat::foodLoadListFromDb();
    pee::peeCreateTable();
    pee::peeLoadFromDb();
    std::thread(timedCommit, std::ref(eat::db)).detach();
    std::thread(timedCommit, std::ref(pee::db)).detach();

    {
        auto t = time(nullptr);
        t -= 60 * 60 * 24; // yesterday
        pee::daily_refresh_time = t;
        pee::daily_refresh_tm_auto = *localtime(&t);
    }

    std::thread([&]() {
        auto &rec = pee::daily_refresh_tm_auto;
        using namespace std::chrono_literals;
        while (enabled)
        {
            std::this_thread::sleep_for(5s);

            auto t = time(nullptr);
            std::tm tm = *localtime(&t);

            // Skip if same day
            if (tm.tm_year <= rec.tm_year && tm.tm_yday <= rec.tm_yday)
                continue;

            if (tm.tm_hour >= pee::NEW_DAY_TIME_HOUR && tm.tm_hour >= pee::NEW_DAY_TIME_MIN)
                pee::flushDailyTimep(true);
        }
    }).detach();

    std::string boot_info = help::boot_info();
    CQ_sendGroupMsg(ac, 479733965, boot_info.c_str());
    CQ_sendGroupMsg(ac, 391406854, boot_info.c_str());

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
        for (auto& [group, round] : duel::flipcoin::groupStat)
        {
            duel::flipcoin::roundCancel(group);
        }
        for (auto& [group, round] : duel::roulette::groupStat)
        {
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

    if (!strcmp(msg, "接近我"))
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

    if (!strcmp(msg, "帮助"))
    {
        std::string help = help::help();
        CQ_sendGroupMsg(ac, fromGroup, help.c_str());
        return EVENT_BLOCK;
    }

    // 吃什么
    auto c = eat::msgDispatcher(msg);
    if (c.func) CQ_sendGroupMsg(ac, fromGroup, c.func(fromGroup, fromQQ, c.args, msg).c_str());

    // 开箱
    auto d = pee::msgDispatcher(msg);
    if (d.func) CQ_sendGroupMsg(ac, fromGroup, d.func(fromGroup, fromQQ, d.args, msg).c_str());

    // 禁烟跌坑
    auto e = pee::smokeIndicator(msg);
    if (e.func) CQ_sendGroupMsg(ac, fromGroup, e.func(fromGroup, fromQQ, e.args, msg).c_str());

    // 
    auto f = duel::msgDispatcher(msg);
    if (f.func) CQ_sendGroupMsg(ac, fromGroup, f.func(fromGroup, fromQQ, f.args, msg).c_str());

    // fate
    auto g = mnp::msgDispatcher(msg);
    if (g.func) CQ_sendGroupMsg(ac, fromGroup, g.func(fromGroup, fromQQ, g.args, msg).c_str());

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
    return (c.func || d.func || e.func || f.func || g.func) ? EVENT_BLOCK : EVENT_IGNORE;
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


//////////////////////////////////
std::vector<std::string> msg2args(const char* msg)
{
    std::vector<std::string> query;
    if (msg == nullptr) return query;
    std::stringstream ss(msg);
    while (ss)
    {
        std::string s;
        ss >> s;
        if (!s.empty())
            query.push_back(s);
    }
    return query;
}

#define byte win_byte_override 
#include <Windows.h>
#undef byte
std::string gbk2utf8(std::string gbk)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string utf82gbk(std::string utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string strip(std::string& s)
{
    int end = (int)s.length() - 1;
    int start = 0;
    while (start < (int)s.length() - 1 && (s[start] == ' ')) ++start;
    while (end > start && (s[end] == ' ' || s[end] == '\n' || s[end] == '\r')) --end;
    return s.substr(start, end + 1 - start);
}

std::string getCardFromGroupInfoV2(const char* base64_decoded)
{
    size_t nick_offset = 8 + 8;
    size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
    size_t card_offset = nick_offset + 2 + nick_len;
    size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
    if (card_len == 0)
        return std::string(&base64_decoded[nick_offset+2], nick_len);
    else
        return std::string(&base64_decoded[card_offset+2], card_len);
}

int getPermissionFromGroupInfoV2(const char* base64_decoded)
{
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
}

#include "cpp-base64/base64.h"
std::string getCard(int64_t group, int64_t qq)
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