#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			void SetName(string name) {
				this->name = name;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

			Vector3 getParentedPosition()
			{
				return transform.GetOriginalPosition() + parent->GetTransform().GetPosition();
			}

			void AddChild(GameObject* g) { children.push_back(g); g->parent = this;}
			virtual void Update(float dt);
			std::vector<GameObject*>::const_iterator GetChildIteratorStart() { return children.begin(); }
			std::vector<GameObject*>::const_iterator GetChildIteratorEnd() { return children.end(); }
			
			enum  actionType
			{
				NO_ACTION,
				SPIN_CLOCKWISE,
				SPIN_ANTICOCKWISE,
				GO_UP,
				GO_DOWN,
				GO_LEFT,
				GO_RIGHT
			};

			void setActionType(actionType t) { action = t; }
			actionType getActionType() { return action; }
			void SetMaxDistance(float dist) { maxDistance = dist; }
			void toggleAction() { performAction = !performAction; }

			void spinObject(Vector3 direction, float speed);
			void MoveObject(Vector3 direction, float speed);
			bool	canToggle = true;
			bool canInteract = false;
			bool	isAI = false;
			void AttachSpringTo(GameObject* rhs, float distance) {
				springParent = rhs;
				springDistance = distance;
			};
			void UpdateSprings();

			float bonusCooldownTimer = 0.0f;
			bool bonusTimerActive = false;
			bool isSpinning = false;
			bool onlyMatchOrientation = false;
		protected:
			Transform			transform;
			actionType action = NO_ACTION;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;

			bool	isActive;
			bool	performAction = false;
			bool	maxDistanceReached = false;
			int		worldID;
			string	name;
			float maxDistance = 0;

			Vector3 broadphaseAABB;
			GameObject* parent;
			GameObject* springParent;
			float springDistance;
			std::vector<GameObject*> children;
		};
	}
}

