#pragma once
#include <string>
#include <map>
#include <vector>
#include "Pointer.h"

struct Config;

class CHackManager
{
public:
    static CHackManager& Get();

    void SyncFromConfig(const Config& cfg);
    void Update();

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
    float m_damageMultiplier = 10.0f;
};
