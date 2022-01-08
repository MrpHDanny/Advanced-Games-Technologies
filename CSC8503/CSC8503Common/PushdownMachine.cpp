#include "PushdownMachine.h"
#include "PushdownState.h"
using namespace NCL::CSC8503;


PushdownMachine::~PushdownMachine()
{
}

bool PushdownMachine::Update(float dt) {
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = activeState->PushdownUpdate(dt, &newState);

		switch (result) {
			case PushdownState::Pop: {
				activeState->OnSleep();
				delete activeState; // tutorial
				stateStack.pop();
				if (stateStack.empty()) {
					return false; // tutprial
					activeState = nullptr; //??????
				}
				else {
					activeState = stateStack.top();
					activeState->OnAwake();
				}
			}break;
			
			case PushdownState::Push: {
				activeState->OnSleep();
				stateStack.push(newState);
				newState->OnAwake();
			}break;
		}
	}
	else
	{
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake();
	}
	return true;
}