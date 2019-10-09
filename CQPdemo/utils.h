#pragma once

////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>
std::vector<std::string> msg2args(const char* msg);
std::string gbk2utf8(std::string gbk);
std::string utf82gbk(std::string utf8);


////////////////////////////////////////////////////////////////////////////////
#include <random>
inline std::random_device random_rd;  // 将用于获得随机数引擎的种子
inline std::mt19937 random_gen(random_rd()); // 以 rd() 播种的标准 mersenne_twister_engine

inline int randInt(int min, int max)   // [min, max]
{
    std::uniform_int_distribution<> dis(min, max);
    return dis(random_gen);
}

inline double randReal(double min = 0.0, double max = 1.0) // [min, max)
{
    std::uniform_real_distribution<> dis(min, max);
    return dis(random_gen);
}

////////////////////////////////////////////////////////////////////////////////

std::string strip(std::string& s);
int64_t stripAt(std::string& s);
std::string stripImage(std::string& s);
std::string stripFace(std::string& s);

////////////////////////////////////////////////////////////////////////////////

class simple_str
{
private:
    int16_t _len = 0;
    char* _data = NULL;

public:
    int16_t length() const { return _len; }
    const char* c_str() const { return _data; }
    std::string operator()()
    {
        return std::string(_data);
    }

    simple_str() {}
    simple_str(int16_t len, const char* str) : _len(len)
    {
        _data = new char[_len];
        memcpy_s(_data, _len, str, _len);
        _data[_len - 1] = 0;
    }
    simple_str(const char* str) : simple_str(int16_t(strlen(str)), str) {}
    simple_str(const std::string& str) : simple_str(int16_t(str.length()), str.c_str()) {}
    ~simple_str() { if (_data) delete _data; }
    simple_str(const simple_str& str) : simple_str(str.length(), str.c_str()) {}
    simple_str& operator=(const simple_str& str)
    {
        if (_data) delete _data;
        _len = str.length();
        _data = new char[_len];
        memcpy_s(_data, _len, str.c_str(), _len);
        _data[_len - 1] = 0;
        return *this;
    }
    operator std::string() const { return std::string(_data); }
};

////////////////////////////////////////////////////////////////////////////////
// ntohs, ntohl, ntohll
#include <WinSock2.h>
////////////////////////////////////////////////////////////////////////////////

#include <ctime>
inline const int TIMEZONE_HR = 8;
inline const int TIMEZONE_MIN = 0;
inline std::tm getLocalTime(int timezone_hr, int timezone_min, time_t offset = 0)
{
    auto t = time(nullptr) + offset;
    t += timezone_hr * 60 * 60 + timezone_min * 60;

    std::tm tm;
    gmtime_s(&tm, &t);
    return tm;
}

////////////////////////////////////////////////////////////////////////////////

inline const std::string EMOJI_HORSE = "[CQ:emoji,id=128052]";
inline const std::string EMOJI_HAMMER = "[CQ:emoji,id=128296]";
inline const std::string EMOJI_DOWN = "[CQ:emoji,id=11015]";
inline const std::string EMOJI_NONE = "[CQ:emoji,id=127514]";
inline const std::string EMOJI_HORN = "[CQ:emoji,id=128227]";