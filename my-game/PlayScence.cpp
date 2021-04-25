#include <iostream>
#include <fstream>

#include "PlayScence.h"
#include "Utils.h"
#include "Textures.h"
#include "Sprites.h"
#include "Portal.h"
#include "Test.h"
#include "Enemy.h"

using namespace std;

CPlayScene::CPlayScene(int id, LPCWSTR filePath) :
	CScene(id, filePath)
{
	key_handler = new CPlayScenceKeyHandler(this);
}

/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/

#define SCENE_SECTION_UNKNOWN -1
#define SCENE_SECTION_TEXTURES 2
#define SCENE_SECTION_SPRITES 3
#define SCENE_SECTION_ANIMATIONS 4
#define SCENE_SECTION_ANIMATION_SETS	5
#define SCENE_SECTION_OBJECTS	6

#define OBJECT_TYPE_MARIO	0
#define OBJECT_TYPE_BRICK	1
#define OBJECT_TYPE_GOOMBA	2
#define OBJECT_TYPE_KOOPAS	3

#define OBJECT_TYPE_PORTAL	50

#define MAX_SCENE_LINE 1024



void CPlayScene::Load()
{
	DebugOut(L"[INFO] Start loading scene resources from : %s \n", sceneFilePath);

	json scene = ReadJsonFIle(sceneFilePath);


	DebugOut(L"[INFO] Scene : %s \n", ToLPCWSTR(scene["name"].dump()));

	//parse texture, sprite, animation

	json object = scene["object"];

	_ParseSection_OBJECTS_FromJson(object);

}

void CPlayScene::Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 

	vector<LPGAMEOBJECT> coObjects;
	for (size_t i = 0; i < objects.size(); i++)
	{
		coObjects.push_back(objects[i]);
	}

	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->Update(dt, &coObjects);
	}

	// skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return;

	// Update camera to follow mario
	//float cx, cy;
	//player->GetPosition(cx, cy);

	//CGame* game = CGame::GetInstance();
	//cx -= game->GetScreenWidth() / 2;
	//cy -= game->GetScreenHeight() / 2;

	//CGame::GetInstance()->SetCamPos(cx, 0.0f /*cy*/);
}

void CPlayScene::Render()
{
	for (int i = 0; i < objects.size(); i++)
		objects[i]->Render();
}

/*
	Unload current scene
*/
void CPlayScene::Unload()
{
	for (int i = 0; i < objects.size(); i++)
		delete objects[i];

	objects.clear();
	player = NULL;

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
}

void CPlayScenceKeyHandler::OnKeyDown(int KeyCode)
{

	CGameObject* player = ((CPlayScene*)scence)->GetPlayer();
	/*switch (KeyCode)
	{
	case DIK_D:
		player->SetState("running-right");
		DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);

		break;
	case DIK_A:
		player->SetState("running-left");
		DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);

		break;
	}*/
}

void CPlayScenceKeyHandler::KeyState(BYTE* states)
{
	CGame* game = CGame::GetInstance();
	//CMario* mario = ((CPlayScene*)scence)->GetPlayer();
	CGameObject* player = ((CPlayScene*)scence)->GetPlayer();

	//// disable control key when Mario die 
	if (player->GetState() == "die") return;
	if (game->IsKeyDown(DIK_RIGHT))
		player->SetState("running-right");
	else if (game->IsKeyDown(DIK_LEFT))
		player->SetState("running-left");
	else
		player->SetState("indie");
}



void  CPlayScene::_ParseSection_TEXTURES_FromJson(LPCWSTR filePath, int id) {

	DebugOut(L"[INFO] Start loading texture resources from : %s \n", filePath);

	int R = 0;
	int G = 0;
	int B = 0;

	CTextures::GetInstance()->Add(id, filePath, D3DCOLOR_XRGB(R, G, B));


}
void  CPlayScene::_ParseSection_SPRITES_FromJson(LPCWSTR filePath, int textID) {

	json sprite = ReadJsonFIle(filePath);
	json frames = sprite["frames"];


	for (json::iterator it = frames.begin(); it != frames.end(); ++it) {

		json data = it.value();
		string id = it.key();
		json frame = data["frame"];


		int l = stoi(frame["x"].dump());
		int t = stoi(frame["y"].dump());
		int r = l + stoi(frame["w"].dump());
		int b = t + stoi(frame["h"].dump());

		LPDIRECT3DTEXTURE9 tex = CTextures::GetInstance()->Get(textID);
		if (tex == NULL)
		{
			DebugOut(L"[ERROR] Texture ID %d not found!\n", IntToLPCWSTR(textID));
			return;
		}

		CSprites::GetInstance()->Add(id, l, t, r, b, tex);
	}

}
void  CPlayScene::_ParseSection_ANIMATIONS_FromJson(LPCWSTR filePath) {



}
void  CPlayScene::_ParseSection_ANIMATION_SETS_FromJson(LPCWSTR filePath) {
}
void  CPlayScene::_ParseSection_OBJECTS_FromJson(json allObjects) {
	unordered_map <string, int> character_code;
	character_code["test"] = 1;
	character_code["enemy"] = 2;


	//each sence has many object
	for (json::iterator it = allObjects.begin(); it != allObjects.end(); ++it) {

		json data = it.value();

		string name = string(data["name"]); //object name;


		CGameObject* obj;

		//DebugOut(L"[ERROR] Texture ID %d not found!\n", IntToLPCWSTR(character_code[name]));

		switch (character_code.at(name))
		{
		case 1:
			/*if (player != NULL)
			{
				DebugOut(L"[ERROR] MARIO object was created before!\n");
				return;
			}*/
			obj = new Test();
			obj->ParseFromJson(data); //remember to set position, animation_set in this function
			player = obj;
			break;
		case 2:
			obj = new Enemy();
			obj->ParseFromJson(data); //remember to set position, animation_set in this function
			break;
		default:
			break;
		}
		objects.push_back(obj);

	}
	// parse from object[] to list of object for each screen
}