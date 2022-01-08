#include "StateGameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject(float count)
{
	counter = 0.0f;
	limit = count;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt) -> void
		{
			this->MoveUp(dt);
		}
	);

	stateA->setName("Move Up");

	State* stateB = new State([&](float dt) -> void
		{
			this->MoveDown(dt);
		}
	);

	stateB->setName("Move Down");

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool
		{
			return this->counter > limit;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool
		{
			return this->counter < 0.0f;
		}
	));

}

StateGameObject::~StateGameObject()
{
	delete stateMachine;
}

void StateGameObject::Update(float dt)
{
	if (parent)
	{	// This node has a parent node

		// Match position
		
		Matrix3 transform = Matrix3(parent->GetTransform().GetOrientation());
		Matrix3 invTransform = Matrix3(parent->GetTransform().GetOrientation().Conjugate());

		this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition());

		// Match orientation
		this->GetTransform().SetOrientation(parent->GetTransform().GetOrientation());
	}
	stateMachine->Update(dt);
}

void StateGameObject::MoveUp(float dt)
{
	if (parent)
	{
		Matrix3 transform = Matrix3(parent->GetTransform().GetOrientation());
		Matrix3 invTransform = Matrix3(parent->GetTransform().GetOrientation().Conjugate());
		this->transform.SetOriginalPosition(this->transform.GetOriginalPosition() + Vector3(0,0, 15* -dt));
		this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition());
	}
	GetPhysicsObject()->SetLinearVelocity({ 0, 0, -15 });
	counter += dt;
}

void StateGameObject::MoveDown(float dt)
{
	if (parent)
	{
		Matrix3 transform = Matrix3(parent->GetTransform().GetOrientation());
		Matrix3 invTransform = Matrix3(parent->GetTransform().GetOrientation().Conjugate());
		this->transform.SetOriginalPosition(this->transform.GetOriginalPosition() + Vector3(0, 0, 15*dt));
		this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition());
	}
	GetPhysicsObject()->SetLinearVelocity({ 0, 0, 15 });
	counter -= dt;
}






