#include "event_case.h"
using namespace event_case;

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include "cqp.h"


case_pool::case_pool(const std::string& p)
{
    fs::path path(p);
    if (!fs::exists(path) || !fs::is_regular_file(path)) return;

    std::ifstream ifs(path);
    std::string buf;
    int loadStage = 0;
    const std::regex typeScheme{ R"(^(.*)$)" };
    const std::regex levelScheme{ R"(^(.*),(.*)$)" };
    const std::regex caseScheme{ R"(^(\d+),(\d+),(.*),(-?\d+)$)" };
    while (std::getline(ifs, buf))
    {
        switch (loadStage)
        {
        case 0:
        {
            if (buf.empty())
            {
                loadStage = 1;
                continue;
            }

            std::smatch matched;
            if (std::regex_match(buf, matched, typeScheme))
            {
                types.emplace_back(matched[1]);
            }
            else
            {
                std::stringstream ss;
                ss << "type parsing error: " << buf;
                CQ_addLog(ac, CQLOG_WARNING, "event_case", ss.str().c_str());
            }
        }
        break;
        case 1:
        {
            if (buf.empty())
            {
                loadStage = 2;
                continue;
            }

            std::smatch matched;
            if (std::regex_match(buf, matched, levelScheme))
            {
                try {
                    levels.emplace_back(matched[1], std::stod(matched[2]));
                }
                catch (std::invalid_argument&) {
                    std::stringstream ss;
                    ss << "level(prob) parsing error: " << matched[2];
                    CQ_addLog(ac, CQLOG_WARNING, "event_case", ss.str().c_str());
                }
            }
            else
            {
                std::stringstream ss;
                ss << "level parsing error: " << buf;
                CQ_addLog(ac, CQLOG_WARNING, "event_case", ss.str().c_str());
            }
        }
        break;
        case 2:
        {
            std::smatch matched;
            if (std::regex_match(buf, matched, caseScheme))
            {
                try {
                    types.emplace_back(std::stoi(matched[1]), std::stoi(matched[2]), matched[3], std::stoi(matched[4]));
                }
                catch (std::invalid_argument& e) {
                    std::stringstream ss;
                    ss << "case(d) parsing error: " << e.what();
                    CQ_addLog(ac, CQLOG_WARNING, "event_case", ss.str().c_str());
                }
            }
            else
            {
                std::stringstream ss;
                ss << "case parsing error: " << buf;
                CQ_addLog(ac, CQLOG_WARNING, "event_case", ss.str().c_str());
            }
        }
        break;
        default:break;
        };
    }
    
    ifs.close();
}


case_detail case_pool::draw(int type)
{
    if (type >= 0 && type < getTypeCount())
    {
        
    }
    else
    {
        return {-1, -1, "??", 0};
    }
}

std::string case_pool::caseFullName(const case_detail& c) const
{
    std::stringstream ss;
    if (c.type >= 0 && c.type < getTypeCount()) ss << "[" << getType(c.type) << "] ";
    if (c.level >= 0 && c.level < getLevelCount()) ss << "<" << getLevel(c.level) << "> ";
    ss << c.name;
    ss << " (" << c.worth << "Åú)";

    std::string ret = ss.str();
    return ret;
}