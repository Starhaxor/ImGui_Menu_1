#include "pch.h"
#include "HackManager.h"
#include "Pointers.h"
#include "config.h"
#include "imgui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

namespace
{
    constexpr uintptr_t OFF_PLAYER_HEALTH   = 0x12345678;
    constexpr uintptr_t OFF_PLAYER_AMMO     = 0x12345679;
    constexpr uintptr_t OFF_PLAYER_POSITION = 0x1234567A;
    constexpr uintptr_t OFF_PLAYER_NAME     = 0x1234567D;
    constexpr uintptr_t OFF_CHAR_COLOR      = 0x1234567E;
    constexpr uintptr_t OFF_WEAPON_DAMAGE   = 0x1234567F;
    constexpr uintptr_t OFF_HEADSHOT        = 0x12345682;
    constexpr uintptr_t OFF_REVIVE_TIME     = 0x12345683;
    constexpr uintptr_t OFF_CAMERA_MODE     = 0x12345684;
    constexpr uintptr_t OFF_REVIVE_LIMIT    = 0x12345685;
    constexpr uintptr_t OFF_RESPAWN_TIME    = 0x12345686;
    constexpr uintptr_t OFF_FALL_DAMAGE     = 0x12345687;
    constexpr uintptr_t OFF_PLAYER_ARMOR    = 0x12345689;
    constexpr uintptr_t OFF_DEATH_COUNT     = 0x1234568A;
    constexpr uintptr_t OFF_GHOST_FLAG      = 0x1234568B;
    constexpr uintptr_t OFF_SNEAK_FLAG      = 0x1234568C;
    constexpr uintptr_t OFF_TEAMKILL_FLAG   = 0x1234568D;
    constexpr uintptr_t OFF_FIRE_RATE       = 0x1234568E;
    constexpr uintptr_t OFF_PERFECTMS_FLAG  = 0x1234568F;
    constexpr uintptr_t OFF_VIP_FLAG        = 0x12345690;
    constexpr uintptr_t OFF_PICKUP_RADIUS   = 0x12345691;
    constexpr uintptr_t OFF_RELOAD_SPEED    = 0x12345692;
    constexpr uintptr_t OFF_TOOL_DURABILITY = 0x12345693;
    constexpr uintptr_t OFF_MOVE_SPEED_MUL  = 0x12345694;
    constexpr uintptr_t OFF_JUMP_HEIGHT_MUL = 0x12345695;
    constexpr uintptr_t OFF_GRAVITY_MUL     = 0x12345696;
    constexpr uintptr_t OFF_FLY_FLAG        = 0x12345697;
    constexpr uintptr_t OFF_NOCLIP_FLAG     = 0x12345698;
    constexpr uintptr_t OFF_GROUNDED_FLAG   = 0x12345699;
    constexpr uintptr_t OFF_AIRCTRL_FLAG    = 0x1234569A;
    constexpr uintptr_t OFF_GAME_MODE       = 0x1234569B;
    constexpr uintptr_t OFF_REMOTE_KILL     = 0x1234569C;
    constexpr uintptr_t OFF_VELOCITY_Y      = 0x1234569D;
    constexpr uintptr_t OFF_ENTITY_LIST     = 0x123456A0;
    constexpr uintptr_t OFF_ENTITY_COUNT    = 0x123456A1;
    constexpr uintptr_t OFF_VIEW_MATRIX     = 0x123456A2;
    constexpr uintptr_t OFF_VOTE_KICK       = 0x123456A5;

    constexpr size_t   ENT_MAX          = 64;
    constexpr uintptr_t ENT_OFF_POS    = 0x00;
    constexpr uintptr_t ENT_OFF_HEALTH = 0x0C;
    constexpr uintptr_t ENT_OFF_NAME   = 0x10;
    constexpr uintptr_t ENT_OFF_ALIVE  = 0x30;

    const float kTeleportSpots[][3] =
    {
        { 100.0f, 50.0f,  100.0f },
        {   0.0f, 50.0f,    0.0f },
        {-200.0f, 50.0f, -200.0f },
        { 200.0f, 50.0f, -200.0f },
        {   0.0f, 50.0f, -400.0f },
    };
    constexpr int kTeleportSpotCount = sizeof(kTeleportSpots) / sizeof(kTeleportSpots[0]);

    template <typename T>
    T ClampV(T v, T lo, T hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    void SendKey(WORD vk, bool down)
    {
        INPUT in{};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = vk;
        in.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(in));
    }

    void SendMouseMoveRel(LONG dx, LONG dy)
    {
        INPUT in{};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_MOVE;
        in.mi.dx = dx;
        in.mi.dy = dy;
        SendInput(1, &in, sizeof(in));
    }

    bool WorldToScreen(const float vm[16], const float pos[3], const ImVec2& disp, ImVec2& out)
    {
        const float w = vm[3] * pos[0] + vm[7] * pos[1] + vm[11] * pos[2] + vm[15];
        if (w < 0.1f)
            return false;
        const float x = vm[0] * pos[0] + vm[4] * pos[1] + vm[8]  * pos[2] + vm[12];
        const float y = vm[1] * pos[0] + vm[5] * pos[1] + vm[9]  * pos[2] + vm[13];
        out.x = disp.x * 0.5f * (1.0f + x / w);
        out.y = disp.y * 0.5f * (1.0f - y / w);
        return true;
    }
}

CHackManager::CHackManager() {
    m_pPointer = new CPointer();

    m_pPointer->AddPointer("PlayerHealth", CPointers::BASE_ADDRESS, { OFF_PLAYER_HEALTH }, 1);
    m_pPointer->AddPointer("PlayerAmmo", CPointers::BASE_ADDRESS, { OFF_PLAYER_AMMO }, 0);
    m_pPointer->AddPointer("PlayerPosition", CPointers::BASE_ADDRESS, { OFF_PLAYER_POSITION, 0x10, 0x20 }, 1);
    m_pPointer->AddPointer("PlayerName", CPointers::BASE_ADDRESS, { OFF_PLAYER_NAME, 0x10 }, 3);
    m_pPointer->AddPointer("WeaponDamage", CPointers::BASE_ADDRESS, { OFF_WEAPON_DAMAGE, 0x10, 0x20 }, 1);
    m_pPointer->AddPointer("FallDamage", CPointers::BASE_ADDRESS, { OFF_FALL_DAMAGE }, 1);
    m_pPointer->AddPointer("RespawnTime", CPointers::BASE_ADDRESS, { OFF_RESPAWN_TIME }, 1);

    m_pPointer->AddPointer("PlayerArmor", CPointers::BASE_ADDRESS, { OFF_PLAYER_ARMOR }, 1);
    m_pPointer->AddPointer("DeathCount", CPointers::BASE_ADDRESS, { OFF_DEATH_COUNT }, 0);
    m_pPointer->AddPointer("RecoilValue", CPointers::BASE_ADDRESS, { OFF_WEAPON_DAMAGE, 0x10 }, 1);
    m_pPointer->AddPointer("SpreadValue", CPointers::BASE_ADDRESS, { 0x12345681, 0x10 }, 1);
    m_pPointer->AddPointer("HeadShotMult", CPointers::BASE_ADDRESS, { OFF_HEADSHOT }, 1);
    m_pPointer->AddPointer("CharColor", CPointers::BASE_ADDRESS, { OFF_CHAR_COLOR }, 0);
    m_pPointer->AddPointer("CameraMode", CPointers::BASE_ADDRESS, { OFF_CAMERA_MODE }, 0);
    m_pPointer->AddPointer("ReviveLimit", CPointers::BASE_ADDRESS, { OFF_REVIVE_LIMIT }, 0);
    m_pPointer->AddPointer("ReviveTime", CPointers::BASE_ADDRESS, { OFF_REVIVE_TIME }, 1);

    m_pPointer->AddPointer("GhostFlag", CPointers::BASE_ADDRESS, { OFF_GHOST_FLAG }, 0);
    m_pPointer->AddPointer("SneakFlag", CPointers::BASE_ADDRESS, { OFF_SNEAK_FLAG }, 0);
    m_pPointer->AddPointer("TeamKillFlag", CPointers::BASE_ADDRESS, { OFF_TEAMKILL_FLAG }, 0);
    m_pPointer->AddPointer("FireRate", CPointers::BASE_ADDRESS, { OFF_FIRE_RATE }, 1);
    m_pPointer->AddPointer("PerfectMsFlag", CPointers::BASE_ADDRESS, { OFF_PERFECTMS_FLAG }, 0);
    m_pPointer->AddPointer("VipFlag", CPointers::BASE_ADDRESS, { OFF_VIP_FLAG }, 0);
    m_pPointer->AddPointer("PickupRadius", CPointers::BASE_ADDRESS, { OFF_PICKUP_RADIUS }, 1);
    m_pPointer->AddPointer("ReloadSpeed", CPointers::BASE_ADDRESS, { OFF_RELOAD_SPEED }, 1);
    m_pPointer->AddPointer("ToolDurability", CPointers::BASE_ADDRESS, { OFF_TOOL_DURABILITY }, 1);

    m_pPointer->AddPointer("MoveSpeedMul", CPointers::BASE_ADDRESS, { OFF_MOVE_SPEED_MUL }, 1);
    m_pPointer->AddPointer("JumpHeightMul", CPointers::BASE_ADDRESS, { OFF_JUMP_HEIGHT_MUL }, 1);
    m_pPointer->AddPointer("GravityMul", CPointers::BASE_ADDRESS, { OFF_GRAVITY_MUL }, 1);
    m_pPointer->AddPointer("FlyFlag", CPointers::BASE_ADDRESS, { OFF_FLY_FLAG }, 0);
    m_pPointer->AddPointer("NoclipFlag", CPointers::BASE_ADDRESS, { OFF_NOCLIP_FLAG }, 0);
    m_pPointer->AddPointer("GroundedFlag", CPointers::BASE_ADDRESS, { OFF_GROUNDED_FLAG }, 0);
    m_pPointer->AddPointer("AirControlFlag", CPointers::BASE_ADDRESS, { OFF_AIRCTRL_FLAG }, 0);
    m_pPointer->AddPointer("GameModeIndex", CPointers::BASE_ADDRESS, { OFF_GAME_MODE }, 0);
    m_pPointer->AddPointer("RemoteKillFlag", CPointers::BASE_ADDRESS, { OFF_REMOTE_KILL }, 0);
    m_pPointer->AddPointer("VelocityY", CPointers::BASE_ADDRESS, { OFF_VELOCITY_Y }, 1);

    m_pPointer->AddPointer("EntityList", CPointers::BASE_ADDRESS, { OFF_ENTITY_LIST }, 4);
    m_pPointer->AddPointer("EntityCount", CPointers::BASE_ADDRESS, { OFF_ENTITY_COUNT }, 0);
    m_pPointer->AddPointer("ViewMatrix", CPointers::BASE_ADDRESS, { OFF_VIEW_MATRIX }, 1);
    m_pPointer->AddPointer("VoteKickTarget", CPointers::BASE_ADDRESS, { OFF_VOTE_KICK }, 0);
}

CHackManager::~CHackManager() {
    if (m_pPointer) delete m_pPointer;
}

CHackManager& CHackManager::Get() {
    static CHackManager instance;
    return instance;
}

void CHackManager::SyncFromConfig(const Config& cfg) {
    static const float damageMults[] = { 1.0f, 1.5f, 2.0f, 5.0f };
    const int dmgIdx = ClampV(cfg.damageIndex, 0, 3);

    m_noRecoil = cfg.noRecoil;
    m_noSpread = cfg.noSpread;
    m_noFallDamage = cfg.noFallDamage;
    m_godMode = cfg.godMode;
    m_thirdPersonView = cfg.thirdPerson;
    m_unlimitedRev = cfg.unlimitedRevive;
    m_nameEsp = cfg.nameTags;
    m_crosshair = cfg.crosshair;
    m_damageHack = dmgIdx > 0;
    m_damageMultiplier = damageMults[dmgIdx];
    m_aimbotEnabled = cfg.aimbot;
    m_aimbotFOV = cfg.fov;
    m_aimbotSmooth = cfg.aimbotSmooth;
    m_infAmmo = cfg.infiniteAmmo;
    m_headShot = cfg.headShot;
    m_noWeaponSP = cfg.noWeaponSP;
    m_ammoType = ClampV(cfg.ammoType, 0, 3);
    m_instantRev = cfg.instantRespawn;
    m_earlyRespawn = cfg.earlyRespawn;
    m_charRed = cfg.charRed;
    m_charBlue = cfg.charBlue;
    m_baseReturn = cfg.baseReturn;

    m_noDeathCount = cfg.noDeathCount;
    m_ghostHack = cfg.ghostHack;
    m_remoteKill = cfg.remoteKill;
    m_sneakHack = cfg.sneakHack;
    m_teamKill = cfg.teamKill;
    m_modeIndex = ClampV(cfg.modeIndex, 0, 3);
    m_rapidFire = cfg.rapidFire;
    m_perfectMs = cfg.perfectMs;
    m_antiAfk = cfg.antiAfk;

    m_vipHack = cfg.vipHack;
    m_autoPickup = cfg.autoPickup;
    m_fastReload = cfg.fastReload;
    m_repairTools = cfg.repairTools;
    m_hpHack = ClampV(cfg.hpHack, 0, 100);
    m_armorValue = ClampV(cfg.armor, 0.0f, 100.0f);

    m_autoTeleport = cfg.autoTeleport;
    m_instantTeleport = cfg.instantTeleport;
    m_safeFall = cfg.safeFall;
    m_teleportLocation = ClampV(cfg.teleportLocation, 0, kTeleportSpotCount - 1);
    if (m_lastTeleportLocation < 0)
        m_lastTeleportLocation = m_teleportLocation;

    m_bunnyHop = cfg.bunnyHop;
    m_airControl = cfg.airControl;
    m_autoSprint = cfg.autoSprint;
    m_flyHack = cfg.flyHack;
    m_noclip = cfg.noclip;
    m_speedMultiplier = ClampV(cfg.speedMultiplier, 0.1f, 10.0f);
    m_jumpMultiplier = ClampV(cfg.jumpMultiplier, 0.1f, 10.0f);
    m_gravity = ClampV(cfg.gravity, 0.0f, 10.0f);

    m_boxEsp = cfg.boxEsp;
    m_skeletonEsp = cfg.skeletonEsp;
    m_healthBar = cfg.healthBar;
    m_distanceText = cfg.distanceText;
    m_boxStyle = ClampV(cfg.boxStyle, 0, 2);

    m_kickPlayer = ClampV(cfg.kickPlayer, 0, 63);
    m_autoKick = cfg.autoKick;
    m_kickCooldown = ClampV(cfg.kickCooldown, 1.0f, 300.0f);
}

void CHackManager::Update(bool allowInput) {
    ApplyGodMode();
    ApplyNoRecoil();
    ApplyNoFallDamage();
    ApplyNoSpread();
    ApplyHeadShot();
    ApplyDamageHack();
    ApplyEarlyRespawn();
    ApplyUnlimitedRev();
    ApplyInstantRev();
    ApplyCharRed();
    ApplyCharBlue();
    ApplyBaseReturn();
    ApplyThirdPersonView();

    ApplyNoDeathCount();
    ApplyGhostHack();
    ApplyRemoteKill();
    ApplySneakHack();
    ApplyTeamKill();
    ApplyModeHack();
    ApplyRapidFire();
    ApplyPerfectMs();

    ApplyVipHack();
    ApplyAutoPickup();
    ApplyFastReload();
    ApplyRepairTools();
    ApplyHpHack();
    ApplyArmorHack();

    ApplyMovement();

    RunTeleport();
    RunAutoKick();

    if (allowInput) {
        RunAimbot();
        RunBunnyHop();
        RunAutoSprint();
    }
    RunAntiAfk(allowInput);
}

bool CHackManager::ReadPlayerHealth(float& health) {
    return m_pPointer->ReadPointer("PlayerHealth", &health);
}

bool CHackManager::WritePlayerHealth(float health) {
    return m_pPointer->WritePointer("PlayerHealth", &health);
}

bool CHackManager::ReadPlayerAmmo(int& ammo) {
    return m_pPointer->ReadPointer("PlayerAmmo", &ammo);
}

bool CHackManager::WritePlayerAmmo(int ammo) {
    return m_pPointer->WritePointer("PlayerAmmo", &ammo);
}

bool CHackManager::ReadPlayerPosition(float& x, float& y, float& z) {
    float pos[3];
    bool success = m_pPointer->ReadPointer("PlayerPosition", pos);
    if (success) {
        x = pos[0];
        y = pos[1];
        z = pos[2];
    }
    return success;
}

bool CHackManager::WritePlayerPosition(float x, float y, float z) {
    float pos[3] = { x, y, z };
    return m_pPointer->WritePointer("PlayerPosition", pos);
}

bool CHackManager::ReadWeaponDamage(float& damage) {
    return m_pPointer->ReadPointer("WeaponDamage", &damage);
}

bool CHackManager::WriteWeaponDamage(float damage) {
    return m_pPointer->WritePointer("WeaponDamage", &damage);
}

bool CHackManager::ReadPlayerName(std::string& name) {
    char buffer[256];
    if (m_pPointer->ReadPointer("PlayerName", buffer)) {
        name = std::string(buffer);
        return true;
    }
    return false;
}

bool CHackManager::WritePlayerName(const std::string& name) {
    return m_pPointer->WritePointer("PlayerName", name.c_str());
}

bool CHackManager::ReadFallDamage(float& fallDamage) {
    return m_pPointer->ReadPointer("FallDamage", &fallDamage);
}

bool CHackManager::WriteFallDamage(float fallDamage) {
    return m_pPointer->WritePointer("FallDamage", &fallDamage);
}

bool CHackManager::ReadRespawnTime(float& time) {
    return m_pPointer->ReadPointer("RespawnTime", &time);
}

bool CHackManager::WriteRespawnTime(float time) {
    return m_pPointer->WritePointer("RespawnTime", &time);
}

bool CHackManager::ReadPlayerArmor(float& armor) {
    return m_pPointer->ReadPointer("PlayerArmor", &armor);
}

bool CHackManager::WritePlayerArmor(float armor) {
    return m_pPointer->WritePointer("PlayerArmor", &armor);
}

bool CHackManager::ReadDeathCount(int& count) {
    return m_pPointer->ReadPointer("DeathCount", &count);
}

bool CHackManager::WriteDeathCount(int count) {
    return m_pPointer->WritePointer("DeathCount", &count);
}

void CHackManager::ApplyFlag(const char* name, bool enabled) {
    bool& applied = m_appliedState[name];
    int value = enabled ? 1 : 0;
    if (enabled || applied) {
        m_pPointer->WritePointer(name, &value);
        applied = enabled;
    }
}

void CHackManager::ApplyScaled(const char* name, float value, float neutral) {
    bool& applied = m_appliedState[name];
    if (value != neutral || applied) {
        m_pPointer->WritePointer(name, &value);
        applied = (value != neutral);
    }
}

void CHackManager::ApplyGodMode() {
    if (m_godMode) {
        WritePlayerHealth(100.0f);
    }
}

void CHackManager::ApplyHpHack() {
    if (m_hpHack > 0 && !m_godMode) {
        WritePlayerHealth((float)m_hpHack);
    }
}

void CHackManager::ApplyArmorHack() {
    if (m_armorValue > 0.0f) {
        WritePlayerArmor(m_armorValue);
    }
}

void CHackManager::ApplyInfAmmo() {
    if (m_infAmmo) {
        WritePlayerAmmo(9999);
    }
}

void CHackManager::ApplyNoRecoil() {
    if (m_noRecoil) {
        float recoil = 0.0f;
        m_pPointer->WritePointer("RecoilValue", &recoil);
    }
}

void CHackManager::ApplyNoFallDamage() {
    if (m_noFallDamage) {
        WriteFallDamage(0.0f);
    }
}

void CHackManager::ApplyNoSpread() {
    if (m_noSpread) {
        float spread = 0.0f;
        m_pPointer->WritePointer("SpreadValue", &spread);
    }
}

void CHackManager::ApplyHeadShot() {
    if (m_headShot) {
        float multiplier = 2.0f;
        m_pPointer->WritePointer("HeadShotMult", &multiplier);
    }
}

void CHackManager::ApplyDamageHack() {
    if (m_damageHack) {
        WriteWeaponDamage(m_damageMultiplier);
    }
}

void CHackManager::ApplyEarlyRespawn() {
    if (m_earlyRespawn) {
        WriteRespawnTime(1.0f);
    }
}

void CHackManager::ApplyUnlimitedRev() {
    if (m_unlimitedRev) {
        int limit = 9999;
        m_pPointer->WritePointer("ReviveLimit", &limit);
    }
}

void CHackManager::ApplyInstantRev() {
    if (m_instantRev) {
        float time = 0.0f;
        m_pPointer->WritePointer("ReviveTime", &time);
    }
}

void CHackManager::ApplyCharRed() {
    if (m_charRed) {
        int color = 0xFF0000;
        m_pPointer->WritePointer("CharColor", &color);
    }
}

void CHackManager::ApplyCharBlue() {
    if (m_charBlue) {
        int color = 0x0000FF;
        m_pPointer->WritePointer("CharColor", &color);
    }
}

void CHackManager::ApplyBaseReturn() {
    if (m_baseReturn) {
        WritePlayerPosition(kTeleportSpots[0][0], kTeleportSpots[0][1], kTeleportSpots[0][2]);
    }
}

void CHackManager::ApplyThirdPersonView() {
    ApplyFlag("CameraMode", m_thirdPersonView);
}

void CHackManager::ApplyNoDeathCount() {
    if (m_noDeathCount) {
        WriteDeathCount(0);
    }
}

void CHackManager::ApplyGhostHack() {
    ApplyFlag("GhostFlag", m_ghostHack);
}

void CHackManager::ApplyRemoteKill() {
    ApplyFlag("RemoteKillFlag", m_remoteKill);
}

void CHackManager::ApplySneakHack() {
    ApplyFlag("SneakFlag", m_sneakHack);
}

void CHackManager::ApplyTeamKill() {
    ApplyFlag("TeamKillFlag", m_teamKill);
}

void CHackManager::ApplyModeHack() {
    if (m_modeIndex > 0) {
        m_pPointer->WritePointer("GameModeIndex", &m_modeIndex);
    }
}

void CHackManager::ApplyRapidFire() {
    if (m_rapidFire) {
        float rate = 0.01f;
        m_pPointer->WritePointer("FireRate", &rate);
    }
}

void CHackManager::ApplyPerfectMs() {
    ApplyFlag("PerfectMsFlag", m_perfectMs);
}

void CHackManager::ApplyVipHack() {
    ApplyFlag("VipFlag", m_vipHack);
}

void CHackManager::ApplyAutoPickup() {
    if (m_autoPickup) {
        float radius = 9999.0f;
        m_pPointer->WritePointer("PickupRadius", &radius);
    }
}

void CHackManager::ApplyFastReload() {
    if (m_fastReload) {
        float speed = 10.0f;
        m_pPointer->WritePointer("ReloadSpeed", &speed);
    }
}

void CHackManager::ApplyRepairTools() {
    if (m_repairTools) {
        float durability = 100.0f;
        m_pPointer->WritePointer("ToolDurability", &durability);
    }
}

void CHackManager::ApplyMovement() {
    ApplyScaled("MoveSpeedMul", m_speedMultiplier, 1.0f);
    ApplyScaled("JumpHeightMul", m_jumpMultiplier, 1.0f);
    ApplyScaled("GravityMul", m_gravity, 1.0f);
    ApplyFlag("FlyFlag", m_flyHack);
    ApplyFlag("NoclipFlag", m_noclip);
    ApplyFlag("AirControlFlag", m_airControl);
}

bool CHackManager::GetEntities(std::vector<EntityInfo>& out) {
    out.clear();
    if (!m_pPointer)
        return false;

    const uintptr_t listAddress = m_pPointer->GetAddress("EntityList");
    if (!listAddress)
        return false;

    uintptr_t arrayBase = 0;
    if (!m_pPointer->ReadRaw(listAddress, &arrayBase, sizeof(arrayBase)) || !arrayBase)
        return false;

    int count = 0;
    if (!m_pPointer->ReadPointer("EntityCount", &count) || count <= 0)
        return false;
    if (count > (int)ENT_MAX)
        count = (int)ENT_MAX;

    out.reserve(count);
    for (int i = 0; i < count; ++i) {
        uintptr_t entity = 0;
        if (!m_pPointer->ReadRaw(arrayBase + i * sizeof(uintptr_t), &entity, sizeof(entity)) || !entity)
            continue;

        EntityInfo e{};
        if (!m_pPointer->ReadRaw(entity + ENT_OFF_POS, e.position, sizeof(e.position)))
            continue;

        BYTE alive = 0;
        m_pPointer->ReadRaw(entity + ENT_OFF_HEALTH, &e.health, sizeof(e.health));
        m_pPointer->ReadRaw(entity + ENT_OFF_ALIVE, &alive, sizeof(alive));
        m_pPointer->ReadRaw(entity + ENT_OFF_NAME, e.name, sizeof(e.name) - 1);

        e.alive = alive != 0 && e.health > 0.0f;
        if (e.alive)
            out.push_back(e);
    }
    return !out.empty();
}

void CHackManager::RunAimbot() {
    if (!m_aimbotEnabled || m_aimbotSmooth < 1.0f)
        return;
    if (m_screenW <= 1.0f || m_screenH <= 1.0f)
        return;

    std::vector<EntityInfo> entities;
    if (!GetEntities(entities))
        return;

    float vm[16]{};
    if (!m_pPointer->ReadRaw(m_pPointer->GetAddress("ViewMatrix"), vm, sizeof(vm)))
        return;

    const ImVec2 disp(m_screenW, m_screenH);
    const ImVec2 center(disp.x * 0.5f, disp.y * 0.5f);

    const EntityInfo* best = nullptr;
    ImVec2 bestScr(center.x, center.y);
    float bestDist = m_aimbotFOV;

    for (const EntityInfo& e : entities) {
        float headPos[3] = { e.position[0], e.position[1] + 1.4f, e.position[2] };
        ImVec2 scr;
        if (!WorldToScreen(vm, headPos, disp, scr))
            continue;
        const float dx = scr.x - center.x;
        const float dy = scr.y - center.y;
        const float dist = sqrtf(dx * dx + dy * dy);
        if (dist <= bestDist) {
            bestDist = dist;
            best = &e;
            bestScr = scr;
        }
    }

    if (!best)
        return;

    const float step = 1.0f / m_aimbotSmooth;
    const LONG moveX = (LONG)((bestScr.x - center.x) * step);
    const LONG moveY = (LONG)((bestScr.y - center.y) * step);
    if (moveX != 0 || moveY != 0)
        SendMouseMoveRel(moveX, moveY);
}

void CHackManager::RunAntiAfk(bool allowInput) {
    if (!m_antiAfk || !allowInput)
        return;

    const DWORD now = GetTickCount();
    if (now - m_lastAntiAfkTick < 25000)
        return;
    m_lastAntiAfkTick = now;

    const LONG dir = m_antiAfkFlip ? -1 : 1;
    m_antiAfkFlip = !m_antiAfkFlip;
    SendMouseMoveRel(dir, 0);
    SendMouseMoveRel(-dir, 0);
}

void CHackManager::RunBunnyHop() {
    if (!m_bunnyHop)
        return;
    if (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
        return;

    int grounded = 0;
    const bool ok = m_pPointer->ReadPointer("GroundedFlag", &grounded);
    if (!ok || grounded) {
        SendKey(VK_SPACE, TRUE);
        SendKey(VK_SPACE, FALSE);
    }
}

void CHackManager::RunAutoSprint() {
    const bool walking = (GetAsyncKeyState('W') & 0x8000) != 0;
    if (m_autoSprint && walking && !m_sprintHeld) {
        SendKey(VK_SHIFT, TRUE);
        m_sprintHeld = true;
    }
    else if ((!m_autoSprint || !walking) && m_sprintHeld) {
        SendKey(VK_SHIFT, FALSE);
        m_sprintHeld = false;
    }
}

void CHackManager::RequestTeleport() {
    m_teleportNow = true;
}

void CHackManager::RequestKick(int playerIndex) {
    m_kickTarget = ClampV(playerIndex, 0, 63);
    m_kickPending = true;
}

void CHackManager::RunTeleport() {
    const DWORD now = GetTickCount();
    const int idx = ClampV(m_teleportLocation, 0, kTeleportSpotCount - 1);
    const bool changed = (idx != m_lastTeleportLocation);

    bool go = false;
    if (m_teleportNow)
        go = true;
    else if (changed && (m_instantTeleport || m_autoTeleport))
        go = true;
    else if (m_autoTeleport && now - m_lastTeleportTick >= 2000)
        go = true;

    if (go) {
        WritePlayerPosition(kTeleportSpots[idx][0], kTeleportSpots[idx][1], kTeleportSpots[idx][2]);
        m_lastTeleportTick = now;
    }

    m_lastTeleportLocation = idx;
    m_teleportNow = false;

    if (m_safeFall) {
        float vy = 0.0f;
        if (m_pPointer->ReadPointer("VelocityY", &vy) && vy < -30.0f)
            WriteFallDamage(0.0f);
    }
}

void CHackManager::RunAutoKick() {
    const DWORD now = GetTickCount();
    const DWORD cooldownMs = (DWORD)(m_kickCooldown * 1000.0f);
    const bool due = m_autoKick && (now - m_lastKickTick >= cooldownMs);

    if (!m_kickPending && !due)
        return;
    if (m_autoKick && !m_kickPending)
        m_kickTarget = m_kickPlayer;

    m_pPointer->WritePointer("VoteKickTarget", &m_kickTarget);
    m_kickPending = false;
    m_lastKickTick = now;
}

void CHackManager::DrawOverlay() {
    if (!m_pPointer)
        return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl)
        return;

    const ImVec2 disp = ImGui::GetIO().DisplaySize;

    if (m_crosshair && disp.x > 0.0f) {
        const float cx = disp.x * 0.5f;
        const float cy = disp.y * 0.5f;
        const float gap = 4.0f;
        const float len = 8.0f;
        const ImU32 col = IM_COL32(240, 240, 240, 220);
        dl->AddLine(ImVec2(cx - gap - len, cy), ImVec2(cx - gap, cy), col, 1.3f);
        dl->AddLine(ImVec2(cx + gap, cy), ImVec2(cx + gap + len, cy), col, 1.3f);
        dl->AddLine(ImVec2(cx, cy - gap - len), ImVec2(cx, cy - gap), col, 1.3f);
        dl->AddLine(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + len), col, 1.3f);
        dl->AddCircleFilled(ImVec2(cx, cy), 1.2f, col);
    }

    if (m_aimbotEnabled && m_aimbotFOV > 0.0f && disp.x > 0.0f) {
        dl->AddCircle(ImVec2(disp.x * 0.5f, disp.y * 0.5f), m_aimbotFOV,
            IM_COL32(212, 175, 55, 120), 64, 1.2f);
    }

    const bool espWanted = m_boxEsp || m_skeletonEsp || m_healthBar ||
        m_distanceText || m_nameEsp;
    if (!espWanted || disp.x <= 0.0f || disp.y <= 0.0f)
        return;

    std::vector<EntityInfo> entities;
    if (!GetEntities(entities))
        return;

    float vm[16]{};
    if (!m_pPointer->ReadRaw(m_pPointer->GetAddress("ViewMatrix"), vm, sizeof(vm)))
        return;

    float localPos[3] = { 0.0f, 0.0f, 0.0f };
    const bool haveLocal = ReadPlayerPosition(localPos[0], localPos[1], localPos[2]);

    for (const EntityInfo& e : entities) {
        ImVec2 feet, headTop;
        float topPos[3] = { e.position[0], e.position[1] + 1.8f, e.position[2] };
        if (!WorldToScreen(vm, e.position, disp, feet) ||
            !WorldToScreen(vm, topPos, disp, headTop))
            continue;

        const float boxH = fabsf(feet.y - headTop.y);
        if (boxH < 2.0f)
            continue;
        const float boxW = boxH * 0.45f;
        const ImVec2 mn(feet.x - boxW * 0.5f, headTop.y);
        const ImVec2 mx(feet.x + boxW * 0.5f, feet.y);

        const ImU32 boxCol = IM_COL32(255, 85, 85, 230);

        if (m_boxEsp) {
            if (m_boxStyle == 0) {
                const float c = boxW * 0.3f;
                dl->AddLine(mn, ImVec2(mn.x + c, mn.y), boxCol, 1.2f);
                dl->AddLine(mn, ImVec2(mn.x, mn.y + c), boxCol, 1.2f);
                dl->AddLine(ImVec2(mx.x - c, mn.y), ImVec2(mx.x, mn.y), boxCol, 1.2f);
                dl->AddLine(ImVec2(mx.x, mn.y), ImVec2(mx.x, mn.y + c), boxCol, 1.2f);
                dl->AddLine(ImVec2(mn.x, mx.y), ImVec2(mn.x + c, mx.y), boxCol, 1.2f);
                dl->AddLine(ImVec2(mn.x, mx.y - c), ImVec2(mn.x, mx.y), boxCol, 1.2f);
                dl->AddLine(ImVec2(mx.x - c, mx.y), mx, boxCol, 1.2f);
                dl->AddLine(ImVec2(mx.x, mx.y - c), mx, boxCol, 1.2f);
            }
            else {
                dl->AddRect(mn, mx, boxCol, 0.0f, 0, m_boxStyle == 2 ? 1.0f : 1.5f);
            }
        }

        if (m_skeletonEsp) {
            const ImU32 skCol = IM_COL32(255, 170, 170, 200);
            const float midY = mn.y + boxH * 0.5f;
            const float shoulderY = mn.y + boxH * 0.22f;
            const ImVec2 mid((mn.x + mx.x) * 0.5f, midY);
            const ImVec2 neck((mn.x + mx.x) * 0.5f, shoulderY);
            dl->AddLine(neck, mid, skCol, 1.2f);
            dl->AddLine(neck, ImVec2(mn.x, shoulderY + boxH * 0.15f), skCol, 1.2f);
            dl->AddLine(neck, ImVec2(mx.x, shoulderY + boxH * 0.15f), skCol, 1.2f);
            dl->AddLine(mid, ImVec2(mn.x, mx.y), skCol, 1.2f);
            dl->AddLine(mid, ImVec2(mx.x, mx.y), skCol, 1.2f);
        }

        if (m_healthBar) {
            const float frac = ClampV(e.health / 100.0f, 0.0f, 1.0f);
            const ImU32 hpCol = IM_COL32(
                (ImU8)(255 * (1.0f - frac)),
                (ImU8)(220 * frac),
                40, 255);
            const float barX = mn.x - 5.0f;
            dl->AddRectFilled(ImVec2(barX, mn.y), ImVec2(barX + 2.5f, mx.y),
                IM_COL32(20, 20, 20, 180));
            dl->AddRectFilled(ImVec2(barX, mx.y - (mx.y - mn.y) * frac),
                ImVec2(barX + 2.5f, mx.y), hpCol);
        }

        char label[96];
        label[0] = '\0';

        if (m_nameEsp && e.name[0])
            _snprintf_s(label, sizeof(label), _TRUNCATE, "%s", e.name);

        if (m_distanceText && haveLocal) {
            const float ddx = e.position[0] - localPos[0];
            const float ddy = e.position[1] - localPos[1];
            const float ddz = e.position[2] - localPos[2];
            const float distM = sqrtf(ddx * ddx + ddy * ddy + ddz * ddz) * 0.01f;
            char distBuf[32];
            _snprintf_s(distBuf, sizeof(distBuf), _TRUNCATE, "[%.0fm]", distM);
            if (label[0])
                strcat_s(label, " ");
            strcat_s(label, distBuf);
        }

        if (label[0]) {
            const ImVec2 ts = ImGui::CalcTextSize(label);
            dl->AddText(ImVec2((mn.x + mx.x - ts.x) * 0.5f, mn.y - ts.y - 2.0f),
                IM_COL32(245, 246, 248, 235), label);
        }
    }
}
