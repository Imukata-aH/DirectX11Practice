#pragma once

#include "LightHelper.h"

struct ConstantsPerFrame
{
	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	XMFLOAT3 mEyePosW;
};