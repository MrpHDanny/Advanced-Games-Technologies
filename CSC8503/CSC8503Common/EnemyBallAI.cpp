#include "EnemyBallAI.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

EnemyBallAI::EnemyBallAI()
{
	waypointTimer = 0.0f;
	stateMachine = new StateMachine();

	State* chase = new State([&](float dt) -> void
		{
			this->followPlayer(dt);
		}
	);
	
	chase->setName("Chase Player");

	State* grabBonus = new State([&](float dt) -> void
		{
			this->moveToBonus(dt);
		}
	);

	grabBonus->setName("Grab Bonus");
	stateMachine->AddState(chase);
	stateMachine->AddState(grabBonus);

	stateMachine->AddTransition(new StateTransition(chase, grabBonus, [&]()->bool
		{
			return !chasePlayer;
		}
	));

	stateMachine->AddTransition(new StateTransition(grabBonus, chase, [&]()->bool
		{
			return chasePlayer;
		}
	));
}

EnemyBallAI::~EnemyBallAI()
{
	delete stateMachine;
}

void EnemyBallAI::Update(float dt)
{
	UpdateParentedPos();
	calculateDistances();

	if (!bonusTimerActive)
	{
		if (distanceToBonus < distanceToPlayer)
			chasePlayer = false;
		else
			chasePlayer = true;
	}
	else if (bonusTimerActive)
	{
		bonusCooldownTimer += dt;
		chasePlayer = true;

		if (bonusCooldownTimer > 12)	
		{
			std::cout << "timer stopping" << std::endl;
			bonusCooldownTimer = 0.0f;
			bonusTimerActive = false;
		}
	}


	stateMachine->Update(dt);
}

void EnemyBallAI::calculateDistances()
{
	distanceToBonus = 999999.0f;
	float current = 0.0f;
	for (GameObject* obj : bonuses)
	{
		current = (this->getParentedPosition() - obj->getParentedPosition()).Length();
		if (current < distanceToBonus)
		{
			distanceToBonus = current;
			closestBonus = obj;
		}
	}

	distanceToPlayer = (this->getParentedPosition() - player->GetTransform().GetPosition()).Length();
}


void NCL::CSC8503::EnemyBallAI::UpdateParentedPos()
{
	// Match position
	Matrix3 transform = Matrix3(parent->GetTransform().GetOrientation());
	Matrix3 invTransform = Matrix3(parent->GetTransform().GetOrientation().Conjugate());
	this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition());

	// Match orientation
	this->GetTransform().SetOrientation(parent->GetTransform().GetOrientation());
}

void NCL::CSC8503::EnemyBallAI::followPlayer(float dt)
{
	Vector3 currentPos = this->getParentedPosition();
	vector <Vector3> pathNodes;

	// Find path to player
	NavigationPath outPath;
	Vector3 to = player->GetTransform().GetPosition();
	navGrid->FindPath(currentPos, to, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
		pathNodes.push_back(pos);

	if (!movingToWaypoint)
	{
		if (pathNodes.size() > 1)
		{
			currentWaypoint = pathNodes[1];
			currentWaypoint.y = 7;
		}
		movingToWaypoint = true;
	}

	float distance = (currentWaypoint - currentPos).Length();
	direction = (currentWaypoint - currentPos).Normalised();

	float speed = 15;

	Vector3 currentOriginalPos = this->GetTransform().GetOriginalPosition();

	if (!world->freezeEnemy)
		this->GetTransform().SetOriginalPosition(currentOriginalPos + direction * dt * speed);

	waypointTimer += dt;

	if (waypointTimer > 3)	// possibly got stuck
	{
		movingToWaypoint = false;
		waypointTimer = 0;
	}

	if (distance < 0.2)
	{
		movingToWaypoint = false;
		waypointTimer = 0;
	}

	// Draw path
	/*
	for (int i = 1; i < pathNodes.size(); ++i)
	{

		Vector3 a = pathNodes[i - 1];
		Vector3 b = pathNodes[i];
		a.y += 20;
		b.y += 20;
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
	*/
}

void NCL::CSC8503::EnemyBallAI::moveToBonus(float dt)
{
	Vector3 currentPos = this->getParentedPosition();
	vector <Vector3> pathNodes;

	// Find path to bonus
	NavigationPath outPath;
	Vector3 to = closestBonus->getParentedPosition();
	navGrid->FindPath(currentPos, to, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
		pathNodes.push_back(pos);

	if (!movingToWaypoint)
	{
		if (pathNodes.size() > 1)
		{
			currentWaypoint = pathNodes[1];
			currentWaypoint.y = 7;
		}
		movingToWaypoint = true;
	}

	float distance = (currentWaypoint - currentPos).Length();
	direction = (currentWaypoint - currentPos).Normalised();

	float speed = 15;

	Vector3 currentOriginalPos = this->GetTransform().GetOriginalPosition();

	if (!world->freezeEnemy)
		this->GetTransform().SetOriginalPosition(currentOriginalPos + direction * dt * speed);

	waypointTimer += dt;

	if (waypointTimer > 3)	// possibly got stuck
	{
		movingToWaypoint = false;
		waypointTimer = 0;
	}

	if (distance < 0.2)
	{
		movingToWaypoint = false;
		waypointTimer = 0;
	}

	// Draw path
	/*
	for (int i = 1; i < pathNodes.size(); ++i)
	{

		Vector3 a = pathNodes[i - 1];
		Vector3 b = pathNodes[i];
		a.y += 20;
		b.y += 20;
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
	*/
}






