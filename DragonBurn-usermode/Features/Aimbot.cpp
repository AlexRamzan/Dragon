#include "Aimbot.h"
#undef max()
#undef min()

// Initialize weapon profiles with realistic CS2 data
void AimControl::InitializeWeaponProfiles()
{
    if (!weaponProfiles.empty()) return;
    
    // Rifles
    weaponProfiles["ak47"] = { 715.0f, 0.0f, 0.95f, 36.0f, 8192.0f, false, true, false };
    weaponProfiles["m4a4"] = { 715.0f, 0.0f, 0.97f, 33.0f, 8192.0f, false, true, false };
    weaponProfiles["m4a1"] = { 715.0f, 0.0f, 0.98f, 33.0f, 8192.0f, false, true, false };
    weaponProfiles["aug"] = { 715.0f, 0.0f, 0.96f, 28.0f, 8192.0f, false, true, false };
    weaponProfiles["galilar"] = { 715.0f, 0.0f, 0.94f, 30.0f, 8192.0f, false, true, false };
    weaponProfiles["famas"] = { 715.0f, 0.0f, 0.93f, 30.0f, 8192.0f, false, true, false };
    
    // Snipers
    weaponProfiles["awp"] = { 3000.0f, 0.0f, 1.0f, 115.0f, 8192.0f, true, false, false };
    weaponProfiles["ssg08"] = { 3000.0f, 0.0f, 0.99f, 88.0f, 8192.0f, true, false, false };
    weaponProfiles["scar20"] = { 3000.0f, 0.0f, 0.98f, 80.0f, 8192.0f, true, false, false };
    weaponProfiles["g3Sg1"] = { 3000.0f, 0.0f, 0.97f, 80.0f, 8192.0f, true, false, false };
    
    // SMGs
    weaponProfiles["mac10"] = { 400.0f, 0.0f, 0.85f, 29.0f, 8192.0f, false, false, false };
    weaponProfiles["mp9"] = { 400.0f, 0.0f, 0.87f, 26.0f, 8192.0f, false, false, false };
    weaponProfiles["mp7"] = { 400.0f, 0.0f, 0.88f, 29.0f, 8192.0f, false, false, false };
    weaponProfiles["ump45"] = { 400.0f, 0.0f, 0.86f, 35.0f, 8192.0f, false, false, false };
    weaponProfiles["p90"] = { 400.0f, 0.0f, 0.84f, 26.0f, 8192.0f, false, false, false };
    weaponProfiles["bizon"] = { 400.0f, 0.0f, 0.82f, 27.0f, 8192.0f, false, false, false };
    
    // Pistols
    weaponProfiles["deagle"] = { 2000.0f, 0.0f, 0.92f, 63.0f, 8192.0f, false, false, true };
    weaponProfiles["glock"] = { 2000.0f, 0.0f, 0.90f, 28.0f, 8192.0f, false, false, true };
    weaponProfiles["usp"] = { 2000.0f, 0.0f, 0.91f, 35.0f, 8192.0f, false, false, true };
    weaponProfiles["p2000"] = { 2000.0f, 0.0f, 0.91f, 35.0f, 8192.0f, false, false, true };
    weaponProfiles["p250"] = { 2000.0f, 0.0f, 0.89f, 38.0f, 8192.0f, false, false, true };
    weaponProfiles["fiveseven"] = { 2000.0f, 0.0f, 0.90f, 32.0f, 8192.0f, false, false, true };
    weaponProfiles["tec9"] = { 2000.0f, 0.0f, 0.88f, 33.0f, 8192.0f, false, false, true };
    weaponProfiles["cz75a"] = { 2000.0f, 0.0f, 0.89f, 31.0f, 8192.0f, false, false, true };
    weaponProfiles["revolver"] = { 2000.0f, 0.0f, 0.93f, 86.0f, 8192.0f, false, false, true };
}

void AimControl::switchToggle()
{
    LegitBotConfig::AimAlways = !LegitBotConfig::AimAlways;
}

std::pair<float, float> AimControl::CalculateTargetOffset(const Vec2& ScreenPos, int ScreenCenterX, int ScreenCenterY)
{
    float TargetX = 0.0f;
    float TargetY = 0.0f;

    /*x*/
    if (ScreenPos.x != ScreenCenterX) {
        TargetX = (ScreenPos.x > ScreenCenterX) ?
            -(ScreenCenterX - ScreenPos.x) :
            ScreenPos.x - ScreenCenterX;

        if (TargetX + ScreenCenterX > ScreenCenterX * 2 || TargetX + ScreenCenterX < 0) {
            TargetX = 0.0f;
        }
    }

    /*y*/
    if (ScreenPos.y != 0 && ScreenPos.y != ScreenCenterY) {
        TargetY = (ScreenPos.y > ScreenCenterY) ?
            -(ScreenCenterY - ScreenPos.y) :
            ScreenPos.y - ScreenCenterY;

        if (TargetY + ScreenCenterY > ScreenCenterY * 2 || TargetY + ScreenCenterY < 0) {
            TargetY = 0.0f;
        }
    }

    return { TargetX, TargetY };
}

std::pair<float, float> AimControl::Humanize(float TargetX, float TargetY) {

    static float HumanizationAmount = HumanizationStrength*2/100;

    if (HumanizationAmount <= 0.0f)
    {
        PrevTargetX = TargetX;
        PrevTargetY = TargetY;
        return { TargetX, TargetY };
    }
    
    // Enhanced humanization with config-based settings
    float microJitterStrength = LegitBotConfig::MicroJitterStrength;
    float reactionVariation = LegitBotConfig::ReactionTimeVariation;
    bool curvedMovement = LegitBotConfig::CurvedMovement;
    bool velocityBasedSmoothing = LegitBotConfig::VelocityBasedSmoothing;
    
    // random distributions for different types of jitter
    std::uniform_real_distribution<float> jitterDist(-10.f, 10.f);
    std::uniform_real_distribution<float> microDist(-5.f, 5.f);
    std::uniform_real_distribution<float> smoothnessDist(0.3f, 0.9f);
    std::uniform_real_distribution<float> reactionDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> curveDist(-0.5f, 0.5f);
    
    // calculate movement distance for dynamic adjustments
    float MovementDistance = std::sqrt(TargetX * TargetX + TargetY * TargetY);
    
    // Enhanced micro-movements with configurable strength
    float MicroJitterX = microDist(gen) * std::min(MovementDistance * 0.2f, 6.0f) * microJitterStrength;
    float MicroJitterY = microDist(gen) * std::min(MovementDistance * 0.2f, 6.0f) * microJitterStrength;
    
    // Velocity-based jitter scaling
    if (velocityBasedSmoothing) {
        float velocityFactor = std::min(MovementDistance / 100.0f, 2.0f);
        MicroJitterX *= velocityFactor;
        MicroJitterY *= velocityFactor;
    }
    
    // add larger jitter for longer movements (scaled by strength)
    float JitterScale = std::min(MovementDistance * 0.1f, 8.0f) * HumanizationAmount;
    float JitterX = jitterDist(gen) * JitterScale;
    float JitterY = jitterDist(gen) * JitterScale;
    
    // Enhanced curved path movement
    float PerpX = 0.0f, PerpY = 0.0f;
    if (curvedMovement) {
        float curveStrength = curveDist(gen) * HumanizationAmount;
        PerpX = -TargetY * 0.25f * curveStrength;
        PerpY = TargetX * 0.25f * curveStrength;
    }
    
    // Advanced smoothing with configurable factors
    float baseSmoothFactor = smoothnessDist(gen);
    float SmoothFactor = 1.0f - ((1.0f - baseSmoothFactor) * HumanizationAmount);
    
    // Velocity-based smoothing adjustment
    if (velocityBasedSmoothing) {
        float velocityAdjustment = std::min(MovementDistance / 200.0f, 1.0f);
        SmoothFactor *= (1.0f - velocityAdjustment * 0.3f);
    }
    
    float SmoothedX = TargetX * SmoothFactor + PrevTargetX * (1.0f - SmoothFactor);
    float SmoothedY = TargetY * SmoothFactor + PrevTargetY * (1.0f - SmoothFactor);
    
    // Enhanced reaction time simulation with configurable variation
    if (reactionDist(gen) < reactionVariation * HumanizationAmount) {
        // Simulate delayed reaction
        SmoothedX = PrevTargetX * 0.8f + TargetX * 0.2f;
        SmoothedY = PrevTargetY * 0.8f + TargetY * 0.2f;
    }
    
    // Add occasional overshoot correction (human-like behavior)
    if (reactionDist(gen) < 0.1f * HumanizationAmount) {
        SmoothedX *= 1.1f; // Slight overshoot
        SmoothedY *= 1.1f;
    }
    
    // combine all effects
    float HumanizedX = SmoothedX + MicroJitterX + JitterX + PerpX;
    float HumanizedY = SmoothedY + MicroJitterY + JitterY + PerpY;
    
    // Apply final smoothing to prevent jittery movement
    if (LegitBotConfig::AdvancedHumanization) {
        float finalSmooth = 0.95f;
        HumanizedX = HumanizedX * finalSmooth + PrevTargetX * (1.0f - finalSmooth);
        HumanizedY = HumanizedY * finalSmooth + PrevTargetY * (1.0f - finalSmooth);
    }
    
    // store current targets for next frame smoothing
    PrevTargetX = TargetX;
    PrevTargetY = TargetY;
    
    return { HumanizedX, HumanizedY };
}

// Advanced prediction and intelligence functions
void AimControl::UpdateTargetHistory(int targetIndex, const CEntity& target)
{
    if (targetIndex < 0) return;
    
    TargetHistory history;
    history.position = target.Pawn.Pos;
    history.timestamp = GetTickCount64() / 1000.0f;
    history.viewAngle = target.Pawn.ViewAngle;
    history.isShooting = target.Pawn.ShotsFired > 0;
    history.health = target.Pawn.Health;
    
    // Calculate velocity from previous position
    if (!targetHistories[targetIndex].empty()) {
        const auto& last = targetHistories[targetIndex].back();
        float deltaTime = history.timestamp - last.timestamp;
        if (deltaTime > 0.0f) {
            history.velocity = (history.position - last.position) / deltaTime;
        } else {
            history.velocity = last.velocity;
        }
    } else {
        history.velocity = Vec3{0, 0, 0};
    }
    
    targetHistories[targetIndex].push_back(history);
    
    // Keep only recent history
    if (targetHistories[targetIndex].size() > HistorySize) {
        targetHistories[targetIndex].pop_front();
    }
}

Vec3 AimControl::CalculatePredictedPosition(const std::deque<TargetHistory>& history, float predictionTime)
{
    if (history.size() < 2) {
        return history.empty() ? Vec3{0, 0, 0} : history.back().position;
    }
    
    const auto& current = history.back();
    const auto& previous = history[history.size() - 2];
    
    // Calculate average velocity over recent frames
    Vec3 avgVelocity = current.velocity;
    if (history.size() >= 3) {
        Vec3 totalVelocity = {0, 0, 0};
        int count = 0;
        for (int i = std::max(0, (int)history.size() - 3); i < history.size() - 1; i++) {
            totalVelocity = totalVelocity + (history[i + 1].position - history[i].position) / 
                           (history[i + 1].timestamp - history[i].timestamp);
            count++;
        }
        if (count > 0) {
            avgVelocity = totalVelocity / count;
        }
    }
    
    // Predict future position
    Vec3 predictedPos = current.position + avgVelocity * predictionTime;
    
    // Apply gravity compensation for falling targets
    if (avgVelocity.z < 0) {
        predictedPos.z += 0.5f * 800.0f * predictionTime * predictionTime; // CS2 gravity
    }
    
    return predictedPos;
}

PredictionData AimControl::PredictTargetMovement(int targetIndex, const CEntity& Local, float deltaTime)
{
    PredictionData prediction;
    prediction.confidence = 0.0f;
    prediction.isMoving = false;
    prediction.isShooting = false;
    prediction.threatLevel = 0.0f;
    
    if (targetHistories.find(targetIndex) == targetHistories.end() || targetHistories[targetIndex].empty()) {
        return prediction;
    }
    
    const auto& history = targetHistories[targetIndex];
    prediction.predictedPosition = CalculatePredictedPosition(history, PredictionTime);
    
    // Calculate movement confidence
    if (history.size() >= 2) {
        float velocityMagnitude = sqrt(history.back().velocity.x * history.back().velocity.x + 
                                     history.back().velocity.y * history.back().velocity.y + 
                                     history.back().velocity.z * history.back().velocity.z);
        
        prediction.isMoving = velocityMagnitude > VelocityThreshold;
        prediction.confidence = std::min(1.0f, velocityMagnitude / 500.0f); // Normalize to 0-1
    }
    
    prediction.isShooting = history.back().isShooting;
    prediction.timeToTarget = deltaTime;
    
    return prediction;
}

float AimControl::CalculateThreatLevel(const CEntity& target, const CEntity& local)
{
    float threat = 0.0f;
    
    // Distance factor (closer = more threat)
    float distance = sqrt(pow(target.Pawn.Pos.x - local.Pawn.Pos.x, 2) + 
                         pow(target.Pawn.Pos.y - local.Pawn.Pos.y, 2) + 
                         pow(target.Pawn.Pos.z - local.Pawn.Pos.z, 2));
    threat += (2000.0f - distance) / 2000.0f;
    
    // Health factor (higher health = more threat)
    threat += target.Pawn.Health / 100.0f;
    
    // Weapon factor
    WeaponProfile weapon = GetWeaponProfile(target.Pawn.WeaponName);
    threat += weapon.damage / 100.0f;
    
    // Shooting factor
    if (target.Pawn.ShotsFired > 0) {
        threat += 0.5f;
    }
    
    // Armor factor
    threat += target.Pawn.Armor / 100.0f;
    
    return std::min(1.0f, threat);
}

bool AimControl::IsTargetShootingAtMe(const CEntity& target, const CEntity& local)
{
    // Calculate angle between target's view and local player
    Vec3 toLocal = {local.Pawn.Pos.x - target.Pawn.Pos.x, 
                   local.Pawn.Pos.y - target.Pawn.Pos.y, 
                   local.Pawn.Pos.z - target.Pawn.Pos.z};
    
    float distance = sqrt(toLocal.x * toLocal.x + toLocal.y * toLocal.y + toLocal.z * toLocal.z);
    if (distance < 0.1f) return false;
    
    // Normalize
    toLocal.x /= distance;
    toLocal.y /= distance;
    toLocal.z /= distance;
    
    // Convert target's view angle to direction vector
    float yaw = target.Pawn.ViewAngle.y * M_PI / 180.0f;
    float pitch = target.Pawn.ViewAngle.x * M_PI / 180.0f;
    
    Vec3 targetDirection = {
        cosf(pitch) * cosf(yaw),
        cosf(pitch) * sinf(yaw),
        -sinf(pitch)
    };
    
    // Calculate dot product
    float dot = toLocal.x * targetDirection.x + toLocal.y * targetDirection.y + toLocal.z * targetDirection.z;
    
    // If dot product is close to 1, target is looking at us
    return dot > 0.8f;
}

bool AimControl::IsWallBetween(const Vec3& start, const Vec3& end)
{
    if (!WallAvoidance) return false;
    
    // Calculate distance (no distance gating)
    float distance = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2) + pow(end.z - start.z, 2));
    
    // Advanced ray tracing implementation
    Vec3 direction = {end.x - start.x, end.y - start.y, end.z - start.z};
    float dirLength = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    if (dirLength < 0.1f) return false;
    
    // Normalize direction
    direction.x /= dirLength;
    direction.y /= dirLength;
    direction.z /= dirLength;
    
    // Perform ray tracing with multiple steps
    int steps = (int)(dirLength / 10.0f); // Check every 10 units
    steps = std::max(1, std::min(steps, 50)); // Limit steps for performance
    
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        Vec3 checkPoint = {
            start.x + direction.x * dirLength * t,
            start.y + direction.y * dirLength * t,
            start.z + direction.z * dirLength * t
        };
        
        // Check if point is inside a wall (simplified check)
        // In a real implementation, you'd query the game's collision system
        if (IsPointInWall(checkPoint)) {
            return true;
        }
    }
    
    return false;
}

// Helper function to check if a point is inside a wall
bool AimControl::IsPointInWall(const Vec3& point)
{
    // This is a simplified implementation
    // In a real scenario, you'd query the game's BSP tree or collision system
    
    // Check for common wall boundaries (simplified)
    // These would be replaced with actual game world queries
    if (point.z < -1000.0f || point.z > 2000.0f) return true; // Z boundaries
    if (point.x < -4000.0f || point.x > 4000.0f) return true; // X boundaries  
    if (point.y < -4000.0f || point.y > 4000.0f) return true; // Y boundaries
    
    // Additional wall checks could be implemented here
    // For example, checking against known wall positions in the map
    
    return false;
}

WeaponProfile AimControl::GetWeaponProfile(const std::string& weaponName)
{
    InitializeWeaponProfiles();
    
    auto it = weaponProfiles.find(weaponName);
    if (it != weaponProfiles.end()) {
        return it->second;
    }
    
    // Default profile
    return { 715.0f, 0.0f, 0.9f, 30.0f, 8192.0f, false, false, false };
}

float AimControl::CalculateBulletDrop(const Vec3& start, const Vec3& end, const WeaponProfile& weapon)
{
    float distance = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2) + pow(end.z - start.z, 2));
    float timeToTarget = distance / weapon.bulletSpeed;
    return 0.5f * 800.0f * timeToTarget * timeToTarget; // CS2 gravity
}

Vec3 AimControl::CompensateForBulletDrop(const Vec3& target, const Vec3& local, const WeaponProfile& weapon)
{
    if (weapon.bulletSpeed < 1000.0f) return target; // No compensation for fast bullets
    
    float distance = sqrt(pow(target.x - local.x, 2) + pow(target.y - local.y, 2) + pow(target.z - local.z, 2));
    float timeToTarget = distance / weapon.bulletSpeed;
    float drop = 0.5f * 800.0f * timeToTarget * timeToTarget;
    
    return {target.x, target.y, target.z + drop};
}

int AimControl::SelectBestHitbox(const CEntity& target, const CEntity& local, const WeaponProfile& weapon)
{
    if (!HeadshotPriority) return BONEINDEX::head;
    
    // For snipers, always go for head
    if (weapon.isSniper) {
        return BONEINDEX::head;
    }
    
    // For rifles, prefer head but can go for neck if head is not visible
    if (weapon.isRifle) {
        return BONEINDEX::head;
    }
    
    // For pistols, prefer head but can go for chest
    if (weapon.isPistol) {
        return BONEINDEX::head;
    }
    
    // For SMGs, prefer head but can go for chest
    return BONEINDEX::head;
}

float AimControl::CalculateDynamicFOV(const CEntity& target, const CEntity& local)
{
    // Stable FOV: keep AimFov constant to allow long-range connection
    return AimFov;
}

bool AimControl::IsValidTarget(const CEntity& target, const CEntity& local)
{
    // Basic validation
    if (!target.IsAlive()) return false;
    if (target.Pawn.Health <= 0) return false;
    if (target.Pawn.TeamID == local.Pawn.TeamID) return false;
    if (target.Pawn.FlashDuration > 0.0f && !IgnoreFlash) return false;
    
    // No distance gating: allow long-range engagement
    return true;
}

float AimControl::CalculateTargetPriority(const CEntity& target, const CEntity& local, const PredictionData& prediction)
{
    float priority = 0.0f;
    
    // Threat level
    priority += CalculateThreatLevel(target, local) * 0.5f;
    
    // Health (lower health = higher priority)
    priority += (100.0f - target.Pawn.Health) / 100.0f * 0.2f;
    
    // Prediction confidence
    priority += prediction.confidence * 0.2f;
    
    // Is shooting at me
    if (IsTargetShootingAtMe(target, local)) {
        priority += 0.1f;
    }
    
    return priority;
}

void AimControl::AimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList)
{
    if (MenuConfig::ShowMenu)
        return;

    // Activation: hold or toggle via HotKey
    static bool toggledActive = LegitBotConfig::AimAlways;
    static SHORT prevKeyState = 0;
    
    // Choose which hotkey to use
    int activeHotKey = UseAltHotKey ? AltHotKey : HotKey;
    SHORT keyState = GetAsyncKeyState(activeHotKey);
    bool keyDown = (keyState & 0x8000) != 0;
    
    // Enhanced hold/toggle logic with better detection
    if (LegitBotConfig::AimToggleMode)
    {
        // Toggle mode: Press key to toggle on/off
        if (keyDown && !(prevKeyState & 0x8000))
        {
            // Key was just pressed (rising edge detection)
            toggledActive = !toggledActive;
        }
        prevKeyState = keyState;

        if (!(toggledActive || LegitBotConfig::AimAlways))
        {
            HasTarget = false;
            return;
        }
    }
    else
    {
        // Hold mode: Only active while key is held down
        if (!(keyDown || LegitBotConfig::AimAlways))
        {
            HasTarget = false;
            return;
        }
    }

    // Initialize weapon profiles if not done
    InitializeWeaponProfiles();

    std::string curWeapon = GetWeapon(Local);
    if (!CheckWeapon(curWeapon))
        return;

    if (onlyAuto && !CheckAutoMode(curWeapon))
        return;

    if (AimBullet != 0 && Local.Pawn.ShotsFired > 0 && Local.Pawn.ShotsFired <= AimBullet - 1)
    {
        HasTarget = false;
        return;
    }

    if (AimControl::ScopeOnly)
    {
        bool isScoped;
        memoryManager.ReadMemory<bool>(Local.Pawn.Address + Offset.Pawn.isScoped, isScoped);
        if (!isScoped && CheckScopeWeapon(curWeapon))
        {
            HasTarget = false;
            return;
        }
    }

    if (!IgnoreFlash && Local.Pawn.FlashDuration > 0.f)
        return;

    const int ListSize = AimPosList.size();
    if (ListSize == 0) {
        HasTarget = false;
        return;
    }

    // Get current weapon profile for advanced calculations
    WeaponProfile currentWeapon = GetWeaponProfile(curWeapon);
    
    // Ultra-low latency: Use frame-perfect timing
    static DWORD lastFrameTime = GetTickCount64();
    DWORD currentFrameTime = GetTickCount64();
    float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
    lastFrameTime = currentFrameTime;

    float BestNorm = MAXV;
    int BestTargetIndex = -1;

    const int ScreenCenterX = Gui.Window.Size.x / 2;
    const int ScreenCenterY = Gui.Window.Size.y / 2;

    // Old-method targeting: choose smallest angular distance within FOV (visible-only if enabled)
    for (int i = 0; i < ListSize; i++)
    {
        // Visibility gate
        if (VisibleOnly)
        {
            DWORD64 playerMask = (DWORD64(1) << Local.Controller.Address);
            DWORD64 mask = AimPosList.size() > 0 ? 0 : 0;
            bool visible = false;
            if (WallAvoidance)
            {
                if (!IsWallBetween(Local.Pawn.CameraPos, AimPosList[i]))
                    visible = true;
            }
            if (!visible)
                continue;
        }

        Vec3 OppPos = AimPosList[i] - LocalPos;
        const float Distance = sqrt(OppPos.x * OppPos.x + OppPos.y * OppPos.y);

        const float Yaw = atan2f(OppPos.y, OppPos.x) * 57.295779513f - Local.Pawn.ViewAngle.y;
        const float Pitch = -atan(OppPos.z / Distance) * 57.295779513f - Local.Pawn.ViewAngle.x;
        const float Norm = sqrt(Yaw * Yaw + Pitch * Pitch);

        if (Norm < BestNorm) {
            BestNorm = Norm;
            BestTargetIndex = i;
        }
    }

    if (BestTargetIndex == -1 || BestNorm > AimFov) {
        HasTarget = false;
        return;
    }

    // Convert to screen coordinates with conditional fallback
    Vec2 ScreenPos;
    if (!gGame.View.WorldToScreen(AimPosList[BestTargetIndex], ScreenPos)) {
        // When visible-only is required, do not fallback; abort to avoid through-wall locks
        if (VisibleOnly) {
            HasTarget = false;
            return;
        }
        // Otherwise fallback to angle-based movement
        Vec3 opp = AimPosList[BestTargetIndex] - LocalPos;
        float distXY = sqrtf(opp.x * opp.x + opp.y * opp.y);
        if (distXY < 0.001f) distXY = 0.001f;

        float yawDelta = atan2f(opp.y, opp.x) * 57.295779513f - Local.Pawn.ViewAngle.y;
        float pitchDelta = -atanf(opp.z / distXY) * 57.295779513f - Local.Pawn.ViewAngle.x;

        const float centerX = Gui.Window.Size.x * 0.5f;
        const float centerY = Gui.Window.Size.y * 0.5f;
        ScreenPos.x = centerX + yawDelta * 2.0f;
        ScreenPos.y = centerY + pitchDelta * 2.0f;
    }

    HasTarget = true;

    // Calculate target offset
    auto [TargetX, TargetY] = CalculateTargetOffset(ScreenPos, ScreenCenterX, ScreenCenterY);

    // Apply sensitivity scaling with safety clamp
    float sens = Local.Client.Sensitivity;
    if (sens < 0.01f) sens = 1.0f;
    TargetX /= (sens / 4.0f);
    TargetY /= (sens / 4.0f);

    // Enhanced smoothing with weapon adaptation
    if (Smooth > 0.0f)
    {
        const float DistanceRatio = BestNorm / AimFov;
        const float SpeedFactor = 1.0f + (1.0f - DistanceRatio);
        
        // Weapon-specific smoothing using config values
        float weaponSmoothFactor = 1.0f;
        if (LegitBotConfig::WeaponSpecificSmoothing) {
            if (currentWeapon.isSniper) {
                weaponSmoothFactor = LegitBotConfig::SniperSmoothFactor;
            } else if (currentWeapon.isRifle) {
                weaponSmoothFactor = LegitBotConfig::RifleSmoothFactor;
            } else if (currentWeapon.isPistol) {
                weaponSmoothFactor = LegitBotConfig::PistolSmoothFactor;
            } else {
                weaponSmoothFactor = LegitBotConfig::SMGSmoothFactor;
            }
        }
        // Precision booster for headshot-focused rifles/snipers
        bool preciseMode = HeadshotPriority && (currentWeapon.isSniper || currentWeapon.isRifle);
        float precisionFactor = preciseMode ? 0.5f : 1.0f; // lower = stronger/faster aim
        
        TargetX /= (Smooth * SpeedFactor * weaponSmoothFactor * precisionFactor);
        TargetY /= (Smooth * SpeedFactor * weaponSmoothFactor * precisionFactor);
    }

    // Anti-detection humanization (suppressed for precision headshots on rifles/snipers)
    bool suppressHumanize = HeadshotPriority && (currentWeapon.isSniper || currentWeapon.isRifle);
    if (HumanizeVar && AntiDetection && !suppressHumanize)
    {
        auto [HumanizedX, HumanizedY] = Humanize(TargetX, TargetY);
        TargetX = HumanizedX;
        TargetY = HumanizedY;
    }

    // Ultra-low latency execution
    static DWORD lastAimTime = GetTickCount64();
    DWORD currentTick = GetTickCount64();

    if (UltraLowLatency) {
        // Frame-perfect timing for maximum responsiveness
        if (currentTick - lastAimTime >= 1) { // 1ms minimum delay for stability
            mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);
            lastAimTime = currentTick;
        }
    } else {
        // Standard timing
    if (currentTick - lastAimTime >= MenuConfig::AimDelay)
    {
        mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);
        lastAimTime = currentTick;
        }
    }
}

bool AimControl::CheckAutoMode(const std::string& WeaponName)
{
    if (WeaponName == "deagle" || WeaponName == "elite" || WeaponName == "fiveseven" || WeaponName == "glock" || WeaponName == "awp" || WeaponName == "xm1014" || WeaponName == "mag7" || WeaponName == "sawedoff" || WeaponName == "tec9" || WeaponName == "zeus" || WeaponName == "p2000" || WeaponName == "nova" || WeaponName == "p250" || WeaponName == "ssg08" || WeaponName == "usp" || WeaponName == "revolver")
        return false;
    else
        return true;
}

std::string AimControl::GetWeapon(const CEntity& LocalEntity)
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

bool AimControl::CheckWeapon(const std::string& WeaponName)
{
    return !(WeaponName == "smokegrenade" || WeaponName == "flashbang" || WeaponName == "hegrenade" ||
        WeaponName == "molotov" || WeaponName == "decoy" || WeaponName == "incgrenade" ||
        WeaponName == "t_knife" || WeaponName == "ct_knife" || WeaponName == "c4");
}

bool AimControl::CheckScopeWeapon(const std::string& WeaponName)
{
    return (WeaponName == "awp" || WeaponName == "g3Sg1" || WeaponName == "ssg08" || WeaponName == "scar20");
}