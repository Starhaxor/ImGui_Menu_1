#include "pch.h"
#include "HackManager.h"
#include "Pointers.h"
#include "config.h"
#include <string.h>

CHackManager::CHackManager() {
    m_pPointer = new CPointer();

    m_pPointer->AddPointer("PlayerHealth", CPointers::BASE_ADDRESS, { 0x12345678 }, 1);
    m_pPointer->AddPointer("PlayerAmmo", CPointers::BASE_ADDRESS, { 0x12345679 }, 0);
    m_pPointer->AddPointer("PlayerPosition", CPointers::BASE_ADDRESS, { 0x1234567A, 0x10, 0x20 }, 1);
    m_pPointer->AddPointer("PlayerName", CPointers::BASE_ADDRESS, { 0x1234567D, 0x10 }, 3);
    m_pPointer->AddPointer("WeaponDamage", CPointers::BASE_ADDRESS, { 0x1234567F, 0x10, 0x20 }, 1);
    m_pPointer->AddPointer("FallDamage", CPointers::BASE_ADDRESS, { 0x12345687 }, 1);
    m_pPointer->AddPointer("RespawnTime", CPointers::BASE_ADDRESS, { 0x12345686 }, 1);
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

    m_noRecoil = cfg.noRecoil;
    m_noSpread = cfg.noSpread;
    m_noFallDamage = cfg.noFallDamage;
    m_godMode = cfg.godMode;
    m_thirdPersonView = cfg.thirdPerson;
    m_unlimitedRev = cfg.unlimitedRevive;
    m_nameEsp = cfg.nameTags;
    m_crosshair = cfg.crosshair;
    m_damageHack = cfg.damageIndex > 0;
    m_damageMultiplier = 10.0f * damageMults[cfg.damageIndex];
    m_aimbotEnabled = cfg.aimbot;
    m_aimbotFOV = cfg.fov;
    m_aimbotSmooth = cfg.aimbotSmooth;
    m_infAmmo = cfg.infiniteAmmo;
    m_headShot = cfg.headShot;
    m_noWeaponSP = cfg.noWeaponSP;
    m_ammoType = cfg.ammoType;
    m_instantRev = cfg.instantRespawn;
    m_earlyRespawn = cfg.earlyRespawn;
    m_charRed = cfg.charRed;
    m_charBlue = cfg.charBlue;
    m_baseReturn = cfg.baseReturn;
}

void CHackManager::Update() {
    ApplyGodMode();
    ApplyInfAmmo();
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

void CHackManager::ApplyGodMode() {
    if (m_godMode) {
        WritePlayerHealth(100.0f);
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
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x1234567F, 0x10 }, &recoil, 1);
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
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x12345681, 0x10 }, &spread, 1);
    }
}

void CHackManager::ApplyHeadShot() {
    if (m_headShot) {
        float multiplier = 2.0f;
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x1234567F, 0x10 }, &multiplier, 1);
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
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x12345685 }, &limit, 0);
    }
}

void CHackManager::ApplyInstantRev() {
    if (m_instantRev) {
        float time = 0.0f;
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x12345685 }, &time, 1);
    }
}

void CHackManager::ApplyCharRed() {
    if (m_charRed) {
        int color = 0xFF0000;
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x1234567E }, &color, 0);
    }
}

void CHackManager::ApplyCharBlue() {
    if (m_charBlue) {
        int color = 0x0000FF;
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x1234567E }, &color, 0);
    }
}

void CHackManager::ApplyBaseReturn() {
    if (m_baseReturn) {
        float basePos[3] = { 100.0f, 50.0f, 100.0f };
        WritePlayerPosition(basePos[0], basePos[1], basePos[2]);
    }
}

void CHackManager::ApplyThirdPersonView() {
    if (m_thirdPersonView) {
        int cameraMode = 1;
        m_pPointer->WritePointerChain(CPointers::BASE_ADDRESS, { 0x12345684 }, &cameraMode, 0);
    }
}
