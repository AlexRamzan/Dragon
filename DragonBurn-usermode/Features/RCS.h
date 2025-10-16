#pragma once
#include "Aimbot.h"

namespace RCS
{
	inline int RCSBullet = 1;
	inline Vec2 RCSScale = { 1.4f,1.4f };
    inline float RCSSmooth = 1.0f; // 1.0 = raw, >1.0 = smoother incremental correction
    inline bool NoRecoil = true;   // default ON: fully cancel aim punch each tick
    inline bool RespectAimbot = true; // if true, pause when aimbot is active

	void UpdateAngles(const CEntity&, Vec2&);
	void RecoilControl(CEntity);
}
