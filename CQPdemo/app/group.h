#pragma once
#include <string>
#include <map>
#include <unordered_set>
#include "duel.h"
#include "appmain.h"

namespace grp
{

class Group
{
public:
    int64_t group_id;
    bool disable_eat = false;
    bool disable_flipcoin = false;
    bool disable_roulette = false;
    bool disable_monopoly = false;

    std::map<int64_t, GroupMemberInfo> members;

    bool flipcoin_running;
    duel::flipcoin::game flipcoin_game;

    bool roulette_running;
    duel::roulette::game roulette_game;

    time_t daily_refresh_time = 0;
    int daily_pool = 0;

public:
    void updateMembers();
    bool haveMember(int64_t qq);
    int64_t getMember(const char* name);
    void sendMsg(const char* msg);
    Group() = default;
    Group(int64_t id) : group_id(id) { updateMembers(); }
};

extern std::map<int64_t, Group> groups;

}