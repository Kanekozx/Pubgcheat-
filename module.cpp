#include "module.hpp"
#include <fstream>
#include <string>

std::optional<Module> find_module(pid_t pid, const std::string& soname) {
    std::ifstream mf("/proc/" + std::to_string(pid) + "/maps");
    if (!mf) return std::nullopt;

    ptr lo = UINTPTR_MAX, hi = 0;
    std::string line, path;
    while (std::getline(mf, line)) {
        auto slash = line.find('/');
        if (slash == std::string::npos) continue;
        path = line.substr(slash);
        if (path.find(soname) == std::string::npos) continue;
        auto dash = line.find('-');
        auto sp   = line.find(' ');
        if (dash == std::string::npos || sp == std::string::npos) continue;
        ptr a = std::stoull(line.substr(0, dash), nullptr, 16);
        ptr b = std::stoull(line.substr(dash + 1, sp - dash - 1), nullptr, 16);
        if (a < lo) lo = a;
        if (b > hi) hi = b;
    }
    if (lo == UINTPTR_MAX || hi <= lo) return std::nullopt;
    return Module{lo, hi - lo, path};
}