#pragma once

#include <d3dx9.h>
#include "KeyEventHandler.h"
#include "Textures.h"
#include "Camera.h"


enum Scences {
	SelectMap = 1,
	World_1_1 = 2,
	IntroScence = 3,
	World_1_3 = 4
};

enum AnimationDirection {
	CLOSING,
	OPENING,
	UNACTIVE,
	DONE
};

class CScene
{
protected:
	CKeyEventHandler* key_handler;
	LPCWSTR sceneFilePath;

public:
	CScene(int id, LPCWSTR filePath);
	int id;
	int timeLimit = 0;
	int playerPoint = 0;
	int playerMoney = 0;
	Camera* camera;
	int nextScene = 0;
	vector<LPGAMEOBJECT> objects;
	static LPDIRECT3DTEXTURE9 blackTexture; // texture to render opening and closing animation
	float animationDuration;
	float animationProgress; //  = (getTickcount64 - animationStartedTime) / animationDuration;
	ULONGLONG animationStartedTime; // get tick count 64 when start loading;
	float lastTime;
	AnimationDirection animationDirection;

	CKeyEventHandler* GetKeyEventHandler() { return key_handler; }
	virtual void Load() = 0;
	virtual void Unload() = 0;
	virtual void Update(DWORD dt) = 0;
	virtual void Render() = 0;
	virtual void addObject(LPGAMEOBJECT obj) = 0;

	virtual void ParseMapObject(json data, vector<LPGAMEOBJECT>* obCollisions) = 0;
	virtual void switchScene(int sence_id);
	virtual void AddPoint(int point) { playerPoint = playerPoint + point; }
	virtual void AddMoney(int money) { playerMoney = playerMoney + money; }

	Camera* getCamera() {
		return this->camera;
	}

	static void LoadBlackTexture(LPCWSTR filePath);
};
typedef CScene* LPSCENE;


class CScenceKeyHandler : public CKeyEventHandler
{
protected:
	CScene* scence;

public:
	virtual void KeyState(BYTE* states) = 0;
	virtual void OnKeyDown(int KeyCode) = 0;
	virtual void OnKeyUp(int KeyCode) = 0;
	CScenceKeyHandler(CScene* s) :CKeyEventHandler() { scence = s; }
};