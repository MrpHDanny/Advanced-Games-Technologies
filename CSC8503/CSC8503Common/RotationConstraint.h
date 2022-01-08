#pragma once
#include "Constraint.h"
#include "../../Common/Vector3.h"

namespace NCL
{
	namespace CSC8503
	{
		class GameObject;

		class RotationConstraint : public Constraint
		{
		public:
			RotationConstraint(GameObject* a, GameObject* b, float d)
			{
				objectA = a;
				objectB = b;
				distance = d;
			}

			~RotationConstraint() {};

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
			float distance;
		};
	}
}
