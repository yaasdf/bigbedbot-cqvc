#pragma once
#include <string>
#include <array>
#include <vector>

namespace event_case
{

struct case_detail
{
    unsigned type;
    unsigned level;
    std::string name;
    int worth = 0;
};


class case_pool
{
private:
    std::vector<std::string> types;
    std::vector<std::string> levels;
    std::vector<std::vector<case_detail>> cases;

public:
    case_pool(const std::string& path);

    size_t getLevelCount() const { return levels.size(); }
    size_t getTypeCount() const { return types.size(); }
    std::string getLevel(size_t n) const { return levels[n]; }
    std::string getType(size_t n) const { return types[n]; }

    case_detail draw(int type);
    std::string caseFullName(const case_detail& c) const;
};

}