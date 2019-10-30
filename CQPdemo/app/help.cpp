#include "help.h"
#include <sstream>
#include <algorithm>
namespace help
{
    std::string boot_info()
    {
        std::stringstream ss;
        ss << "bot活了！";
        ss << help();
        return ss.str();
    }

    std::string help()
    {
        std::stringstream ss;
        ss << "最后更新日期：" << __TIMESTAMP__ << std::endl <<
            "帮助文档：https://github.com/yaasdf/bigbedbot-cqvc/blob/master/CQPdemo/README.md";
        return ss.str();
    }
}
