#pragma once
#include <string>
#include <array>
#include <vector>
#include <functional>
#include <map>

namespace event_case
{

struct case_detail
{
    int type;
    int level;
    std::string name;
    int worth = 0;
};

class case_pool
{
public:
    typedef std::vector<std::pair<std::string, int>> types_t;
    typedef std::vector<std::pair<std::string, double>> levels_t;
    typedef std::vector<std::vector<std::pair<double, std::vector<case_detail>>>> cases_t;   // cases[type][level][n]

private:
    types_t types;
    levels_t levels;
    cases_t cases;

public:
    case_pool(const types_t&, const levels_t&, const std::vector<case_detail>&);

    size_t getLevelCount() const { return levels.size(); }
    size_t getTypeCount() const { return types.size(); }
    std::string getLevel(size_t n) const { return levels[n].first; }
    std::string getType(size_t n) const { return types[n].first; }
    int getTypeCost(size_t n) const { return types[n].second; }

    case_detail draw(int type);
    std::string casePartName(const case_detail& c) const;
    std::string caseFullName(const case_detail& c) const;
};

enum class commands : size_t {
    开箱
};
typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
//typedef std::string(*callback)(::int64_t, ::int64_t, std::vector<std::string>);
struct command
{
    commands c = (commands)0;
    std::vector<std::string> args;
    callback func = nullptr;
};

inline std::map<std::string, commands> commands_str
{
    {"活动开箱", commands::开箱},
    {"活動開箱", commands::开箱}  //繁體化
};
command msgDispatcher(const char* msg);

inline int type = -1;
void startEvent();
void stopEvent();

namespace evt
{
extern const case_pool::types_t types;
extern const case_pool::levels_t levels;
extern const std::vector<case_detail> cases;
}
namespace drop
{
extern const case_pool::types_t types;
extern const case_pool::levels_t levels;
extern const std::vector<case_detail> cases;
}

extern case_pool pool_event;
extern case_pool pool_drop;

// 活动开箱
inline std::tm event_case_tm;
inline std::tm event_case_end_tm;
const int EVENT_CASE_TIME_HOUR_START = 18;
const int EVENT_CASE_TIME_MIN_START = 0;
const int EVENT_CASE_TIME_HOUR_END = 19;
const int EVENT_CASE_TIME_MIN_END = 0;
}