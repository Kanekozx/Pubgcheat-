#pragma once
#include "common.hpp"
#include <sys/uio.h>

class Proc {
public:
    explicit Proc(pid_t p) : pid_(p) {}
    pid_t pid() const { return pid_; }

    bool read(ptr addr, void* out, size_t n) const {
        iovec l{out, n}, r{reinterpret_cast<void*>(addr), n};
        return process_vm_readv(pid_, &l, 1, &r, 1, 0) == (ssize_t)n;
    }
    bool write(ptr addr, const void* in, size_t n) const {
        iovec l{const_cast<void*>(in), n}, r{reinterpret_cast<void*>(addr), n};
        return process_vm_writev(pid_, &l, 1, &r, 1, 0) == (ssize_t)n;
    }

    template<class T> T rd(ptr a) const { T v{}; read(a, &v, sizeof(T)); return v; }
    template<class T> void wr(ptr a, T v) const { write(a, &v, sizeof(T)); }

private:
    pid_t pid_;
};
