#pragma once
#include <string>
#include "../CSC8503Common/GameObject.h"
#include "StateMachine.h"
namespace NCL
{
	namespace CSC8503
	{
		class StateMachine;
		class SpinningGameObject : public GameObject
		{
		public:
			SpinningGameObject(float count);
			~SpinningGameObject();
			std::string getActiveStateName() { return stateMachine->getName(); }
			virtual void Update(float dt);
		protected:
			void RotateLeft(float dt);
			void RotateRight(float dt);

			StateMachine* stateMachine;
			float counter;
			float limit;
		};
	}
}

