#define CQAPPID "com.yaasdf.bigbedbot" //请修改AppID，规则见 http://d.cqp.me/Pro/开发/基础信息
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

inline bool enabled = false;

#include "app/dbconn.h"
enum enumCQBOOL
{
    FALSE,
    TRUE
};
extern int64_t QQME;

////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>
inline void timedCommit(SQLite& db)
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

////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>
#include <sstream>
std::vector<std::string> msg2args(const char* msg);
std::string gbk2utf8(std::string gbk);
std::string utf82gbk(std::string utf8);


////////////////////////////////////////////////////////////////////////////////
#include <random>
inline std::random_device random_rd;  // 将用于获得随机数引擎的种子
inline std::mt19937 random_gen(random_rd()); // 以 rd() 播种的标准 mersenne_twister_engine

inline int randInt(int min, int max)   // [min, max]
{
    std::uniform_int_distribution<> dis(min, max);
    return dis(random_gen);
}

inline double randReal(double min = 0.0, double max = 1.0) // [min, max)
{
    std::uniform_real_distribution<> dis(min, max);
    return dis(random_gen);
}

////////////////////////////////////////////////////////////////////////////////

std::string strip(std::string& s);
std::string stripImage(std::string& s);
std::string stripFace(std::string& s);


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

//card: 8+8+2+?+2+?
std::string getCardFromGroupInfoV2(const char* base64_decoded);
std::string getCard(int64_t group, int64_t qq);

// 1成员，2管理员，3群主
int getPermissionFromGroupInfoV2(const char* base64_decoded);

inline std::string CQ_At(int64_t qq)
{
    std::stringstream ss;
    ss << "[CQ:at,qq=" << qq << "]";
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

inline const std::string EMOJI_HORSE    = "[CQ:emoji,id=128052]";
inline const std::string EMOJI_HAMMER   = "[CQ:emoji,id=128296]";
inline const std::string EMOJI_DOWN     = "[CQ:emoji,id=11015]";
inline const std::string EMOJI_NONE     = "[CQ:emoji,id=127514]";
inline const std::string EMOJI_HORN     = "[CQ:emoji,id=128227]";