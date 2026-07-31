#pragma once
#include "imgui.h"

struct ImDrawCmd {
    ImVec4 ClipRect;
    unsigned int ElemCount;
    unsigned int IdxOffset;
    unsigned int VtxOffset;
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
};

typedef unsigned short ImDrawIdx;

struct ImDrawList {
    void Clear() {}
};

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
};

struct ImGuiContext {
    bool Initialized;
    ImGuiIO IO;
    ImGuiStyle Style;
    ImDrawData DrawData;
    ImGuiContext() : Initialized(true) {}
};
