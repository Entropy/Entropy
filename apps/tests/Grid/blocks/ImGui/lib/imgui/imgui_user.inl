
bool ImGui::TextButton(const char* label, const ImVec2& size_arg, bool repeat_when_held)
{
    ImGuiState& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 text_size = CalcTextSize(label, NULL, true);
    
    const ImVec2 size(size_arg.x != 0.0f ? size_arg.x : text_size.x, size_arg.y != 0.0f ? size_arg.y : text_size.y);
    
    const ImGuiAabb bb(window->DC.CursorPos, window->DC.CursorPos + size + style.FramePadding*2.0f);
    ItemSize(bb);
    if (!ItemAdd(bb, &id))
        return false;
    
    bool hovered, held;
    bool pressed = ButtonBehaviour(bb, id, &hovered, &held, true, repeat_when_held);
    
    // Render
    //const ImVec4 col = (ImVec4) window->Color((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    
    const ImVec4 col = g.Style.Colors[(hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_Button : ImGuiCol_ButtonHovered ];
    //RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    
    PushStyleColor( ImGuiCol_Text, col );
    const ImVec2 off = ImVec2(ImMax(0.0f, size.x - text_size.x) * 0.5f, ImMax(0.0f, size.y - text_size.y) * 0.5f);  // Center (only applies if we explicitly gave a size bigger than the text size, which isn't the common path)
    RenderTextClipped(bb.Min + style.FramePadding + off, label, NULL, &text_size, bb.Max);                          // Render clip (only applies if we explicitly gave a size smaller than the text size, which isn't the commmon path)
    PopStyleColor();
    
    return pressed;
}


bool ImGui::SmallTextButton(const char* label)
{
    ImGuiState& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 text_size = CalcTextSize(label, NULL, true);
    
    const ImGuiAabb bb(window->DC.CursorPos, window->DC.CursorPos + text_size + ImVec2(style.FramePadding.x*2,0));
    ItemSize(bb);
    if (!ItemAdd(bb, &id))
        return false;
    
    bool hovered, held;
    bool pressed = ButtonBehaviour(bb, id, &hovered, &held, true);
    
    // Render
    //const ImU32 col = window->Color((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    
    //const ImVec4 col = (ImVec4) window->Color((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    
    const ImVec4 col = g.Style.Colors[(hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_Button : ImGuiCol_ButtonHovered ];
    //RenderFrame(bb.Min, bb.Max, col);
    PushStyleColor( ImGuiCol_Text, col );
    RenderText(bb.Min + ImVec2(style.FramePadding.x,0), label);
    PopStyleColor();
    
    return pressed;
}
