#include "help.h"
#include <sstream>
#include <algorithm>
namespace help
{
    const std::vector<std::string> changelog{
        {R"(20190916
Fix: 特殊开箱（测试，3体力+表格批一次）
Mod: 事件[电子烟] 概率->0.5%
Mod: 事件[流感病毒] 概率->0.1%
)"},
        {R"(20190909
Del: 事件[循环药水]（其实上一次就删了）
Add: 特殊开箱（测试，3体力+表格批一次）
)"},
        {R"(20190902
Fix: 气泵没用
Mod: 融合药水排除0
Add: 事件[世界线震动预警]
)"},
        {R"(20190830
Add: changelog
Add: 事件[电子烟]
Add: 事件[流感病毒]
Mod: 气泵次数为5
Mod: 真空抽气机会覆盖气泵效果
Fix: 未注册的人可以摇号
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
