#pragma once
#include "Matrix3x3.hpp"

struct Transform
{
	Vec3 position = { 0,0,0 };
	Vec3 eulerRotation = { 0,0,0 };
	Vec3 scale = { 1,1,1 };
};