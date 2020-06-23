/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
#include "cqp.h"
#include "appmain.h" //Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����

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
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {
    curl_global_init(CURL_GLOBAL_ALL);
	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
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
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
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
	
	// ÿ��ˢ��Ⱥ��Ƭ
	for (auto&[group, groupObj] : grp::groups)
		addTimedEvent([&]() { groupObj.updateMembers(); }, 0, 0);

	// steam game list
	addTimedEvent([&]() { eat::updateSteamGameList(); }, 0, 0);

	// ÿ��ˢ��
	user_op::daily_refresh_time = time(nullptr) - 60 * 60 * 24; // yesterday
	user_op::daily_refresh_tm_auto = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
	addTimedEvent([&]() { user_op::flushDailyTimep(true); }, user_op::NEW_DAY_TIME_HOUR, user_op::NEW_DAY_TIME_MIN);

    // �չ�������
	addTimedEvent([&]() { event_case::startEvent(); }, 4, 0);
	addTimedEvent([&]() { event_case::stopEvent(); }, 4, 0);

	// ����6������
	addTimedEvent([&]() { event_case::startEvent(); }, event_case::EVENT_CASE_TIME_HOUR_START, event_case::EVENT_CASE_TIME_MIN_START);
	addTimedEvent([&]() { event_case::stopEvent(); }, event_case::EVENT_CASE_TIME_HOUR_END, event_case::EVENT_CASE_TIME_MIN_END);

    std::string boot_info = help::boot_info();
    broadcastMsg(boot_info.c_str(), grp::Group::MASK_BOOT_ANNOUNCE);

	return 0;
}


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
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
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {

    if (!strcmp(msg, "�ӽ���") || !strcmp(msg, "�����"))
    {
        CQ_sendPrivateMsg(ac, fromQQ, smoke::selfUnsmoke(fromQQ).c_str());
        return EVENT_BLOCK;
    }

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ
	return EVENT_IGNORE;
}

time_t banTime_me = 0;

/*
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
    
    if (time(NULL) <= banTime_me) return EVENT_IGNORE;

	bool handled = false;

	// Ⱥ������
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

	// �û�ָ��
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

    // ��ʲô
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

    // ����
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

    // ���̵���
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
	//return EVENT_BLOCK; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=4 ��������Ϣ
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=201 �����¼�-���������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
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

