#pragma once
#include "../../Common/Matrix4.h"
#include "../../Common/Matrix3.h"
#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"

#include <vector>

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);
			void SetOriginalPosition(const Vector3& worldPos) { originalPosition = worldPos; }
			Vector3 GetOriginalPosition() const {
				return originalPosition;
			}

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return matrix;
			}

			void SetMatrix(Matrix4 mat)
			{
				matrix = mat;
			}
			void UpdateMatrix();

			void LockTransform() { locked = true; }
			void UnlockTransform() { locked = false; }
		protected:
			Matrix4		matrix;
			Quaternion	orientation;
			Vector3		position;
			Vector3		originalPosition;
			bool		locked = false;
			Vector3		scale;
		};
	}
}

