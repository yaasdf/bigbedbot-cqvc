#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <iostream>
#include "dbconn.h"
namespace eat {

inline SQLite db("eat.db", "eat");

enum class commands: size_t {
    吃什么,
    喝什么,
    玩什么,
    吃什么十连,
    加菜,
    菜单,
    删库,
};

inline std::map<std::string, commands> commands_str
{
    {"吃什么", commands::吃什么},
    {"喝什么", commands::喝什么},
    {"玩什么", commands::玩什么},
    {"吃什么十连", commands::吃什么十连},
    {"加菜", commands::加菜},
    {"菜单", commands::菜单},
    //{"drop", commands::删库},
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

struct food
{
    std::string name;
    enum {ANONYMOUS, NAME, QQ} offererType;
    struct { std::string name; int64_t qq; } offerer;
    std::string to_string(int64_t group = 0);
};
inline std::vector<food> foodList;
void foodCreateTable();
void foodLoadListFromDb();

struct drink
{
    std::string name;
    int price_d2;
};
inline std::vector<drink> drinkList;
void drinkCreateTable();
void drinkLoadListFromDb();
    
}
