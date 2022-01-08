#pragma once
#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"
#include <chrono>
#include "../CSC8503Common/NavigationGrid.h"


namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld {
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c, bool andDelete = false);

			void DeleteGameObject(GameObject* obj);

			// Time
			void StartClock();
			double GetElapsedTime();
			double GetTotalTime();
			void StopClock();

			void AddBonus();
			void ResetBonus() { bonus = 0;}
			int GetBonus() { return bonus;}

			Camera* GetMainCamera() const {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void setGoalReached(bool b) { reachedGoal = b; }
			bool isGoalReached() { return reachedGoal; }

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			void setTimeLimit(int time) { this->timeLimit = time; }
			bool isGameLost() { return gameLost; }
			void setIsGameLost(bool b) { gameLost = b; }

			int getLockpadCount() { return lockpads_reached; }
			void AddLockpad() { lockpads_reached++; }
			void resetLockpadCount() { lockpads_reached = 0; }
			void DropUnlockableObstacles();
			void UpdateBonusObjects(float dt);

			void SetNavGrid(NavigationGrid* grid) { navGrid = grid; }

			void AddBonusObject(GameObject* bonus) { bonusObjects.emplace_back(bonus); }
			
			void UpdateFreezeTimer(float dt)
			{
				if (freezeEnemy || freezePlayer)
				{
					freezeTimer += dt;
				}
				else
					return;

				if (freezeTimer > freezeLimit)
				{
					freezeEnemy = false;
					freezePlayer = false;
					freezeTimer = 0.0f;
				}
			}

			bool isLevelTwo = false;
			bool freezeEnemy = false;
			bool freezePlayer = false;
			float freezeTimer = 0.0f;
			float freezeLimit = 5.0f;
			bool resetLevel = false;

			int GetLives() { return lives; }
			void ReduceLives() { lives--; }
			void ResetLives() { lives = 3; }
			bool clockRunning = false;
			std::vector<GameObject*> bonusObjects;

		protected:
			std::vector<GameObject*> gameObjects;
			std::vector<Constraint*> constraints;
			float bonusCooldownTime = 10.0f;
			Camera* mainCamera;

			NavigationGrid* navGrid;
			int lives = 3;
			int lockpads_reached = 0;
			int bonus;
			bool	shuffleConstraints;
			bool	shuffleObjects;
			int		worldIDCounter;
			bool reachedGoal = false;
			bool gameLost = false;
			int timeLimit = 999999;
			std::chrono::time_point<std::chrono::steady_clock> startTime;
			std::chrono::time_point<std::chrono::steady_clock> stopTime;
			std::chrono::duration<double> elapsedTime;
		};
	}
}

