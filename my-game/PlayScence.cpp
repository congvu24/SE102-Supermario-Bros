#include <iostream>
#include <fstream>
#include "Config.h"
#include "PlayScence.h"
#include "Utils.h"
#include "Textures.h"
#include "Sprites.h"
#include "Portal.h"
#include "Mario.h"
#include "Enemy.h"
#include "MisteryBox.h"
#include "Coin.h"
#include "Mushroom.h"
#include "Quadtree.h"
#include "Death.h"
#include "Venus.h"
#include "VenusBullet.h"
#include "DefineCharacter.h"
#include "RedGoomba.h"
#include "GoldenBrick.h"
#include "PButton.h"
#include "Effect.h"
#include "Brick.h"
#include "CUI.h"
#include "BoomerangBrother.h"
#include "Boomerang.h"
#include "MusicBox.h"
#include "FlyGoomba.h"
#include "MiniGoomba.h"
#include "EndSceneItem.h"
#include "FlyKoopas.h"
#include "Spawn.h"

using namespace std;



CPlayScene::CPlayScene(int id, LPCWSTR filePath) :
	CScene(id, filePath)
{
	key_handler = new CPlayScenceKeyHandler(this);
	UI = new CUI();
}


void CPlayScene::Load()
{
	DebugOut(L"[INFO] Start loading scene resources from : %s \n", sceneFilePath);

	json scene = ReadJsonFIle(sceneFilePath);


	DebugOut(L"[INFO] Scene : %s \n", ToLPCWSTR(scene["name"].dump()));

	//init the time limit, point and money
	timeLimit = stoi(scene["limit-time"].dump());
	playerPoint = 0;
	playerMoney = 0;

	//parse texture, sprite, animation
	json object = scene["object"];

	_ParseSection_OBJECTS_FromJson(object);
	DebugOut(L"[INFO] PARSE OBJECT DONE  \n");


	string map = string(scene["map"]);

	_ParseSection_MAP_FromJson(map);
	DebugOut(L"[INFO] LOAD MAP : %s \n", ToLPCWSTR(map));


	animationDirection = UNACTIVE;
	animationStartedTime = GetTickCount64();
	animationProgress = 0;
	lastTime = 0;
}

void CPlayScene::Update(DWORD dt)
{
	if (isPaused) return;

	CScene::Update(dt);
	UpdateTime(dt);
	Camera* camera = CGame::GetInstance()->GetCurrentScene()->camera;

	Rect base = { camera->cam_x, camera->cam_y, camera->cam_x + camera->cam_width ,camera->cam_y + camera->cam_height };
	Quadtree* quadtree = new Quadtree(1, new Rect(base));


	for (auto i = objects.begin(); i != objects.end(); i++) {
		quadtree->Insert(*i);
	}
	int count = 0;

	for (size_t i = 0; i < objects.size(); i++)
	{
		if (quadtree->IsContain(objects[i]) && objects[i]->isUniversal == false) {
			count++;
			vector<CGameObject*>* return_objects_list = new vector<CGameObject*>();
			quadtree->Retrieve(return_objects_list, objects[i]);

			if (Mario* v = dynamic_cast<Mario*>(objects[i])) {
				v->Update(dt, return_objects_list);
			}

			else if (objects[i]->state != "hidden") {
				objects[i]->Update(dt, return_objects_list);
			}
			else {
				objects.erase(std::remove(objects.begin(), objects.end(), objects[i]), objects.end());
			}

			delete return_objects_list;
		}
		else if (objects[i]->isUniversal == true && objects[i]->state != "hidden") {
			count++;
			LPGAMEOBJECT obj = objects[i];

			float left, top, right, bottom;
			obj->GetBoundingBox(left, top, right, bottom);

			Rect miniBase = { left - 200, top - 200, right + 200, bottom + 200 };
			Quadtree* miniQuadtree = new Quadtree(5, new Rect(miniBase));

			for (auto i = objects.begin(); i != objects.end(); i++) {
				miniQuadtree->Insert(*i);
			}
			vector<CGameObject*>* mini_return_objects_list = new vector<CGameObject*>();
			miniQuadtree->Retrieve(mini_return_objects_list, obj);

			obj->Update(dt, mini_return_objects_list);
			miniQuadtree->Clear();
			delete miniQuadtree;
			delete mini_return_objects_list;
		}
	}
	quadtree->Clear();
	delete quadtree;


	if (player == NULL) return;

	float cx, cy;
	player->GetPosition(cx, cy);

	CGame* game = CGame::GetInstance();
	cx -= game->GetScreenWidth() / 2;
	cy -= game->GetScreenHeight() / 2;

	if (camera->isCameraMoving == false) {

		if (((Mario*)player)->isAllowCameraFollow == true) {
			if (player->p.y + player->height + 100 < camera->camera_default_top) {
				camera->setCamPos(cx, cy < camera->camera_default_top - 600 ? camera->camera_default_top - 600 : cy);
			}
			else {
				camera->setCamPos(cx, camera->camera_default_top);
			}
		}
	}
	else {
		camera->setCamPos(cx, cy < camera->camera_default_top - 600 ? camera->camera_default_top - 600 : cy - 100);
	}
}

bool comparePtrToNode(CGameObject* a, CGameObject* b) { return (a->renderOrder < b->renderOrder); }


void CPlayScene::Render()
{

	Camera* camera = CGame::GetInstance()->GetCurrentScene()->camera;

	Rect* base = new Rect();
	base->left = camera->cam_x - 200;
	base->top = camera->cam_y - 200;
	base->right = camera->cam_x + camera->cam_width + 200;
	base->bottom = camera->cam_y + camera->cam_height + 200;

	Quadtree* quadtree = new Quadtree(5, base); // set the level to 5 to stop split function
	vector<CGameObject*>* return_objects_list = new vector<CGameObject*>();

	for (auto i = objects.begin(); i != objects.end(); i++) {
		quadtree->Insert(*i);
	}
	quadtree->Retrieve(return_objects_list, this->player);

	sort(return_objects_list->begin(), return_objects_list->end(), comparePtrToNode);

	for (size_t i = 0; i < return_objects_list->size(); i++) { // order < 1
		if (return_objects_list->at(i)->state != "hidden" && return_objects_list->at(i)->renderOrder < 1)
			return_objects_list->at(i)->Render();
	}
	if (player->renderOrder < 1)
		player->Render(); // render player 

	map->render();  // render map here


	for (size_t i = 0; i < return_objects_list->size(); i++) { // order >= 1
		if (return_objects_list->at(i)->state != "hidden" && return_objects_list->at(i)->renderOrder >= 1)
			return_objects_list->at(i)->Render();
	}

	if (player->renderOrder >= 1)
		player->Render();
	//player->Render(); // render player 
	DrawUI();
	CScene::Render();  //render scene animation


	// clear data
	quadtree->Clear();
	return_objects_list->clear();
	delete return_objects_list;
	delete quadtree;
	//delete base;
}

/*
	Unload current scene
*/
void CPlayScene::Unload()
{
	player = NULL;
	//map->unload();
	CScene::Unload();
}

void CPlayScenceKeyHandler::OnKeyDown(int KeyCode)
{
	if (((CPlayScene*)scence)->isPaused == true) return;

	CGameObject* player = ((CPlayScene*)scence)->GetPlayer();
	((Character*)player)->ProcessKeyboard(CGame::GenerateKeyboardEvent(KeyCode));

	std::vector<int> SpawnKeyCode = { DIK_5, DIK_6, DIK_7, DIK_8, DIK_9 };

	CGame* game = CGame::GetInstance();



	for (size_t i = 0; i < SpawnKeyCode.size(); i++) {


		if (game->IsKeyDown(SpawnKeyCode[i])) {
			DebugOut(L"key clicked \n");

			for (size_t j = 0; j < scence->objects.size(); j++) {
				if (Spawn* obj = dynamic_cast<Spawn*>(scence->objects[j])) {
					if (obj->key == i) {
						player->p = scence->objects[j]->p;
						((CPlayScene*)scence)->camera->isCameraMoving = false;
					}
				}
			}
		}
	}

}

void CPlayScenceKeyHandler::OnKeyUp(int KeyCode)
{

	CGameObject* player = ((CPlayScene*)scence)->GetPlayer();
	((Character*)player)->ProcessKeyboard(CGame::GenerateKeyboardEvent(KeyCode, false, true));

	switch (KeyCode)
	{
	case DIK_ESCAPE:
		((CPlayScene*)scence)->Pause();
		break;
	}


}


void CPlayScenceKeyHandler::KeyState(BYTE* states)
{
	if (((CPlayScene*)scence)->isPaused == true) return;

	CGame* game = CGame::GetInstance();
	CGameObject* player = ((CPlayScene*)scence)->GetPlayer();

	std::vector<int> UnOrderProcessKey = { DIK_A, DIK_S, DIK_SPACE };
	std::vector<int> OrderProcessKey = { DIK_LEFT,DIK_RIGHT, DIK_DOWN, DIK_UP };


	for (size_t i = 0; i < UnOrderProcessKey.size(); i++) {
		if (game->IsKeyDown(UnOrderProcessKey[i])) {
			((Character*)player)->ProcessKeyboard(CGame::GenerateKeyboardEvent(UnOrderProcessKey[i], true));
		}
	}

	for (size_t i = 0; i < OrderProcessKey.size(); i++) {
		if (game->IsKeyDown(OrderProcessKey[i])) {
			((Character*)player)->ProcessKeyboard(CGame::GenerateKeyboardEvent(OrderProcessKey[i], true));
			return;
		}
	}
	((Character*)player)->ProcessKeyboard(CGame::GenerateKeyboardEvent(-1, true));


	if (game->IsKeyDown(DIK_R))
		((CPlayScene*)scence)->restart();
	else
		((Mario*)player)->SetAction(MarioAction::IDLE);


	// move camera
	/*if (game->IsKeyDown(DIK_J))
		((CPlayScene*)scence)->moveCamera(LEFT);
	if (game->IsKeyDown(DIK_K))
		((CPlayScene*)scence)->moveCamera(DOWN);
	if (game->IsKeyDown(DIK_I))
		((CPlayScene*)scence)->moveCamera(UP);
	if (game->IsKeyDown(DIK_L))
		((CPlayScene*)scence)->moveCamera(RIGHT);*/


}

void CPlayScene::restart() {
	CGame::GetInstance()->Restart();
}

void CPlayScene::moveCamera(CameraMoveDirection direction) {
	camera->isCameraMoving = true;
	if (direction == UP) {
		camera->move(0, -20);
	}
	else if (direction == DOWN) {
		camera->move(0, 20);
	}
	else if (direction == RIGHT) {
		camera->move(20, 0);
	}
	else if (direction == LEFT) {
		camera->move(-20, 0);
	}
}

void  CPlayScene::_ParseSection_OBJECTS_FromJson(json allObjects) {
	for (json::iterator it = allObjects.begin(); it != allObjects.end(); ++it) {

		json data = it.value();

		string name = string(data["name"]); //object name;
		bool visible = bool(data["visible"]); //object name;

		CGameObject* obj = NULL;

		switch (fromNameToCode(name))
		{
		case ObjectType::Mario:
			if (player != NULL)
			{
				DebugOut(L"[ERROR] MARIO object was created before!\n");
				return;
			}
			obj = new Mario();
			obj->ParseFromJson(data); //remember to set position, animation_set in this function
			player = obj;
			break;

		case  ObjectType::QuestionBox_Item:
			MisteryBox::SaveStaticData(data);
		case  ObjectType::Coin:
			Coin::SaveStaticData(data);
			break;
		case  ObjectType::Goomba:
			Goomba::SaveStaticData(data);
			break;
		case  ObjectType::RectPlatform:
			RectPlatform::SaveStaticData(data);
			break;
		case  ObjectType::Leaf:
			Leaf::SaveStaticData(data);
			break;
		case  ObjectType::Mushroom:
			Mushroom::SaveStaticData(data);
			break;
		case  ObjectType::Venus:
			Venus::SaveStaticData(data);
			break;
		case  ObjectType::Venus_Bullet:
			VenusBullet::SaveStaticData(data);
			break;
		case  ObjectType::Koopas:
			Koopas::SaveStaticData(data);
			break;
		case  ObjectType::RedGoomba:
			RedGoomba::SaveStaticData(data);
			break;
		case  ObjectType::GoldenBrick:
			GoldenBrick::SaveStaticData(data);
			break;
		case  ObjectType::PButton:
			PButton::SaveStaticData(data);
			break;
		case  ObjectType::Effect:
			Effect::SaveStaticData(data);
			break;
		case ObjectType::UI:
			CUI::SaveStaticData(data);
			break;
		case ObjectType::BoomerangBrother:
			BoomerangBrother::SaveStaticData(data);
			break;
		case ObjectType::Boomerang:
			Boomerang::SaveStaticData(data);
			break;
		case ObjectType::MusicBox:
			MusicBox::SaveStaticData(data);
			break;
		case ObjectType::FlyGoomba:
			FlyGoomba::SaveStaticData(data);
			break;
		case ObjectType::MiniGoomba:
			MiniGoomba::SaveStaticData(data);
			break;
		case ObjectType::EndSceneItem:
			EndSceneItem::SaveStaticData(data);
			break;
		case ObjectType::FlyKoopas:
			Koopas::SaveStaticData(data);
			break;
		default:
			break;
		}

		if (obj != NULL)
			objects.push_back(obj);
		else delete obj;
	}

}

void  CPlayScene::_ParseSection_MAP_FromJson(string mapPath) {
	vector<LPGAMEOBJECT> obCollision;

	this->map = new Map();
	map->load(mapPath, &obCollision, this);

	for (size_t i = 0; i < obCollision.size(); i++)
	{
		objects.push_back(obCollision[i]);
	}
}

void CPlayScene::addObject(LPGAMEOBJECT obj) {
	objects.push_back(obj);
}


void CPlayScene::ParseMapObject(json data, vector<LPGAMEOBJECT>* obCollisions) {
	string type = string(data["type"]);
	string name = string(data["name"]);
	if (type == "objectgroup" && name == "Spawn") {
		json objects = data["objects"];

		for (json::iterator objData = objects.begin(); objData != objects.end(); ++objData) {
			json value = objData.value();
			Spawn* obj = new Spawn();


			float width = float(value["width"]);
			float height = float(value["height"]);
			float x = float(value["x"]);
			float y = float(value["y"]);
			string type = value["type"];

			obj->key = stoi(getPropertyFromData("Key", value));

			obj->name = "Spawn";
			obj->type = type;
			obj->p = Vector(x, y);

			obCollisions->push_back(obj);
		}
	}
	else if (type == "objectgroup" && name == "MiniPortal") {
		json objects = data["objects"];

		for (json::iterator objData = objects.begin(); objData != objects.end(); ++objData) {
			json value = objData.value();
			MiniPortal* obj = new MiniPortal();

			obj->ParseFromOwnJson();

			float width = float(value["width"]);
			float height = float(value["height"]);
			float x = float(value["x"]);
			float y = float(value["y"]);
			string portalName = value["name"];
			string type = value["type"];

			if (type == "Out") {
				obj->camera_x = stof(getPropertyFromData("CameraLeftTopLimitX", value));
				obj->camera_y = stof(getPropertyFromData("CameraLeftTopLimitY", value));
				obj->camera_left_limit = stof(getPropertyFromData("CameraLeftTopLimitX", value));
				obj->camera_top_limit = stof(getPropertyFromData("CameraLeftTopLimitY", value));
				obj->camera_right_limit = stof(getPropertyFromData("CameraRightBottomLimitX", value));
				obj->camera_bottom_limit = stof(getPropertyFromData("CameraRightBottomLimitY", value));
				obj->direction = getPropertyFromData("AccessKeyCode", value);
			}

			obj->width = width;
			obj->height = height;
			obj->portalName = portalName;
			obj->name = "MiniPortal";
			obj->type = type;
			obj->p = Vector(x, y);

			obCollisions->push_back(obj);
		}
	}
	else if (type == "objectgroup") {
		json objects = data["objects"];

		for (json::iterator objData = objects.begin(); objData != objects.end(); ++objData) {
			json value = objData.value();

			float width = float(value["width"]);
			float height = float(value["height"]);
			float x = float(value["x"]);
			float y = float(value["y"]);
			string type = value["type"];

			CGameObject* obj = NULL;
			Camera* camera = CGame::GetInstance()->GetCurrentScene()->getCamera();

			switch (fromNameToCode(name))
			{
			case ObjectType::RectCollision:
				obj = new Collision();
				break;
			case ObjectType::Death:
				obj = new Death();
				break;
			case ObjectType::RectPlatform:
				obj = new RectPlatform();
				break;
			case ObjectType::QuestionBox_Item:
				obj = new MisteryBox();
				break;
			case ObjectType::Coin:
				obj = new Coin();
				break;
			case ObjectType::Goomba:
				obj = new Goomba();
				break;
			case ObjectType::Venus:
				obj = new Venus();
				break;
			case ObjectType::Koopas:
				obj = new Koopas();
				break;
			case ObjectType::RedGoomba:
				obj = new RedGoomba();
				break;
			case ObjectType::GoldenBrick:
				obj = new GoldenBrick();
				((GoldenBrick*)obj)->group = getPropertyFromData("group", value);
				break;
			case ObjectType::BoomerangBrother:
				obj = new BoomerangBrother();
				break;
			case ObjectType::MusicBox:
				obj = new MusicBox();
				((MusicBox*)obj)->portName = getPropertyFromData("portName", value);
				break;
			case ObjectType::FlyGoomba:
				obj = new FlyGoomba();
				break;
			case ObjectType::EndSceneItem:
				obj = new EndSceneItem();
				break;
			case ObjectType::FlyKoopas:
				obj = new FlyKoopas();
				break;
				/*case ObjectType::Spawn:
					obj = new Spawn();
					((Spawn*)obj)->key = stoi(getPropertyFromData("Key", value));
					break;*/
			case ObjectType::Camera:
				camera->setCamPos(x, y);
				camera->camera_default_left = x;
				camera->camera_default_top = y;
				camera->cam_left_limit = x;
				camera->cam_top_limit = y;
				camera->cam_bottom_limit = y + 600;
				break;
			default:
				break;
			}

			if (obj != NULL) {
				obj->width = width;
				obj->height = height;
				obj->name = name;
				obj->p.x = x;
				obj->p.y = y;

				if (type != "") {
					obj->type = type;
				}
				obj->ParseFromOwnJson();
				obCollisions->push_back(obj);
			}
			else delete obj;

		}


	}
}


void CPlayScene::GameOver() {
	switchScene(Scences::SelectMap);
}

bool CPlayScene::IsPlayer(LPGAMEOBJECT obj) {
	if (Mario* player = dynamic_cast<Mario*>(obj))  return true;
	return false;
}

void CPlayScene::DrawUI() {
	UI->DrawUI("hub", Vector(0, 555));
	UI->DrawUI("reward-slot", Vector(500, 555));
	UI->DrawUI("1", Vector(124, 580), Vector(0.8, 0.8));
	UI->DrawUI("M-icon", Vector(12, 600));


	UI->DrawText("123", Vector(413, 578), Vector(0.8, 0.8));

	//draw mario life
	UI->DrawUI(to_string(((Mario*)player)->life), Vector(90, 599));

	//draw player point
	string point = to_string(playerPoint);
	int numberOfZero = 7 - point.length();
	for (int i = 1; i <= numberOfZero; i++) {
		point = "0" + point;
	}
	UI->DrawText(point, Vector(150, 600));

	////draw remain time
	int remain = trunc(timeLimit / 1000);
	UI->DrawText(to_string(remain >= 0 ? remain : 0), Vector(393, 603), Vector(0.8, 0.8));

	////draw mario speed 
	float levelSpeed = abs((((Mario*)player)->powerX / 1000) * 6);
	for (int i = 0; i < 6; i++) {
		UI->DrawUI(levelSpeed > i ? "arrow-white" : "arrow", Vector(152 + i * 24, 578));
	}

	UI->DrawUI(abs(((Mario*)player)->powerX) == 1000 ? "power-white" : "power-1", Vector(300, 578));

	// draw paused ui
	if (isPaused == true) {
		CGame::GetInstance()->Draw(camera->cam_x, camera->cam_y, blackTexture, 0, 0, camera->cam_width, camera->cam_height, 50);
		UI->DrawText("pause", Vector(SCREEN_WIDTH / 2 - (5 * 24 / 2), SCREEN_HEIGHT / 2 - 24), Vector(1.5, 1.5));
	}
}


void CPlayScene::UpdateTime(DWORD dt) {
	if (timeLimit >= 0 && timeLimit - dt >= 0) {
		timeLimit = timeLimit - dt;
	}
	else {
		GameOver();
	}
}


void CPlayScene::Pause() {
	isPaused = !isPaused;
}