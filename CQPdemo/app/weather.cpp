#include <vector>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "weather.h"
#include "../utils.h"
#include "../cqp.h"
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
        args[3] << "��C (" <<  args[4]<< "��C~" << args[5] << "��C), " <<
        args[6] << "%";
    return ss.str();
}

command weather::msgDispatcher(const char* msg)
{
    auto args = msg2args(msg, 2);
    if (args.size() < 1)
        return command();
    if (args.size() == 1 && args[0].length() > 4 && args[0].substr(args[0].length() - 4) == "����")
    {
        decltype(args) newargs(2);
        size_t len = args[0].length();
        newargs[1] = args[0].substr(0, len - 4);
        newargs[0] = args[0].substr(len - 4);
        args = newargs;
    }

    if ((args[0] != "����" && args[0] != "���") && args[0] != "weather" || args.size() <= 1)
        return command();

    if (inQuery)
    {
        command c;
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "API is busy";
        };
        return c;
    }

    if ((args[0] == "����" || args[0] == "���") && utf82gbk(args[1]) != args[1])
        return weather_cn(args[1]);
    else
        return weather_global(args[1]);
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
                return "�������ӳ�ʱ";
            };
            break;
        default:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                using namespace std::string_literals;
                return "��������ʧ��("s + std::to_string(ret) + ")";
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
            c.c = commands::ȫ������;
            c.func = [](::int64_t, ::int64_t, std::vector<std::string>& args, const char*)
            {
                std::stringstream ss;
                ss <<
                    args[1] << ", " << args[0] << ": " <<
                    args[2] << ", " <<
                    args[3] << "��C (" << args[4] << "��C~" << args[5] << "��C), " <<
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
            return "����ʧ�ܣ�" + args[0];
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
            return "δ�ҵ��ó���";
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
                return "�������ӳ�ʱ";
            };
            break;
        default:
            c.func = [ret](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
            {
                using namespace std::string_literals;
                return "��������ʧ��("s + std::to_string(ret) + ")";
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
            sprintf(buf, "%.1f", dbuf);
            c.args.push_back(buf);

            dbuf = json["data"]["pm10"];
            sprintf(buf, "%.1f", dbuf);
            c.args.push_back(buf);

            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["type"]));
            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["low"]));
            c.args.push_back(utf82gbk(json["data"]["forecast"][0]["high"]));

            ibuf = json["data"]["forecast"][0]["aqi"];
            sprintf(buf, "%d", ibuf);
            c.args.push_back(buf);

            c.c = commands::��������;
        }
        else throw std::exception();
    }
    catch (...)
    {
        c.args.clear();
        CQ_addLog(ac, CQLOG_WARNING, "weather", url.c_str());
        c.func = [](::int64_t, ::int64_t, std::vector<std::string>&, const char*)
        {
            return "��������ʧ��";
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
            "�¶ȣ�" << args[2] << "�棨" << args[7] << "��" << args[8] << "�� " << std::endl <<
            "ʪ�ȣ�" << args[3] << std::endl <<
            "PM2.5: " << args[4] << std::endl << 
            "PM10: " << args[5] << std::endl << 
            "AQI: " << args[9];

        int aqi = atoi(args[9].c_str());
        if (0 <= aqi && aqi <= 50)
            ss << " һ�����ţ�";
        else if (51 <= aqi && aqi <= 100)
            ss << " ����������";
        else if (101 <= aqi && aqi <= 150)
            ss << " �����������Ⱦ��";
        else if (151 <= aqi && aqi <= 200)
            ss << " �ļ����ж���Ⱦ��";
        else if (201 <= aqi && aqi <= 300)
            ss << " �弶���ض���Ⱦ��";
        else if (301 <= aqi)
            ss << " ������������Ⱦ��";
        else
            ss << " undefined������";

        return ss.str();
    };

    return c;
}