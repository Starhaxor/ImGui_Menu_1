#pragma once
#include <string>
#include <map>
#include <vector>
#include "Pointer.h"

struct Config;

struct EntityInfo
{
    float position[3];
    float health;
    bool alive;
    char name[32];
};

class CHackManager
{
public:
    static CHackManager& Get();

    void SyncFromConfig(const Config& cfg);
    void Update(bool allowInput);
    void SetScreenSize(float width, float height) { m_screenW = width; m_screenH = height; }

    bool IsNameEspEnabled() const { return m_nameEsp; }
    void SetNameEsp(bool enabled) { m_nameEsp = enabled; }

    bool IsEarlyRespawnEnabled() const { return m_earlyRespawn; }
    void SetEarlyRespawn(bool enabled) { m_earlyRespawn = enabled; }

    bool IsNoFallDamageEnabled() const { return m_noFallDamage; }
    void SetNoFallDamage(bool enabled) { m_noFallDamage = enabled; }

    bool IsUnlimitedRevEnabled() const { return m_unlimitedRev; }
    void SetUnlimitedRev(bool enabled) { m_unlimitedRev = enabled; }

    bool IsInstantRevEnabled() const { return m_instantRev; }
    void SetInstantRev(bool enabled) { m_instantRev = enabled; }

    bool IsCharRedEnabled() const { return m_charRed; }
    void SetCharRed(bool enabled) { m_charRed = enabled; }

    bool IsCharBlueEnabled() const { return m_charBlue; }
    void SetCharBlue(bool enabled) { m_charBlue = enabled; }

    bool IsBaseReturnEnabled() const { return m_baseReturn; }
    void SetBaseReturn(bool enabled) { m_baseReturn = enabled; }

    bool IsThirdPersonViewEnabled() const { return m_thirdPersonView; }
    void SetThirdPersonView(bool enabled) { m_thirdPersonView = enabled; }

    bool IsInfAmmoEnabled() const { return m_infAmmo; }
    void SetInfAmmo(bool enabled) { m_infAmmo = enabled; }

    bool IsNoRecoilEnabled() const { return m_noRecoil; }
    void SetNoRecoil(bool enabled) { m_noRecoil = enabled; }

    bool IsNoSpreadEnabled() const { return m_noSpread; }
    void SetNoSpread(bool enabled) { m_noSpread = enabled; }

    bool IsNoWeaponSPEnabled() const { return m_noWeaponSP; }
    void SetNoWeaponSP(bool enabled) { m_noWeaponSP = enabled; }

    int GetAmmoType() const { return m_ammoType; }
    void SetAmmoType(int type) { m_ammoType = type; }

    bool IsHeadShotEnabled() const { return m_headShot; }
    void SetHeadShot(bool enabled) { m_headShot = enabled; }

    bool IsDamageHackEnabled() const { return m_damageHack; }
    void SetDamageHack(bool enabled) { m_damageHack = enabled; }

    bool IsCrosshairEnabled() const { return m_crosshair; }
    void SetCrosshair(bool enabled) { m_crosshair = enabled; }

    bool IsGodModeEnabled() const { return m_godMode; }
    void SetGodMode(bool enabled) { m_godMode = enabled; }

    bool IsAimbotEnabled() const { return m_aimbotEnabled; }
    void SetAimbotEnabled(bool enabled) { m_aimbotEnabled = enabled; }

    float GetAimbotFOV() const { return m_aimbotFOV; }
    void SetAimbotFOV(float fov) { m_aimbotFOV = fov; }

    float GetAimbotSmooth() const { return m_aimbotSmooth; }
    void SetAimbotSmooth(float smooth) { m_aimbotSmooth = smooth; }

    bool IsNoDeathCountEnabled() const { return m_noDeathCount; }
    void SetNoDeathCount(bool enabled) { m_noDeathCount = enabled; }

    bool IsGhostHackEnabled() const { return m_ghostHack; }
    void SetGhostHack(bool enabled) { m_ghostHack = enabled; }

    bool IsRemoteKillEnabled() const { return m_remoteKill; }
    void SetRemoteKill(bool enabled) { m_remoteKill = enabled; }

    bool IsSneakHackEnabled() const { return m_sneakHack; }
    void SetSneakHack(bool enabled) { m_sneakHack = enabled; }

    bool IsTeamKillEnabled() const { return m_teamKill; }
    void SetTeamKill(bool enabled) { m_teamKill = enabled; }

    int GetModeIndex() const { return m_modeIndex; }
    void SetModeIndex(int mode) { m_modeIndex = mode; }

    bool IsRapidFireEnabled() const { return m_rapidFire; }
    void SetRapidFire(bool enabled) { m_rapidFire = enabled; }

    bool IsPerfectMsEnabled() const { return m_perfectMs; }
    void SetPerfectMs(bool enabled) { m_perfectMs = enabled; }

    bool IsAntiAfkEnabled() const { return m_antiAfk; }
    void SetAntiAfk(bool enabled) { m_antiAfk = enabled; }

    bool IsVipHackEnabled() const { return m_vipHack; }
    void SetVipHack(bool enabled) { m_vipHack = enabled; }

    bool IsAutoPickupEnabled() const { return m_autoPickup; }
    void SetAutoPickup(bool enabled) { m_autoPickup = enabled; }

    bool IsFastReloadEnabled() const { return m_fastReload; }
    void SetFastReload(bool enabled) { m_fastReload = enabled; }

    bool IsRepairToolsEnabled() const { return m_repairTools; }
    void SetRepairTools(bool enabled) { m_repairTools = enabled; }

    int GetHpHack() const { return m_hpHack; }
    void SetHpHack(int hp) { m_hpHack = hp; }

    float GetArmorValue() const { return m_armorValue; }
    void SetArmorValue(float armor) { m_armorValue = armor; }

    bool IsAutoTeleportEnabled() const { return m_autoTeleport; }
    void SetAutoTeleport(bool enabled) { m_autoTeleport = enabled; }

    bool IsInstantTeleportEnabled() const { return m_instantTeleport; }
    void SetInstantTeleport(bool enabled) { m_instantTeleport = enabled; }

    bool IsSafeFallEnabled() const { return m_safeFall; }
    void SetSafeFall(bool enabled) { m_safeFall = enabled; }

    int GetTeleportLocation() const { return m_teleportLocation; }
    void SetTeleportLocation(int location) { m_teleportLocation = location; }

    bool IsBunnyHopEnabled() const { return m_bunnyHop; }
    void SetBunnyHop(bool enabled) { m_bunnyHop = enabled; }

    bool IsAirControlEnabled() const { return m_airControl; }
    void SetAirControl(bool enabled) { m_airControl = enabled; }

    bool IsAutoSprintEnabled() const { return m_autoSprint; }
    void SetAutoSprint(bool enabled) { m_autoSprint = enabled; }

    bool IsFlyHackEnabled() const { return m_flyHack; }
    void SetFlyHack(bool enabled) { m_flyHack = enabled; }

    bool IsNoclipEnabled() const { return m_noclip; }
    void SetNoclip(bool enabled) { m_noclip = enabled; }

    float GetSpeedMultiplier() const { return m_speedMultiplier; }
    void SetSpeedMultiplier(float value) { m_speedMultiplier = value; }

    float GetJumpMultiplier() const { return m_jumpMultiplier; }
    void SetJumpMultiplier(float value) { m_jumpMultiplier = value; }

    float GetGravity() const { return m_gravity; }
    void SetGravity(float value) { m_gravity = value; }

    bool IsBoxEspEnabled() const { return m_boxEsp; }
    void SetBoxEsp(bool enabled) { m_boxEsp = enabled; }

    bool IsSkeletonEspEnabled() const { return m_skeletonEsp; }
    void SetSkeletonEsp(bool enabled) { m_skeletonEsp = enabled; }

    bool IsHealthBarEnabled() const { return m_healthBar; }
    void SetHealthBar(bool enabled) { m_healthBar = enabled; }

    bool IsDistanceTextEnabled() const { return m_distanceText; }
    void SetDistanceText(bool enabled) { m_distanceText = enabled; }

    int GetBoxStyle() const { return m_boxStyle; }
    void SetBoxStyle(int style) { m_boxStyle = style; }

    int GetKickPlayer() const { return m_kickPlayer; }
    void SetKickPlayer(int player) { m_kickPlayer = player; }

    bool IsAutoKickEnabled() const { return m_autoKick; }
    void SetAutoKick(bool enabled) { m_autoKick = enabled; }

    float GetKickCooldown() const { return m_kickCooldown; }
    void SetKickCooldown(float seconds) { m_kickCooldown = seconds; }

    bool ReadPlayerHealth(float& health);
    bool WritePlayerHealth(float health);

    bool ReadPlayerAmmo(int& ammo);
    bool WritePlayerAmmo(int ammo);

    bool ReadPlayerPosition(float& x, float& y, float& z);
    bool WritePlayerPosition(float x, float y, float z);

    bool ReadWeaponDamage(float& damage);
    bool WriteWeaponDamage(float damage);

    bool ReadPlayerName(std::string& name);
    bool WritePlayerName(const std::string& name);

    bool ReadFallDamage(float& fallDamage);
    bool WriteFallDamage(float fallDamage);

    bool ReadRespawnTime(float& time);
    bool WriteRespawnTime(float time);

    bool ReadPlayerArmor(float& armor);
    bool WritePlayerArmor(float armor);

    bool ReadDeathCount(int& count);
    bool WriteDeathCount(int count);

    void ApplyGodMode();
    void ApplyInfAmmo();
    void ApplyNoRecoil();
    void ApplyNoFallDamage();
    void ApplyNoSpread();
    void ApplyHeadShot();
    void ApplyDamageHack();
    void ApplyEarlyRespawn();
    void ApplyUnlimitedRev();
    void ApplyInstantRev();
    void ApplyCharRed();
    void ApplyCharBlue();
    void ApplyBaseReturn();
    void ApplyThirdPersonView();

    void ApplyNoDeathCount();
    void ApplyGhostHack();
    void ApplyRemoteKill();
    void ApplySneakHack();
    void ApplyTeamKill();
    void ApplyModeHack();
    void ApplyRapidFire();
    void ApplyPerfectMs();
    void ApplyVipHack();
    void ApplyAutoPickup();
    void ApplyFastReload();
    void ApplyRepairTools();
    void ApplyHpHack();
    void ApplyArmorHack();
    void ApplyMovement();

    void RunAimbot();
    void RunAntiAfk(bool allowInput);
    void RunBunnyHop();
    void RunAutoSprint();
    void RunTeleport();
    void RunAutoKick();

    void RequestTeleport();
    void RequestKick(int playerIndex);
    void DrawOverlay();

private:
    CHackManager();
    ~CHackManager();
    CHackManager(const CHackManager&) = delete;
    CHackManager& operator=(const CHackManager&) = delete;

    CPointer* m_pPointer = nullptr;

    bool m_nameEsp = false;
    bool m_earlyRespawn = false;
    bool m_noFallDamage = false;
    bool m_unlimitedRev = false;
    bool m_instantRev = false;
    bool m_charRed = false;
    bool m_charBlue = false;
    bool m_baseReturn = false;
    bool m_thirdPersonView = false;

    bool m_infAmmo = false;
    bool m_noRecoil = false;
    bool m_noSpread = false;
    bool m_noWeaponSP = false;
    int m_ammoType = 0;
    bool m_headShot = false;
    bool m_damageHack = false;
    bool m_crosshair = false;

    bool m_godMode = false;

    bool m_aimbotEnabled = false;
    float m_aimbotFOV = 90.0f;
    float m_aimbotSmooth = 50.0f;
    float m_damageMultiplier = 1.0f;

    bool m_noDeathCount = false;
    bool m_ghostHack = false;
    bool m_remoteKill = false;
    bool m_sneakHack = false;
    bool m_teamKill = false;
    int m_modeIndex = 0;
    bool m_rapidFire = false;
    bool m_perfectMs = false;
    bool m_antiAfk = false;

    bool m_vipHack = false;
    bool m_autoPickup = false;
    bool m_fastReload = false;
    bool m_repairTools = false;
    int m_hpHack = 0;
    float m_armorValue = 0.0f;

    bool m_autoTeleport = false;
    bool m_instantTeleport = false;
    bool m_safeFall = false;
    int m_teleportLocation = 0;

    bool m_bunnyHop = false;
    bool m_airControl = false;
    bool m_autoSprint = false;
    bool m_flyHack = false;
    bool m_noclip = false;
    float m_speedMultiplier = 1.0f;
    float m_jumpMultiplier = 1.0f;
    float m_gravity = 1.0f;

    bool m_boxEsp = false;
    bool m_skeletonEsp = false;
    bool m_healthBar = false;
    bool m_distanceText = false;
    int m_boxStyle = 0;

    int m_kickPlayer = 0;
    bool m_autoKick = false;
    float m_kickCooldown = 15.0f;

    float m_screenW = 0.0f;
    float m_screenH = 0.0f;

    DWORD m_lastAntiAfkTick = 0;
    bool m_antiAfkFlip = false;

    DWORD m_lastKickTick = 0;
    bool m_kickPending = false;
    int m_kickTarget = -1;

    int m_lastTeleportLocation = -1;
    DWORD m_lastTeleportTick = 0;
    bool m_teleportNow = false;

    bool m_sprintHeld = false;

    std::map<std::string, bool> m_appliedState;

    bool GetEntities(std::vector<EntityInfo>& out);
    void ApplyFlag(const char* name, bool enabled);
    void ApplyScaled(const char* name, float value, float neutral);
};
