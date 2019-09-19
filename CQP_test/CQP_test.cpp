// CQP_test.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
using std::cout;
using std::endl;

#define CQAPI(ReturnType) extern "C" ReturnType __stdcall 
typedef int32_t CQBOOL;

/*
* 发送私聊消息, 成功返回消息ID
* QQID 目标QQ号
* msg 消息内容
*/
CQAPI(int32_t) CQ_sendPrivateMsg(int32_t AuthCode, int64_t QQID, const char* msg)
{
    cout << "To QQ " << QQID << ":" << endl;
    cout << msg;
    cout << endl;
    return 0;
}

/*
* 发送群消息, 成功返回消息ID
* groupid 群号
* msg 消息内容
*/
CQAPI(int32_t) CQ_sendGroupMsg(int32_t AuthCode, int64_t groupid, const char* msg)
{
    cout << "To Group " << groupid << ":" << endl;
    cout << msg;
    cout << endl;
    return 0;
}

/*
* 发送讨论组消息, 成功返回消息ID
* discussid 讨论组号
* msg 消息内容
*/
CQAPI(int32_t) CQ_sendDiscussMsg(int32_t AuthCode, int64_t discussid, const char* msg)
{
    return 0;
}

/*
* 撤回消息
* msgid 消息ID
*/
CQAPI(int32_t) CQ_deleteMsg(int32_t AuthCode, int64_t msgid)
{
    return 0;
}

/*
* 发送赞 发送手机赞
* QQID QQ号
*/
CQAPI(int32_t) CQ_sendLike(int32_t AuthCode, int64_t QQID)
{
    return 0;
}

/*
* 置群员移除
* groupid 目标群
* QQID QQ号
* rejectaddrequest 不再接收此人加群申请，请慎用
*/
CQAPI(int32_t) CQ_setGroupKick(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL rejectaddrequest)
{
    return 0;
}

/*
* 置群员禁言
* groupid 目标群
* QQID QQ号
* duration 禁言的时间，单位为秒。如果要解禁，这里填写0。
*/
CQAPI(int32_t) CQ_setGroupBan(int32_t AuthCode, int64_t groupid, int64_t QQID, int64_t duration)
{
    cout << "Set Group ban: group " << groupid << ", QQ " << QQID << ", duration " << duration << "s" << endl;
    return 0;
}

/*
* 置群管理员
* groupid 目标群
* QQID QQ号
* setadmin true:设置管理员 false:取消管理员
*/
CQAPI(int32_t) CQ_setGroupAdmin(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL setadmin)
{
    return 0;
}

/*
* 置全群禁言
* groupid 目标群
* enableban true:开启 false:关闭
*/
CQAPI(int32_t) CQ_setGroupWholeBan(int32_t AuthCode, int64_t groupid, CQBOOL enableban)
{
    return 0;
}

/*
* 置匿名群员禁言
* groupid 目标群
* anomymous 群消息事件收到的 anomymous 参数
* duration 禁言的时间，单位为秒。不支持解禁。
*/
CQAPI(int32_t) CQ_setGroupAnonymousBan(int32_t AuthCode, int64_t groupid, const char* anomymous, int64_t duration)
{
    return 0;
}

/*
* 置群匿名设置
* groupid 目标群
* enableanomymous true:开启 false:关闭
*/
CQAPI(int32_t) CQ_setGroupAnonymous(int32_t AuthCode, int64_t groupid, CQBOOL enableanomymous)
{
    return 0;
}

/*
* 置群成员名片
* groupid 目标群
* QQID 目标QQ
* newcard 新名片(昵称)
*/
CQAPI(int32_t) CQ_setGroupCard(int32_t AuthCode, int64_t groupid, int64_t QQID, const char* newcard)
{
    return 0;
}

/*
* 置群退出 慎用, 此接口需要严格授权
* groupid 目标群
* isdismiss 是否解散 true:解散本群(群主) false:退出本群(管理、群成员)
*/
CQAPI(int32_t) CQ_setGroupLeave(int32_t AuthCode, int64_t groupid, CQBOOL isdismiss)
{
    return 0;
}

/*
* 置群成员专属头衔 需群主权限
* groupid 目标群
* QQID 目标QQ
* newspecialtitle 头衔（如果要删除，这里填空）
* duration 专属头衔有效期，单位为秒。如果永久有效，这里填写-1。
*/
CQAPI(int32_t) CQ_setGroupSpecialTitle(int32_t AuthCode, int64_t groupid, int64_t QQID, const char* newspecialtitle, int64_t duration)
{
    return 0;
}

/*
* 置讨论组退出
* discussid 目标讨论组号
*/
CQAPI(int32_t) CQ_setDiscussLeave(int32_t AuthCode, int64_t discussid)
{
    return 0;
}

/*
* 置好友添加请求
* responseflag 请求事件收到的 responseflag 参数
* responseoperation REQUEST_ALLOW 或 REQUEST_DENY
* remark 添加后的好友备注
*/
CQAPI(int32_t) CQ_setFriendAddRequest(int32_t AuthCode, const char* responseflag, int32_t responseoperation, const char* remark)
{
    return 0;
}

/*
* 置群添加请求
* responseflag 请求事件收到的 responseflag 参数
* requesttype根据请求事件的子类型区分 REQUEST_GROUPADD 或 REQUEST_GROUPINVITE
* responseoperation  REQUEST_ALLOW 或 REQUEST_DENY
* reason 操作理由，仅 REQUEST_GROUPADD 且 REQUEST_DENY 时可用
*/
CQAPI(int32_t) CQ_setGroupAddRequestV2(int32_t AuthCode, const char* responseflag, int32_t requesttype, int32_t responseoperation, const char* reason)
{
    return 0;
}

/*
* 取群成员信息
* groupid 目标QQ所在群
* QQID 目标QQ号
* nocache 不使用缓存
*/
CQAPI(const char*) CQ_getGroupMemberInfoV2(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL nocache)
{
    return 0;
}

/*
* 取陌生人信息
* QQID 目标QQ
* nocache 不使用缓存
*/
CQAPI(const char*) CQ_getStrangerInfo(int32_t AuthCode, int64_t QQID, CQBOOL nocache)
{
    return 0;
}

/*
* 日志
* priority 优先级，CQLOG 开头的常量
* category 类型
* content 内容
*/
CQAPI(int32_t) CQ_addLog(int32_t AuthCode, int32_t priority, const char* category, const char* content)
{
    return 0;
}

/*
* 取Cookies 慎用, 此接口需要严格授权
*/
CQAPI(const char*) CQ_getCookies(int32_t AuthCode)
{
    return 0;
}

/*
* 取CsrfToken 慎用, 此接口需要严格授权
*/
CQAPI(int32_t) CQ_getCsrfToken(int32_t AuthCode)
{
    return 0;
}

/*
* 取登录QQ
*/
CQAPI(int64_t) CQ_getLoginQQ(int32_t AuthCode)
{
    return 0;
}

/*
* 取登录QQ昵称
*/
CQAPI(const char*) CQ_getLoginNick(int32_t AuthCode)
{
    return 0;
}

/*
* 取应用目录，返回的路径末尾带"\"
*/
CQAPI(const char*) CQ_getAppDirectory(int32_t AuthCode)
{
    return 0;
}

/*
* 置致命错误提示
* errorinfo 错误信息
*/
CQAPI(int32_t) CQ_setFatal(int32_t AuthCode, const char* errorinfo)
{
    return 0;
}

/*
* 接收语音，接收消息中的语音(record),返回保存在 \data\record\ 目录下的文件名
* file 收到消息中的语音文件名(file)
* outformat 应用所需的语音文件格式，目前支持 mp3 amr wma m4a spx ogg wav flac
*/
CQAPI(const char*) CQ_getRecord(int32_t AuthCode, const char* file, const char* outformat)
{
    return 0;
}
