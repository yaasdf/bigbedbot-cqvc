#pragma once
#include "pee.h"
#include <map>

namespace duel
{

using pee::db;

using pee::modifyCurrency;
using pee::modifyBoxCount;
using pee::modifyDrawTime;

///////////////////////////////////////////////////////////////////////////////

enum class commands : size_t {
    // basic
    下注,

    // flipcoin
    flipcoin,
    正,
    反,
    roulette,
    摇号,
};

inline std::map<std::string, commands> commands_str
{
    {"开始翻批", commands::flipcoin},
    {"正", commands::正},
    {"反", commands::反},
    {"开始摇号", commands::roulette},
    {"摇号", commands::摇号},
    {"摇", commands::摇号}
};

typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
struct command
{
    commands c = (commands)0;
    std::vector<std::string> args;
    callback func = nullptr;
};

command msgDispatcher(const char* msg);

///////////////////////////////////////////////////////////////////////////////
// flipcoin
namespace flipcoin
{

struct bet
{
    int64_t front;
    int64_t back;
};
struct game
{
    int64_t total, front, back;
    std::map<int64_t, bet> pee_per_player;
    time_t startTime;
};
inline std::map<int64_t, game> groupStat;   // only 1 game for each group

void roundStart(int64_t group);
void roundAnnounce(int64_t group);
void roundEnd(int64_t group);
void roundCancel(int64_t group);

void put(int64_t group, int64_t qq, bet s);

}

///////////////////////////////////////////////////////////////////////////////
// roulette

namespace roulette
{

enum grid
{
    N0,
    N1,
    N2,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,
    N10,
    N11,
    N12,
    N13,
    N14,
    N15,
    N16,
    N17,
    N18,
    N19,
    N20,
    N21,
    N22,
    N23,
    N24,
    N25,
    N26,
    N27,
    N28,
    N29,
    N30,
    N31,
    N32,
    N33,
    N34,
    N35,
    N36,
    Cblack,
    Cred,
    Aodd,
    Aeven,
    P1st,
    P2nd,
    P3rd,
    GRID_COUNT
};

inline constexpr char* gridName[] = 
{
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28",
    "29",
    "30",
    "31",
    "32",
    "33",
    "34",
    "35",
    "36",
    "黑",
    "红",
    "单",
    "双",
    "1st",
    "2nd",
    "3rd"
};

inline const std::map<std::string, grid> gridTokens
{
    {"0", N0},
    {"1", N1},
    {"2", N2},
    {"3", N3},
    {"4", N4},
    {"5", N5},
    {"6", N6},
    {"7", N7},
    {"8", N8},
    {"9", N9},
    {"10", N10},
    {"11", N11},
    {"12", N12},
    {"13", N13},
    {"14", N14},
    {"15", N15},
    {"16", N16},
    {"17", N17},
    {"18", N18},
    {"19", N19},
    {"20", N20},
    {"21", N21},
    {"22", N22},
    {"23", N23},
    {"24", N24},
    {"25", N25},
    {"26", N26},
    {"27", N27},
    {"28", N28},
    {"29", N29},
    {"30", N30},
    {"31", N31},
    {"32", N32},
    {"33", N33},
    {"34", N34},
    {"35", N35},
    {"36", N36},
    {"黑", Cblack},
    {"红", Cred},
    {"黑色", Cblack},
    {"红色", Cred},
    {"单", Aodd},
    {"双", Aeven},
    {"单数", Aodd},
    {"双数", Aeven},
    {"奇", Aodd},
    {"偶", Aeven},
    {"奇数", Aodd},
    {"偶数", Aeven},
    {"1st", P1st},
    {"2nd", P2nd},
    {"3rd", P3rd},
    {"1ST", P1st},
    {"2ND", P2nd},
    {"3RD", P3rd},
};

struct bet
{
    int64_t amount[GRID_COUNT];
};
struct game
{
    int64_t total, amount[GRID_COUNT];
    std::map<int64_t, bet> pee_per_player;
    time_t startTime;
};
inline std::map<int64_t, game> groupStat;   // only 1 game for each group

void roundStart(int64_t group);
void roundAnnounce(int64_t group);
void roundEnd(int64_t group);
void roundCancel(int64_t group);

void put(int64_t group, int64_t qq, grid g, int64_t amount);
}
}
