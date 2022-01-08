#pragma once

#include "../GameTech/GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/RotationConstraint.h"
#include "../CSC8503Common/StateGameObject.h"
#include "../CSC8503Common/SpinningGameObject.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/EnemyBallAI.h"
using namespace NCL;

class CourseworkGame
{
public:
	CourseworkGame();
	~CourseworkGame();
	virtual void UpdateGame(float dt);
	void UpdateKeys();

	// Object manipulation
	bool SelectObject();
	void MoveSelectedObject();
	void lockOntoObject();
	
	void ControlFloor();

	// Camera
	void lockOntoBall_2();
	void lockOntoBall_1();

	// Object spawning
	GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size);
	GameObject* AddCubeToWorld(const Vector3& position, const Vector3& size, float inverseMass);
	GameObject* AddOBBCubeToWorld(const Vector3& position, const Vector3& size, float inverseMass);
	GameObject* AddSphereToWorld(const Vector3& position, const float& radius, float inverseMass);
	GameObject* AddCapsuleToWorld(const Vector3& position, const float& halfHeight, const float& radius, const float& inverseMass);
	GameObject* AddPlayerToWorld(const Vector3& position);
	GameObject* AddEnemyToWorld(const Vector3& position);
	GameObject* AddCoinToWorld(const Vector3& position, float radius, float inverseMass, bool rotate);

	// Level spawning
	void CreateMenu();
	void CreateLevel1();
	void AddBonusesLevel1();
	void CreateLevel2();

	// Object physics
	void SpinObject(GameObject& object, const Vector3& direction, const float speed);
	void MoveObject(GameObject& object, const Vector3& direction, const float speed);
	void ToggleSelectedObject();

	// Misc
	void ChangeOrientation_y(GameObject& obj, float change);
	void ChangeOrientation_x(GameObject& obj, float change);
	void DrawUI_1();
	void DrawUI_2();
	void DrawScore();
	void DrawWinScreen();
	void DrawDebugInfo();
	void DrawMenu();
	void DrawLossScreen();
	void AddObstacleToMap(GameObject* obj);
	void FreezePlayer();

	// AI
	StateGameObject* AddMovingObstacle(const Vector3& position, const Vector3& size, float inverseMass, float count);
	SpinningGameObject* AddSpinningObstacle(const Vector3& position, const Vector3& size, float inverseMass, float count);
	EnemyBallAI* AddEnemyBallAI(const Vector3& position, const float& radius, float inverseMass);
	void UpdateStateObjects(float dt) { for (GameObject* g : stateObjects) { g->Update(dt); } }
	void MoveEnemies(float dt);
	Vector3 currentWaypoint;
	Vector3 direction;
	bool movingToWaypont = false;

	Vector3 indexToCoord(int i);
	int coordToIndex(Vector3 pos);

	bool exitGameEnabled() { return exitGame; }
	void printGrid();
	void updateGrid();

protected:
	GameObject* floor;
	GameObject* ball;
	GameObject* enemyBall;
	GameTechRenderer* renderer;
	PhysicsSystem* physics;
	GameWorld* world;
	NavigationGrid* navGrid;

	Vector3 enemyBallSpawn;
	Vector3 playerBallSpawn;
	float waypointTimer = 0;

	int grid[12][12] = { 0 };
	std::vector<GameObject*> stateObjects;
	std::vector<GameObject*> level2Objects;
	bool exitGame = false;
	bool debugMode = false;
	bool isMenu = true;
	bool isLevelOne = false;
	bool isLevelTwo = false;

	GameObject* menu_item_1;
	GameObject* menu_item_2;
	GameObject* menu_item_3;

	float floorTiltSpeed = 0.2f;
	float forceMagnitude;
	float gravityScale = 1.0f;
	Vector3 lockedOffset = Vector3(0, 14, 20);
	bool useGravity = true;
	bool inSelectionMode;
	GameObject* selectionObject = nullptr;
	GameObject* lockedObject = nullptr;

	// Asset meshes
	OGLMesh* charMeshA = nullptr;
	OGLMesh* charMeshB = nullptr;
	OGLMesh* enemyMesh = nullptr;
	OGLMesh* bonusMesh = nullptr;
	OGLMesh* capsuleMesh = nullptr;
	OGLMesh* cubeMesh = nullptr;
	OGLMesh* sphereMesh = nullptr;
	// Asset Texture and Shader
	OGLTexture* basicTex = nullptr;
	OGLShader* basicShader = nullptr;

	StateGameObject* stateObj;

	/* Initialisation functions	*/
	void InitialiseGame() { InitialiseAssets(); InitialiseCamera(); InitialiseWorld(); };

	void InitialiseAssets() {
		auto loadFunc = [](const string& name, OGLMesh** into) {
			*into = new OGLMesh(name);
			(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
			(*into)->UploadToGPU();
		};

		loadFunc("cube.msh", &cubeMesh);
		loadFunc("sphere.msh", &sphereMesh);
		loadFunc("Male1.msh", &charMeshA);
		loadFunc("courier.msh", &charMeshB);
		loadFunc("security.msh", &enemyMesh);
		loadFunc("coin.msh", &bonusMesh);
		loadFunc("capsule.msh", &capsuleMesh);

		basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
		basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	}

	void InitialiseCamera()
	{
		world->GetMainCamera()->SetNearPlane(0.1f);
		world->GetMainCamera()->SetFarPlane(500.0f);
		world->GetMainCamera()->SetPitch(-15.0f);
		world->GetMainCamera()->SetYaw(315.0f);
		world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));

		lockedObject = nullptr;
	}

	void InitialiseWorld();
};

