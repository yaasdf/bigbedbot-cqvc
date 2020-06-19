#pragma once
#include <map>
#include <functional>

namespace gambol
{
// gambol (x)
// gamble (o)

///////////////////////////////////////////////////////////////////////////////

enum class commands : size_t {
    // basic
    ��ע,

    // flipcoin
    flipcoin,
    ��,
    ��,
    ��ʼҡ��,
    ҡ��,
};

inline std::map<std::string, commands> commands_str
{
    {"��ʼ����", commands::flipcoin},
    {"�_ʼ����", commands::flipcoin},   //���w��
    {"��", commands::��},  //���w���wһ��
    {"��", commands::��},  //���w���wһ��
    {"��ʼҡ��", commands::��ʼҡ��},
    {"�_ʼ�u̖", commands::��ʼҡ��},  //���w��
    {"ҡ��", commands::ҡ��},
    {"�u̖", commands::ҡ��},  //���w��
    {"ҡ", commands::ҡ��},
    {"�u", commands::ҡ��}  //���w��
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
    int64_t total = 0, front = 0, back = 0;
    std::map<int64_t, bet> pee_per_player;
    time_t startTime = 0;
};

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
    "��",
    "��",
    "��",
    "˫",
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
    {"��", Cblack},
    {"��", Cred},
    {"��ɫ", Cblack},
    {"��ɫ", Cred},
    {"��", Aodd},
    {"˫", Aeven},
    {"����", Aodd},
    {"˫��", Aeven},
    {"��", Aodd},
    {"ż", Aeven},
    {"����", Aodd},
    {"ż��", Aeven},
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
    int64_t total = 0, amount[GRID_COUNT]{ 0 };
    std::map<int64_t, bet> pee_per_player;
    time_t startTime = 0;
};

void roundStart(int64_t group);
void roundAnnounce(int64_t group);
void roundEnd(int64_t group);
void roundCancel(int64_t group);

void put(int64_t group, int64_t qq, grid g, int64_t amount);
}

struct gameData
{
	bool flipcoin_running;
	flipcoin::game flipcoin;

	bool roulette_running;
	roulette::game roulette;
};
extern std::map<int64_t, gameData> groupMap;

}
