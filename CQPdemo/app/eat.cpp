#include <sstream>
#include "cqp.h"
#include "eat.h"
#include "appmain.h"
#include "private/qqid.h"
#include "data/group.h"
#include <Windows.h>
#include <curl/curl.h>
#include "cpp-base64/base64.h"
#include "common/steam_parser.h"
#include "utils/encoding.h"
#include "utils/string_util.h"
#include "utils/rand.h"

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
#ifdef _DEBUG
    return;
#endif

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        CQ_addLog(ac, CQLOG_WARNING, "play", "curl init error");
    }

    curl_buffer curlbuf(8 * 1024 * 1024);   // 8MB
    curl_easy_setopt(curl, CURLOPT_URL, "http://api.steampowered.com/ISteamApps/GetAppList/v0002/?format=json");
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
	{
		steam::SteamAppListParser gamestmp;
		switch (gamestmp.parse(curlbuf.content))
		{
		case 0:
			games.games.clear();
			games = std::move(gamestmp);
			char msg[128];
			sprintf_s(msg, "added %u games", games.games.size());
			CQ_addLog(ac, CQLOG_DEBUG, "play", msg);
			break;

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
	}
		break;

    default:
    {
        char msg[128];
        sprintf_s(msg, "curl error: %d", ret);
        CQ_addLog(ac, CQLOG_WARNING, "play", msg);
    }
        break;
    }

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
    //foodList.push_back(f);
    return 0;
}

int getFood(food& f)
{
	const char query[] = "SELECT * FROM food ORDER BY RANDOM() limit 1";
	auto list = db.query(query, 4);
	if (list.empty()) return 1;
	else
	{
		auto &row = list[0];

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
	}

	return 0;

    //return foodList[randInt(0, foodList.size() - 1)];
}

int getFood10(food(&f)[10])
{
	const char query[] = "SELECT * FROM food ORDER BY RANDOM() limit 10";
	auto list = db.query(query, 4);
	size_t idx = 0;
	if (list.empty()) return 0;
	else
	{
		for (auto &row : list)
		{
			f[idx].name = utf82gbk(std::any_cast<std::string>(row[1]));
			if (row[2].has_value())
			{
				f[idx].offererType = f[idx].NAME;
				f[idx].offerer.name = utf82gbk(std::any_cast<std::string>(row[2]));
			}
			else if (row[3].has_value())
			{
				f[idx].offererType = f[idx].QQ;
				f[idx].offerer.qq = std::any_cast<int64_t>(row[3]);
			}
			else
				f[idx].offererType = f[idx].ANONYMOUS;

			idx++;
		}
	}

	return idx;

	//return foodList[randInt(0, foodList.size() - 1)];
}


int delFood(const std::string& name)
{
	const char query[] = "DELETE FROM food WHERE name=?";
	int ret = db.exec(query, { gbk2utf8(name) });
	if (ret != SQLITE_OK)
	{
		std::stringstream ss;
		ss << "delFood: " << db.errmsg() << ", " << gbk2utf8(name);
		CQ_addLog(ac, CQLOG_ERROR, "eat", ss.str().c_str());
		return 1;
	}
	return 0;
}

inline int addDrink(drink& d)
{
	const char query[] = "INSERT INTO drink(name, qq, grp) VALUES (?,?,?)";
	int ret;
	auto nameutf8 = gbk2utf8(d.name);
	ret = db.exec(query, { nameutf8, d.qq, d.group });
	if (ret != SQLITE_OK)
	{
		std::stringstream ss;
		ss << "addDrink: " << db.errmsg() << ", " << d.name;
		CQ_addLog(ac, CQLOG_ERROR, "drink", ss.str().c_str());
		return 1;
	}
	//foodList.push_back(f);
	return 0;
}

inline int64_t haveFood(const std::string& name = "")
{
	if (!name.empty())
	{
		const char query[] = "SELECT COUNT(*) FROM food WHERE name=?";
		auto list = db.query(query, 1, { gbk2utf8(name) });
		return list.empty()? 0 : std::any_cast<int64_t>(list[0][0]);
	}
	else
	{
		const char query[] = "SELECT COUNT(*) FROM food";
		auto list = db.query(query, 1);
		return list.empty() ? 0 : std::any_cast<int64_t>(list[0][0]);
	}
}

int getDrink(drink& f)
{
	const char query[] = "SELECT * FROM drink ORDER BY RANDOM() limit 1";
	auto list = db.query(query, 4);
	if (list.empty()) return 1;
	else
	{
		auto &row = list[0];

		f.name = utf82gbk(std::any_cast<std::string>(row[1]));
		f.qq = std::any_cast<int64_t>(row[2]);
		f.group = std::any_cast<int64_t>(row[3]);
	}

	return 0;
}

int delDrink(const std::string& name)
{
	const char query[] = "DELETE FROM drink WHERE name=?";
	int ret = db.exec(query, { gbk2utf8(name) });
	if (ret != SQLITE_OK)
	{
		std::stringstream ss;
		ss << "delDrink: " << db.errmsg() << ", " << gbk2utf8(name);
		CQ_addLog(ac, CQLOG_ERROR, "drink", ss.str().c_str());
		return 1;
	}
	return 0;
}

inline int64_t haveDrink(const std::string& name = "")
{
	if (!name.empty())
	{
		const char query[] = "SELECT COUNT(*) FROM drink WHERE name=?";
		auto list = db.query(query, 1, { gbk2utf8(name) });
		return list.empty() ? 0 : std::any_cast<int64_t>(list[0][0]);
	}
	else
	{
		const char query[] = "SELECT COUNT(*) FROM drink";
		auto list = db.query(query, 1);
		return list.empty() ? 0 : std::any_cast<int64_t>(list[0][0]);
	}
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
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "，你阔以选择";
			food f;
			if (getFood(f)) return "無";
            ss << f.to_string(group);
            return ss.str();
        };
        break;
    case commands::喝什么:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
			std::stringstream ss;
			ss << CQ_At(qq);
			ss << "，你阔以选择";
			drink d;
			if (getDrink(d)) return "無";
			ss << d.name;
			return ss.str();
        };
        break;
    case commands::玩什么:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (games.available && !games.games.empty())
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
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "，你阔以选择：\n";
			if (!haveFood()) return "無";

			food f[10];
			int size = getFood10(f);
			for (int i = 0; i < size; ++i)
			{
				ss << " - " << f[i].to_string(group) << "\n";
			}
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
			if (haveFood(r))
                return r + "已经有了！！！";

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
	case commands::删菜:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (isGroupManager(group, qq))
			{
				if (args.size() == 1) return "空气不能删的";

				std::string r(raw);
				r = strip(r.substr(5));    // 加菜：BC D3 / B2 CB / 20
				if (raw.empty()) return "空气不能删的";
				if (r == "空气") return "空气不能删的";
				if (r.length() > 30) return "你就是飞天意面神教人？";

				int64_t count = haveFood(r);
				if (count)
				{
					delFood(r);
				}

				std::stringstream ss;
				ss << "已删除" << count << "条" << r;
				return ss.str();
			}
			return "你删个锤子？";
		};
		break;
	case commands::加饮料:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupOwner(group, qq)) return "你加个锤子？";

			if (args.size() == 1) return "空气不能喝的";

			std::string r(raw);
			r = strip(r.substr(7));    // 加菜：BC D3 / B2 CB / 20
			if (raw.empty()) return "空气不能喝的";
			if (r == "空气") return "空气不能喝的";
			if (r.length() > 30) return "你就是飞天意面神教人？";

			//TODO filter

			// check repeat
			if (haveDrink(r))
				return r + "已经有了！！！";

			drink d;
			d.name = r;
			d.qq = qq;
			d.group = group;
			if (addDrink(d))
			{
				return "不准加";
			}
			std::stringstream ss;
			ss << "已添加" << d.name;
			return ss.str();
		};
		break;
	case commands::删饮料:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupOwner(group, qq)) return "你加个锤子？";

			if (args.size() == 1) return "空气不能删的";

			std::string r(raw);
			r = strip(r.substr(5));    // 加菜：BC D3 / B2 CB / 20
			if (raw.empty()) return "空气不能删的";
			if (r == "空气") return "空气不能删的";
			if (r.length() > 30) return "你就是飞天意面神教人？";

			int64_t count = haveDrink(r);
			if (count)
			{
				delDrink(r);
			}

			std::stringstream ss;
			ss << "已删除" << count << "条" << r;
			return ss.str();
		};
		break;
    case commands::菜单:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
			int64_t count = haveFood();
            if (!count) return "无";

			return "菜单暂时不可用！";
			/*
            // defuault: last 9 entries
            size_t range_min = (count <= 9) ? 0 : (count - 9);
            size_t range_max = (count <= 9) ? (count - 1) : (range_min + 8);


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
			*/
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
            //foodList.clear();
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
            name  TEXT    NOT NULL,       \
            adder TEXT,                   \
            qq    INTEGER                 \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "eat", db.errmsg());
}
/*
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
*/


void drinkCreateTable()
{
	if (db.exec(
		"CREATE TABLE IF NOT EXISTS drink( \
            id    INTEGER PRIMARY KEY AUTOINCREMENT, \
            name  TEXT    NOT NULL,        \
            qq    INTEGER,                 \
            grp   INTEGER                  \
         )") != SQLITE_OK)
		CQ_addLog(ac, CQLOG_ERROR, "drink", db.errmsg());
}

}
