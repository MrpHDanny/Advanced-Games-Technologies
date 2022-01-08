#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	bonusCooldownTimer = false;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

void NCL::CSC8503::GameObject::Update(float dt)
{
	if (parent)
	{	// This node has a parent node

		// Match position
		if (!onlyMatchOrientation)
		{
			Matrix3 transform = Matrix3(parent->transform.GetOrientation());
			Matrix3 invTransform = Matrix3(parent->transform.GetOrientation().Conjugate());

			this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition());

		}
		
		// Match orientation
		if (!isSpinning)
		{
			this->GetTransform().SetOrientation(parent->GetTransform().GetOrientation());
		}
		else
		// Only used for bonus coins
		this->GetPhysicsObject()->SetAngularVelocity(Vector3(1, 3, 2));
	}

	if (springParent)
	{
		UpdateSprings();
	}

	for (vector<GameObject*>::iterator i = children.begin(); i != children.end(); ++i)
	{
		(*i)->Update(dt);
	}
	
	if (!performAction)
		return;

	/* Perform action when enabled by player */
	switch (action)
	{
		case(actionType::SPIN_CLOCKWISE):
			spinObject(Vector3(0, -1, 0), 6);
		break;

		case(actionType::SPIN_ANTICOCKWISE):
			spinObject(Vector3(0, 1, 0), 6);
		break;

		case(actionType::GO_RIGHT):
			MoveObject(Vector3(1, 0, 0), 25);
		break;

		case(actionType::GO_LEFT):
			MoveObject(Vector3(-1, 0, 0), 25);
		break;

		case(actionType::GO_DOWN):
			MoveObject(Vector3(0, 0, 1), 25);
		break;

		case(actionType::GO_UP):
			MoveObject(Vector3(0, 0, -1), 25);
		break;
	}
}

void GameObject::spinObject(Vector3 direction, float speed)
{
	physicsObject->SetAngularVelocity(direction * speed);
}

void GameObject::MoveObject(Vector3 direction, float speed)
{
	canToggle = false;

	if (maxDistanceReached)
		physicsObject->SetLinearVelocity(-direction * speed);
	else
		physicsObject->SetLinearVelocity(direction * speed);

	Vector3 diff = transform.GetPosition() - transform.GetOriginalPosition();
	float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);


	if (distance > maxDistance)
		maxDistanceReached = true;

	if (maxDistanceReached)
	{
		if (distance <= 1)
		{
			maxDistanceReached = false;
			physicsObject->SetLinearVelocity(Vector3(0,0,0));
			performAction = false;
			canToggle = true;
		}
	}
}

void GameObject::UpdateSprings()
{
	const float c = 0.5;
	const float snappiness = 0.5; // k
	const float restingDistance = springDistance;
	Vector3 lhs_vel = physicsObject->GetLinearVelocity();
	Vector3 rhs_vel = springParent->GetPhysicsObject()->GetLinearVelocity();

	Vector3 normal = (transform.GetPosition() - springParent->GetTransform().GetPosition()).Normalised();
	float distance = (transform.GetPosition() - springParent->GetTransform().GetPosition()).Length() - restingDistance; // x

	float force = -snappiness * distance; // F
	float velOnSpringAxis = Vector3::Dot(lhs_vel - rhs_vel, normal);

	float lhs_force_damped = force - c * velOnSpringAxis;

	Vector3 pos = transform.GetPosition();
	pos.y = springParent->GetTransform().GetPosition().y;
	transform.SetPosition(pos);
	GetPhysicsObject()->SetLinearVelocity(normal * lhs_force_damped);
}