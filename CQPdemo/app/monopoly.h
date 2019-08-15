#pragma once
#include "pee.h"

using pee::plist;
using pee::modifyCurrency;
using pee::modifyKeyCount;
using pee::updateStamina;
using pee::nosmoking;

namespace mnp
{

enum class commands : size_t {
    ³é¿¨,
};

inline std::map<std::string, commands> commands_str{
    {"³é¿¨", commands::³é¿¨},
};

typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
//typedef std::string(*callback)(::int64_t, ::int64_t, std::vector<std::string>);
struct command
{
    commands c = (commands)0;
    std::vector<std::string> args;
    callback func = nullptr;
};
command msgDispatcher(const char* msg);

////////////////////////////////////////////////////////////////////////////////
typedef std::function<std::string(int64_t grp, int64_t qq)> evt_callback;
// ÃüÔË¿¨
class event_type
{
private:
    double _prob;
    evt_callback _func;
public:
    event_type(double p, evt_callback f) : _prob(p), _func(f) {}
    double prob() const { return _prob; }
    evt_callback func() const { return _func; }
    bool operator== (const event_type& rhs)
    {
        auto p1 = _func.target<std::string(*)(int64_t, int64_t)>();
        auto p2 = rhs._func.target<std::string(*)(int64_t, int64_t)>();
        if (p1 == nullptr && p2 == nullptr) return true;
        if (p1 != nullptr && p2 != nullptr) return *p1 == *p2;
        return false;
    }
};
const event_type& draw_event(double p);



}