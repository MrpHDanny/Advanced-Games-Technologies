#include "RotationConstraint.h"
#include "GameObject.h"

void NCL::CSC8503::RotationConstraint::UpdateConstraint(float dt)
{
	
	Vector3 relativeAng = objectA->GetTransform().GetOrientation().ToEuler() - objectB->GetTransform().GetOrientation().ToEuler(); // J

	float currentDistance = relativeAng.Length();
	float offset = distance - currentDistance;

	if (abs(offset) > 0.0f)
	{
		Vector3 offsetDir = relativeAng.Normalised(); // J normalized

		PhysicsObject* physA = objectA->GetPhysicsObject();
		PhysicsObject* physB = objectB->GetPhysicsObject();

		Vector3 relativeVelocity = (physA->GetAngularVelocity() - physB->GetAngularVelocity()); // v
		Vector3 j = Vector3::Cross(relativeVelocity, relativeAng).Normalised();

		float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

		if (constraintMass > 0.0f)
		{
			//how much of their relative force is affecting the constraint
			float velocityDot = Vector3::Dot(j, relativeVelocity); // C
			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 aImpulse = offsetDir * lambda / 5;
			Vector3 bImpulse = -offsetDir * lambda / 5;

			physA->ApplyAngularImpulse(aImpulse);// multiplied by mass here
			physB->ApplyAngularImpulse(bImpulse);// multiplied by mass here
		}
	}
}
