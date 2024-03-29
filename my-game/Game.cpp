﻿#include <iostream>
#include <fstream>
#include "library/json.hpp"

// for convenience

#include "Game.h"
#include "Utils.h"

#include "PlayScence.h"
#include "WorldSelect.h"
#include "Intro.h"
#include "Scence.h"

using json = nlohmann::json;

CGame* CGame::__instance = NULL;

void CGame::Init(HWND hWnd)
{
	LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);

	this->hWnd = hWnd;

	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;

	RECT r;
	GetClientRect(hWnd, &r);	// retrieve Window width & height 

	d3dpp.BackBufferHeight = r.bottom + 1;
	d3dpp.BackBufferWidth = r.right + 1;

	screen_height = r.bottom + 1;
	screen_width = r.right + 1;

	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddv);

	if (d3ddv == NULL)
	{
		OutputDebugString(L"[ERROR] CreateDevice failed\n");
		return;
	}

	d3ddv->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);

	// Initialize sprite helper from Direct3DX helper library
	D3DXCreateSprite(d3ddv, &spriteHandler);

	OutputDebugString(L"[INFO] InitGame done;\n");
}

/*
	Utility function to wrap LPD3DXSPRITE::Draw
*/
void CGame::Draw(float x, float y, LPDIRECT3DTEXTURE9 texture, float left, float top, float right, float bottom, int alpha)
{
	Camera* camera = this->GetCurrentScene()->camera;
	D3DXVECTOR3 p = camera->calcInCamPosition(x, y);
	RECT r;
	r.left = LONG(left);
	r.top = LONG(top);
	r.right = LONG(right);
	r.bottom = LONG(bottom);
	spriteHandler->Draw(texture, &r, NULL, &p, D3DCOLOR_ARGB(alpha, 255, 255, 255));
}



void CGame::DrawWithScale(Vector p, LPDIRECT3DTEXTURE9 texture, Rect r, int opacity, D3DXVECTOR2 scale)
{
	Camera* camera = this->GetCurrentScene()->camera;
	D3DXVECTOR3 position = camera->calcInCamPosition(p.x, p.y);

	D3DXMATRIX oldMatrix, newMatrix;
	D3DXVECTOR3 deltaToCenter = D3DXVECTOR3((r.right - r.left) / 2, (r.bottom - r.top) / 2, 0);
	D3DXVECTOR3 pCenter = D3DXVECTOR3(deltaToCenter.x + 0, deltaToCenter.y + 0, 0); //pivot x, y instead of 0 ?

	RECT rect;
	rect.left = LONG(r.left);
	rect.top = LONG(r.top);
	rect.right = LONG(r.right);
	rect.bottom = LONG(r.bottom);

	if (camera->isInCam(position.x, position.y, 100) == true) {

		if (scale.x == 1 && scale.y == 1) {
			spriteHandler->Draw(texture, &rect, NULL, &position, D3DCOLOR_ARGB(opacity, 255, 255, 255));
		}
		else {
			D3DXVECTOR2 ppp = D3DXVECTOR2(position.x, position.y);
			spriteHandler->GetTransform(&oldMatrix);

			D3DXMatrixTransformation2D(&newMatrix, &(ppp), 0.0f, &scale, &ppp, 0.0f, NULL);
			spriteHandler->SetTransform(&newMatrix);

			//position.x = position.x - 100;
			position.x = position.x - deltaToCenter.x;
			position.y = position.y + deltaToCenter.y;
			spriteHandler->Draw(texture, &rect, &pCenter, &position, D3DCOLOR_ARGB(opacity, 255, 255, 255));
			spriteHandler->SetTransform(&oldMatrix);
		}
	}
}

void CGame::DrawPositionInCamera(Vector p, LPDIRECT3DTEXTURE9 texture, Rect r, int opacity, D3DXVECTOR2 scale)
{
	Camera* camera = this->GetCurrentScene()->camera;
	D3DXVECTOR3 position = D3DXVECTOR3(p.x, p.y, 0);


	D3DXMATRIX oldMatrix, newMatrix;
	D3DXVECTOR3 deltaToCenter = D3DXVECTOR3((r.right - r.left) / 2, (r.bottom - r.top) / 2, 0);
	D3DXVECTOR3 pCenter = D3DXVECTOR3(deltaToCenter.x + 0, deltaToCenter.y + 0, 0); //pivot x, y instead of 0 ?

	RECT rect;
	rect.left = LONG(r.left);
	rect.top = LONG(r.top);
	rect.right = LONG(r.right);
	rect.bottom = LONG(r.bottom);

	if (camera->isInCam(position.x, position.y, 100) == true) {

		if (scale.x == 1 && scale.y == 1) {
			spriteHandler->Draw(texture, &rect, NULL, &position, D3DCOLOR_ARGB(opacity, 255, 255, 255));
		}
		else {
			D3DXVECTOR2 ppp = D3DXVECTOR2(position.x, position.y);
			spriteHandler->GetTransform(&oldMatrix);

			D3DXMatrixTransformation2D(&newMatrix, &(ppp), 0.0f, &scale, &ppp, 0.0f, NULL);
			spriteHandler->SetTransform(&newMatrix);

			//position.x = position.x - 100;
			position.x = position.x - deltaToCenter.x;
			position.y = position.y + deltaToCenter.y;
			spriteHandler->Draw(texture, &rect, &pCenter, &position, D3DCOLOR_ARGB(opacity, 255, 255, 255));
			spriteHandler->SetTransform(&oldMatrix);
		}
	}
}

int CGame::IsKeyDown(int KeyCode)
{
	return (keyStates[KeyCode] & 0x80) > 0;
}

void CGame::InitKeyboard()
{
	HRESULT
		hr = DirectInput8Create
		(
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			DIRECTINPUT_VERSION,
			IID_IDirectInput8, (VOID**)&di, NULL
		);

	if (hr != DI_OK)
	{
		DebugOut(L"[ERROR] DirectInput8Create failed!\n");
		return;
	}

	hr = di->CreateDevice(GUID_SysKeyboard, &didv, NULL);

	// TO-DO: put in exception handling
	if (hr != DI_OK)
	{
		DebugOut(L"[ERROR] CreateDevice failed!\n");
		return;
	}

	// Set the data format to "keyboard format" - a predefined data format 
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing an array
	// of 256 bytes to IDirectInputDevice::GetDeviceState.

	hr = didv->SetDataFormat(&c_dfDIKeyboard);

	hr = didv->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);


	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
	//
	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
	// If you want to read buffered data, you need to set a nonzero
	// buffer size.
	//
	// Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
	//
	// The buffer size is a DWORD property associated with the device.
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = KEYBOARD_BUFFER_SIZE; // Arbitary buffer size

	hr = didv->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	hr = didv->Acquire();
	if (hr != DI_OK)
	{
		DebugOut(L"[ERROR] DINPUT8::Acquire failed!\n");
		return;
	}

	DebugOut(L"[INFO] Keyboard has been initialized successfully\n");
}

void CGame::ProcessKeyboard()
{
	HRESULT hr;

	// Collect all key states first
	hr = didv->GetDeviceState(sizeof(keyStates), keyStates);
	if (FAILED(hr))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			HRESULT h = didv->Acquire();
			if (h == DI_OK)
			{
				DebugOut(L"[INFO] Keyboard re-acquired!\n");
			}
			else {
				if (CPlayScene* playScene = dynamic_cast<CPlayScene*>(CGame::GetCurrentScene())) {
					if (playScene->isPaused == false)
						playScene->Pause();
				}
				return;
			}
		}
		else
		{
			DebugOut(L"[ERROR] DINPUT::GetDeviceState failed. Error: %d\n", hr);
				return;
		}
	}

	keyHandler->KeyState((BYTE*)&keyStates);



	// Collect all buffered events
	DWORD dwElements = KEYBOARD_BUFFER_SIZE;
	hr = didv->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), keyEvents, &dwElements, 0);
	if (FAILED(hr))
	{
		return;
	}

	// Scan through all buffered events, check if the key is pressed or released
	for (DWORD i = 0; i < dwElements; i++)
	{
		int KeyCode = keyEvents[i].dwOfs;
		int KeyState = keyEvents[i].dwData;
		if ((KeyState & 0x80) > 0)
			keyHandler->OnKeyDown(KeyCode);
		else
			keyHandler->OnKeyUp(KeyCode);
	}
}

CGame::~CGame()
{
	if (spriteHandler != NULL) spriteHandler->Release();
	if (backBuffer != NULL) backBuffer->Release();
	if (d3ddv != NULL) d3ddv->Release();
	if (d3d != NULL) d3d->Release();
}

/*
	Standard sweptAABB implementation
	Source: GameDev.net
*/
void CGame::SweptAABB(
	float ml, float mt, float mr, float mb,
	float dx, float dy,
	float sl, float st, float sr, float sb,
	float& t, float& nx, float& ny)
{

	float dx_entry, dx_exit, tx_entry, tx_exit;
	float dy_entry, dy_exit, ty_entry, ty_exit;

	float t_entry;
	float t_exit;

	t = -1.0f;			// no collision
	nx = ny = 0;

	//
	// Broad-phase test 
	//

	float bl = dx > 0 ? ml : ml + dx;
	float bt = dy > 0 ? mt : mt + dy;
	float br = dx > 0 ? mr + dx : mr;
	float bb = dy > 0 ? mb + dy : mb;

	if (br < sl || bl > sr || bb < st || bt > sb) return;


	if (dx == 0 && dy == 0) return;		// moving object is not moving > obvious no collision

	if (st == sb || sl == sr) return;

	if (dx > 0)
	{
		dx_entry = sl - mr;
		dx_exit = sr - ml;
	}
	else if (dx < 0)
	{
		dx_entry = sr - ml;
		dx_exit = sl - mr;
	}


	if (dy > 0)
	{
		dy_entry = st - mb;
		dy_exit = sb - mt;
	}
	else if (dy < 0)
	{
		dy_entry = sb - mt;
		dy_exit = st - mb;
	}

	if (dx == 0)
	{
		tx_entry = -999999.0f;
		tx_exit = 999999.0f;
	}
	else
	{
		tx_entry = dx_entry / dx;
		tx_exit = dx_exit / dx;
	}

	if (dy == 0)
	{
		ty_entry = -99999.0f;
		ty_exit = 99999.0f;
	}
	else
	{
		ty_entry = dy_entry / dy;
		ty_exit = dy_exit / dy;
	}


	if ((tx_entry < 0.0f && ty_entry < 0.0f) || tx_entry > 1.0f || ty_entry > 1.0f) return;

	t_entry = max(tx_entry, ty_entry);
	t_exit = min(tx_exit, ty_exit);

	if (t_entry > t_exit) return;

	t = t_entry;

	if (tx_entry > ty_entry)
	{
		ny = 0.0f;
		dx > 0 ? nx = -1.0f : nx = 1.0f;
	}
	else
	{
		nx = 0.0f;
		dy > 0 ? ny = -1.0f : ny = 1.0f;
	}

}

CGame* CGame::GetInstance()
{
	if (__instance == NULL) __instance = new CGame();
	return __instance;
}

void CGame::_ParseSection_SCENES_FromJson(json data)
{
	for (json::iterator it = data.begin(); it != data.end(); ++it) {

		int id = stoi(it.key());
		json value = it.value();
		string type = value["type"];
		LPCWSTR path = ToLPCWSTR(value["path"]);
		DebugOut(L"[INFO] Load data : %s\n", path);

		LPSCENE scene = NULL;

		if (type == "Intro") {
			scene = new Intro(id, path);
		}
		else if (type == "SelectMap") {
			scene = new WorldSelect(id, path);
		}
		else {
			scene = new CPlayScene(id, path);
		}
		if (scene != NULL)
			scenes[id] = scene;
	}
}


/*
	Load game campaign file and load/initiate first scene
*/
void CGame::Load(LPCWSTR gameFile)
{
	json gameData = ReadJsonFIle(gameFile);
	json configData = gameData["config"];

	string redTexture = configData["red-texture"];
	string blackTexture = configData["black-texture"];
	CGameObject::LoadBoundedBox(ToLPCWSTR(redTexture));
	CScene::LoadBlackTexture(ToLPCWSTR(blackTexture));

	string active = gameData["active"];
	DebugOut(L"[INFO] Active id : %s\n", ToLPCWSTR(active));
	_ParseSection_SCENES_FromJson(gameData["scene"]); // now parse the json to game data;
	current_scene = -1;
	SwitchScene(stoi(active));
}

void CGame::SwitchScene(int scene_id)
{
	if (current_scene != scene_id && current_scene != -1) {
		scenes[current_scene]->Unload();
		DebugOut(L"[INFO] Switching to scene %d\n", scene_id);
	}
	else if (current_scene == -1) {
		DebugOut(L"[INFO] Start with scene %d\n", scene_id);
	}
	current_scene = scene_id;
	LPSCENE s = scenes[scene_id];
	CGame::GetInstance()->SetKeyHandler(s->GetKeyEventHandler());
	s->Load();
}

void CGame::Restart()
{
	scenes[current_scene]->Unload();
	DebugOut(L"[INFO] Switching to scene %d\n", current_scene);
	current_scene = current_scene;
	LPSCENE s = scenes[current_scene];
	CGame::GetInstance()->SetKeyHandler(s->GetKeyEventHandler());
	s->Load();
}


LPDIRECT3DTEXTURE9 CGame::LoadTexture(LPCWSTR filePath) {

	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filePath, &info);
	if (result != D3D_OK)
	{
		DebugOut(L"[ERROR] GetImageInfoFromFile failed: %s\n", filePath);
		return NULL;
	}

	LPDIRECT3DDEVICE9 d3ddv = CGame::GetInstance()->GetDirect3DDevice();
	LPDIRECT3DTEXTURE9 texture;

	result = D3DXCreateTextureFromFileEx(
		d3ddv,								// Pointer to Direct3D device object
		filePath,							// Path to the image to load
		info.Width,							// Texture width
		info.Height,						// Texture height
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_XRGB(255, 0, 255),		// transparentColor	
		&info,
		NULL,
		&texture);								// Created texture pointer

	if (result != D3D_OK)
	{
		DebugOut(L"[INFO] GetImageInfoFromFile success: %s\n", filePath);
		return NULL;
	}

	return texture;
}