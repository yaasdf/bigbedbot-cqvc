#pragma once
#include <string>
#include <vector>
namespace help {
    extern const std::vector<std::string> changelog;

    std::string boot_info();

    std::string help(unsigned count = 1);
}