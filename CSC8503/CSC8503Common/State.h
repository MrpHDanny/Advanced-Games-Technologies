#pragma once
#include <functional>
#include <string>

namespace NCL {
	namespace CSC8503 {

		typedef std::function <void(float)> StateUpdateFunction;
		
		class State		{
		public:
			State() {}
			State(StateUpdateFunction someFunc) {
				func = someFunc;
			}
			virtual ~State() {}

			std::string getName() { return name; }
			void setName(std::string n) { name = n; }

			void Update(float dt)
			{
				if (func != nullptr)
					func(dt);
			}
		protected:
			std::string name;
			StateUpdateFunction func;
		};


			//  return type name    params
		typedef void   (*StateFunc) (void*);

		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* someData) {
				func		= someFunc;
				funcData	= someData;
			}
			virtual void Update() {
				if (funcData != nullptr) {
					func(funcData);
				}
			}
		protected:
			StateFunc func;
			void* funcData;
		};
	}
}