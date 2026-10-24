#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#define LOG_TAG "pc"
#define LOG(...) printf(__VA_ARGS__); printf("\n")
#define ERR(...) printf("ERROR: "); printf(__VA_ARGS__); printf("\n")

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using ptr = uintptr_t;
