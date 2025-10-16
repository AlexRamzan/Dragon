#include "RCS.h"
#include "Aimbot.h"
#include <algorithm>

void RCS::UpdateAngles(const CEntity& Local, Vec2& Angles)
{
	auto oldPunch = Vec2{ };
	auto shotsFired = Local.Pawn.ShotsFired;

	int ScreenCenterX = Gui.Window.Size.x / 2;
	int ScreenCenterY = Gui.Window.Size.y / 2;

	if (shotsFired)
	{
		uintptr_t clientState;
		auto viewAngles = Local.Pawn.ViewAngle;
		auto aimPunch = Local.Pawn.AimPunchAngle;

		auto newAngles = Vec2
		{
			viewAngles.x + oldPunch.x - aimPunch.x * 2.f,
			viewAngles.y + oldPunch.y - aimPunch.y * 2.f,
		};

		if (newAngles.x > 89.f)
			newAngles.x = 89.f;

		if (newAngles.x < -89.f)
			newAngles.x = -89.f;

		while (newAngles.y > 180.f)
			newAngles.y -= 360.f;

		while (newAngles.y < -180.f)
			newAngles.y += 360.f;

		newAngles.x += ScreenCenterX;
		newAngles.y += ScreenCenterY;

		Angles = newAngles;
		oldPunch = aimPunch;
	}
	else
	{
		oldPunch.x = oldPunch.y = 0.f;
	}

    Vec2 PunchAngle;
    if (Local.Pawn.AimPunchCache.Count <= 0 || Local.Pawn.AimPunchCache.Count > 0xFFFF)
        return;
	if (!memoryManager.ReadMemory<Vec2>(Local.Pawn.AimPunchCache.Data + (Local.Pawn.AimPunchCache.Count - 1) * sizeof(Vec3), PunchAngle))
		return;

	Angles.x = PunchAngle.x;
	Angles.y = PunchAngle.y;
}

void RCS::RecoilControl(CEntity LocalPlayer)
{
	if (!LegitBotConfig::RCS)
		return;

    // Optionally do not let RCS fight the aimbot; pause if actively tracking
    if (RespectAimbot && AimControl::HasTarget)
        return;

	static Vec2 OldPunch;

    if (LocalPlayer.Pawn.ShotsFired > RCSBullet)
	{
        // Read current punch (pitch=x, yaw=y) and compute delta from last tick
        Vec2 currentPunch = LocalPlayer.Pawn.AimPunchAngle * 2.0f;
        Vec2 deltaPunch = { currentPunch.x - OldPunch.x, currentPunch.y - OldPunch.y };

        // Choose strength: no-recoil applies full counter; otherwise smooth it
        float smooth = (RCSSmooth < 1.0f) ? 1.0f : RCSSmooth;
        Vec2 applied = NoRecoil ? deltaPunch : Vec2{ deltaPunch.x / smooth, deltaPunch.y / smooth };

        // Map to mouse deltas: dx counters yaw, dy counters pitch
        // Positive dy moves cursor down; positive pitch recoil (up) needs positive dy to compensate
        float sens = LocalPlayer.Client.Sensitivity;
        if (sens < 0.01f) sens = 1.0f;
        int dx = static_cast<int>(std::round(( -applied.y * RCSScale.x / sens) / 0.011f));
        int dy = static_cast<int>(std::round((  applied.x * RCSScale.y / sens) / 0.011f));

        mouse_event(MOUSEEVENTF_MOVE, dx, dy, NULL, NULL);

        // Update last punch for next frame
        OldPunch = currentPunch;
	}
	else
		OldPunch = Vec2{ 0,0 };
}