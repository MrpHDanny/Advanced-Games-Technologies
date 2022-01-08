#include "SpinningGameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

SpinningGameObject::SpinningGameObject(float count)
{
	counter = 0.0f;
	limit = count;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt) -> void
		{
			this->RotateRight(dt);
		}
	);

	stateA->setName("Rotate Right");

	State* stateB = new State([&](float dt) -> void
		{
			this->RotateLeft(dt);
		}
	);

	stateB->setName("Rotate Left");

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

SpinningGameObject::~SpinningGameObject()
{
	delete stateMachine;
}

void SpinningGameObject::Update(float dt)
{
	stateMachine->Update(dt);
}

void SpinningGameObject::RotateLeft(float dt)
{
	GetPhysicsObject()->SetAngularVelocity({ 0, 5, 0 });
	counter -= dt;
}

void SpinningGameObject::RotateRight(float dt)
{
	GetPhysicsObject()->SetAngularVelocity({ 0, -5, 0 });
	counter += dt;
}






