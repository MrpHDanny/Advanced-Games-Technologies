#include "CourseworkGame.h"
#include <chrono>
#include <iomanip>

CourseworkGame::CourseworkGame()
{
	forceMagnitude = 10.0f;
	gravityScale = 10.0f;
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);
	physics->SetGravity(Vector3(0.0f, -9.8f * gravityScale, 9.8f * 6));
	Debug::SetRenderer(renderer);
	InitialiseGame();
}

CourseworkGame::~CourseworkGame()
{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void CourseworkGame::InitialiseWorld()
{
	world->ClearAndErase();
	physics->Clear();
	selectionObject = nullptr;
	stateObjects.clear();
	world->setGoalReached(false);
	world->ResetBonus();
	if(isLevelOne || isMenu)
		world->setIsGameLost(false);

	world->setGoalReached(false);
	world->freezeEnemy = false;
	world->freezePlayer = false;
	world->freezeTimer = 0.0f;
	world->resetLockpadCount();

	if (isLevelOne)
	{
		inSelectionMode = true;
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		CreateLevel1();
		world->StartClock();
		return;
	}
	if (isLevelTwo)
	{
		if(world->GetLives() == 0)
			world->ResetLives();
		CreateLevel2();
		world->StartClock();
		return;
	}
	if (isMenu)
	{
		inSelectionMode = true;
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		CreateMenu();
		return;
	}
}

void CourseworkGame::UpdateGame(float dt)
{
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	if (selectionObject && selectionObject->canInteract)
		ToggleSelectedObject();
	
	if (isMenu)
	{ 
		DrawMenu();
	}

	if (isLevelOne)
	{
		lockOntoBall_1();
		DrawScore();
		DrawUI_1();
		if (debugMode)
			DrawDebugInfo();
	}

	if (isLevelTwo)
	{
		lockOntoBall_2();
		DrawUI_2();
		navGrid->UpdateGrid();
		//navGrid->PrintGrid();	// Print grid to console
		
		world->UpdateBonusObjects(dt);
		world->UpdateFreezeTimer(dt);

		if (world->freezePlayer)
		{
			FreezePlayer();
			floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		}
		else
		{
			ControlFloor();
		}

		if (world->resetLevel)
		{
			world->resetLevel = false;
			InitialiseWorld();
		}

		if (world->isGoalReached() || world->isGameLost())
		{
			world->freezeEnemy = true;
			world->freezePlayer = true;
		}
	}

	UpdateStateObjects(dt);
	physics->Update(dt);

	world->UpdateWorld(dt);
	renderer->Update(dt);
	Debug::FlushRenderables(dt);
	renderer->Render();

	SelectObject();
	UpdateKeys();
}

void CourseworkGame::lockOntoObject()
{
	Vector3 objPos = lockedObject->GetTransform().GetPosition();
	Vector3 camPos = objPos + lockedOffset;

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);

	//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
}

void CourseworkGame::lockOntoBall_2()
{
	lockedObject = ball;
	Vector3 camOffset(0, 150, 200);
	
	Vector3 ballPos = ball->GetTransform().GetPosition();
	Vector3 camPos = ballPos + camOffset;

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, ballPos, Vector3(0, 1, 0));

	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetPitch(-45);
	world->GetMainCamera()->SetYaw(0);
}

void CourseworkGame::lockOntoBall_1()
{
	float ball_z = ball->GetTransform().GetPosition().z;
	Vector3 camPos = world->GetMainCamera()->GetPosition();
	camPos.y = 400;
	camPos.z = ball_z;
	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetPitch(-90);
	world->GetMainCamera()->SetYaw(0);
}

void CourseworkGame::UpdateKeys()
{
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitialiseWorld(); //We can reset the simulation at any time with F1
	if (isLevelTwo)
		world->setIsGameLost(false);
		world->ResetLives();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitialiseCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		debugMode = !debugMode;
	}

	// back to menu
	if ((isLevelOne || isLevelTwo) && Window::GetKeyboard()->KeyPressed(KeyboardKeys::M))
	{
		isLevelOne = false;
		isLevelTwo = false;
		isMenu = true;
		InitialiseGame();
		return;
	}
		

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}
}

void CourseworkGame::ControlFloor()
{
	floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));

	floor->GetPhysicsObject()->InitCubeInertia();
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP))
	{
		floor->GetPhysicsObject()->SetAngularVelocity(Vector3(-1, 0, 0) * floorTiltSpeed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN))
	{
		floor->GetPhysicsObject()->SetAngularVelocity(Vector3(1, 0, 0) * floorTiltSpeed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT))
	{
		floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, -1) * floorTiltSpeed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT))
	{
		floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 1) * floorTiltSpeed);
	}

	floor->Update(0);
}

bool CourseworkGame::SelectObject()
{
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				if (selectionObject->isAI)
					selectionObject->GetRenderObject()->SetColour(Debug::CYAN);
				else
					selectionObject->GetRenderObject()->SetColour(Debug::RED);
				selectionObject = nullptr;
				lockedObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {

				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		//renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}
	if (lockedObject) {
		//renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}
	else if (selectionObject) {
		//renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}
	}
	return false;
}

void CourseworkGame::ToggleSelectedObject()
{
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::E) && selectionObject->canToggle)
	{
		// Platforms in level 1
		selectionObject->toggleAction();
		
		// Menu controls
		if (selectionObject->GetName() == "menu_level_1")
		{
			isMenu = false;
			isLevelOne = true;
			InitialiseWorld();
			return;
		}
		if (selectionObject->GetName() == "menu_level_2")
		{
			isMenu = false;
			isLevelOne = false;
			isLevelTwo = true;
			InitialiseWorld();
			return;
		}
		if (selectionObject->GetName() == "exit")
		{
			exitGame = true;
			return;
		}

	}

	
}

void CourseworkGame::MoveSelectedObject()
{
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude),
		Vector2(10, 20)); //Draw debug text at 10,20

	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject)
	{
		return;
	}

	// Push the selected object
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT))
	{
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true))
		{
			if (closestCollision.node == selectionObject)
			{
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

/* Object Spawning */
GameObject* CourseworkGame::AddFloorToWorld(const Vector3& position, const Vector3& size)
{
	GameObject* cube = new GameObject();
	AABBVolume* volume = new AABBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* CourseworkGame::AddCubeToWorld(const Vector3& position, const Vector3& size, float inverseMass)
{
	GameObject* cube = new GameObject();
	AABBVolume* volume = new AABBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* CourseworkGame::AddOBBCubeToWorld(const Vector3& position, const Vector3& size, float inverseMass)
{
	GameObject* cube = new GameObject();
	OBBVolume* volume = new OBBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	cube->GetRenderObject()->SetColour(Debug::CYAN);

	return cube;
}

GameObject* CourseworkGame::AddSphereToWorld(const Vector3& position, const float& radius, float inverseMass) {
	GameObject* sphere = new GameObject("ball");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetOriginalPosition(position);
	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* CourseworkGame::AddCapsuleToWorld(const Vector3& position, const float& halfHeight, const float& radius, const float& inverseMass)
{
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* CourseworkGame::AddPlayerToWorld(const Vector3& position)
{
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* CourseworkGame::AddEnemyToWorld(const Vector3& position)
{
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* CourseworkGame::AddCoinToWorld(const Vector3& position, float radius, float inverseMass, bool rotate)
{
	GameObject* coin = new GameObject("coin");
	SphereVolume* volume = new SphereVolume(radius*5);
	coin->SetBoundingVolume((CollisionVolume*)volume);
	coin->GetTransform().SetOriginalPosition(position);
	coin->GetTransform()
		.SetScale(Vector3(radius, radius, radius))
		.SetPosition(position);

	coin->SetRenderObject(new RenderObject(&coin->GetTransform(), bonusMesh, nullptr, basicShader));
	coin->SetPhysicsObject(new PhysicsObject(&coin->GetTransform(), coin->GetBoundingVolume()));

	coin->GetPhysicsObject()->SetInverseMass(inverseMass);
	coin->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(coin);

	coin->GetRenderObject()->SetColour(Debug::YELLOW);

	ChangeOrientation_x(*coin, 1);
	return coin;
}

/* Level spawning */

void CourseworkGame::CreateLevel1()
{
	physics->UseGravity(true);
	physics->useBroadPhase = true;
	world->isLevelTwo = false;
	physics->SetGravity(Vector3(0.0f, -9.8f * gravityScale, 9.8f * 6));

	world->GetMainCamera()->SetPosition(Vector3(0, 400, 0));
	world->GetMainCamera()->SetPitch(-90);
	world->GetMainCamera()->SetYaw(0);

	float floor_height = 200;
	float floor_width = 120;
	float floor_depth = 2;
	float object_depth = 10;
	float object_width = 3;
	float object_y = floor_depth + object_depth;
	
	/* Floor is painted into tiles, each tile representing a segment of the level */
	float segment_height = floor_height / 2;
	float segment_width = floor_width / 4;


	/* Note: x-width, y-depth, z-height*/

	floor = AddOBBCubeToWorld(Vector3(0, 0, 0), Vector3(floor_width, 2, floor_height), 0);
	floor->GetRenderObject()->SetColour(Debug::CYAN);
	GameObject* outerWall = AddOBBCubeToWorld(Vector3(-floor_width + 1, object_y, 0), Vector3(1, object_depth, floor_height), 0);
	GameObject* outerWall2 = AddOBBCubeToWorld(Vector3(floor_width - 1, object_y, 0), Vector3(1, object_depth, floor_height), 0);
	GameObject* outerWall3 = AddOBBCubeToWorld(Vector3(0, object_y, floor_height-1), Vector3(floor_width, object_depth, 1), 0);
	GameObject* outerWall4 = AddOBBCubeToWorld(Vector3(0, object_y, -floor_height + 1), Vector3(floor_width, object_depth, 1), 0);


	/* Row 1 */
	GameObject* platform_1 = AddOBBCubeToWorld(Vector3(segment_width * 3, object_y, -segment_height * 1.5), Vector3(segment_width, object_depth, object_width), 0);
	GameObject* platform_2 = AddOBBCubeToWorld(Vector3(0, object_y, -segment_height * 1.25), Vector3(segment_width * 2 + 5, object_depth, object_width), 0);
	ChangeOrientation_y(*platform_2, 0.20);

	
	GameObject* s_platform_1 = AddOBBCubeToWorld(Vector3(segment_width * 3.25, object_y, -segment_height * 1.73), Vector3(object_width, object_depth, segment_height / 4), 0);
	GameObject* s_platform_2 = AddOBBCubeToWorld(Vector3(-segment_width * 3, object_y, -segment_height * 1), Vector3(segment_width, object_depth, object_width), 0);
	s_platform_1->canInteract = true;
	s_platform_2->canInteract = true;

	s_platform_1->setActionType(GameObject::actionType::SPIN_CLOCKWISE);
	s_platform_1->GetPhysicsObject()->setElasticity(2);

	s_platform_2->setActionType(GameObject::actionType::SPIN_CLOCKWISE);
	s_platform_2->GetPhysicsObject()->setElasticity(1.2);

	/* Row 2*/
	GameObject* platform_3 = AddOBBCubeToWorld(Vector3(segment_width * 2, object_y, -segment_height * 0.5), Vector3(object_width, object_depth, segment_height / 2), 0);
	GameObject* platform_4 = AddOBBCubeToWorld(Vector3(segment_width * 2.5, object_y, -segment_height * 1 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	GameObject* platform_5 = AddOBBCubeToWorld(Vector3(segment_width * 3.5, object_y, -segment_height * 0.75 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	GameObject* platform_6 = AddOBBCubeToWorld(Vector3(segment_width * 2.5, object_y, -segment_height * 0.5 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	GameObject* platform_7 = AddOBBCubeToWorld(Vector3(segment_width * 3.5, object_y, -segment_height * 0.25 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	GameObject* platform_8 = AddOBBCubeToWorld(Vector3(segment_width * 2.5, object_y, -segment_height * 0 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);

	ChangeOrientation_y(*platform_4, -0.25);
	ChangeOrientation_y(*platform_5, 0.25);
	ChangeOrientation_y(*platform_6, -0.25);
	ChangeOrientation_y(*platform_7, 0.25);
	ChangeOrientation_y(*platform_8, -0.25);

	GameObject* s_platform_3 = AddOBBCubeToWorld(Vector3(-segment_width * 5, object_y, -segment_height / 2), Vector3(object_width, object_depth, segment_height/2 + 10), 0);
	GameObject* s_platform_4 = AddOBBCubeToWorld(Vector3(-segment_width * 1, object_y, 0), Vector3(segment_width * 3, object_depth, object_width), 0);

	s_platform_3->canInteract = true;
	s_platform_4->canInteract = true;
	
	s_platform_3->setActionType(GameObject::actionType::GO_RIGHT);
	s_platform_3->SetMaxDistance(195);
	s_platform_3->GetPhysicsObject()->setElasticity(1.2);
	

	s_platform_4->setActionType(GameObject::actionType::GO_UP);
	s_platform_4->SetMaxDistance(100);
	
	/* Row 3 */
	GameObject* platform_9 = AddOBBCubeToWorld(Vector3(segment_width * 1, object_y, segment_height), Vector3(segment_width * 3, object_depth, object_width), 0);
	GameObject* platform_10 = AddOBBCubeToWorld(Vector3(segment_width * 3 - 4, object_y, segment_height / 3 + 5), Vector3(object_width, object_depth, segment_height / 4), 0);
	
	GameObject* platform_11 = AddOBBCubeToWorld(Vector3(segment_width * 2, object_y, segment_height / 2 + 12), Vector3(object_width, object_depth, segment_height / 3 + 5), 0);
	GameObject* platform_12 = AddOBBCubeToWorld(Vector3(segment_width * 0, object_y, segment_height / 2 + 12), Vector3(object_width, object_depth, segment_height / 3 + 5), 0);
	GameObject* platform_13 = AddOBBCubeToWorld(Vector3(-segment_width *2, object_y, segment_height / 2 + 12), Vector3(object_width, object_depth, segment_height / 3 + 5), 0);

	GameObject* platform_14 = AddOBBCubeToWorld(Vector3(segment_width * 0.5, object_y, -segment_height * 0 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	GameObject* platform_15 = AddOBBCubeToWorld(Vector3(-segment_width * 1.5, object_y, -segment_height * 0 + 9), Vector3(segment_width / 2, object_depth, object_width), 0);
	ChangeOrientation_y(*platform_14, -0.25);
	ChangeOrientation_y(*platform_15, -0.25);

	GameObject* platform_16 = AddOBBCubeToWorld(Vector3(segment_width * 1 - 4, object_y, segment_height / 3 + 5), Vector3(object_width, object_depth, segment_height / 4), 0);
	GameObject* platform_17 = AddOBBCubeToWorld(Vector3(-segment_width * 1 - 4, object_y, segment_height / 3 + 5), Vector3(object_width, object_depth, segment_height / 4), 0);

	GameObject* platform_18 = AddOBBCubeToWorld(Vector3(segment_width * 3.5 - 3, object_y, segment_height / 2 + 25), Vector3(segment_width / 2 + 3, object_depth, object_width), 0);
	GameObject* platform_19 = AddOBBCubeToWorld(Vector3(segment_width * 1.5 - 3, object_y, segment_height / 2 + 25), Vector3(segment_width / 2 + 3, object_depth, object_width), 0);
	GameObject* platform_20 = AddOBBCubeToWorld(Vector3(-segment_width * 0.5 - 3, object_y, segment_height / 2 + 25), Vector3(segment_width / 2 + 3, object_depth, object_width), 0);
	ChangeOrientation_y(*platform_18, 0.2);
	ChangeOrientation_y(*platform_19, 0.2);
	ChangeOrientation_y(*platform_20, 0.2);

	GameObject* platform_21 = AddOBBCubeToWorld(Vector3(segment_width * 3 - 4 , object_y, segment_height - 10), Vector3(object_width, object_depth, segment_height / 10), 0);
	GameObject* platform_22 = AddOBBCubeToWorld(Vector3(segment_width * 1 - 4 , object_y, segment_height - 10), Vector3(object_width, object_depth, segment_height / 10), 0);
	GameObject* platform_23 = AddOBBCubeToWorld(Vector3(-segment_width * 1 - 4, object_y, segment_height - 10), Vector3(object_width, object_depth, segment_height / 10), 0);

	GameObject* s_platform_5 = AddOBBCubeToWorld(Vector3(segment_width * 2.5 - 3, object_y, segment_height - 5), Vector3(segment_height / 10 + 3, object_depth, object_width ), 0);
	GameObject* s_platform_6 = AddOBBCubeToWorld(Vector3(segment_width * 0.5 - 3, object_y, segment_height - 5), Vector3(segment_height / 10 + 3, object_depth, object_width ), 0);
	GameObject* s_platform_7 = AddOBBCubeToWorld(Vector3(-segment_width * 1.5 - 3, object_y, segment_height - 5), Vector3(segment_height / 10 + 3, object_depth, object_width ), 0);
	s_platform_5->canInteract = true;
	s_platform_6->canInteract = true;
	s_platform_7->canInteract = true;

	s_platform_5->GetPhysicsObject()->setElasticity(2);
	s_platform_6->GetPhysicsObject()->setElasticity(2);
	s_platform_7->GetPhysicsObject()->setElasticity(2);

	s_platform_5->setActionType(GameObject::actionType::GO_UP);
	s_platform_6->setActionType(GameObject::actionType::GO_UP);
	s_platform_7->setActionType(GameObject::actionType::GO_UP);

	s_platform_5->SetMaxDistance(20);
	s_platform_6->SetMaxDistance(20);
	s_platform_7->SetMaxDistance(20);

	/* Row 4 */

	GameObject* platform_24 = AddOBBCubeToWorld(Vector3(-segment_width * 2, object_y, segment_height * 1.75), Vector3(object_width, object_depth, segment_height / 4), 0);
	GameObject* platform_25 = AddOBBCubeToWorld(Vector3(0, object_y, segment_height * 1.75), Vector3(object_width, object_depth, segment_height / 4), 0);
	GameObject* platform_26 = AddOBBCubeToWorld(Vector3(segment_width * 2, object_y, segment_height * 1.75), Vector3(object_width, object_depth, segment_height / 4), 0);
	GameObject* goal = AddOBBCubeToWorld(Vector3(segment_width * 3, object_y, segment_height * 1.95), Vector3(segment_width, object_depth, object_width), 0);
	goal->SetName("goal");
	goal->GetRenderObject()->SetColour(Debug::MAGENTA);

	GameObject* s_platform_8 = AddOBBCubeToWorld(Vector3(-segment_width * 3, object_y, segment_height * 2.25), Vector3(segment_width, object_depth, object_width), 0);
	GameObject* s_platform_9 = AddOBBCubeToWorld(Vector3(-segment_width * 1, object_y, segment_height * 2.25), Vector3(segment_width, object_depth, object_width), 0);
	GameObject* s_platform_10 = AddOBBCubeToWorld(Vector3(segment_width * 1, object_y, segment_height * 2.25), Vector3(segment_width, object_depth, object_width), 0);

	s_platform_8->canInteract = true;
	s_platform_9->canInteract = true;
	s_platform_10->canInteract = true;

	s_platform_8->GetPhysicsObject()->setElasticity(3);
	s_platform_9->GetPhysicsObject()->setElasticity(3);
	s_platform_10->GetPhysicsObject()->setElasticity(3);

	s_platform_8->setActionType(GameObject::actionType::GO_UP);
	s_platform_9->setActionType(GameObject::actionType::GO_UP);
	s_platform_10->setActionType(GameObject::actionType::GO_UP);

	s_platform_8->SetMaxDistance(40);
	s_platform_9->SetMaxDistance(40);
	s_platform_10->SetMaxDistance(40);

	ChangeOrientation_y(*s_platform_8, -0.15);
	ChangeOrientation_y(*s_platform_9, -0.15);
	ChangeOrientation_y(*s_platform_10, -0.15);
	
	// AI

	AddMovingObstacle(Vector3(-segment_width * 1.5, object_y, -segment_height * 1.6), Vector3(object_width, object_depth, object_width), 0, 2)->canInteract = true;
	AddMovingObstacle(Vector3(segment_width * 0, object_y, segment_height * 1.25), Vector3(object_width, object_depth, object_width), 0, 2)->canInteract = true;
	AddMovingObstacle(Vector3(segment_width * 2, object_y, segment_height * 1.25), Vector3(object_width, object_depth, object_width), 0, 2)->canInteract = true;
	AddMovingObstacle(Vector3(-segment_width * 2, object_y, segment_height * 1.25), Vector3(object_width, object_depth, object_width), 0, 2)->canInteract = true;
	AddSpinningObstacle(Vector3(0, object_y, segment_height * -1.25), Vector3(segment_width / 2, object_depth, object_width), 0, 2)->canInteract = true;
	AddSpinningObstacle(Vector3(-segment_width * 3, object_y, segment_height * 0.5), Vector3(segment_width / 2, object_depth, object_width), 0, 2)->canInteract = true;
	
	/* Bonuses */

	GameObject* coin_1 = AddCoinToWorld(Vector3(-segment_width * 2, 6, -segment_height * 1.8),1, 0, true);
	GameObject* coin_2 = AddCoinToWorld(Vector3(-segment_width * 3, 6, -segment_height * 1.8),1, 0 , true);
	GameObject* coin_3 = AddCoinToWorld(Vector3(-segment_width * 3, 6, -segment_height * 1.6),1, 0, true);
	GameObject* coin_4 = AddCoinToWorld(Vector3(-segment_width * 2, 6, -segment_height * 1.6),1, 0, true);
	
	GameObject* coin_5 = AddCoinToWorld(Vector3(segment_width * 1.5, 6, -segment_height * 1.1),1, 0, true);
	GameObject* coin_6 = AddCoinToWorld(Vector3(-segment_width * 1, 6, -segment_height * 0.6),1, 0, true);
	GameObject* coin_7 = AddCoinToWorld(Vector3(-segment_width * 3, 6, -segment_height * 0.2),1, 0, true);
	
	GameObject* coin_8 = AddCoinToWorld(Vector3(-segment_width * 1, 6, segment_height * 1.2),1, 0, true);
	GameObject* coin_9 = AddCoinToWorld(Vector3(segment_width * 1, 6, segment_height * 1.2),1, 0, true);

	GameObject* springObstacle = AddSphereToWorld(Vector3(segment_width * 0 - 5, 4, -segment_height * 1.60), 5, 1);
	springObstacle->GetPhysicsObject()->setElasticity(1);
	springObstacle->GetRenderObject()->SetColour(Debug::CYAN);


	ball = AddSphereToWorld(Vector3(segment_width * 3 - 5, 4, -segment_height * 1.60), 5, 1);
	//ball = AddSphereToWorld(Vector3(segment_width * 3 - 5, 4, segment_height * 1.50), 5, 1);
	ball->GetPhysicsObject()->setElasticity(1);

	springObstacle->AttachSpringTo(ball, 15);


	// selectable platform color
	s_platform_1->GetRenderObject()->SetColour(Debug::RED);
	s_platform_2->GetRenderObject()->SetColour(Debug::RED);
	s_platform_3->GetRenderObject()->SetColour(Debug::RED);
	s_platform_4->GetRenderObject()->SetColour(Debug::RED);
	s_platform_5->GetRenderObject()->SetColour(Debug::RED);
	s_platform_6->GetRenderObject()->SetColour(Debug::RED);
	s_platform_7->GetRenderObject()->SetColour(Debug::RED);
	s_platform_8->GetRenderObject()->SetColour(Debug::RED);
	s_platform_9->GetRenderObject()->SetColour(Debug::RED);
	s_platform_10->GetRenderObject()->SetColour(Debug::RED);

	// Time limit
	world->setTimeLimit(120);
}

void CourseworkGame::CreateLevel2()
{
	inSelectionMode = false;
	navGrid = new NavigationGrid();
	world->isLevelTwo = true;
	physics->UseGravity(true);
	gravityScale = 20;
	physics->SetGravity(Vector3(0.0f, -9.8f * gravityScale, 0));
	world->GetMainCamera()->SetPitch(-20);
	world->GetMainCamera()->SetYaw(0);
	world->GetMainCamera()->SetPosition(Vector3(40, 60, 100));

	float tile_size = 20;
	float grid_width = tile_size * 2 * 4;	// 160
	float grid_height = tile_size * 2 * 4;	// 160
	const int grid_rows = grid_height / tile_size;
	const int grid_columns = grid_width / tile_size;

	// Create floor with grid pattern
	floor = AddOBBCubeToWorld(Vector3(tile_size * 2, 0, -tile_size * 2), Vector3(tile_size*2, 2, tile_size * 2), 0);							// Bottom left
	GameObject* floor1 = AddOBBCubeToWorld(Vector3(tile_size * 4, 0, -tile_size * 0), Vector3(tile_size * 2, 2, tile_size * 2), 0);				// Bottom middle
	GameObject* floor2 = AddOBBCubeToWorld(Vector3(tile_size * 8, 0, -tile_size * 0), Vector3(tile_size * 2, 2, tile_size * 2), 0);				// Bottom right


	GameObject* floor3 = AddOBBCubeToWorld(Vector3(tile_size * 0 , 0, -tile_size * 4), Vector3(tile_size*2, 2, tile_size * 2), 0);				// Center left
	GameObject* floor4 = AddOBBCubeToWorld(Vector3(tile_size * 4, 0, -tile_size * 4 ), Vector3(tile_size*2, 2, tile_size * 2), 0);				// Center middle
	GameObject* floor5 = AddOBBCubeToWorld(Vector3(tile_size * 8, 0, -tile_size * 4), Vector3(tile_size * 2, 2, tile_size * 2), 0);				// Center top

	GameObject* floor6 = AddOBBCubeToWorld(Vector3(tile_size * 0, 0, -tile_size * 8 ), Vector3(tile_size*2, 2, tile_size * 2), 0);				// Top left
	GameObject* floor7 = AddOBBCubeToWorld(Vector3(tile_size * 4, 0, -tile_size * 8 ), Vector3(tile_size*2, 2, tile_size * 2), 0);				// Top middle
	GameObject* floor8 = AddOBBCubeToWorld(Vector3(tile_size * 8, 0, -tile_size * 8 ), Vector3(tile_size*2, 2, tile_size * 2), 0);				// Top right

	
	// Add outer border walls
	for (int i = -1; i < 13; ++i)
	{
		floor->AddChild(AddOBBCubeToWorld(Vector3(tile_size * -1 - tile_size * 1.5, 10, -tile_size * (i + 1) + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
		floor->AddChild(AddOBBCubeToWorld(Vector3(tile_size * 12 - tile_size * 1.5, 10, -tile_size * (i + 1) + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
		floor->AddChild(AddOBBCubeToWorld(Vector3(tile_size * i - tile_size * 1.5, 10, -tile_size * 0 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
		floor->AddChild(AddOBBCubeToWorld(Vector3(tile_size * i - tile_size * 1.5, 10, -tile_size * 13 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	}
	

	/* Corner walls
		xx      xx
		x	     x


		x	     x
		xx	    xx
	*/
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 1 - tile_size * 1.5, 10, -tile_size * 2 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 10, -tile_size * 2 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 1 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 10 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 10 - tile_size * 1.5, 10, -tile_size * 2 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 9 - tile_size * 1.5, 10, -tile_size * 2 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 10 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 10 - tile_size * 1.5, 10, -tile_size * 11 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 9 - tile_size * 1.5, 10, -tile_size * 11 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 1 - tile_size * 1.5, 10, -tile_size * 11 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 1 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 10, -tile_size * 11 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));

	/* Top & bottom 'L' pieces
		-----
		x
			x
		xxxxx

		xxxxx
		    x
        x
		-----
	*/

	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 5 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 6 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 3 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 2 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 1 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 12 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 5 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 6 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 10 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 11 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));

	/* Left & Right wall pieces
	 -x				   x-
	 -	 x			x	-
	 -	 x			x	-
	 -x				   x-
	*/

	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 0 - tile_size * 1.5, 10, -tile_size * 5 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 0 - tile_size * 1.5, 10, -tile_size * 8 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 10, -tile_size * 7 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 10, -tile_size * 6 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 9 - tile_size * 1.5, 10, -tile_size * 6 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 9 - tile_size * 1.5, 10, -tile_size * 7 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 11 - tile_size * 1.5, 10, -tile_size *8 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	AddObstacleToMap(AddOBBCubeToWorld(Vector3(tile_size * 11 - tile_size * 1.5, 10, -tile_size *5 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0));
	
	/* Center goal pieces
			xx
		   x--x
		   x--x
			xx
	*/

	GameObject* unlockable_obj_1 = AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 6 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_2 = AddOBBCubeToWorld(Vector3(tile_size * 4 - tile_size * 1.5, 10, -tile_size * 7 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_3 = AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 6 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_4 = AddOBBCubeToWorld(Vector3(tile_size * 7 - tile_size * 1.5, 10, -tile_size * 7 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_5 = AddOBBCubeToWorld(Vector3(tile_size * 5 - tile_size * 1.5, 10, -tile_size * 8 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_6 = AddOBBCubeToWorld(Vector3(tile_size * 6 - tile_size * 1.5, 10, -tile_size * 8 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_7 = AddOBBCubeToWorld(Vector3(tile_size * 5 - tile_size * 1.5, 10, -tile_size * 5 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);
	GameObject* unlockable_obj_8 = AddOBBCubeToWorld(Vector3(tile_size * 6 - tile_size * 1.5, 10, -tile_size * 5 + tile_size * 2.5), Vector3(tile_size / 2, 10, tile_size / 2), 0);

	unlockable_obj_1->SetName("unlockable");
	unlockable_obj_2->SetName("unlockable");
	unlockable_obj_3->SetName("unlockable");
	unlockable_obj_4->SetName("unlockable");
	unlockable_obj_5->SetName("unlockable");
	unlockable_obj_6->SetName("unlockable");
	unlockable_obj_7->SetName("unlockable");
	unlockable_obj_8->SetName("unlockable");

	unlockable_obj_1->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_2->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_3->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_4->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_5->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_6->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_7->GetRenderObject()->SetColour(Debug::RED);
	unlockable_obj_8->GetRenderObject()->SetColour(Debug::RED);

	AddObstacleToMap(unlockable_obj_1);
	AddObstacleToMap(unlockable_obj_2);
	AddObstacleToMap(unlockable_obj_3);
	AddObstacleToMap(unlockable_obj_4);
	AddObstacleToMap(unlockable_obj_5);
	AddObstacleToMap(unlockable_obj_6);
	AddObstacleToMap(unlockable_obj_7);
	AddObstacleToMap(unlockable_obj_8);

	/* Goal */
	GameObject* goal = AddOBBCubeToWorld(Vector3(tile_size * 5 - tile_size * 1, 2, -tile_size * 6 + tile_size * 2), Vector3(tile_size, 2, tile_size ), 0);
	goal->GetRenderObject()->SetColour(Debug::MAGENTA);
	goal->SetName("goal");

	GameObject* unlockPad_1 = AddOBBCubeToWorld(Vector3(tile_size * 0 - tile_size * 1.5, 1, -tile_size * 12 + tile_size * 2.5), Vector3(tile_size / 2, 2, tile_size / 2), 0);
	GameObject* unlockPad_2 = AddOBBCubeToWorld(Vector3(tile_size * 11 - tile_size * 1.5, 1, -tile_size * 12 + tile_size * 2.5), Vector3(tile_size / 2, 2, tile_size / 2), 0);
	GameObject* unlockPad_3 = AddOBBCubeToWorld(Vector3(tile_size * 11 - tile_size * 1.5, 1, -tile_size * 1 + tile_size * 2.5), Vector3(tile_size / 2, 2, tile_size / 2), 0);

	unlockPad_1->GetRenderObject()->SetColour(Debug::GREEN);
	unlockPad_2->GetRenderObject()->SetColour(Debug::GREEN);
	unlockPad_3->GetRenderObject()->SetColour(Debug::GREEN);
	unlockPad_1->SetName("lockpad");
	unlockPad_2->SetName("lockpad");
	unlockPad_3->SetName("lockpad");
	
	/* Bonuses */
	GameObject* bonus_1 = AddCoinToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 15, -tile_size * 3 + tile_size * 2.5), 1, 0, 1);
	GameObject* bonus_2 = AddCoinToWorld(Vector3(tile_size * 2 - tile_size * 1.5, 15, -tile_size * 10 + tile_size * 2.5), 1, 0, 1);
	GameObject* bonus_3 = AddCoinToWorld(Vector3(tile_size * 9- tile_size * 1.5, 15, -tile_size * 3 + tile_size * 2.5), 1, 0, 1);
	GameObject* bonus_4 = AddCoinToWorld(Vector3(tile_size * 9 - tile_size * 1.5, 15, -tile_size * 10 + tile_size * 2.5), 1, 0, 1);
	
	bonus_1->isSpinning = true;
	bonus_2->isSpinning = true;
	bonus_3->isSpinning = true;
	bonus_4->isSpinning = true;

	floor->AddChild(bonus_1);
	floor->AddChild(bonus_2);
	floor->AddChild(bonus_3);
	floor->AddChild(bonus_4);

	world->AddBonusObject(bonus_1);
	world->AddBonusObject(bonus_2);
	world->AddBonusObject(bonus_3);
	world->AddBonusObject(bonus_4);

	/* Moving constraint obstacles */
	GameObject* constraintObstacle_1 = AddMovingObstacle(Vector3(tile_size * 3 - tile_size * 1.5, 55, -tile_size * 1 - tile_size * 0.2), Vector3(10, 2, 2), 0, 8);
	GameObject* constraintObstacle_2 = AddMovingObstacle(Vector3(tile_size * 8 - tile_size * 1.5, 55, -tile_size * 1 - tile_size * 0.2), Vector3(10, 2, 2), 0, 8);
	constraintObstacle_1->canInteract = true;
	constraintObstacle_2->canInteract = true;

	floor->AddChild(constraintObstacle_1);
	floor->AddChild(constraintObstacle_2);
	
	float maxDistance = 10;
	Vector3 startPos = constraintObstacle_1->getParentedPosition();
	Vector3 cubeSize(10, 2, 2);

	GameObject* start = constraintObstacle_1;
	GameObject* current;

	for (int i = 0; i < 5; i++)
	{
		current = AddOBBCubeToWorld(Vector3(startPos.x, startPos.y - i * maxDistance, startPos.z), cubeSize, 2);
		start->AddChild(current);
		current->onlyMatchOrientation = true;
		PositionConstraint* constraint = new PositionConstraint(start, current, maxDistance);
		world->AddConstraint(constraint);
		start = current;
	}
	
	start = constraintObstacle_2;
	startPos = constraintObstacle_2->getParentedPosition();
	for (int i = 0; i < 5; i++)
	{
		current = AddOBBCubeToWorld(Vector3(startPos.x, startPos.y - i * maxDistance, startPos.z), cubeSize, 2);
		start->AddChild(current);
		current->onlyMatchOrientation = true;
		PositionConstraint* constraint = new PositionConstraint(start, current, maxDistance);
		world->AddConstraint(constraint);
		start = current;
	}


	floor->AddChild(goal);
	floor->AddChild(unlockPad_1);
	floor->AddChild(unlockPad_2);
	floor->AddChild(unlockPad_3);
	floor->AddChild(floor1);
	floor->AddChild(floor2);
	floor->AddChild(floor3);
	floor->AddChild(floor4);
	floor->AddChild(floor5);
	floor->AddChild(floor6);
	floor->AddChild(floor7);
	floor->AddChild(floor8);

	// Spawn enemy ball
	Vector3 enemyBallSpawn = indexToCoord(12*12-27);
	EnemyBallAI* enemy = AddEnemyBallAI(enemyBallSpawn, 5, 0);
	enemy->setNavGrid(navGrid);
	enemy->setGameWorld(world);
	enemy->setBonuses(world->bonusObjects);

	floor->AddChild(enemy);
	enemy->GetTransform().SetPosition(enemy->getParentedPosition());

	playerBallSpawn = indexToCoord(0);
	playerBallSpawn.y = 30;

	ball = AddSphereToWorld(playerBallSpawn, 5, 1);
	ball->GetPhysicsObject()->setElasticity(0);
	enemy->setPlayer(ball);
	// Navigation
	navGrid->gameObjects.emplace_back(ball);
	navGrid->gameObjects.emplace_back(enemy);

	navGrid->createConnectivity();
	physics->useBroadPhase = false;
	physics->SetNavGrid(navGrid);
	world->SetNavGrid(navGrid);

	navGrid->emplaceBonus(bonus_1);
	navGrid->emplaceBonus(bonus_2);
	navGrid->emplaceBonus(bonus_3);
	navGrid->emplaceBonus(bonus_4);
	world->setTimeLimit(9999);
}

void CourseworkGame::MoveEnemies(float dt)
{
	Vector3 currentPos = enemyBall->getParentedPosition();
	vector <Vector3 > pathNodes;

	// Find path to player
	NavigationPath outPath;
	Vector3 from = enemyBall->GetTransform().GetPosition();
	Vector3 to = ball->GetTransform().GetPosition();
	navGrid->FindPath(from, to, outPath);

	// Put positions into a vector
	Vector3 pos;
	while (outPath.PopWaypoint(pos))
		pathNodes.push_back(pos);

	if (!movingToWaypont)
	{
		if (pathNodes.size() > 1)
		{
			currentWaypoint = pathNodes[1];
			currentWaypoint.y = 7;
		}
		movingToWaypont = true;
	}

	float distance = (currentWaypoint - currentPos).Length();
	direction = (currentWaypoint - currentPos).Normalised();

	float speed = 15;

	Vector3 currentOriginalPos = enemyBall->GetTransform().GetOriginalPosition();

	if(!world->freezeEnemy)
		enemyBall->GetTransform().SetOriginalPosition(currentOriginalPos + direction * dt * speed);
	
	waypointTimer += dt;

	if (waypointTimer > 3)
	{
		movingToWaypont = false;
		waypointTimer = 0;
	}

	if (distance < 0.2)
	{
		movingToWaypont = false;
		waypointTimer = 0;
	}

	// Draw path
	for (int i = 1; i < pathNodes.size(); ++i)
	{
		
		Vector3 a = pathNodes[i - 1];
		Vector3 b = pathNodes[i];
		a.y += 20;
		b.y += 20;
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

/* Misc */

void CourseworkGame::FreezePlayer()
{
	ball->GetPhysicsObject()->SetAngularVelocity({0,0,0});
	ball->GetPhysicsObject()->SetLinearVelocity({0,0,0});
}

void CourseworkGame::AddObstacleToMap(GameObject* obj)
{
	floor->AddChild(obj);
	navGrid->emplaceObstacle(obj);
}

void CourseworkGame::updateGrid()
{
	// Reset grid to 0's
	for (int i = 11; i >= 0; --i)
	{
		for (int j = 0; j < 8; ++j)
		{
			grid[i][j] = 0;
		}
	}

	for (GameObject* g : level2Objects)
	{
		Vector3 pos = g->GetTransform().GetPosition();
		int index = coordToIndex(pos);
		int row = index / 12;
		int col = index - row * 12;
		grid[row][col] = 1;
	}
}

void CourseworkGame::printGrid()
{
	std::string out;

	for (int i = 11; i >= 0; --i)
	{
		for (int j = 0; j < 12; ++j)
		{
			out = out + std::to_string(grid[i][j]) + " ";
		}
		out += '\n';
	}
	out += '\n';

	std::cout << out;
}

Vector3 CourseworkGame::indexToCoord(int i)
{
	float tile_size = 20;
	int row;
	int col;

	row = i / 12;
	col = i - row * 12;

	return { col * tile_size + tile_size / 2, 2, -row*tile_size - tile_size / 2};
}

int CourseworkGame::coordToIndex(Vector3 pos)
{
	float tile_size = 20;
	int row = abs(pos.z / tile_size);
	int col = abs(pos.x / tile_size);

	return row * 12 + col;
}

void CourseworkGame::ChangeOrientation_y(GameObject& obj, float change)
{
	Quaternion orientation = obj.GetTransform().GetOrientation();
	orientation.y += change;
	obj.GetTransform().SetOrientation(orientation);
}

void CourseworkGame::ChangeOrientation_x(GameObject& obj, float change)
{
	Quaternion orientation = obj.GetTransform().GetOrientation();
	orientation.x += change;
	obj.GetTransform().SetOrientation(orientation);
}

void CourseworkGame::DrawUI_1()
{
	Debug::Print("Press 'E' on a selected object to activate it", Vector2(2, 95));
	Debug::Print("Press 'P' for debug info", Vector2(2, 90));
	Debug::Print("Press 'M' to return to menu", Vector2(2, 85));
	Debug::Print("Press 'F1' to restart", Vector2(60, 5));
	
	if (world->isGameLost())
	{
		DrawLossScreen();
		return;
	}
	
	if (!world->isGoalReached())
	{
		DrawScore();
	}
	else DrawWinScreen();
}

void CourseworkGame::DrawUI_2()
{
	if (world->isGameLost())
	{
		DrawLossScreen();
		return;
	}

	if (!world->isGoalReached())
	{
		DrawScore();
	}
	else DrawWinScreen();
}

void CourseworkGame::DrawScore()
{
	if (isLevelOne)
	{
		Debug::Print("Time : " + std::to_string((int)world->GetElapsedTime()) + " s", Vector2(2, 5));
		Debug::Print("Bonuses : " + std::to_string((world->GetBonus())), Vector2(2, 10));
	}
	else if (isLevelTwo)
	{
		Debug::Print("Time : " + std::to_string((int)world->GetElapsedTime()) + " s", Vector2(2, 5));
		Debug::Print("Lives : " + std::to_string(world->GetLives()), Vector2(2, 10));
	}
}

void CourseworkGame::DrawWinScreen()
{
	Debug::Print("Congratulations!", Vector2(35, 50));
	Debug::Print("Your time: " + std::to_string((int)world->GetTotalTime()) + " s", Vector2(35, 60));
	if(isLevelOne)
		Debug::Print("Your bonuses: " + std::to_string((world->GetBonus())), Vector2(35, 65));
	Debug::Print("Press 'M' to return to menu", Vector2(20, 75));
	Debug::Print("Press 'F1' to restart", Vector2(20, 80));
}

void CourseworkGame::DrawDebugInfo()
{
	Debug::Print("Debug info:", Vector2(5, 45));

	if (!selectionObject) return;

	Vector3 pos = selectionObject->GetTransform().GetPosition();
	Quaternion orient = selectionObject->GetTransform().GetOrientation();
	int action = selectionObject->getActionType();

	string name = selectionObject->GetName();
	if (name.empty()) name = "No name";
	string position = std::to_string(pos.x) + ',' + std::to_string(pos.y) + ',' + std::to_string(pos.z);
	string orientation = std::to_string(orient.x) + ',' + std::to_string(orient.y) + ',' + std::to_string(orient.z) + ',' + std::to_string(orient.w);
	string actionType;
	string stateName;

	switch (action) 
	{
	case(0):
		actionType = "Nothing";
		break;
	case(1):
		actionType = "Spin clockwise";
		break;
	case (2):
		actionType = "Spin anti-clockwise";
		break;
	case (3):
		actionType = "Go up";
		break;
	case (4):
		actionType = "Go down";
		break;
	case (5):
		actionType = "Go left";
		break;
	case (6):
		actionType = "Go right";
		break;
	}

	if (selectionObject->isAI)
	{
		StateGameObject* obj = (StateGameObject*)selectionObject;
		stateName = obj->getActiveStateName();
	}
	
	
	Debug::Print("Selected: " + name, Vector2(5, 50));
	Debug::Print("Position: " + position, Vector2(5, 55));
	Debug::Print("Orientation: " + orientation, Vector2(5, 60));
	Debug::Print("Action: " + actionType, Vector2(5, 65));
	Debug::Print("State: " + stateName, Vector2(5, 70));
}

StateGameObject* CourseworkGame::AddMovingObstacle(const Vector3& position, const Vector3& size, float inverseMass, float count)
{
	StateGameObject* cube = new StateGameObject(count);
	OBBVolume* volume = new OBBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->isAI = true;

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	cube->GetRenderObject()->SetColour(Debug::CYAN);

	stateObjects.emplace_back(cube);
	return cube;
}

SpinningGameObject* CourseworkGame::AddSpinningObstacle(const Vector3& position, const Vector3& size, float inverseMass, float count)
{
	SpinningGameObject* cube = new SpinningGameObject(count);
	OBBVolume* volume = new OBBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->isAI = true;

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	cube->GetRenderObject()->SetColour(Debug::CYAN);

	cube->GetPhysicsObject()->setElasticity(1.2);
	stateObjects.emplace_back(cube);
	return cube;
}

EnemyBallAI* CourseworkGame::AddEnemyBallAI(const Vector3& position, const float& radius, float inverseMass)
{
	EnemyBallAI* sphere = new EnemyBallAI();
	sphere->SetName("enemy");
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetOriginalPosition(position);
	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->isAI = true;

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	stateObjects.emplace_back(sphere);
	sphere->GetRenderObject()->SetColour(Debug::BLACK);
	return sphere;
}

void CourseworkGame::CreateMenu()
{

	physics->UseGravity(true);
	physics->SetGravity(Vector3(0, -9.8f * gravityScale, 0));
	world->GetMainCamera()->SetPosition(Vector3(-40, 30, 10));
	world->GetMainCamera()->SetYaw(-90);
	world->GetMainCamera()->SetPitch(0);
	
	Vector3 cubeSize = Vector3(2, 2, 6);

	float invCubeMass = 1;
	float maxDistance = 10;
	float cubeDistance = 8;

	Vector3 startPos = Vector3(0, 50, 0);

	GameObject* start = AddOBBCubeToWorld(startPos, cubeSize, 0);

	menu_item_1 = AddCubeToWorld(startPos + Vector3(1 * cubeDistance, 1 * cubeDistance, 0), cubeSize, invCubeMass);
	menu_item_2 = AddCubeToWorld(startPos + Vector3(2 * cubeDistance, 2 * cubeDistance, 0), cubeSize, invCubeMass);
	menu_item_3 = AddCubeToWorld(startPos + Vector3(3 * cubeDistance, 3 * cubeDistance, 0), cubeSize, invCubeMass);
	menu_item_1->GetRenderObject()->SetColour(Debug::RED);
	menu_item_2->GetRenderObject()->SetColour(Debug::RED);
	menu_item_3->GetRenderObject()->SetColour(Debug::RED);
	menu_item_1->canInteract = true;
	menu_item_2->canInteract = true;
	menu_item_3->canInteract = true;
	menu_item_1->SetName("menu_level_1");
	menu_item_2->SetName("menu_level_2");
	menu_item_3->SetName("exit");

	PositionConstraint* constraint_1 = new PositionConstraint(menu_item_1, start, maxDistance);
	PositionConstraint* constraint_2 = new PositionConstraint(menu_item_2, menu_item_1, maxDistance);
	PositionConstraint* constraint_3 = new PositionConstraint(menu_item_3, menu_item_2, maxDistance);

	RotationConstraint* constraint_4 = new RotationConstraint(menu_item_1, start, 15);
	RotationConstraint* constraint_5 = new RotationConstraint(menu_item_2, menu_item_1, 15);
	RotationConstraint* constraint_6 = new RotationConstraint(menu_item_3, menu_item_2, 15);

	world->AddConstraint(constraint_1);
	world->AddConstraint(constraint_2);
	world->AddConstraint(constraint_3);

	//world->AddConstraint(constraint_4);
	//world->AddConstraint(constraint_5);
	//world->AddConstraint(constraint_6);


}

void CourseworkGame::DrawMenu()
{
	
	Debug::Print("Level 1:", Vector2(5, 20));
	Debug::Print("Level 2:", Vector2(5, 50));
	Debug::Print("   Exit:", Vector2(5, 85));

	Debug::Print("CSC 8503 Coursework", Vector2(50, 60));
	Debug::Print("Click on menu item", Vector2(50, 70));
	Debug::Print("and press 'E' to select", Vector2(50, 75));

}

void CourseworkGame::DrawLossScreen()
{
	Debug::Print("You lost!", Vector2(35, 50));
	if(isLevelOne)
		Debug::Print("Time limit reached", Vector2(35, 60));
	else if (isLevelTwo)
		Debug::Print("No lives left", Vector2(35, 60));

	Debug::Print("Press 'M' to return to menu", Vector2(25, 70));
	Debug::Print("Press 'F1' to restart", Vector2(25, 75));
}