#ifdef PTRACE_MODE
#include <xhook.h>
#include <dlfcn.h>
#include <unistd.h>

// --- stub zygisk::Api поверх xhook ---
class PtraceApi : public zygisk::Api {
public:
    void pltHookRegister(dev_t dev, ino_t inode, const char* symbol,
                         void* fn, void** backup) override {
        // xhook требует ИМЯ библиотеки, а не dev/ino
        // Найдём имя по dev/ino из /proc/self/maps
        char libname[256] = {0};
        if (!find_lib_by_devino(dev, inode, libname, sizeof(libname))) {
            LOG("cannot resolve dev=%lx inode=%lx", dev, inode);
            return;
        }
        xhook_register(libname, symbol, fn, backup);
    }
    void pltHookCommit() override {
        xhook_refresh(0);
    }
    void setOption(zygisk::Option) override { /* no-op */ }
    void pltHookExclude(dev_t, ino_t, const char*) override {}
    int connectCompanion() override { return -1; }
    int getCompanionFd() override { return -1; }
};

__attribute__((constructor))
static void ptrace_entry() {
    LOG("[pubgcheat] ptrace mode: waiting for libUE4.so");
    // ждём, пока игра загрузит libUE4
    while (!is_lib_loaded("libUE4.so")) sleep(1);
    LOG("[pubgcheat] libUE4 ready, init");

    xhook_enable_sigsegv_protection(0);
    auto* api = new PtraceApi();
    auto* mod = new PubgModule();
    mod->onLoad(api, nullptr);
    mod->postAppSpecialize(nullptr);
    xhook_refresh(0);
    LOG("[pubgcheat] ptrace mode: initialized");
}
#endif // PTRACE_MODE
