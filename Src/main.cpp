#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h" // <-- Добавили заголовочный файл Android
#include "dobby.h"

#define LOG_TAG "Internal"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

bool g_Initialized = false;
int g_Width = 0, g_Height = 0;

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        // Узнаем разрешение экрана
        eglQuerySurface(dpy, surface, EGL_WIDTH, &g_Width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_Height);

        // Инициализация ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);

        // ВОТ ОНА - Инициализация ввода для Android!
        ImGui_ImplAndroid_Init();
        
        // Инициализация отрисовки OpenGL
        ImGui_ImplOpenGL3_Init("#version 300 es");
        
        g_Initialized = true;
        LOGD("ImGui Initialized. Screen: %dx%d", g_Width, g_Height);
    }

    if (g_Initialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame(g_Width, g_Height); // <-- Добавили новый кадр Android
        ImGui::NewFrame();

        // --- РИСУЕМ НАШЕ МЕНЮ ---
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin("Internal Cheat", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Hello from Internal!");
        
        static bool esp_enabled = true;
        ImGui::Checkbox("Enable ESP", &esp_enabled);
        
        ImGui::End();
        // -------------------------

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

// Ждем загрузки игры и ставим хуки
void* main_thread(void*) {
    LOGD("Waiting for libUE4.so...");
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libUE4.so", RTLD_NOW);
        if (!handle) sleep(1);
    }
    LOGD("libUE4.so loaded!");

    // Хукаем функцию отрисовки EGL
    void* eglSwapBuffers = dlsym(RTLD_NEXT, "eglSwapBuffers");
    if (eglSwapBuffers) {
        DobbyHook(eglSwapBuffers, (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        LOGD("Hooked eglSwapBuffers successfully!");
    }

    return nullptr;
}

__attribute__((constructor))
void _init() {
    pthread_t pt;
    pthread_create(&pt, nullptr, main_thread, nullptr);
}
