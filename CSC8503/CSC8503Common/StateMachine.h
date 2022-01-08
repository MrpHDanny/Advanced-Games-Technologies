#pragma once
#include <vector>
#include <map>
#include <string>
#include "State.h"

namespace NCL {
	namespace CSC8503 {

		class State;
		class StateTransition;

		typedef std::multimap<State*, StateTransition*> TransitionContainer;
		typedef TransitionContainer::iterator TransitionIterator;

		class StateMachine	{
		public:
			StateMachine();
			~StateMachine();

			void AddState(State* s);
			void AddTransition(StateTransition* t);
			std::string getName() { return activeState->getName(); }
			void Update(float dt);

		protected:
			State * activeState;

			std::vector<State*> allStates;

			TransitionContainer allTransitions;
		};
	}
}