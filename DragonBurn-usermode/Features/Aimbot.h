#pragma once
#define _USE_MATH_DEFINES
#define MAXV 10000e9
#include <math.h>
#include <thread>
#include <chrono>
#include "..\Game\Game.h"
#include "..\Game\Entity.h"
#include "..\Core\Config.h"
#include <iostream>
#include "..\Game\View.h"
#include <random>
#include <deque>
#include <algorithm>
#include <unordered_map>

extern "C" {
#include "..\Helpers\Mouse.h"
#include "..\Game\Entity.h"
}

// Enhanced prediction structures
struct TargetHistory {
    Vec3 position;
    Vec3 velocity;
    float timestamp;
    Vec2 viewAngle;
    bool isShooting;
    int health;
};

struct PredictionData {
    Vec3 predictedPosition;
    float confidence;
    float timeToTarget;
    bool isMoving;
    bool isShooting;
    float threatLevel;
};

struct WeaponProfile {
    float bulletSpeed;
    float bulletDrop;
    float accuracy;
    float damage;
    float range;
    bool isSniper;
    bool isRifle;
    bool isPistol;
};

namespace AimControl
{
    inline int HotKey = VK_MENU; // default to ALT; no LMB by default
    inline int AltHotKey = VK_RBUTTON; // Alternative hotkey for hold mode
    inline bool UseAltHotKey = false; // Use alternative hotkey toggle
    inline int AimBullet = 1;
    inline bool ScopeOnly = true;
    inline bool IgnoreFlash = false;
    inline bool HumanizeVar = true;
    inline int HumanizationStrength = 5;
    inline float AimFov = 10;
    inline float AimFovMin = 0.4f;
    inline float Smooth = 5.0f;
    inline std::vector<int> HitboxList{ BONEINDEX::head };
    inline bool HasTarget = false;
    inline bool onlyAuto = false;
    inline bool VisibleOnly = true;

    // Enhanced aimbot settings - now using config values
    inline bool& AdvancedPrediction = LegitBotConfig::AdvancedPrediction;
    inline bool& IntelligentTargeting = LegitBotConfig::IntelligentTargeting;
    inline bool& WallAvoidance = LegitBotConfig::WallAvoidance;
    inline bool& WeaponAdaptive = LegitBotConfig::WeaponAdaptive;
    inline bool& UltraLowLatency = LegitBotConfig::UltraLowLatency;
    inline bool& HeadshotPriority = LegitBotConfig::HeadshotPriority;
    inline bool& DynamicFOV = LegitBotConfig::DynamicFOV;
    inline bool& AntiDetection = LegitBotConfig::AntiDetection;
    
    // Prediction settings
    inline float& PredictionTime = LegitBotConfig::PredictionTime;
    inline int& HistorySize = LegitBotConfig::HistorySize;
    inline float& VelocityThreshold = LegitBotConfig::VelocityThreshold;
    inline float& ThreatMultiplier = LegitBotConfig::ThreatMultiplier;
    
    // Performance settings
    inline float& MaxPredictionDistance = LegitBotConfig::MaxPredictionDistance;
    inline float& MinTargetHealth = LegitBotConfig::MinTargetHealth;
    inline float& MaxTargetHealth = LegitBotConfig::MaxTargetHealth;
    inline float& WallCheckDistance = LegitBotConfig::WallCheckDistance;

    static float PrevTargetX = 0.0f;
    static float PrevTargetY = 0.0f;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // Target tracking
    static std::unordered_map<int, std::deque<TargetHistory>> targetHistories;
    static std::unordered_map<int, PredictionData> targetPredictions;
    static std::unordered_map<std::string, WeaponProfile> weaponProfiles;
    static int lastTargetIndex = -1;
    static float lastPredictionTime = 0.0f;

    // Enhanced functions
    std::pair<float, float> Humanize(float TargetX, float TargetY);
    void AimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList);
    void switchToggle();
    std::pair<float, float> CalculateTargetOffset(const Vec2& ScreenPos, int ScreenCenterX, int ScreenCenterY);
    bool CheckAutoMode(const std::string& WeaponName);
    
    // Advanced prediction functions
    PredictionData PredictTargetMovement(int targetIndex, const CEntity& Local, float deltaTime);
    Vec3 CalculatePredictedPosition(const std::deque<TargetHistory>& history, float predictionTime);
    float CalculateThreatLevel(const CEntity& target, const CEntity& local);
    bool IsTargetShootingAtMe(const CEntity& target, const CEntity& local);
    bool IsWallBetween(const Vec3& start, const Vec3& end);
    bool IsPointInWall(const Vec3& point);
    WeaponProfile GetWeaponProfile(const std::string& weaponName);
    float CalculateBulletDrop(const Vec3& start, const Vec3& end, const WeaponProfile& weapon);
    Vec3 CompensateForBulletDrop(const Vec3& target, const Vec3& local, const WeaponProfile& weapon);
    int SelectBestHitbox(const CEntity& target, const CEntity& local, const WeaponProfile& weapon);
    float CalculateDynamicFOV(const CEntity& target, const CEntity& local);
    void UpdateTargetHistory(int targetIndex, const CEntity& target);
    void InitializeWeaponProfiles();
    bool IsValidTarget(const CEntity& target, const CEntity& local);
    float CalculateTargetPriority(const CEntity& target, const CEntity& local, const PredictionData& prediction);
    std::string GetWeapon(const CEntity& LocalEntity);
    bool CheckWeapon(const std::string& WeaponName);
    bool CheckScopeWeapon(const std::string& WeaponName);
}
