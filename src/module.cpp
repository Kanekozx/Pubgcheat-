#include <sys/types.h>   // ← вот это, ПЕРВОЙ строкой
#include "zygisk.hpp"
#include "memory.h"
#include "sdk.h"
#include "esp.h"
#include "menu.h"
#include "config.h"
#include "offsets.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/stat.h>
#include <string>
#include <cstring>

// ---------- хуки ----------
static EGLBoolean (*origSwap)(EGLDisplay, EGLSurface) = nullptr;
static int32_t    (*origInputGetEvent)(AInputQueue*, AInputEvent**) = nullptr;

static bool g_imguiReady = false;
static int  g_w = 0, g_h = 0;

static void applyTouch(float x, float y, int action) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);
    if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_MOVE)
        io.AddMouseButtonEvent(0, true);
    else if (action == AMOTION_EVENT_ACTION_UP)
        io.AddMouseButtonEvent(0, false);
}

static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    if (!g_imguiReady) {
        // libUE4 грузится позже, чем postAppSpecialize — ждём
        if (!SDK::init()) return origSwap(dpy, surf);

        eglQuerySurface(dpy, surf, EGL_WIDTH,  &g_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &g_h);
        if (g_w <= 0 || g_h <= 0) return origSwap(dpy, surf);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2((float)g_w, (float)g_h);
        io.FontGlobalScale = 2.f;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_imguiReady = true;
        LOG("ImGui init %dx%d", g_w, g_h);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    drawESP(g_w, g_h);
    drawMenu(g_w, g_h);

    ImGui::Render();

    GLint last_prog, last_vbo, last_vao, last_tex, last_active;
    GLint last_vp[4];
    GLint last_bs_src_rgb, last_bs_dst_rgb, last_bs_src_a, last_bs_dst_a;
    glGetIntegerv(GL_CURRENT_PROGRAM,      &last_prog);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_vbo);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,   &last_tex);
    glGetIntegerv(GL_ACTIVE_TEXTURE,       &last_active);
    glGetIntegerv(GL_VIEWPORT,             last_vp);
    glGetIntegerv(GL_BLEND_SRC_RGB,  &last_bs_src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB,  &last_bs_dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_bs_src_a);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &last_bs_dst_a);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glUseProgram(last_prog);
    glBindBuffer(GL_ARRAY_BUFFER, last_vbo);
    glBindVertexArray(last_vao);
    glActiveTexture(last_active);
    glBindTexture(GL_TEXTURE_2D, last_tex);
    glBlendFuncSeparate(last_bs_src_rgb, last_bs_dst_rgb,
                        last_bs_src_a,   last_bs_dst_a);
    glViewport(last_vp[0], last_vp[1], last_vp[2], last_vp[3]);

    return origSwap(dpy, surf);
}

static int32_t hookInput(AInputQueue* q, AInputEvent** out) {
    int32_t r = origInputGetEvent(q, out);
    if (g_imguiReady && r == 0 && out && *out) {
        AInputEvent* e = *out;
        if (AInputEvent_getType(e) == AINPUT_EVENT_TYPE_MOTION) {
            int action = AMotionEvent_getAction(e) & AMOTION_EVENT_ACTION_MASK;
            int pointerCount = AMotionEvent_getPointerCount(e);

            // 3-пальцевый тап → toggle меню
            if (action == AMOTION_EVENT_ACTION_POINTER_DOWN && pointerCount == 3)
                g_cfg.menuOpen = !g_cfg.menuOpen;

            float x = AMotionEvent_getX(e, 0);
            float y = AMotionEvent_getY(e, 0);
            applyTouch(x, y, action);
        }
    }
    return r;
}

// ---------- Zygisk ----------
static const char* TARGETS[] = {
    "com.tencent.ig",
    "com.pubg.krmobile",
    "com.rekoo.pubgm",
    "com.vng.pubgmobile",
    "com.pubg.imobile",
};

static std::string g_pkg;
static bool g_target = false;

static void getModDevIno(const char* lib, dev_t& d, ino_t& i) {
    struct Ctx { const char* n; dev_t* d; ino_t* i; } c{ lib, &d, &i };
    dl_iterate_phdr([](dl_phdr_info* info, size_t, void* data) -> int {
        auto* c = (Ctx*)data;
        if (info->dlpi_name && strstr(info->dlpi_name, c->n)) {
            struct stat st{};
            if (stat(info->dlpi_name, &st) == 0) { *c->d = st.st_dev; *c->i = st.st_ino; }
            return 1;
        }
        return 0;
    }, &c);
}

class EspModule : public zygisk::ModuleBase {
    zygisk::Api* api = nullptr;
public:
    void onLoad(zygisk::Api* a, JNIEnv*) override { api = a; }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        JNIEnv* env = api->getEnv();
        const char* nn = env->GetStringUTFChars(args->nice_name, nullptr);
        g_pkg = nn ? nn : "";
        env->ReleaseStringUTFChars(args->nice_name, nn);
        auto pos = g_pkg.find(':');
        if (pos != std::string::npos) g_pkg = g_pkg.substr(0, pos);
        g_target = false;
        for (auto* t : TARGETS) if (g_pkg == t) { g_target = true; break; }
        if (!g_target) api->setOption(zygisk::Option::FORCE_DENYLIST_UNMOUNT);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs*) override {
        if (!g_target) return;

        Mem::install();

        dlopen("libEGL.so",    RTLD_NOW);
        dlopen("libandroid.so", RTLD_NOW);

        dev_t ed, ad; ino_t ei, ai;
        getModDevIno("libEGL.so",     ed, ei);
        getModDevIno("libandroid.so", ad, ai);

        api->pltHookRegister(ed, ei, "eglSwapBuffers",       (void*)hookSwap,  (void**)&origSwap);
        api->pltHookRegister(ad, ai, "AInputQueue_getEvent", (void*)hookInput, (void**)&origInputGetEvent);
        api->pltHookCommit();

        LOG("hooks installed for %s", g_pkg.c_str());
    }
};

REGISTER_ZYGISK_MODULE(EspModule)
