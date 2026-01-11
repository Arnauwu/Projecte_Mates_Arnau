#include "Camera.hpp"

Camera::Camera(Vec3 pos, float fov, float nPlane, float fPlane)
{
	this->cameraTranform.position = pos;
	FOV = fov;
	nearPlane = nPlane;
	farPlane = fPlane;
	this->cameraTranform.eulerRotation = Vec3(0, 0, 0);
	this->cameraTranform.scale = Vec3(1, 1, 1);
}

Camera::Camera(Transform trans, float fov, float nPlane, float fPlane)
{
	this->cameraTranform = trans;
	FOV = fov;
	nearPlane = nPlane;
	farPlane = fPlane;
}

Camera::~Camera(){}

Transform Camera::GetTransform() { return cameraTranform; }
float Camera::GetFOV() { return this->FOV; }
float Camera::GetNearPlane() { return nearPlane; }
float Camera::GetFarPlane() { return farPlane; }

void Camera::SetPosition(float x, float y, float z) { cameraTranform.position.x = x; cameraTranform.position.y = y; cameraTranform.position.z = z; }
void Camera::SetRotation(float x, float y, float z) { cameraTranform.eulerRotation.x = x; cameraTranform.eulerRotation.y = y; cameraTranform.eulerRotation.z = z; }
void Camera::SetFOV(float fov) { FOV = fov; }
void Camera::SetAspectRatio(float a) { aspectRatio = a; }
void Camera::SetNearPlane(float n) { nearPlane = n; }
void Camera::SetFarPlane(float f) { farPlane = f; }

Matrix4x4 Camera::GetGlobalMatrix()
{
	Matrix4x4 global;
	Matrix3x3 rotation = Matrix3x3::FromEulerZYX(cameraTranform.eulerRotation.x, cameraTranform.eulerRotation.y, cameraTranform.eulerRotation.z);
	global = global.FromTRS(cameraTranform.position, rotation, cameraTranform.scale);
	
	return global;
}


Matrix4x4 Camera::GetViewMatrix()
{
	return GetGlobalMatrix().InverseTRS();
}

Matrix4x4 Camera::GetProjectionMatrix() 
{
	Matrix4x4 projection;
	
	float fovRad = FOV * (3.14159265359f / 180.0f);
	float f = 1.0f / tanf(fovRad * 0.5f);

	projection.At(0, 0) = f / aspectRatio;
	projection.At(1, 1) = f;
	projection.At(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
	projection.At(2, 3) = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
	projection.At(3, 2) = -1.0f;

	return projection;
}