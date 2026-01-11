#pragma once
#include "Matrix3x3.hpp"
#include "Matrix4x4.hpp"
#include "Transform.hpp"
#include <vector>

using namespace std;

class GameObject
{
public:

	GameObject(Transform transform, GameObject* parent);
	~GameObject();

	Vec3 GetPosition();
	Vec3 GetRotation();
	Vec3 GetScale();
	vector<GameObject*> GetChilds();
	string GetName();
	Vec3 GetColor();
	GameObject* GetParent() { return parent; }

	void SetPosition(Vec3 pos);
	void SetRotation(Vec3 rot);
	void SetScale(Vec3 scale);
	void SetName(string name);
	void SetColor(float r, float g, float b);

	Matrix4x4 GetLocalMatrix();
	Matrix4x4 GetGlobalMatrix();

	void AddChild(GameObject* child);
	bool isLeaf();
	void RemoveChild(GameObject* child);

private:

	string name = "Nameless";
	Transform transform;
	GameObject* parent = nullptr;
	vector<GameObject*> childs;
	Vec3 Color = { 1.0f, 0.58f, 0.0f };
};