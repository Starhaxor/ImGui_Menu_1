#include "pch.h"
#include "menu.h"
#include "config.h"
#include "theme.h"
#include "widgets.h"
#include "overlay.h"
#include "modern_style.h"
#include "HackManager.h"

#include <string>

ImFont* g_fontBody = nullptr;
ImFont* g_fontBold = nullptr;
ImFont* g_fontTabs = nullptr;

namespace
{
    enum Tab { TabVisuals = 0, TabAimbot, TabTeleport, TabInventory, TabMovement, TabMisc, TabKick, TabAutoFarmer, TabRemoteKill, TabGameOver };
    int g_activeTab = TabAimbot;

    void DrawTitleBar()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 wp = ImGui::GetWindowPos();
        const ImVec2 ws = ImGui::GetWindowSize();
        const float tbH = 30.0f;

        dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + tbH), IM_COL32(36, 39, 44, 255), 5.0f, ImDrawFlags_RoundCornersTop);
        dl->AddRectFilled(ImVec2(wp.x, wp.y + tbH - 3.0f), ImVec2(wp.x + ws.x, wp.y + tbH), COL_GOLD);
        dl->AddCircleFilled(ImVec2(wp.x + 13.0f, wp.y + tbH * 0.5f - 1.0f), 3.0f, COL_GOLD);

        ImGui::PushFont(g_fontBold);
        ImGui::SetCursorPos(ImVec2(22.0f, (tbH - ImGui::GetTextLineHeight()) * 0.5f - 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(245, 246, 248, 255));
        ImGui::TextUnformatted("StarHack S1");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ImGui::InvisibleButton("##titlebar_drag", ImVec2(ws.x - 26.0f, tbH));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            ImGui::SetWindowPos(ImVec2(wp.x + io.MouseDelta.x, wp.y + io.MouseDelta.y));

        ImGui::SetCursorPos(ImVec2(ws.x - 27.0f, 5.0f));
        ImGui::PushID("##close");
        const bool closeClicked = ImGui::InvisibleButton("##x", ImVec2(21.0f, 19.0f));
        ImGui::PopID();
        {
            const ImVec2 a = ImGui::GetItemRectMin();
            const ImVec2 b = ImGui::GetItemRectMax();
            const bool hov = ImGui::IsItemHovered();
            dl->AddRectFilled(a, b, hov ? IM_COL32(67, 71, 78, 255) : IM_COL32(47, 50, 56, 255), 3.0f);
            const float cx = (a.x + b.x) * 0.5f;
            const float cy = (a.y + b.y) * 0.5f;
            const ImU32 xColor = hov ? COL_GOLD : IM_COL32(205, 208, 214, 255);
            dl->AddLine(ImVec2(cx - 3.0f, cy - 3.0f), ImVec2(cx + 3.0f, cy + 3.0f), xColor, 1.4f);
            dl->AddLine(ImVec2(cx + 3.0f, cy - 3.0f), ImVec2(cx - 3.0f, cy + 3.0f), xColor, 1.4f);
        }
        if (closeClicked)
            InterlockedExchange((volatile long*)&g_showMenu, 0);
    }

    void DrawTabBar()
    {
        static const char* tabs[] = { "Visual", "Aim", "Teleport", "Inventory", "MOVE", "MISC", "KICK", "FARMER", "REMOTE", "EXIT" };
        ImDrawList* dl = ImGui::GetWindowDrawList();

        float x = 8.0f;
        const float y = 36.0f;
        const float tabH = 24.0f;

        for (int i = 0; i < IM_ARRAYSIZE(tabs); i++)
        {
            ImGui::PushFont(g_fontTabs);
            const ImVec2 ts = ImGui::CalcTextSize(tabs[i]);
            const float w = ts.x + 12.0f;

            ImGui::SetCursorPos(ImVec2(x, y));
            ImGui::PushID(i);
            ImGui::InvisibleButton("##tab", ImVec2(w, tabH));
            const bool clicked = ImGui::IsItemClicked();
            ImGui::PopID();

            const ImVec2 a = ImGui::GetItemRectMin();
            const ImVec2 b = ImGui::GetItemRectMax();
            const bool active = (g_activeTab == i);
            const bool hovered = ImGui::IsItemHovered();
            const bool held = ImGui::IsItemActive();

            const ModernControlVisual visual = ResolveModernControlVisual(ModernControlKind::Tab, active, hovered, held);
            dl->AddRectFilled(a, b, visual.fill, 3.0f);
            dl->AddRect(a, b, visual.border, 3.0f, 0, 1.0f);
            if (visual.accentVisible)
                dl->AddRectFilled(ImVec2(a.x + 3.0f, b.y - 3.0f), ImVec2(b.x - 3.0f, b.y), visual.accent, 2.0f);
            dl->AddText(ImVec2((a.x + b.x - ts.x) * 0.5f, (a.y + b.y - ts.y) * 0.5f), visual.text, tabs[i]);

            ImGui::PopFont();

            if (clicked)
                g_activeTab = i;
            x += w + 1.0f;
        }

        ImGui::SetCursorPos(ImVec2(8.0f, y + tabH + 8.0f));
    }

    void DrawSectionHeader(const char* label)
    {
        const ModernSectionVisual visual = ResolveModernSectionVisual();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;

        ImGui::PushFont(g_fontBold);
        const ImVec2 textSize = ImGui::CalcTextSize(label);
        ImGui::PopFont();

        const float textY = pos.y + (visual.height - textSize.y) * 0.5f;
        const float textX = pos.x + visual.markerWidth + 7.0f;
        dl->AddRectFilled(
            ImVec2(pos.x, pos.y + 5.0f),
            ImVec2(pos.x + visual.markerWidth, pos.y + visual.height - 5.0f),
            visual.accent,
            1.5f);
        ImGui::PushFont(g_fontBold);
        dl->AddText(ImVec2(textX, textY), visual.text, label);
        ImGui::PopFont();

        const float ruleX = textX + textSize.x + 9.0f;
        if (ruleX < pos.x + width)
            dl->AddLine(ImVec2(ruleX, pos.y + visual.height * 0.5f), ImVec2(pos.x + width, pos.y + visual.height * 0.5f), visual.rule, 1.0f);

        ImGui::Dummy(ImVec2(width, visual.height));
    }

    bool BeginColumns()
    {
        const float availW = ImGui::GetContentRegionAvail().x;
        const float leftW = availW * 0.48f;
        ImGui::BeginChild("##col_left", ImVec2(leftW, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
        return true;
    }

    void SwitchColumns()
    {
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##col_right", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
    }

    void EndColumns()
    {
        ImGui::EndChild();
    }

    void DrawTabAim()
    {
        BeginColumns();
        DrawSectionHeader("PLAYER");
        ClassicCheckbox("No Death Count", &g_config.noDeathCount);
        ClassicCheckbox("No Recoil", &g_config.noRecoil);
        ClassicCheckbox("No Spread", &g_config.noSpread);
        ClassicCheckbox("No Fall Damage", &g_config.noFallDamage);
        ClassicCheckbox("Ghost Hack", &g_config.ghostHack);
        ClassicCheckbox("God Mode", &g_config.godMode);
        ClassicCheckbox("Remote Kill", &g_config.remoteKill);
        ImGui::Spacing();
        static const char* damages[] = { "x 1", "x 1.5", "x 2", "x 5" };
        ClassicCombo("Damage Hack:", &g_config.damageIndex, damages, IM_ARRAYSIZE(damages));
        ImGui::Spacing();
        static const char* modes[] = { "Deathmatch", "Team Deathmatch", "Zombie", "Survival" };
        ClassicCombo("Mode Hack:", &g_config.modeIndex, modes, IM_ARRAYSIZE(modes));
        ImGui::Spacing();
        ClassicCheckbox("3rd Person View", &g_config.thirdPerson);
        ClassicCheckbox("Sneak Hack", &g_config.sneakHack);
        ClassicCheckbox("Unlimited Revive", &g_config.unlimitedRevive);

        SwitchColumns();

        DrawSectionHeader("COMBAT");
        ClassicCheckbox("Respawn where died", &g_config.instantRespawn);
        ClassicCheckbox("Infinite Ammo", &g_config.infiniteAmmo);
        ClassicCheckbox("Rapid Fire", &g_config.rapidFire);
        ClassicCheckbox("Perfect Ms", &g_config.perfectMs);
        ClassicCheckbox("Early Respawn", &g_config.earlyRespawn);
        ClassicCheckbox("Team Kill", &g_config.teamKill);
        ImGui::Spacing();
        ImGui::TextUnformatted("HP Hack:");
        ImGui::SameLine(0.0f, 5.0f);
        ImGui::SetNextItemWidth(70.0f);
        ImGui::InputInt("##aim_hp", &g_config.hpHack, 10, 0);
        g_config.hpHack = (g_config.hpHack < 0) ? 0 : (g_config.hpHack > 100 ? 100 : g_config.hpHack);
        ImGui::TextUnformatted("Armor Hack:");
        ImGui::SameLine(0.0f, 5.0f);
        ImGui::SetNextItemWidth(70.0f);
        ImGui::InputFloat("##aim_armor", &g_config.armor, 5.0f, 0.0f, "%.0f");
        ClassicCheckbox("Anti AFK", &g_config.antiAfk);
        ImGui::Spacing();
        DrawSectionHeader("AIMBOT");
        ClassicCheckbox("Crosshair", &g_config.crosshair);
        ClassicCheckbox("Aimbot", &g_config.aimbot);
        ClassicSliderFloat("Smoothness", &g_config.aimbotSmooth, 1.0f, 50.0f, "%.0f");
        ClassicSliderFloat("FOV", &g_config.fov, 30.0f, 180.0f, "%.0f");
        EndColumns();
    }

    void DrawTabTeleport()
    {
        BeginColumns();
        DrawSectionHeader("TELEPORT");
        ClassicCheckbox("Auto Teleport", &g_config.autoTeleport);
        ClassicCheckbox("Instant Teleport", &g_config.instantTeleport);
        ClassicCheckbox("Safe Fall", &g_config.safeFall);
        ImGui::Spacing();
        static const char* locations[] = { "Base", "Mid", "A Site", "B Site", "Enemy Spawn" };
        ClassicCombo("Location:", &g_config.teleportLocation, locations, IM_ARRAYSIZE(locations));

        SwitchColumns();

        DrawSectionHeader("WAYPOINTS");
        if (ClassicButton("Base", ImVec2(110.0f, 22.0f))) { g_config.teleportLocation = 0; CHackManager::Get().RequestTeleport(); }
        if (ClassicButton("Mid", ImVec2(110.0f, 22.0f))) { g_config.teleportLocation = 1; CHackManager::Get().RequestTeleport(); }
        if (ClassicButton("A Site", ImVec2(110.0f, 22.0f))) { g_config.teleportLocation = 2; CHackManager::Get().RequestTeleport(); }
        if (ClassicButton("B Site", ImVec2(110.0f, 22.0f))) { g_config.teleportLocation = 3; CHackManager::Get().RequestTeleport(); }
        if (ClassicButton("Enemy Spawn", ImVec2(110.0f, 22.0f))) { g_config.teleportLocation = 4; CHackManager::Get().RequestTeleport(); }
        EndColumns();
    }

    void DrawTabInventory()
    {
        BeginColumns();
        DrawSectionHeader("LOADOUT");
        ClassicCheckbox("Infinite Ammo", &g_config.infiniteAmmo);
        ClassicCheckbox("VIP Hack", &g_config.vipHack);
        ClassicCheckbox("Rapid Fire", &g_config.rapidFire);
        ClassicCheckbox("Perfect Ms", &g_config.perfectMs);
        ClassicCheckbox("Instant Respawn", &g_config.instantRespawn);
        ClassicCheckbox("Team Kill", &g_config.teamKill);
        ClassicCheckbox("HeadShot", &g_config.headShot);
        ClassicCheckbox("No Weapon SP", &g_config.noWeaponSP);
        ImGui::Spacing();
        ImGui::TextUnformatted("HP Hack:");
        ImGui::SameLine(0.0f, 6.0f);
        ImGui::SetNextItemWidth(80.0f);
        ImGui::InputInt("##hp", &g_config.hpHack, 10, 0);
        if (g_config.hpHack < 0) g_config.hpHack = 0;
        if (g_config.hpHack > 100) g_config.hpHack = 100;
        ImGui::Spacing();
        ClassicSliderFloat("Armor Hack", &g_config.armor, 0.0f, 100.0f, "%.0f");
        ImGui::Spacing();
        static const char* ammoTypes[] = { "Normal", "Armor Piercing", "Hollow Point", "Incendiary" };
        ClassicCombo("Ammo Type:", &g_config.ammoType, ammoTypes, IM_ARRAYSIZE(ammoTypes));

        SwitchColumns();

        DrawSectionHeader("UTILITIES");
        ClassicCheckbox("Auto Pickup", &g_config.autoPickup);
        ClassicCheckbox("Fast Reload", &g_config.fastReload);
        ClassicCheckbox("Repair Tools", &g_config.repairTools);
        EndColumns();
    }

    void DrawTabMovement()
    {
        BeginColumns();
        DrawSectionHeader("MOVEMENT");
        ClassicCheckbox("Bunny Hop", &g_config.bunnyHop);
        ClassicCheckbox("Air Control", &g_config.airControl);
        ClassicCheckbox("Auto Sprint", &g_config.autoSprint);
        ClassicCheckbox("Fly Hack", &g_config.flyHack);
        ClassicCheckbox("Noclip", &g_config.noclip);

        SwitchColumns();

        DrawSectionHeader("PHYSICS");
        ClassicSliderFloat("Speed", &g_config.speedMultiplier, 0.5f, 3.0f, "x%.2f");
        ImGui::Spacing();
        ClassicSliderFloat("Jump Height", &g_config.jumpMultiplier, 0.5f, 3.0f, "x%.2f");
        ImGui::Spacing();
        ClassicSliderFloat("Gravity", &g_config.gravity, 0.1f, 2.0f, "%.2f");
        EndColumns();
    }

    void DrawTabMisc()
    {
        BeginColumns();
        DrawSectionHeader("GENERAL");
        ClassicCheckbox("Anti AFK", &g_config.antiAfk);
        ClassicCheckbox("Stream Proof", &g_config.streamProof);
        ClassicCheckbox("Show Watermark", &g_config.watermark);
        ImGui::Spacing();
        ClassicSliderFloat("Menu Opacity", &g_config.opacity, 0.30f, 1.00f, "%.0f%%");
        ImGui::Spacing();
        DrawSectionHeader("CHARACTER");
        ClassicCheckbox("Char Red", &g_config.charRed);
        ClassicCheckbox("Char Blue", &g_config.charBlue);
        ClassicCheckbox("Base Return", &g_config.baseReturn);

        SwitchColumns();

        DrawSectionHeader("SHORTCUTS");
        ImGui::BulletText("INSERT - toggle menu");
        ImGui::BulletText("END - unload");
        ImGui::Spacing();
        if (ClassicButton("Unload", ImVec2(110.0f, 22.0f)))
            InterlockedExchange((volatile long*)&g_wantUnload, 1);
        EndColumns();
    }

    void DrawTabKick()
    {
        BeginColumns();
        DrawSectionHeader("VOTE KICK");
        static const char* players[] = { "Player 1", "Player 2", "Player 3", "Player 4", "Player 5", "Player 6", "Player 7", "Player 8" };
        ClassicCombo("Vote Kick:", &g_config.kickPlayer, players, IM_ARRAYSIZE(players));
        ImGui::Spacing();
        ClassicCheckbox("Auto Kick", &g_config.autoKick);
        ImGui::Spacing();
        ClassicSliderFloat("Kick Cooldown", &g_config.kickCooldown, 5.0f, 60.0f, "%.0f sn");

        SwitchColumns();

        DrawSectionHeader("ACTIONS");
        if (ClassicButton("Kick Selected", ImVec2(110.0f, 22.0f)))
        {
            CHackManager::Get().RequestKick(g_config.kickPlayer);
            ImGui::OpenPopup("##kick_toast");
        }
        if (ImGui::BeginPopup("##kick_toast"))
        {
            ImGui::Text("Vote kick sent.");
            ImGui::EndPopup();
        }
        EndColumns();
    }

    void DrawWatermark()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12.0f, 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(V_BG.x, V_BG.y, V_BG.z, 0.95f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##watermark", nullptr, flags);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 a = ImGui::GetWindowPos();
        const ImVec2 b = ImVec2(a.x + ImGui::GetWindowWidth(), a.y + ImGui::GetWindowHeight());
        Sunken(dl, a, b, COL_BG);
        ImGui::Text("Star Hack 0.1 | %d fps", (int)io.Framerate);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}

void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontConfig.PixelSnapH = true;

    char winDir[MAX_PATH]{};
    GetWindowsDirectoryA(winDir, MAX_PATH);
    std::string fontRegular = std::string(winDir) + "\\Fonts\\tahoma.ttf";
    std::string fontBold = std::string(winDir) + "\\Fonts\\tahomabd.ttf";
    g_fontBody = io.Fonts->AddFontFromFileTTF(fontRegular.c_str(), 13.0f, &fontConfig);
    g_fontBold = io.Fonts->AddFontFromFileTTF(fontBold.c_str(), 13.0f, &fontConfig);
        g_fontTabs = io.Fonts->AddFontFromFileTTF(fontRegular.c_str(), 10.0f, &fontConfig);
    if (!g_fontBody)
    {
        g_fontBody = io.Fonts->AddFontFromFileTTF((std::string(winDir) + "\\Fonts\\segoeui.ttf").c_str(), 13.0f, &fontConfig);
        g_fontBold = io.Fonts->AddFontFromFileTTF((std::string(winDir) + "\\Fonts\\segoeuib.ttf").c_str(), 13.0f, &fontConfig);
        g_fontTabs = io.Fonts->AddFontFromFileTTF((std::string(winDir) + "\\Fonts\\segoeui.ttf").c_str(), 10.0f, &fontConfig);
    }
    if (!g_fontBody)
        g_fontBody = io.Fonts->AddFontDefault();
    if (!g_fontBold)
        g_fontBold = g_fontBody;
    if (!g_fontTabs)
        g_fontTabs = g_fontBody;
    io.FontDefault = g_fontBody;
}

void RenderUI()
{
    CHackManager::Get().DrawOverlay();

    if (g_config.watermark)
        DrawWatermark();
    if (g_showMenu)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(620.0f, 460.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

        const float alpha = g_config.opacity;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(V_BG.x, V_BG.y, V_BG.z, alpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, alpha));

        ImGuiWindowFlags wflags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##MainMenu", nullptr, wflags);

        DrawTitleBar();
        DrawTabBar();

        ImGui::BeginChild("##classic_content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
        switch (g_activeTab)
        {
        case TabVisuals:
        case TabAimbot:
        case TabRemoteKill: DrawTabAim();       break;
        case TabTeleport:  DrawTabTeleport();  break;
        case TabInventory: DrawTabInventory(); break;
        case TabMovement:  DrawTabMovement();  break;
        case TabMisc:
        case TabAutoFarmer:
        case TabGameOver:  DrawTabMisc();      break;
        case TabKick:      DrawTabKick();      break;
        }
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleColor(2);
    }
}
