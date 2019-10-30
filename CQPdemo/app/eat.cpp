#include <sstream>
#include "cqp.h"
#include "eat.h"
#include "appmain.h"
#include "cpp-base64/base64.h"
#include "private/qqid.h"
#include "group.h"
#include <Windows.h>
#include <curl/curl.h>
#include "steam_parser.h"

namespace eat {

class curl_buffer
{
public:
    int length = 0;
    char* content = NULL;
    curl_buffer() = delete;
    curl_buffer(int len) { content = new char[len]; memset(content, 0, len * sizeof(char)); }
    ~curl_buffer() { if (content) delete content; }
};

size_t curl_write(void* buffer, size_t size, size_t count, void* stream)
{
    curl_buffer* buf = (curl_buffer*)stream;
    int newsize = size * count;
    memcpy(buf->content + buf->length, buffer, newsize);
    buf->length += newsize;
    return newsize;
}

steam::SteamAppListParser games;
//std::vector<std::pair<long, std::string>> steamGameList;
void updateSteamGameList()
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        CQ_addLog(ac, CQLOG_WARNING, "play", "curl init error");
    }

    curl_buffer curlbuf(8 * 1024 * 1024);   // 8MB
    curl_easy_setopt(curl, CURLOPT_URL, "http://api.steampowered.com/ISteamApps/GetAppList/v0002/?key=STEAMKEY&format=json");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlbuf);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_PROXY, "http://localhost:1080");
#endif
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
    int ret = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    switch (ret)
    {
    case CURLE_OK:
        switch (games.parse(curlbuf.content))
        {
        case 2:
            CQ_addLog(ac, CQLOG_WARNING, "play", "parse error");
            break;

        case 3:
            CQ_addLog(ac, CQLOG_WARNING, "play", "parsing fsm state error");
            break;

        case 4:
            CQ_addLog(ac, CQLOG_WARNING, "play", "unexpected end of brace");
            break;

        case 5:
            CQ_addLog(ac, CQLOG_WARNING, "play", "deadloop detected");
            break;

        default:
            break;
        }
        break;

    default:
    {
        char msg[128];
        sprintf(msg, "curl error: %d", ret);
        CQ_addLog(ac, CQLOG_WARNING, "play", msg);
    }
        break;
    }

    char msg[128];
    sprintf(msg, "added %u games", games.games.size());
    CQ_addLog(ac, CQLOG_DEBUG, "play", msg);
}

std::string food::to_string(int64_t group)
{
    std::string qqname = "隔壁群友";
    if (group != 0 && offererType == food::QQ && offerer.qq != 0)
    {
        if (grp::groups.find(group) != grp::groups.end())
        {
            if (grp::groups[group].haveMember(offerer.qq))
                qqname = getCard(group, offerer.qq);
        }
        else
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, offerer.qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                //CQ_addLog(ac, CQLOG_DEBUG, "eat", cqinfo);
                std::string decoded = base64_decode(std::string(cqinfo));
                if (!decoded.empty())
                {
                    qqname = getCardFromGroupInfoV2(decoded.c_str());
                }
                else
                    CQ_addLog(ac, CQLOG_ERROR, "eat", "groupmember base64 decode error");
            }
        }
    }

    std::stringstream ss;
    ss << name;
    switch (offererType)
    {
    case food::NAME:  ss << " (" << offerer.name << "提供)"; break;
    case food::QQ:    ss << " (" << qqname << "提供)"; break;
    default: break;
    }

    //std::stringstream ssd;
    //ssd << offererType << ": " << name << " | " << offerer.name << " | " << offerer.qq;
    //CQ_addLog(ac, CQLOG_DEBUG, "eat", ssd.str().c_str());
    return ss.str();
}

inline int addFood(food& f)
{
    const char query[] = "INSERT INTO food(name, adder, qq) VALUES (?,?,?)";
    int ret;
    auto nameutf8 = gbk2utf8(f.name);
    auto offererutf8 = gbk2utf8(f.offerer.name);
    switch (f.offererType)
    {
    case food::NAME:        ret = db.exec(query, { nameutf8, offererutf8, nullptr });  break;
    case food::QQ:          ret = db.exec(query, { nameutf8, nullptr, f.offerer.qq });    break;
    case food::ANONYMOUS:   ret = db.exec(query, { nameutf8, nullptr, nullptr });         break;
    default: break;
    }
    if (ret != SQLITE_OK)
    {
        std::stringstream ss;
        ss << "addFood: " << db.errmsg() << ", " << f.name;
        CQ_addLog(ac, CQLOG_ERROR, "eat", ss.str().c_str());
        return 1;
    }
    foodList.push_back(f);
    return 0;
}

inline food& getFood()
{
    return foodList[randInt(0, foodList.size() - 1)];
}


command msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];
    if (commands_str.find(cmd) == commands_str.end()) return c;

    c.args = query;
    switch (c.c = commands_str[cmd])
    {
    case commands::吃什么:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "無";
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "，你阔以选择";
            ss << getFood().to_string(group);
            return ss.str();
        };
        break;
    case commands::喝什么:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            return "不知道";
        };
        break;
    case commands::玩什么:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (games.available)
            {
                int idx = randInt(0, games.games.size());
                std::stringstream ss;
                ss << CQ_At(qq) << "，你阔以选择 " <<
                    games.games[idx].name << std::endl <<
                    "https://store.steampowered.com/app/" << games.games[idx].appid;
                return ss.str();
            }
            else
            {
                return "Steam游戏列表不可用";
            }
        };
        break;
    case commands::吃什么十连:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "無";
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "，你阔以选择：\n";
            for (size_t i = 0; i < 10; ++i)
                ss << " - " << getFood().to_string(group) << "\n";
            ss << "吃东西还十连，祝您撑死，哈";
            return ss.str();
        };
        break;
    case commands::加菜:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (args.size() == 1) return "空气不能吃的";

            std::string r(raw);
            r = strip(r.substr(5));    // 加菜：BC D3 / B2 CB / 20
            if (raw.empty()) return "空气不能吃的";
            if (r == "空气") return "空气不能吃的";
            if (r.length() > 30) return "你就是飞天意面神教人？";

            //TODO filter

            // check repeat
            for (auto& f : foodList)
            {
                if (f.name == r)
                    return f.name + "已经有了！！！";
            }

            food f;
            f.name = r;
            f.offerer.qq = qq;
            f.offererType = f.QQ;
            if (addFood(f))
            {
                return "不准加";
            }
            std::stringstream ss;
            ss << "已添加" << f.to_string(group);
            return ss.str();
        };
        break;
    case commands::菜单:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "无";
            // defuault: last 9 entries
            size_t range_min = (foodList.size() <= 9) ? 0 : (foodList.size() - 9);
            size_t range_max = (foodList.size() <= 9) ? (foodList.size() - 1) : (range_min + 8);

            // arg[1] is range_mid
            if (args.size() > 1 && foodList.size() > 9) try
            {
                int tmp = std::stoi(args[1]) - 1 - 4;
                if (tmp < 0)
                {
                    range_min = 0;
                    range_max = 8;
                }
                else
                {
                    range_min = tmp;
                    range_max = range_min + 8;
                }

                if (range_max >= foodList.size()) {
                    range_max = foodList.size() - 1;
                    range_min = range_max - 8;
                }

            }
            catch (std::invalid_argument) {}

            if (range_min > range_max) return "";
            std::stringstream ret;
            for (size_t i = range_min; i <= range_max; ++i)
            {
                ret << i + 1 << ": " << foodList[i].to_string(group);
                if (i != range_max) ret << '\n';
            }

            return ret.str();
        };
        break;
    case commands::删库:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (qq != qqid_admin)
                return "不准删";

            if (db.exec(
                "DELETE FROM food \
            ") != SQLITE_OK)
            {
                CQ_addLog(ac, CQLOG_ERROR, "eat", db.errmsg());
                return db.errmsg();
            }
            foodList.clear();
            return "drop了";
        };
        break;
    default: break;
    }

    return c;
}
void foodCreateTable()
{
    if (db.exec(
        "CREATE TABLE IF NOT EXISTS food( \
            id    INTEGER PRIMARY KEY AUTOINCREMENT, \
            name  TEXT            NOT NULL,      \
            adder TEXT,                          \
            qq    INTEGER                        \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "eat", db.errmsg());
}

void foodLoadListFromDb()
{
    auto list = db.query("SELECT * FROM food", 4);
    for (auto& row : list)
    {
        food f;
        f.name = utf82gbk(std::any_cast<std::string>(row[1]));
        if (row[2].has_value())
        {
            f.offererType = f.NAME;
            f.offerer.name = utf82gbk(std::any_cast<std::string>(row[2]));
        }
        else if (row[3].has_value())
        {
            f.offererType = f.QQ;
            f.offerer.qq = std::any_cast<int64_t>(row[3]);
        }
        else
            f.offererType = f.ANONYMOUS;
        foodList.push_back(f);
    }
    char msg[128];
    sprintf(msg, "added %u foods", foodList.size());
    CQ_addLog(ac, CQLOG_DEBUG, "eat", msg);
}

}
