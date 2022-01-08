#pragma once
#include <string>
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/NavigationPath.h"
#include "StateMachine.h"
#include "GameWorld.h"
namespace NCL
{
	namespace CSC8503
	{
		class StateMachine;
		class EnemyBallAI : public GameObject
		{
		public:
			EnemyBallAI();
			~EnemyBallAI();
			void calculateDistances();
			void setNavGrid(NavigationGrid* grid) { navGrid = grid; }
			void setGameWorld(GameWorld* world) { this->world = world; }
			void setBonuses(vector<GameObject*> objects) { bonuses = objects; }
			void setPlayer(GameObject* p) { player = p; }
			std::string getActiveStateName() { return stateMachine->getName(); }
			virtual void Update(float dt);
			void followPlayer(float dt);
			void moveToBonus(float dt);
			void UpdateParentedPos();
		protected:
			vector<GameObject*> bonuses;
			GameObject* player;
			GameObject* closestBonus;
			GameWorld* world;
			NavigationGrid* navGrid;
			bool chasePlayer = true;
			bool grabBonus = false;
			bool movingToWaypoint = false;
			Vector3 currentWaypoint;
			float waypointTimer;
			Vector3 direction;
			float bonusGrabTimer = 0.0f;
			float distanceToBonus = 0.0f;
			float distanceToPlayer = 0.0f;
			StateMachine* stateMachine;
		};
	}
}

