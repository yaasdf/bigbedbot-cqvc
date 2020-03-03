#pragma once
#include <string>
#include <map>
#include <unordered_set>
#include "dbconn.h"
#include "duel.h"
#include "appmain.h"

namespace grp
{
inline SQLite db("group.db", "group");

class Group
{
public:
    int64_t group_id;
	int64_t flags;
    std::map<int64_t, GroupMemberInfo> members;

	static const int64_t MASK_EAT = 1 << 0;
	static const int64_t MASK_FLIPCOIN = 1 << 1;
	static const int64_t MASK_ROULETTE = 1 << 2;
	static const int64_t MASK_MONOPOLY = 1 << 3;
	static const int64_t MASK_SMOKE = 1 << 4;
	void setFlag(int64_t mask, bool set = true);
	bool getFlag(int64_t mask);

	bool flipcoin_running = false;
	duel::flipcoin::game flipcoin_game;

	bool roulette_running = false;
	duel::roulette::game roulette_game;

public:
    void updateMembers();
    bool haveMember(int64_t qq);
    int64_t getMember(const char* name);
    void sendMsg(const char* msg);
    Group() = default;
    Group(int64_t id) : group_id(id) {}
};

inline std::map<int64_t, Group> groups;
void CreateTable();
void LoadListFromDb();

enum class commands : size_t {
	º§ªÓ,
	…Ë÷√
};
typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
struct command
{
	commands c = (commands)0;
	std::vector<std::string> args;
	callback func = nullptr;
};

int newGroup(int64_t id);
command msgDispatcher(const char* msg);

}

void broadcastMsg(const char* msg, int64_t flags = -1);