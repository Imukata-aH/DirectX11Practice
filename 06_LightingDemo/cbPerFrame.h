#pragma once

#include "LightHelper.h"

struct ConstantsPerFrame
{
	DirectionalLight mDirLight;
	PointLight mPointLight;
	XMFLOAT3 mEyePosW;
};