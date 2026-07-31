#include "imgui_impl_gles3.h"
#include <GLES3/gl3.h>

static GLuint g_FontTexture = 0;
static GLuint g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static GLint g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static GLint g_AttribLocationProjMtx = 0, g_AttribLocationTexture = 0;
static GLuint g_VboHandle = 0, g_ElementsHandle = 0;

bool ImGui_ImplGLES3_Init(const char* glsl_version) {
    return true;
}

void ImGui_ImplGLES3_Shutdown() {
    ImGui_ImplGLES3_DestroyDeviceObjects();
}

void ImGui_ImplGLES3_NewFrame() {
    if (!g_FontTexture)
        ImGui_ImplGLES3_CreateDeviceObjects();
}

bool ImGui_ImplGLES3_CreateDeviceObjects() {
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

void ImGui_ImplGLES3_DestroyDeviceObjects() {
    if (g_FontTexture) {
        glDeleteTextures(1, &g_FontTexture);
        g_FontTexture = 0;
    }
}

void ImGui_ImplGLES3_RenderDrawData(ImDrawData* draw_data) {
    if (!draw_data || draw_data->CmdListsCount == 0) return;
}
