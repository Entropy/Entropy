#include "CinderImGui.h"

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
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, true, repeat_when_held);
    
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
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, true);
    
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


//bool ImGui::Begin(const char* name, bool* p_opened, const ImVec2& size, float bg_alpha, ImGuiWindowFlags flags)
bool ImGui::Begin(const char* name, const char* font)
{
    bool* p_opened = NULL; const ImVec2& size = ImVec2(0,0); float bg_alpha = -1.0f; ImGuiWindowFlags flags = 0;
    ImGuiState& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    IM_ASSERT(g.Initialized);                       // Forgot to call ImGui::NewFrame()
    IM_ASSERT(name != NULL);                        // Must pass a name
    
    // Find or create
    bool window_is_new = false;
    ImGuiWindow* window = FindWindowByName(name);
    if (!window)
    {
        window = CreateNewWindow(name, size, flags);
        window_is_new = true;
    }
    window->Flags = (ImGuiWindowFlags)flags;
    
    // Add to stack
    g.CurrentWindowStack.push_back(window);
    g.CurrentWindow = window;
    
    // Process SetNextWindow***() calls
    bool window_pos_set_by_api = false;
    if (g.SetNextWindowPosCond)
    {
        const ImVec2 backup_cursor_pos = window->DC.CursorPos;                  // FIXME: not sure of the exact reason of this anymore :( need to look into that.
        ImGui::SetWindowPos(g.SetNextWindowPosVal, g.SetNextWindowPosCond);
        window->DC.CursorPos = backup_cursor_pos;
        window_pos_set_by_api = true;
        g.SetNextWindowPosCond = 0;
    }
    if (g.SetNextWindowSizeCond)
    {
        ImGui::SetWindowSize(g.SetNextWindowSizeVal, g.SetNextWindowSizeCond);
        g.SetNextWindowSizeCond = 0;
    }
    if (g.SetNextWindowCollapsedCond)
    {
        ImGui::SetWindowCollapsed(g.SetNextWindowCollapsedVal, g.SetNextWindowCollapsedCond);
        g.SetNextWindowCollapsedCond = 0;
    }
    if (g.SetNextWindowFocus)
    {
        ImGui::SetWindowFocus();
        g.SetNextWindowFocus = false;
    }
    
    // Find parent
    ImGuiWindow* parent_window = (flags & ImGuiWindowFlags_ChildWindow) != 0 ? g.CurrentWindowStack[g.CurrentWindowStack.size()-2] : NULL;
    
    // Update known root window (if we are a child window, otherwise window == window->RootWindow)
    size_t root_idx = g.CurrentWindowStack.size() - 1;
    while (root_idx > 0)
    {
        if ((g.CurrentWindowStack[root_idx]->Flags & ImGuiWindowFlags_ChildWindow) == 0)
            break;
        root_idx--;
    }
    window->RootWindow = g.CurrentWindowStack[root_idx];
    
    // Default alpha
    if (bg_alpha < 0.0f)
        bg_alpha = style.WindowFillAlphaDefault;
    
    // When reusing window again multiple times a frame, just append content (don't need to setup again)
    const int current_frame = ImGui::GetFrameCount();
    const bool first_begin_of_the_frame = (window->LastFrameDrawn != current_frame);
    if (first_begin_of_the_frame)
    {
        window->DrawList->Clear();
        window->Visible = true;
        
        // New windows appears in front
        if (!(flags & ImGuiWindowFlags_ChildWindow))
            if (window->LastFrameDrawn < current_frame - 1)
                FocusWindow(window);
        
        window->LastFrameDrawn = current_frame;
        window->ClipRectStack.resize(0);
        
        // Reset contents size for auto-fitting
        window->SizeContents = window_is_new ? ImVec2(0.0f, 0.0f) : window->DC.CursorMaxPos - window->Pos;
        window->SizeContents.y += window->ScrollY;
        
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            parent_window->DC.ChildWindows.push_back(window);
            window->Pos = window->PosFloat = parent_window->DC.CursorPos;
            window->SizeFull = size;
        }
    }
    
    // Setup texture
    window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
    
    // Setup outer clipping rectangle
    if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_ComboBox))
        PushClipRect(parent_window->ClipRectStack.back());
    else if (g.IO.DisplayVisibleMin.x != g.IO.DisplayVisibleMax.x && g.IO.DisplayVisibleMin.y != g.IO.DisplayVisibleMax.y)
        PushClipRect(ImVec4(g.IO.DisplayVisibleMin.x, g.IO.DisplayVisibleMin.y, g.IO.DisplayVisibleMax.x, g.IO.DisplayVisibleMax.y));
    else
        PushClipRect(ImVec4(0.0f, 0.0f, g.IO.DisplaySize.x, g.IO.DisplaySize.y));
    
    // Setup and draw window
    if (first_begin_of_the_frame)
    {
        // Reset ID stack
        window->IDStack.resize(1);
        
        // Move window (at the beginning of the frame to avoid input lag or sheering). Only valid for root windows.
        RegisterAliveId(window->MoveID);
        if (g.ActiveId == window->MoveID)
        {
            if (g.IO.MouseDown[0])
            {
                if (!(window->Flags & ImGuiWindowFlags_NoMove))
                {
                    window->PosFloat += g.IO.MouseDelta;
                    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
                        MarkSettingsDirty();
                }
                IM_ASSERT(g.MovedWindow != NULL);
                FocusWindow(g.MovedWindow);
            }
            else
            {
                SetActiveId(0);
                g.MovedWindow = NULL;   // Not strictly necessary but doing it for sanity.
            }
        }
        
        // Tooltips always follows mouse
        if (!window_pos_set_by_api && (window->Flags & ImGuiWindowFlags_Tooltip) != 0)
        {
            window->PosFloat = g.IO.MousePos + ImVec2(32,16) - style.FramePadding*2;
        }
        
        // Clamp into view
        if (!(window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_Tooltip))
        {
            if (window->AutoFitFrames == 0 && g.IO.DisplaySize.x > 0.0f && g.IO.DisplaySize.y > 0.0f) // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
            {
                ImVec2 clip_min = style.DisplaySafeAreaPadding;
                ImVec2 clip_max = g.IO.DisplaySize - style.DisplaySafeAreaPadding;
                window->PosFloat = ImMax(window->PosFloat + window->Size, clip_min) - window->Size;
                window->PosFloat = ImMin(window->PosFloat, clip_max);
            }
            window->SizeFull = ImMax(window->SizeFull, style.WindowMinSize);
        }
        window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);
        
        // Default item width. Make it proportional to window size if window manually resizes
        if (window->Size.x > 0.0f && !(window->Flags & ImGuiWindowFlags_Tooltip) && !(window->Flags & ImGuiWindowFlags_AlwaysAutoResize))
            window->ItemWidthDefault = (float)(int)(window->Size.x * 0.65f);
        else
            window->ItemWidthDefault = 200.0f;
        
        // Prepare for focus requests
        if (window->FocusIdxAllRequestNext == IM_INT_MAX || window->FocusIdxAllCounter == -1)
            window->FocusIdxAllRequestCurrent = IM_INT_MAX;
        else
            window->FocusIdxAllRequestCurrent = (window->FocusIdxAllRequestNext + (window->FocusIdxAllCounter+1)) % (window->FocusIdxAllCounter+1);
        if (window->FocusIdxTabRequestNext == IM_INT_MAX || window->FocusIdxTabCounter == -1)
            window->FocusIdxTabRequestCurrent = IM_INT_MAX;
        else
            window->FocusIdxTabRequestCurrent = (window->FocusIdxTabRequestNext + (window->FocusIdxTabCounter+1)) % (window->FocusIdxTabCounter+1);
        window->FocusIdxAllCounter = window->FocusIdxTabCounter = -1;
        window->FocusIdxAllRequestNext = window->FocusIdxTabRequestNext = IM_INT_MAX;
        
        ImRect title_bar_rect = window->TitleBarRect();
        
        // Apply and ImClamp scrolling
        window->ScrollY = window->NextScrollY;
        window->ScrollY = ImMax(window->ScrollY, 0.0f);
        if (!window->Collapsed && !window->SkipItems)
            window->ScrollY = ImMin(window->ScrollY, ImMax(0.0f, window->SizeContents.y - window->SizeFull.y));
        window->NextScrollY = window->ScrollY;
        
        // At this point we don't have a clipping rectangle setup yet, so we can test and draw in title bar
        // Collapse window by double-clicking on title bar
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
        {
            if (!(window->Flags & ImGuiWindowFlags_NoCollapse) && g.HoveredWindow == window && IsMouseHoveringRect(title_bar_rect) && g.IO.MouseDoubleClicked[0])
            {
                window->Collapsed = !window->Collapsed;
                if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
                    MarkSettingsDirty();
                FocusWindow(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }
        
        // Calculate auto-fit size
        ImVec2 size_auto_fit;
        if ((window->Flags & ImGuiWindowFlags_Tooltip) != 0)
        {
            // Tooltip always resize. We keep the spacing symmetric on both axises for aesthetic purpose.
            size_auto_fit = window->SizeContents + style.WindowPadding - ImVec2(0.0f, style.ItemSpacing.y);
        }
        else
        {
            size_auto_fit = ImClamp(window->SizeContents + style.AutoFitPadding, style.WindowMinSize, ImMax(style.WindowMinSize, g.IO.DisplaySize - style.AutoFitPadding));
        }
        
        const float window_rounding = (window->Flags & ImGuiWindowFlags_ChildWindow) ? style.ChildWindowRounding : style.WindowRounding;
        if (window->Collapsed)
        {
            // We still process initial auto-fit on collapsed windows to get a window width
            // But otherwise we don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
            if (window->AutoFitFrames > 0)
            {
                window->SizeFull = window->AutoFitOnlyGrows ? ImMax(window->SizeFull, size_auto_fit) : size_auto_fit;
                title_bar_rect = window->TitleBarRect();
            }
            
            // Draw title bar only
            window->Size = title_bar_rect.GetSize();
            window->DrawList->AddRectFilled(title_bar_rect.GetTL(), title_bar_rect.GetBR(), window->Color(ImGuiCol_TitleBgCollapsed), window_rounding);
            if (window->Flags & ImGuiWindowFlags_ShowBorders)
            {
                window->DrawList->AddRect(title_bar_rect.GetTL()+ImVec2(1,1), title_bar_rect.GetBR()+ImVec2(1,1), window->Color(ImGuiCol_BorderShadow), window_rounding);
                window->DrawList->AddRect(title_bar_rect.GetTL(), title_bar_rect.GetBR(), window->Color(ImGuiCol_Border), window_rounding);
            }
        }
        else
        {
            ImU32 resize_col = 0;
            if ((window->Flags & ImGuiWindowFlags_Tooltip) != 0)
            {
                window->Size = window->SizeFull = size_auto_fit;
            }
            else
            {
                if ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
                {
                    // Don't continuously mark settings as dirty, the size of the window doesn't need to be stored.
                    window->SizeFull = size_auto_fit;
                }
                else if (window->AutoFitFrames > 0)
                {
                    // Auto-fit only grows during the first few frames
                    window->SizeFull = window->AutoFitOnlyGrows ? ImMax(window->SizeFull, size_auto_fit) : size_auto_fit;
                    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
                        MarkSettingsDirty();
                }
                else if (!(window->Flags & ImGuiWindowFlags_NoResize))
                {
                    // Manual resize grip
                    const ImRect resize_rect(window->Rect().GetBR()-ImVec2(14,14), window->Rect().GetBR());
                    const ImGuiID resize_id = window->GetID("#RESIZE");
                    bool hovered, held;
                    ButtonBehavior(resize_rect, resize_id, &hovered, &held, true);
                    resize_col = window->Color(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
                    
                    if (hovered || held)
                        g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
                    
                    if (g.HoveredWindow == window && held && g.IO.MouseDoubleClicked[0])
                    {
                        // Manual auto-fit when double-clicking
                        window->SizeFull = size_auto_fit;
                        if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
                            MarkSettingsDirty();
                        SetActiveId(0);
                    }
                    else if (held)
                    {
                        // Resize
                        window->SizeFull = ImMax(window->SizeFull + g.IO.MouseDelta, style.WindowMinSize);
                        if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
                            MarkSettingsDirty();
                    }
                }
                
                // Update rectangle immediately so that rendering right below us isn't one frame late
                window->Size = window->SizeFull;
                title_bar_rect = window->TitleBarRect();
            }
            
            // Scrollbar
            window->ScrollbarY = (window->SizeContents.y > window->Size.y) && !(window->Flags & ImGuiWindowFlags_NoScrollbar);
            
            // Window background
            if (bg_alpha > 0.0f)
            {
                if ((window->Flags & ImGuiWindowFlags_ComboBox) != 0)
                    window->DrawList->AddRectFilled(window->Pos, window->Pos+window->Size, window->Color(ImGuiCol_ComboBg, bg_alpha), window_rounding);
                else if ((window->Flags & ImGuiWindowFlags_Tooltip) != 0)
                    window->DrawList->AddRectFilled(window->Pos, window->Pos+window->Size, window->Color(ImGuiCol_TooltipBg, bg_alpha), window_rounding);
                else if ((window->Flags & ImGuiWindowFlags_ChildWindow) != 0)
                    window->DrawList->AddRectFilled(window->Pos, window->Pos+window->Size-ImVec2(window->ScrollbarY?style.ScrollbarWidth:0.0f,0.0f), window->Color(ImGuiCol_ChildWindowBg, bg_alpha), window_rounding, window->ScrollbarY ? (1|8) : (0xF));
                else
                    window->DrawList->AddRectFilled(window->Pos, window->Pos+window->Size, window->Color(ImGuiCol_WindowBg, bg_alpha), window_rounding);
            }
            
            // Title bar
            if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
                window->DrawList->AddRectFilled(title_bar_rect.GetTL(), title_bar_rect.GetBR(), window->Color(ImGuiCol_TitleBg), window_rounding, 1|2);
            
            // Borders
            if (window->Flags & ImGuiWindowFlags_ShowBorders)
            {
                window->DrawList->AddRect(window->Pos+ImVec2(1,1), window->Pos+window->Size+ImVec2(1,1), window->Color(ImGuiCol_BorderShadow), window_rounding);
                window->DrawList->AddRect(window->Pos, window->Pos+window->Size, window->Color(ImGuiCol_Border), window_rounding);
                if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
                    window->DrawList->AddLine(title_bar_rect.GetBL(), title_bar_rect.GetBR(), window->Color(ImGuiCol_Border));
            }
            
            // Scrollbar
            if (window->ScrollbarY)
                Scrollbar(window);
            
            // Render resize grip
            // (after the input handling so we don't have a frame of latency)
            if (!(window->Flags & ImGuiWindowFlags_NoResize))
            {
                const float r = window_rounding;
                const ImVec2 br = window->Rect().GetBR();
                if (r == 0.0f)
                {
                    window->DrawList->AddTriangleFilled(br, br-ImVec2(0,14), br-ImVec2(14,0), resize_col);
                }
                else
                {
                    // FIXME: We should draw 4 triangles and decide on a size that's not dependent on the rounding size (previously used 18)
                    window->DrawList->AddArc(br - ImVec2(r,r), r, resize_col, 6, 9, true);
                    window->DrawList->AddTriangleFilled(br+ImVec2(0,-2*r),br+ImVec2(0,-r),br+ImVec2(-r,-r), resize_col);
                    window->DrawList->AddTriangleFilled(br+ImVec2(-r,-r), br+ImVec2(-r,0),br+ImVec2(-2*r,0), resize_col);
                }
            }
        }
        
        // Setup drawing context
        window->DC.ColumnsStartX = window->WindowPadding().x;
        window->DC.ColumnsOffsetX = 0.0f;
        window->DC.CursorStartPos = window->Pos + ImVec2(window->DC.ColumnsStartX + window->DC.ColumnsOffsetX, window->TitleBarHeight() + window->WindowPadding().y) - ImVec2(0.0f, window->ScrollY);
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.CurrentLineHeight = window->DC.PrevLineHeight = 0.0f;
        window->DC.CurrentLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;
        window->DC.ChildWindows.resize(0);
        window->DC.ItemWidth.resize(0);
        window->DC.ItemWidth.push_back(window->ItemWidthDefault);
        window->DC.AllowKeyboardFocus.resize(0);
        window->DC.AllowKeyboardFocus.push_back(true);
        window->DC.TextWrapPos.resize(0);
        window->DC.TextWrapPos.push_back(-1.0f); // disabled
        window->DC.ColorEditMode = ImGuiColorEditMode_UserSelect;
        window->DC.ColumnsCurrent = 0;
        window->DC.ColumnsCount = 1;
        window->DC.ColumnsStartPos = window->DC.CursorPos;
        window->DC.ColumnsCellMinY = window->DC.ColumnsCellMaxY = window->DC.ColumnsStartPos.y;
        window->DC.TreeDepth = 0;
        window->DC.StateStorage = &window->StateStorage;
        window->DC.GroupStack.resize(0);
        
        if (window->AutoFitFrames > 0)
            window->AutoFitFrames--;
        
        // Title bar
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
        {
            if (p_opened != NULL)
                CloseWindowButton(p_opened);
            
            ImVec2 text_min = window->Pos + style.FramePadding;
            if (!(window->Flags & ImGuiWindowFlags_NoCollapse))
            {
                RenderCollapseTriangle(window->Pos + style.FramePadding, !window->Collapsed, 1.0f, true);
                text_min.x += window->FontSize() + style.ItemInnerSpacing.x;
            }
            
            ui::ScopedFont scopedFont( font );
            const ImVec2 text_size = CalcTextSize(name, NULL, true);
            const ImVec2 text_max = window->Pos + ImVec2(window->Size.x - (p_opened ? (title_bar_rect.GetHeight()-3) : style.FramePadding.x), style.FramePadding.y*2 + text_size.y);
            
            RenderTextClipped(text_min, name, NULL, &text_size, text_max);
        }
        
        // Save clipped aabb so we can access it in constant-time in FindHoveredWindow()
        window->ClippedRect = window->Rect();
        window->ClippedRect.Clip(window->ClipRectStack.front());
        
        // Pressing CTRL+C while holding on a window copy its content to the clipboard
        // This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
        // Maybe we can support CTRL+C on every element?
        /*
         if (g.ActiveId == move_id)
         if (g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_C))
         ImGui::LogToClipboard();
         */
    }
    
    // Inner clipping rectangle
    // We set this up after processing the resize grip so that our clip rectangle doesn't lag by a frame
    const ImRect title_bar_rect = window->TitleBarRect();
    ImVec4 clip_rect(title_bar_rect.Min.x+0.5f+window->WindowPadding().x*0.5f, title_bar_rect.Max.y+0.5f, window->Rect().Max.x+0.5f-window->WindowPadding().x*0.5f, window->Rect().Max.y-1.5f);
    if (window->ScrollbarY)
        clip_rect.z -= style.ScrollbarWidth;
    PushClipRect(clip_rect);
    
    // Clear 'accessed' flag last thing
    if (first_begin_of_the_frame)
        window->Accessed = false;
    
    // Child window can be out of sight and have "negative" clip windows.
    // Mark them as collapsed so commands are skipped earlier (we can't manually collapse because they have no title bar).
    if (flags & ImGuiWindowFlags_ChildWindow)
    {
        IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
        window->Collapsed = parent_window && parent_window->Collapsed;
        
        const ImVec4 clip_rect_t = window->ClipRectStack.back();
        window->Collapsed |= (clip_rect_t.x >= clip_rect_t.z || clip_rect_t.y >= clip_rect_t.w);
        
        // We also hide the window from rendering because we've already added its border to the command list.
        // (we could perform the check earlier in the function but it is simpler at this point)
        if (window->Collapsed)
            window->Visible = false;
    }
    if (style.Alpha <= 0.0f)
        window->Visible = false;
    
    // Return false if we don't intend to display anything to allow user to perform an early out optimization
    window->SkipItems = (window->Collapsed || !window->Visible) && window->AutoFitFrames == 0;
    return !window->SkipItems;
}
