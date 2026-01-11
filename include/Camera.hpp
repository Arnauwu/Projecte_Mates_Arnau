#pragma once
#include "Matrix3x3.hpp"
#include "Matrix4x4.hpp"
#include "Transform.hpp"

class Camera
{
public:
	Camera(Vec3 pos, float fov, float nPlane, float fPlane);
	Camera(Transform trans, float fov, float nPlane, float fPlane);
	~Camera();

	Transform GetTransform();
	float GetFOV();
	float GetNearPlane();
	float GetFarPlane();

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetFOV(float nFOV);
	void SetAspectRatio(float a);
	void SetNearPlane(float n);
	void SetFarPlane(float f);

	Matrix4x4 GetGlobalMatrix();
	Matrix4x4 GetViewMatrix();
	Matrix4x4 GetProjectionMatrix();


private:

	Transform cameraTranform;
	float FOV;
	float aspectRatio;
	float nearPlane;
	float farPlane;
};
