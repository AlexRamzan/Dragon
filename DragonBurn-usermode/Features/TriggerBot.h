#pragma once
#include <chrono>
#include <deque>
#include <unordered_map>

#include "../Game/Game.h"
#include "../Game/Entity.h"
#include "../Core/Config.h"

// Enhanced TriggerBot structures
struct TriggerHistory {
    DWORD64 entityHandle;
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    bool isValid;
    float confidence;
};

struct TriggerWeaponProfile {
    float reactionTime;
    float shotDuration;
    float accuracy;
    bool isSniper;
    bool isRifle;
    bool isPistol;
    bool isSMG;
};

namespace TriggerBot
{
    // Enhanced Configuration
	inline int TriggerDelay = 1; // Ultra-fast: 1ms minimum
	inline int ShotDuration = 50; // Ultra-fast: 50ms shot duration
	inline bool ScopeOnly = true;
	inline bool IgnoreFlash = false;
	inline bool StopedOnly = false;
	inline bool TTDtimeout = false;
	inline bool VisibleCheck = true;

    // Ultra-fast settings
    inline bool UltraFastMode = true;
    inline bool PredictiveTrigger = true;
    inline bool WeaponAdaptive = true;
    inline bool AntiDetection = true;
    inline bool SmartTiming = true;
    inline bool VelocityPrediction = true;
    inline bool HeadshotPriority = true;
    inline bool MultiTarget = true;
    
    // Advanced detection
    inline float ConfidenceThreshold = 0.8f;
    inline int HistorySize = 5;
    inline float PredictionTime = 0.05f; // 50ms prediction
    inline float VelocityThreshold = 100.0f;
    inline float DistanceThreshold = 2000.0f;
    
    // Anti-detection
    inline bool HumanizedTiming = true;
    inline float TimingVariation = 0.1f;
    inline bool RandomDelay = true;
    inline float MinDelay = 0.5f;
    inline float MaxDelay = 2.0f;
    
    // Performance
    inline bool BatchProcessing = true;
    inline bool MemoryOptimized = true;
    inline bool FramePerfect = true;

    // Input configuration
    inline int HotKey = VK_XBUTTON2;
    inline bool ToggleMode = false; // false = hold, true = toggle
    inline bool ToggleState = false; // current toggle state
    inline bool LastKeyState = false; // for toggle detection

    // Enhanced timing variables
	inline std::chrono::time_point<std::chrono::high_resolution_clock> g_LastShotTime;
    inline std::chrono::time_point<std::chrono::high_resolution_clock> g_TargetFoundTime;
	inline bool g_HasValidTarget = false;
    
    // Enhanced tracking
    inline std::deque<TriggerHistory> triggerHistory;
    inline std::unordered_map<std::string, TriggerWeaponProfile> weaponProfiles;
    inline std::unordered_map<DWORD64, float> targetConfidence;
    inline DWORD64 lastTargetHandle = 0;
    inline float lastConfidence = 0.0f;
    
    // Performance counters
    inline int frameCount = 0;
    inline int triggerCount = 0;
    inline float averageReactionTime = 0.0f;

    // Main functions
	void Run(const CEntity& LocalEntity, const int& LocalPlayerControllerIndex);
	bool IsTriggerActive(); // Check if trigger should be active (hold/toggle logic)

    // Enhanced validation functions
    bool CanTrigger(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex);
    bool CanTriggerUltraFast(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex);
    float CalculateTargetConfidence(const CEntity& LocalEntity, const CEntity& TargetEntity);
    bool IsTargetMoving(const CEntity& TargetEntity);
    bool IsTargetShooting(const CEntity& TargetEntity);
    bool IsTargetVisible(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex);
    bool IsTargetInRange(const CEntity& LocalEntity, const CEntity& TargetEntity);
    bool IsTargetThreat(const CEntity& LocalEntity, const CEntity& TargetEntity);

    // Enhanced execution functions
	void ExecuteShot();
	void ExecuteUltraFastShot();
	void ExecutePredictiveShot(const CEntity& TargetEntity);
	void ExecuteHumanizedShot();

    // Advanced detection
    void UpdateTriggerHistory(DWORD64 handle, bool isValid, float confidence);
    float GetTargetConfidence(DWORD64 handle);
    bool ShouldTriggerNow(const CEntity& LocalEntity, const CEntity& TargetEntity);
    bool IsOptimalTriggerTime(const CEntity& LocalEntity, const CEntity& TargetEntity);
    
    // Weapon profiling
    void InitializeWeaponProfiles();
    TriggerWeaponProfile GetWeaponProfile(const std::string& weaponName);
    float GetOptimalReactionTime(const std::string& weaponName);
    float GetOptimalShotDuration(const std::string& weaponName);
    
    // Anti-detection
    float GetHumanizedDelay();
    float GetRandomTiming();
    bool ShouldAddRandomDelay();
    
    // Performance optimization
    void OptimizeMemoryUsage();
    void UpdatePerformanceMetrics();
    void BatchProcessTargets(const CEntity& LocalEntity, const int& LocalPlayerControllerIndex);

    // Utility functions
	std::string GetWeapon(const CEntity& LocalEntity);
	bool CheckWeapon(const std::string& WeaponName);
	bool CheckScopeWeapon(const std::string& WeaponName);
	bool CheckWeaponUltraFast(const std::string& WeaponName);
}