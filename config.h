#pragma once

struct Config
{
    bool noDeathCount = false;
    bool noRecoil = false;
    bool noSpread = false;
    bool noFallDamage = false;
    bool ghostHack = false;
    bool godMode = false;
    bool remoteKill = false;
    int damageIndex = 1;
    int modeIndex = 0;
    bool thirdPerson = false;
    bool sneakHack = false;
    bool unlimitedRevive = false;
    float fov = 90.0f;
    bool aimbot = false;
    float aimbotSmooth = 50.0f;
    bool crosshair = false;

    bool boxEsp = true;
    bool skeletonEsp = false;
    bool healthBar = false;
    bool distanceText = false;
    bool nameTags = false;
    int boxStyle = 0;

    bool autoTeleport = false;
    bool instantTeleport = false;
    bool safeFall = true;
    int teleportLocation = 0;

    bool infiniteAmmo = false;
    bool vipHack = false;
    bool rapidFire = false;
    bool perfectMs = false;
    bool instantRespawn = false;
    bool earlyRespawn = false;
    bool teamKill = false;
    bool infiniteWolfBalls = false;
    int wolfMode = 0;
    int hpHack = 10;
    float armor = 50.0f;
    bool autoPickup = false;
    bool fastReload = false;
    bool repairTools = false;
    bool headShot = false;
    bool noWeaponSP = false;
    int ammoType = 0;

    bool bunnyHop = false;
    bool airControl = false;
    bool autoSprint = true;
    bool flyHack = false;
    bool noclip = false;
    float speedMultiplier = 1.00f;
    float jumpMultiplier = 1.00f;
    float gravity = 1.00f;

    bool antiAfk = false;
    bool streamProof = false;
    bool watermark = false;
    float opacity = 1.00f;
    bool charRed = false;
    bool charBlue = false;
    bool baseReturn = false;

    int kickPlayer = 0;
    bool autoKick = false;
    float kickCooldown = 15.0f;
};

extern Config g_config;
