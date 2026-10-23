#pragma once
#include "common.hpp"
#include <string>
#include <optional>

struct Module { ptr base; size_t size; std::string path; };
std::optional<Module> find_module(pid_t pid, const std::string& soname);
