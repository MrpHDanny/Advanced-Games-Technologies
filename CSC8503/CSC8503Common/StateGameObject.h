#pragma once
#include <string>
#include "../CSC8503Common/GameObject.h"
#include "StateMachine.h"
namespace NCL
{
	namespace CSC8503
	{
		class StateMachine;
		class StateGameObject : public GameObject
		{
		public:
			StateGameObject(float count);
			~StateGameObject();
			std::string getActiveStateName() { return stateMachine->getName(); }
			virtual void Update(float dt);
		protected:
			void MoveUp(float dt);
			void MoveDown(float dt);

			StateMachine* stateMachine;
			float counter;
			float limit;
		};
	}
}

