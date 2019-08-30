#include "help.h"
#include <sstream>
#include <algorithm>
namespace help
{
    const std::vector<std::string> changelog{
        {R"(20190830
增加changelog
增加事件：电子烟
增加事件：流感病毒
事件调整：气泵次数为5
事件调整：真空抽气机会覆盖气泵效果
fix: 未注册的人可以摇号
)"},
    };

    std::string boot_info()
    {
        std::stringstream ss;
        ss << "bot活了！";
        ss << help(1);
        return ss.str();
    }

    std::string help(unsigned count)
    {
        std::stringstream ss;
        ss << "最后更新日期：" << __TIMESTAMP__ << std::endl;
        ss << "修改记录：" << std::endl;
        for (unsigned i = 0; i < count && i < changelog.size(); ++i)
        {
            ss << changelog[i];
        }
        std::string ret = ss.str();
        while (ret.back() == '\n' || ret.back() == 'r')
        {
            while (ret.back() == '\n') ret.pop_back();
            while (ret.back() == '\r') ret.pop_back();
        }
        return ret;
    }
}
