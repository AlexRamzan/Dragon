#include "TriggerBot.h"
#include <chrono>
#include <random>
#include <thread>
#include <algorithm>
#include <cmath>

// Initialize weapon profiles for ultra-fast detection
void TriggerBot::InitializeWeaponProfiles()
{
    if (!weaponProfiles.empty()) return;
    
    // Snipers - Ultra-fast reaction
    weaponProfiles["awp"] = { 0.5f, 30.0f, 1.0f, true, false, false, false };
    weaponProfiles["ssg08"] = { 0.3f, 25.0f, 0.95f, true, false, false, false };
    weaponProfiles["scar20"] = { 0.4f, 35.0f, 0.9f, true, false, false, false };
    weaponProfiles["g3Sg1"] = { 0.4f, 35.0f, 0.9f, true, false, false, false };
    
    // Rifles - Fast reaction
    weaponProfiles["ak47"] = { 1.0f, 50.0f, 0.95f, false, true, false, false };
    weaponProfiles["m4a4"] = { 0.8f, 45.0f, 0.97f, false, true, false, false };
    weaponProfiles["m4a1"] = { 0.7f, 40.0f, 0.98f, false, true, false, false };
    weaponProfiles["aug"] = { 0.9f, 55.0f, 0.96f, false, true, false, false };
    weaponProfiles["galilar"] = { 1.1f, 60.0f, 0.94f, false, true, false, false };
    weaponProfiles["famas"] = { 1.2f, 65.0f, 0.93f, false, true, false, false };
    
    // SMGs - Very fast reaction
    weaponProfiles["mac10"] = { 0.5f, 30.0f, 0.85f, false, false, false, true };
    weaponProfiles["mp9"] = { 0.4f, 25.0f, 0.87f, false, false, false, true };
    weaponProfiles["mp7"] = { 0.6f, 35.0f, 0.88f, false, false, false, true };
    weaponProfiles["ump45"] = { 0.7f, 40.0f, 0.86f, false, false, false, true };
    weaponProfiles["p90"] = { 0.3f, 20.0f, 0.84f, false, false, false, true };
    weaponProfiles["bizon"] = { 0.2f, 15.0f, 0.82f, false, false, false, true };
    
    // Pistols - Ultra-fast reaction
    weaponProfiles["deagle"] = { 0.1f, 10.0f, 0.92f, false, false, true, false };
    weaponProfiles["glock"] = { 0.05f, 5.0f, 0.90f, false, false, true, false };
    weaponProfiles["usp"] = { 0.08f, 8.0f, 0.91f, false, false, true, false };
    weaponProfiles["p2000"] = { 0.08f, 8.0f, 0.91f, false, false, true, false };
    weaponProfiles["p250"] = { 0.06f, 6.0f, 0.89f, false, false, true, false };
    weaponProfiles["fiveseven"] = { 0.07f, 7.0f, 0.90f, false, false, true, false };
    weaponProfiles["tec9"] = { 0.05f, 5.0f, 0.88f, false, false, true, false };
    weaponProfiles["cz75a"] = { 0.06f, 6.0f, 0.89f, false, false, true, false };
    weaponProfiles["revolver"] = { 0.15f, 15.0f, 0.93f, false, false, true, false };
}

void TriggerBot::Run(const CEntity& LocalEntity, const int& LocalPlayerControllerIndex)
{
    if (MenuConfig::ShowMenu)
        return;

    if (LocalEntity.Controller.AliveStatus == 0)
        return;

    // Initialize weapon profiles if needed
    InitializeWeaponProfiles();
    
    // Update performance metrics
    frameCount++;
    UpdatePerformanceMetrics();

    // Ultra-fast target detection
    DWORD uHandle = 0;
    if (!memoryManager.ReadMemory<DWORD>(LocalEntity.Pawn.Address + Offset.Pawn.iIDEntIndex, uHandle))
    {
        g_HasValidTarget = false;
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
        return;
    }

    if (uHandle == -1)
    {
        g_HasValidTarget = false;
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
        return;
    }

    // Ultra-fast entity resolution
    DWORD64 PawnAddress = CEntity::ResolveEntityHandle(uHandle);
    if (PawnAddress == 0)
    {
        g_HasValidTarget = false;
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
        return;
    }

    CEntity targetEntity;
    if (!targetEntity.UpdatePawn(PawnAddress))
    {
        g_HasValidTarget = false;
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
        return;
    }

    // Enhanced validation with confidence scoring
    float confidence = CalculateTargetConfidence(LocalEntity, targetEntity);
    UpdateTriggerHistory(uHandle, true, confidence);

    if (!CanTrigger(LocalEntity, targetEntity, LocalPlayerControllerIndex))
    {
        g_HasValidTarget = false;
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
        return;
    }

    if (!g_HasValidTarget)
    {
        g_TargetFoundTime = std::chrono::high_resolution_clock::now();
    }
    g_HasValidTarget = true;

    // Ultra-fast timing with high-resolution clock
    auto now = std::chrono::high_resolution_clock::now();
    auto timeSinceLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_LastShotTime).count();
    auto timeSinceTargetFound = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_TargetFoundTime).count();

    // Get current weapon for adaptive timing
    std::string currentWeapon = GetWeapon(LocalEntity);
    TriggerWeaponProfile weapon = GetWeaponProfile(currentWeapon);
    
    // Use original timing logic as fallback
    long long originalShotDuration = ShotDuration;
    long long originalTriggerDelay = TriggerDelay;
    
    // Check if trigger should be active (hold/toggle logic)
    bool isTriggerActive = IsTriggerActive();
    
    // Ultra-fast trigger conditions with fallback
    bool shouldTrigger = (isTriggerActive || LegitBotConfig::TriggerAlways) &&
                       timeSinceLastShot >= originalShotDuration &&
                       timeSinceTargetFound >= originalTriggerDelay;

    if (shouldTrigger)
    {
        if (UltraFastMode)
        {
            ExecuteUltraFastShot();
        }
        else if (PredictiveTrigger && IsTargetMoving(targetEntity))
        {
            ExecutePredictiveShot(targetEntity);
        }
        else if (AntiDetection)
        {
            ExecuteHumanizedShot();
        }
        else
        {
            ExecuteShot();
        }
        
        triggerCount++;
    }
}

bool TriggerBot::CanTrigger(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex)
{
    // Check if target is in a valid state
    if (TargetEntity.Pawn.Address == 0)
        return false;

    // Check team
    if (MenuConfig::TeamCheck && LocalEntity.Pawn.TeamID == TargetEntity.Pawn.TeamID)
        return false;

    // Check if weapon is ready
    bool waitForNoAttack = false;
    if (!memoryManager.ReadMemory<bool>(LocalEntity.Pawn.Address + Offset.Pawn.m_bWaitForNoAttack, waitForNoAttack))
        return false;

    if (waitForNoAttack)
        return false;

    // Check weapon type
    std::string currentWeapon = GetWeapon(LocalEntity);
    if (!CheckWeapon(currentWeapon))
        return false;

    //check is velocity == 0
    if(StopedOnly && LocalEntity.Pawn.Speed != 0)
        return false;

    // Check flash duration
    if (!IgnoreFlash && LocalEntity.Pawn.FlashDuration > 0.0f)
        return false;

    // Check TTD timout
    DWORD64 playerMask = (DWORD64(1) << LocalPlayerControllerIndex);
    bool bIsVisible = (TargetEntity.Pawn.bSpottedByMask & playerMask) || (LocalEntity.Pawn.bSpottedByMask & playerMask);
    if (TTDtimeout && !bIsVisible)
        return false;

    // Check scope requirement
    if (ScopeOnly && CheckScopeWeapon(currentWeapon))
    {
        bool isScoped = false;
        memoryManager.ReadMemory<bool>(LocalEntity.Pawn.Address + Offset.Pawn.isScoped, isScoped);
        if (!isScoped)
            return false;
    }

    return true;
}

bool TriggerBot::IsTriggerActive()
{
    bool currentKeyState = (GetAsyncKeyState(HotKey) & 0x8000) != 0;
    
    if (ToggleMode) {
        // Toggle mode: key press toggles state
        if (currentKeyState && !LastKeyState) {
            // Key was just pressed (rising edge)
            ToggleState = !ToggleState;
        }
        LastKeyState = currentKeyState;
        return ToggleState;
    } else {
        // Hold mode: trigger only when key is held
        return currentKeyState;
    }
}

// Enhanced validation functions
bool TriggerBot::CanTriggerUltraFast(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex)
{
    // Ultra-fast basic checks
    if (TargetEntity.Pawn.Address == 0) return false;
    if (MenuConfig::TeamCheck && LocalEntity.Pawn.TeamID == TargetEntity.Pawn.TeamID) return false;
    
    // Ultra-fast weapon check
    std::string currentWeapon = GetWeapon(LocalEntity);
    if (!CheckWeaponUltraFast(currentWeapon)) return false;
    
    // Ultra-fast visibility check
    if (!IsTargetVisible(LocalEntity, TargetEntity, LocalPlayerControllerIndex)) return false;
    
    // Ultra-fast range check (optional for basic functionality)
    // if (!IsTargetInRange(LocalEntity, TargetEntity)) return false;
    
    // Ultra-fast threat assessment (optional for basic functionality)
    // if (!IsTargetThreat(LocalEntity, TargetEntity)) return false;
    
    return true;
}

float TriggerBot::CalculateTargetConfidence(const CEntity& LocalEntity, const CEntity& TargetEntity)
{
    float confidence = 0.0f;
    
    // Distance factor (closer = higher confidence)
    float distance = sqrt(pow(TargetEntity.Pawn.Pos.x - LocalEntity.Pawn.Pos.x, 2) + 
                         pow(TargetEntity.Pawn.Pos.y - LocalEntity.Pawn.Pos.y, 2) + 
                         pow(TargetEntity.Pawn.Pos.z - LocalEntity.Pawn.Pos.z, 2));
    confidence += (2000.0f - distance) / 2000.0f * 0.3f;
    
    // Health factor (lower health = higher confidence)
    confidence += (100.0f - TargetEntity.Pawn.Health) / 100.0f * 0.2f;
    
    // Movement factor (moving targets = higher confidence)
    if (IsTargetMoving(TargetEntity)) confidence += 0.2f;
    
    // Shooting factor (shooting targets = higher confidence)
    if (IsTargetShooting(TargetEntity)) confidence += 0.3f;
    
    return (confidence > 1.0f) ? 1.0f : confidence;
}

bool TriggerBot::IsTargetMoving(const CEntity& TargetEntity)
{
    return TargetEntity.Pawn.Speed > VelocityThreshold;
}

bool TriggerBot::IsTargetShooting(const CEntity& TargetEntity)
{
    return TargetEntity.Pawn.ShotsFired > 0;
}

bool TriggerBot::IsTargetVisible(const CEntity& LocalEntity, const CEntity& TargetEntity, const int& LocalPlayerControllerIndex)
{
    DWORD64 playerMask = (DWORD64(1) << LocalPlayerControllerIndex);
    return (TargetEntity.Pawn.bSpottedByMask & playerMask) || (LocalEntity.Pawn.bSpottedByMask & playerMask);
}

bool TriggerBot::IsTargetInRange(const CEntity& LocalEntity, const CEntity& TargetEntity)
{
    float distance = sqrt(pow(TargetEntity.Pawn.Pos.x - LocalEntity.Pawn.Pos.x, 2) + 
                         pow(TargetEntity.Pawn.Pos.y - LocalEntity.Pawn.Pos.y, 2) + 
                         pow(TargetEntity.Pawn.Pos.z - LocalEntity.Pawn.Pos.z, 2));
    return distance <= DistanceThreshold;
}

bool TriggerBot::IsTargetThreat(const CEntity& LocalEntity, const CEntity& TargetEntity)
{
    // Check if target is looking at us
    Vec3 toLocal = {LocalEntity.Pawn.Pos.x - TargetEntity.Pawn.Pos.x, 
                   LocalEntity.Pawn.Pos.y - TargetEntity.Pawn.Pos.y, 
                   LocalEntity.Pawn.Pos.z - TargetEntity.Pawn.Pos.z};
    
    float distance = sqrt(toLocal.x * toLocal.x + toLocal.y * toLocal.y + toLocal.z * toLocal.z);
    if (distance < 0.1f) return true;
    
    // Normalize
    toLocal.x /= distance;
    toLocal.y /= distance;
    toLocal.z /= distance;
    
    // Convert target's view angle to direction vector
    float yaw = TargetEntity.Pawn.ViewAngle.y * M_PI / 180.0f;
    float pitch = TargetEntity.Pawn.ViewAngle.x * M_PI / 180.0f;
    
    Vec3 targetDirection = {
        cosf(pitch) * cosf(yaw),
        cosf(pitch) * sinf(yaw),
        -sinf(pitch)
    };
    
    // Calculate dot product
    float dot = toLocal.x * targetDirection.x + toLocal.y * targetDirection.y + toLocal.z * targetDirection.z;
    
    // If target is looking at us, it's a threat
    return dot > 0.7f;
}

// Enhanced execution functions
void TriggerBot::ExecuteShot()
{
    // Check if already shooting to avoid double-click
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return;

    // Update timing
    g_LastShotTime = std::chrono::high_resolution_clock::now();

    // Execute shot with random timing
    std::random_device RandomDevice;
    std::mt19937 RandomNumber(RandomDevice());
    std::uniform_int_distribution<> Range(1, 5);
    auto rand = std::chrono::microseconds(Range(RandomNumber));

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::microseconds(Range(RandomNumber)));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void TriggerBot::ExecuteUltraFastShot()
{
    // Ultra-fast execution with minimal delay
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return;

    g_LastShotTime = std::chrono::high_resolution_clock::now();

    // Instant execution for maximum speed
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void TriggerBot::ExecutePredictiveShot(const CEntity& TargetEntity)
{
    // Predictive execution for moving targets
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return;

    g_LastShotTime = std::chrono::high_resolution_clock::now();

    // Calculate prediction delay based on target movement
    float predictionDelay = 0.0f;
    if (IsTargetMoving(TargetEntity)) {
        predictionDelay = PredictionTime * 1000.0f; // Convert to microseconds
    }

    // Execute with prediction
    if (predictionDelay > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds((int)predictionDelay));
    }
    
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void TriggerBot::ExecuteHumanizedShot()
{
    // Humanized execution with anti-detection
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return;

    g_LastShotTime = std::chrono::high_resolution_clock::now();

    // Add humanized delay
    float delay = GetHumanizedDelay();
    if (delay > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds((int)(delay * 1000)));
    }

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    
    // Humanized shot duration
    float shotDuration = GetRandomTiming();
    std::this_thread::sleep_for(std::chrono::microseconds((int)(shotDuration * 1000)));
    
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

// Advanced detection functions
void TriggerBot::UpdateTriggerHistory(DWORD64 handle, bool isValid, float confidence)
{
    TriggerHistory history;
    history.entityHandle = handle;
    history.timestamp = std::chrono::high_resolution_clock::now();
    history.isValid = isValid;
    history.confidence = confidence;
    
    triggerHistory.push_back(history);
    
    // Keep only recent history
    if (triggerHistory.size() > HistorySize) {
        triggerHistory.pop_front();
    }
    
    // Update confidence map
    targetConfidence[handle] = confidence;
}

float TriggerBot::GetTargetConfidence(DWORD64 handle)
{
    auto it = targetConfidence.find(handle);
    return (it != targetConfidence.end()) ? it->second : 0.0f;
}

bool TriggerBot::ShouldTriggerNow(const CEntity& LocalEntity, const CEntity& TargetEntity)
{
    // Check if this is the optimal time to trigger
    float confidence = GetTargetConfidence(TargetEntity.Pawn.Address);
    return confidence >= ConfidenceThreshold && IsOptimalTriggerTime(LocalEntity, TargetEntity);
}

bool TriggerBot::IsOptimalTriggerTime(const CEntity& LocalEntity, const CEntity& TargetEntity)
{
    // Check if target is in optimal position for trigger
    if (IsTargetMoving(TargetEntity) && VelocityPrediction) {
        return true; // Moving targets are good for prediction
    }
    
    if (IsTargetShooting(TargetEntity)) {
        return true; // Shooting targets are high priority
    }
    
    return true; // Default to trigger
}

// Weapon profiling functions
TriggerWeaponProfile TriggerBot::GetWeaponProfile(const std::string& weaponName)
{
    InitializeWeaponProfiles();
    
    auto it = weaponProfiles.find(weaponName);
    if (it != weaponProfiles.end()) {
        return it->second;
    }
    
    // Default profile for unknown weapons (more permissive)
    return { 0.1f, 10.0f, 0.9f, false, false, false, false };
}

float TriggerBot::GetOptimalReactionTime(const std::string& weaponName)
{
    TriggerWeaponProfile profile = GetWeaponProfile(weaponName);
    return profile.reactionTime;
}

float TriggerBot::GetOptimalShotDuration(const std::string& weaponName)
{
    TriggerWeaponProfile profile = GetWeaponProfile(weaponName);
    return profile.shotDuration;
}

// Anti-detection functions
float TriggerBot::GetHumanizedDelay()
{
    if (!HumanizedTiming) return 0.0f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(MinDelay, MaxDelay);
    
    return dist(gen);
}

float TriggerBot::GetRandomTiming()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(5.0f, 25.0f);
    
    return dist(gen);
}

bool TriggerBot::ShouldAddRandomDelay()
{
    if (!RandomDelay) return false;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    return dist(gen) < TimingVariation;
}

// Performance optimization functions
void TriggerBot::OptimizeMemoryUsage()
{
    // Clean up old history
    if (triggerHistory.size() > HistorySize * 2) {
        triggerHistory.erase(triggerHistory.begin(), triggerHistory.begin() + HistorySize);
    }
    
    // Clean up old confidence data
    if (targetConfidence.size() > 50) {
        targetConfidence.clear();
    }
}

void TriggerBot::UpdatePerformanceMetrics()
{
    // Update average reaction time
    if (frameCount % 100 == 0) {
        // Calculate performance metrics every 100 frames
        OptimizeMemoryUsage();
    }
}

void TriggerBot::BatchProcessTargets(const CEntity& LocalEntity, const int& LocalPlayerControllerIndex)
{
    // Batch processing for multiple targets (future enhancement)
    // This would process multiple potential targets simultaneously
}

std::string TriggerBot::GetWeapon(const CEntity& LocalEntity)
{
    // Single memory read to get the weapon pointer
    DWORD64 CurrentWeapon;
    if (!memoryManager.ReadMemory(LocalEntity.Pawn.Address + Offset.Pawn.pClippingWeapon, CurrentWeapon) || CurrentWeapon == 0)
        return "";

    // Calculate the final address for weapon index directly
    DWORD64 weaponIndexAddress = CurrentWeapon + Offset.EconEntity.AttributeManager +
        Offset.WeaponBaseData.Item + Offset.WeaponBaseData.ItemDefinitionIndex;

    // Single memory read to get weapon index
    short weaponIndex;
    if (!memoryManager.ReadMemory(weaponIndexAddress, weaponIndex) || weaponIndex == -1)
        return "";

    // Inline weapon name lookup
    static const std::string defaultWeapon = "";
    auto it = CEntity::weaponNames.find(weaponIndex);
    return (it != CEntity::weaponNames.end()) ? it->second : defaultWeapon;
}

bool TriggerBot::CheckScopeWeapon(const std::string& WeaponName)
{
    return (WeaponName == "awp" || WeaponName == "g3Sg1" || WeaponName == "ssg08" || WeaponName == "scar20");
}

bool TriggerBot::CheckWeapon(const std::string& WeaponName)
{
    return !(WeaponName == "smokegrenade" || WeaponName == "flashbang" || WeaponName == "hegrenade" ||
        WeaponName == "molotov" || WeaponName == "decoy" || WeaponName == "incgrenade" ||
        WeaponName == "t_knife" || WeaponName == "ct_knife" || WeaponName == "c4");
}

bool TriggerBot::CheckWeaponUltraFast(const std::string& WeaponName)
{
    // Ultra-fast weapon check with optimized logic
    if (WeaponName.empty()) return false;
    
    // Fast rejection for obvious non-weapons
    if (WeaponName == "smokegrenade" || WeaponName == "flashbang" || WeaponName == "hegrenade" ||
        WeaponName == "molotov" || WeaponName == "decoy" || WeaponName == "incgrenade" ||
        WeaponName == "t_knife" || WeaponName == "ct_knife" || WeaponName == "c4") {
        return false;
    }
    
    // Fast acceptance for known weapons
    return true;
}