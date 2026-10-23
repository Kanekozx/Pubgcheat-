#include "overlay.hpp"
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>

using namespace android;

static sp<SurfaceComposerClient> g_client;
static sp<SurfaceControl>        g_ctrl;
static sp<Surface>               g_surface;
static EGLDisplay g_dpy = EGL_NO_DISPLAY;
static EGLSurface g_surf = EGL_NO_SURFACE;
static EGLContext g_ctx = EGL_NO_CONTEXT;
static int  g_w = 0, g_h = 0;
static bool g_on = false;

static GLuint g_prog = 0;
static GLuint g_vbo = 0;
static GLint g_color_loc = -1;

static const char* VERT = R"(
    #version 300 es
    layout(location = 0) in vec2 pos;
    void main() { gl_Position = vec4(pos, 0.0, 1.0); }
)";

static const char* FRAG = R"(
    #version 300 es
    precision mediump float;
    uniform vec4 u_color;
    out vec4 fragColor;
    void main() { fragColor = u_color; }
)";

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

bool Overlay::init() {
    g_client = new SurfaceComposerClient();
    if (g_client->initCheck() != NO_ERROR) { ERR("SCC init failed"); return false; }

    g_ctrl = g_client->createSurface(String8("pc_ovl"), 0, 0, PIXEL_FORMAT_RGBA_8888, ISurfaceComposerClient::eFXSurfaceBufferState, nullptr, nullptr);
    if (g_ctrl == nullptr) { ERR("createSurface failed"); return false; }

    g_client->openGlobalTransaction();
    g_ctrl->setLayer(0x7FFFFFFF);
    g_ctrl->setPosition(0, 0);
    g_ctrl->show();
    g_client->closeGlobalTransaction();

    g_surface = g_ctrl->getSurface();
    ANativeWindow* anw = g_surface.get();

    g_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_dpy, nullptr, nullptr);
    EGLConfig cfg; EGLint nc = 0;
    EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
    eglChooseConfig(g_dpy, attribs, &cfg, 1, &nc);
    g_ctx  = eglCreateContext(g_dpy, cfg, EGL_NO_CONTEXT, (const EGLint[]){ EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE });
    g_surf = eglCreateWindowSurface(g_dpy, cfg, anw, nullptr);
    eglMakeCurrent(g_dpy, g_surf, g_surf, g_ctx);

    EGLint w = 0, h = 0; eglQuerySurface(g_dpy, g_surf, EGL_WIDTH, &w); eglQuerySurface(g_dpy, g_surf, EGL_HEIGHT, &h);
    g_w = w; g_h = h;

    GLuint vs = compile_shader(GL_VERTEX_SHADER, VERT);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FRAG);
    g_prog = glCreateProgram();
    glAttachShader(g_prog, vs); glAttachShader(g_prog, fs); glLinkProgram(g_prog);
    g_color_loc = glGetUniformLocation(g_prog, "u_color");
    glGenBuffers(1, &g_vbo);

    g_on = true;
    LOG("overlay: EGL %dx%d initialized", g_w, g_h);
    return true;
}

void Overlay::shutdown() {
    if (g_dpy) { eglMakeCurrent(g_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); eglTerminate(g_dpy); }
    if (g_client) g_client->openGlobalTransaction();
    if (g_ctrl)   { g_ctrl->hide(); g_ctrl->destroy(); }
    if (g_client) g_client->closeGlobalTransaction();
    g_on = false;
}

void Overlay::begin() {
    eglMakeCurrent(g_dpy, g_surf, g_surf, g_ctx);
    glViewport(0, 0, g_w, g_h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(g_prog);
}

void Overlay::end() { eglSwapBuffers(g_dpy, g_surf); }

void Overlay::box(float x, float y, float w, float h, u32 argb, float thick) {
    if (!g_on) return;
    float r = ((argb >> 16) & 0xFF) / 255.f, g = ((argb >>  8) & 0xFF) / 255.f, b = ( argb        & 0xFF) / 255.f, a = ((argb >> 24) & 0xFF) / 255.f;
    glUniform4f(g_color_loc, r, g, b, a);

    float x1 = (x / g_w) * 2.f - 1.f, y1 = 1.f - (y / g_h) * 2.f;
    float x2 = ((x + w) / g_w) * 2.f - 1.f, y2 = 1.f - ((y + h) / g_h) * 2.f;
    float t = (thick / g_w) * 2.f;

    float v[] = {
        x1, y1, x2, y1, x2, y1+t, x1, y1, x2, y1+t, x1, y1+t, // top
        x1, y2-t, x2, y2-t, x2, y2, x1, y2-t, x2, y2, x1, y2, // bottom
        x1, y1+t, x1+t, y1+t, x1+t, y2-t, x1, y1+t, x1+t, y2-t, x1, y2-t, // left
        x2-t, y1+t, x2, y1+t, x2, y2-t, x2-t, y1+t, x2, y2-t, x2-t, y2-t  // right
    };
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glDrawArrays(GL_TRIANGLES, 0, 24);
}

void Overlay::text(float, float, const char*, u32, float) {}

int Overlay::width()  const { return g_w; }
int Overlay::height() const { return g_h; }