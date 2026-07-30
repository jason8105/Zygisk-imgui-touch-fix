#include "imgui.h"
#include <stdio.h>
#include <stdlib.h>

static ImIO g_IO;
static bool g_Initialized = false;

ImIO& ImGui::GetIO() {
    return g_IO;
}

void ImGui::CreateContext() {
    memset(&g_IO, 0, sizeof(g_IO));
    g_Initialized = true;
}

void ImGui::DestroyContext() {
    g_Initialized = false;
}

void ImGui::NewFrame() {
}

void ImGui::Render() {
}

bool ImGui::Begin(const char* name, bool* p_open, int flags) {
    return true;
}

void ImGui::End() {
}

void ImGui::Text(const char* fmt, ...) {
}

bool ImGui::Button(const char* label, const ImVec2& size) {
    return false;
}
