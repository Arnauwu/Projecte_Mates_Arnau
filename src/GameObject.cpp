#include "GameObject.hpp"

GameObject::GameObject(Transform transform, GameObject* parent)
{
	this->transform = transform;
	if (parent != nullptr)
	{
		this->parent = parent;
	}
}

GameObject::~GameObject() {
	for (GameObject* child : childs) 
	{
		delete child;
	}
	childs.clear();
}

Vec3 GameObject::GetPosition(){return transform.position;}
Vec3 GameObject::GetRotation(){return transform.eulerRotation;}
Vec3 GameObject::GetScale(){return transform.scale;}
vector<GameObject*> GameObject::GetChilds(){return childs;}
string GameObject::GetName() { return name; }
Vec3 GameObject::GetColor() { return Color; }

void GameObject::SetPosition(Vec3 pos){transform.position = pos;}
void GameObject::SetRotation(Vec3 rot){transform.eulerRotation = rot;}
void GameObject::SetScale(Vec3 scale){transform.scale = scale;}
void GameObject::SetName(string nName) { name = nName; }
void GameObject::SetColor(float r, float g, float b) { Color.x = r; Color.y = g; Color.z = b; }

Matrix4x4 GameObject::GetLocalMatrix()
{
	Matrix4x4 local;
	Matrix3x3 rotation = Matrix3x3::FromEulerZYX(transform.eulerRotation.x, transform.eulerRotation.y, transform.eulerRotation.z);
	local = local.FromTRS(transform.position, rotation, transform.scale);
	return local;
}

Matrix4x4 GameObject::GetGlobalMatrix()
{
	GameObject* parent = this->parent;
	Matrix4x4 local = GetLocalMatrix();
	while (parent != nullptr)
	{
		Matrix4x4 parentLocal = parent->GetLocalMatrix();
		local = parentLocal.Multiply(local);
		parent = parent->parent;
	}

	return local;
}

void GameObject::AddChild(GameObject* child){childs.push_back(child);}

bool GameObject::isLeaf()
{
	if (childs.size() == 0)
	{
		return true;
	}

		return false;
}

void GameObject::RemoveChild(GameObject* child) {
	for (auto i = childs.begin(); i != childs.end(); ++i) 
	{
		if (*i == child) 
		{
			childs.erase(i);
			break;
		}
	}
}
