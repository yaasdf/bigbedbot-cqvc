#pragma once
#define CQAPPID "com.yaasdf.bigbedbot" //请修改AppID，规则见 http://d.cqp.me/Pro/开发/基础信息
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

enum enumCQBOOL
{
    FALSE,
    TRUE
};

inline bool enabled = false;

class SQLite;
void timedCommit(SQLite& db);

extern int64_t QQME;
extern time_t banTime_me;

#include "utils.h"

/*
群成员信息
即**CQ_getGroupMemberInfoV2**返回的信息
前8个字节，即一个Int64_t长度，QQ群号；
接下来8个字节，即一个Int64_t长度，QQ号；
接下来2个字节，即一个short长度，昵称长度；
接下来昵称长度个字节，昵称文本；
接下来2个字节，即一个short长度，群名片长度；
接下来群名片长度个字节，群名片文本；
接下来4个字节，即一个int长度，性别，0男1女；
接下来4个字节，即一个int长度，年龄，QQ里不能直接修改年龄，以出生年为准；
接下来2个字节，即一个short长度，地区长度；
接下来地区长度个字节，地区文本；
接下来4个字节，即一个int长度，入群时间戳；
接下来4个字节，即一个int长度，最后发言时间戳；
接下来2个字节，即一个short长度，群等级长度；
接下来群等级长度个字节，群等级文本；
接下来4个字节，即一个int长度，管理权限，1成员，2管理员，3群主；
接下来4个字节，即一个int长度，0，不知道是什么，可能是不良记录成员；
接下来2个字节，即一个short长度，专属头衔长度；
接下来专属头衔长度长度个字节，专属头衔长度文本；
接下来4个字节，即一个int长度，专属头衔过期时间戳；
接下来4个字节，即一个int长度，允许修改名片，1允许，猜测0是不允许；
*/

class GroupMemberInfo
{
public:
    int64_t group;
    int64_t qqid;
    simple_str nick;
    simple_str card;
    int32_t gender;
    int32_t age;
    simple_str area;
    int32_t joinTime;
    int32_t speakTime;
    simple_str level;
    int32_t permission;
    int32_t dummy1;
    simple_str title;
    int32_t titleExpireTime;
    int32_t canModifyCard;

    GroupMemberInfo() {}
    GroupMemberInfo(const char* base64_decoded);
};

//card: 8+8+2+?+2+?
std::string getCardFromGroupInfoV2(const char* base64_decoded);
std::string getCard(int64_t group, int64_t qq);

// 1成员，2管理员，3群主
int getPermissionFromGroupInfoV2(const char* base64_decoded);

inline std::string CQ_At(int64_t qq)
{
    using namespace std::string_literals;
    return "[CQ:at,qq="s + std::to_string(qq) + "]"s;
}
