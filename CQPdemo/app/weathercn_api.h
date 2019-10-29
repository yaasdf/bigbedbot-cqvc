#pragma once
#include <string>
#include <map>
#include "dbconn.h"
namespace weathercn
{
inline SQLite db("city.db", "weather");

inline std::string getReqUrl(const std::string& name)
{
    auto ret = db.query("select id from cityid where name=?", 1, { name });
    if (ret.empty()) return "";
    std::string id = std::any_cast<std::string>(ret[0][0]);

    char buf[128];
    sprintf_s(buf, "http://t.weather.sojson.com/api/weather/city/%s",
        id.c_str());
    return buf;
}

}