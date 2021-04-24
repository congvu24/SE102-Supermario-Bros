#pragma once
#include "GameObject.h"
#include <iostream>



class Test : public CGameObject
{
	virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	virtual void ParseFromJson(json data);
	virtual void Render();

public:
	Test();
	virtual void SetState(string state);
};