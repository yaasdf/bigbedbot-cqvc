#include <vector>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "weather.h"
#include "cqp.h"

#include "utils/string_util.h"
#include "utils/encoding.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2/pcre2.h>

using namespace weather;

bool inQuery = false;

struct curl_buffer
{
    int length = 0;
    char content[CURL_MAX_WRITE_SIZE] = "";
};

size_t perform_write(void* buffer, size_t size, size_t count, void* stream)
{
    curl_buffer* buf = (curl_buffer*)stream;
    int newsize = size * count;
    memcpy(buf->content + buf->length, buffer, newsize);
    buf->length += newsize;
    return newsize;
}

std::string success(::int64_t, ::int64_t, std::vector<std::string>& args, const char*)
{
    std::stringstream ss;
    ss <<
        args[1] << "," << args[0] << ": " <<
        args[2] << ", " <<
        args[3] << "°C (" <<  args[4]<< "°C~" << args[5] << "°C), " <<
        args[6] << "%";
    return ss.str();
}

command weather::msgDispatcher(const char* msg)
{
    command c;
    //std::vector<std::string> args;
    std::string city;
    for (const auto& [regstr, cmd] : commands_regex)
    {
        int errcode;
        size_t erroffset;
        unsigned char* tableptr;
        auto* pcre2 = pcre2_compile((PCRE2_SPTR8)regstr.c_str(), PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &errcode, &erroffset, NULL);
        pcre2_match_data* mdata = pcre2_match_data_create_from_pattern(pcre2, NULL);
        int m = pcre2_match(pcre2, (PCRE2_SPTR8)msg, PCRE2_ZERO_TERMINATED, 0, 0, mdata, NULL);
        if (m > 0)
        {
            /*
            size_t plen = 0;
            pcre2_pattern_info(pcre2, PCRE2_INFO_CAPTURECOUNT, &plen);
            for (size_t i = 1; i < plen; ++i)
            {
                PCRE2_UCHAR* str;
                PCRE2_SIZE strlen;
                if (pcre2_substring_get_bynumber(mdata, i, &str, &strlen) == 0)
                {
                    CQ_addLog(ac, CQLOG_DEBUG, "weather", (char*)str);
                    args.push_back((char*)str);
                    pcre2_substring_free(str);
                }
                else
                    args.push_back("");
            }
            */
            PCRE2_UCHAR* str = NULL;
            PCRE2_SIZE strlen;
            if (pcre2_substring_get_byname(mdata, (PCRE2_SPTR8)"city", &str, &strlen) == 0 && str)
            {
                city = std::string((char*)str);
                switch (cmd)
                {
                case commands::全球天气:
                    c = weather_global(city);
                    break;
                case commands::国内天气:
                    c = weather_cn(city);
                    break;
                default:
                    break;
                }
                pcre2_substring_free(str);
            }
        }
        pcre2_match_data_free(mdata);
    }
    
    auto args = msg2args(msg, 2);
    if (args.size() < 1)
        return command();

    auto query = msg2args(msg);
    if (query.empty()) return command();

    if (inQuery)
    {
        command c;
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "API is busy";
        };
        return c;
    }

    if ((args[0] == "天气" || args[0] == "天氣") && utf82gbk(args[1]) != args[1])
        return weather_cn(args[1]);
    else if (args.size() >= 2)
        return weather_global(args[1]);

    return command();
}

command weather::weather_global(const std::string& city)
{
    command c;
    CURL* curl = NULL;

    inQuery = true;
    std::string name = city;
    curl = curl_easy_init();
    if (!curl)
    {
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "curl init failed";
        };
        inQuery = false;
        return c;
    }

    //auto url = seniverse::getReqUrl(name);
    auto url = openweather::getReqUrl(name);

    curl_buffer curlbuf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, perform_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlbuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    memset(curlbuf.content, 0, CURL_MAX_WRITE_SIZE);
    if (int ret; CURLE_OK != (ret = curl_easy_perform(curl)))
    {
        switch (ret)
        {
        case CURLE_OPERATION_TIMEDOUT:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                return "网络连接超时";
            };
            break;
        default:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                using namespace std::string_literals;
                return "网络连接失败("s + std::to_string(ret) + ")";
            };
            break;
        }
        inQuery = false;
        curl_easy_cleanup(curl);
        return c;
    }

    curl_easy_cleanup(curl);

    inQuery = false;

    nlohmann::json json = nlohmann::json::parse(curlbuf.content);
    try
    {
        if (json.contains("cod") && json["cod"] == 200 && json.contains("coord"))
        {
            c.args.push_back(json["sys"]["country"]);
            c.args.push_back(json["name"]);
            c.args.push_back(json["weather"][0]["main"]);
            c.args.push_back(std::to_string(int(std::round(json["main"]["temp"] - 273.15))));
            c.args.push_back(std::to_string(int(std::round(json["main"]["temp_min"] - 273.15))));
            c.args.push_back(std::to_string(int(std::round(json["main"]["temp_max"] - 273.15))));
            c.args.push_back(std::to_string(int(std::round(json["main"]["humidity"] + 0))));
            c.c = commands::全球天气;
            c.func = [](::int64_t, ::int64_t, std::vector<std::string>& args, const char*)
            {
                std::stringstream ss;
                ss <<
                    args[1] << ", " << args[0] << ": " <<
                    args[2] << ", " <<
                    args[3] << "°C (" << args[4] << "°C~" << args[5] << "°C), " <<
                    args[6] << "%";
                return ss.str();
            };
        }
        else throw std::exception();
    }
    catch (...)
    {
        c.args.clear();
        if (json.contains("cod") && json.contains("message"))
            c.args.push_back(json["message"]);
        else
            c.args.push_back("unknown");
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>& args, const char*)
        {
            return "请求失败：" + args[0];
        };
    }

    return c;
}

command weather::weather_cn(const std::string& city)
{
    command c;
    CURL* curl = NULL;

    std::string name = gbk2utf8(city);
    auto url = weathercn::getReqUrl(name);
    if (url.empty())
    {
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "未找到该城市";
        };
        return c;
    }

    inQuery = true;
    curl = curl_easy_init();
    if (!curl)
    {
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "curl init failed";
        };
        inQuery = false;
        return c;
    }

    curl_buffer curlbuf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, perform_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlbuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    memset(curlbuf.content, 0, CURL_MAX_WRITE_SIZE);
    if (int ret; CURLE_OK != (ret = curl_easy_perform(curl)))
    {
        switch (ret)
        {
        case CURLE_OPERATION_TIMEDOUT:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                return "网络连接超时";
            };
            break;
        default:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                using namespace std::string_literals;
                return "网络连接失败("s + std::to_string(ret) + ")";
            };
            break;
        }
        inQuery = false;
        curl_easy_cleanup(curl);
        return c;
    }

    nlohmann::json json = nlohmann::json::parse(curlbuf.content);
    try
    {
        if (json.contains("cityInfo"))
        {
            int ibuf;
            double dbuf;
            char buf[16];
            c.args.push_back(utf82gbk(json["cityInfo"]["parent"]));
            c.args.push_back(utf82gbk(json["cityInfo"]["city"]));
            c.args.push_back(json["data"]["wendu"]);
            c.args.push_back(json["data"]["shidu"]);

            dbuf = json["data"]["pm25"];
            sprintf_s(buf, "%.1f", dbuf);
            c.args.push_back(buf);

            dbuf = json["data"]["pm10"];
            sprintf_s(buf, "%.1f", dbuf);
            c.args.push_back(buf);

            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["type"]));
            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["low"]));
            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["high"]));

            ibuf = json["data"]["forecast"][0]["aqi"];
            sprintf_s(buf, "%d", ibuf);
            c.args.push_back(buf);

            c.c = commands::国内天气;
        }
        else throw std::exception();
    }
    catch (...)
    {
        c.args.clear();
        CQ_addLog(ac, CQLOG_WARNING, "weather", url.c_str());
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "天气解析失败";
        };
        inQuery = false;
        return c;
    }

    curl_easy_cleanup(curl);
    inQuery = false;

    c.func = [](::int64_t, ::int64_t, std::vector<std::string>& args, const char*)
    {
        std::stringstream ss;
        ss << args[0] << " " << args[1] << " " << args[6] << std::endl <<
            "温度：" << args[2] << "℃（" << args[7] << "，" << args[8] << "） " << std::endl <<
            "湿度：" << args[3] << std::endl <<
            "PM2.5: " << args[4] << std::endl << 
            "PM10: " << args[5] << std::endl << 
            "AQI: " << args[9];

        int aqi = atoi(args[9].c_str());
        if (0 <= aqi && aqi <= 50)
            ss << " 一级（优）";
        else if (51 <= aqi && aqi <= 100)
            ss << " 二级（良）";
        else if (101 <= aqi && aqi <= 150)
            ss << " 三级（轻度污染）";
        else if (151 <= aqi && aqi <= 200)
            ss << " 四级（中度污染）";
        else if (201 <= aqi && aqi <= 300)
            ss << " 五级（重度污染）";
        else if (301 <= aqi)
            ss << " 六级（严重污染）";
        else
            ss << " undefined（？）";

        return ss.str();
    };

    return c;
}