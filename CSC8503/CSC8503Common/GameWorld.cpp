#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "../../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld()	{
	mainCamera = new Camera();

	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;
	lives = 3;
}

GameWorld::~GameWorld()	{
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
	bonusObjects.clear();
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();

}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete) {
		delete o;
	}
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last) const {

	first	= gameObjects.begin();
	last	= gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
		g->Update(0);
	}
}

void GameWorld::UpdateWorld(float dt) {
	if (shuffleObjects) {
		std::random_shuffle(gameObjects.begin(), gameObjects.end());
	}

	if (shuffleConstraints) {
		std::random_shuffle(constraints.begin(), constraints.end());
	}
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject) const {
	//The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;

	for (auto& i : gameObjects) {
		if (!i->GetBoundingVolume() || !i->canInteract) { //objects might not be collideable etc...
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
				
			if (!closestObject) {	
				closestCollision		= collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		} 
	}
	if (collision.node) {
		
		// Draw raycast
		//Debug::DrawLine(Vector3(mainCamera->GetPosition().x, mainCamera->GetPosition().y-50, mainCamera->GetPosition().z), collision.collidedAt, Debug::BLUE, 1);
		
		closestCollision		= collision;
		closestCollision.node	= collision.node;
		return true;
	}
	return false;
}


/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c, bool andDelete) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete) {
		delete c;
	}
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin();
	last	= constraints.end();
}

void GameWorld::StartClock()
{	
	clockRunning = true;
	startTime = std::chrono::steady_clock::now();
}

double GameWorld::GetElapsedTime()
{
	std::chrono::time_point<std::chrono::steady_clock> current = std::chrono::steady_clock::now();
	float total = (current - startTime).count() / pow(1000, 3);

	if (isGoalReached())
	{
		StopClock();
		return GetTotalTime();
	}

	if (total > timeLimit)
	{
		gameLost = true;
		StopClock();
		return GetTotalTime();
	}
	return (current - startTime).count() / pow(1000,3);
}

double GameWorld::GetTotalTime()
{
	return (stopTime - startTime).count() / pow(1000,3);
}

void GameWorld::StopClock()
{
	if (!clockRunning) return;

	clockRunning = false;
	stopTime = std::chrono::steady_clock::now();
}

void GameWorld::DropUnlockableObstacles()
{
	for (GameObject* obj : gameObjects)
	{
		if (obj->GetName() == "unlockable")
		{
			navGrid->removeBonus(obj); // should rename it to removeObject...
			obj->GetTransform().SetOriginalPosition(obj->GetTransform().GetOriginalPosition() + Vector3(0,-40,0));
		}
	}
}

void GameWorld::DeleteGameObject(GameObject* obj)
{
	std::vector<GameObject*>::iterator position = std::find(gameObjects.begin(), gameObjects.end(), obj);
	if(position != gameObjects.end()) 
		gameObjects.erase(position);
}

void GameWorld::AddBonus()
{
	bonus++;
}

void NCL::CSC8503::GameWorld::UpdateBonusObjects(float dt)
{
	for (GameObject* obj : bonusObjects)
	{
		if (!obj->bonusTimerActive)
			continue;
		
		obj->bonusCooldownTimer += dt;

		if (obj->bonusCooldownTimer > bonusCooldownTime)
		{
			obj->bonusTimerActive = false;
			obj->bonusCooldownTimer = 0.0f;
			obj->GetTransform().SetOriginalPosition(obj->GetTransform().GetOriginalPosition() + Vector3(0,30,0));
			navGrid->emplaceBonus(obj);
		}
	}
}
