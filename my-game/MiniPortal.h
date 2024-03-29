#pragma once
#include "GameObject.h"
#include "MapEntity.h"
#include "Game.h"
#include <iostream>

class MiniPortal :public MapEntity<MiniPortal>
{

public:
	MiniPortal();
	string portalName;
	float camera_x;
	float camera_y;
	float camera_left_limit;
	float camera_right_limit;
	float camera_top_limit;
	float camera_bottom_limit;
	string direction;

	virtual void Render();
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	virtual void SetState(string state);
	virtual void HandleCollision(LPCOLLISIONEVENT e);
	virtual void OnHadCollided(LPGAMEOBJECT obj, LPCOLLISIONEVENT event);

	static json spriteData;
	static json data;
	static LPDIRECT3DTEXTURE9 texture;
	static unordered_map<string, LPSPRITE> sprites; //save all sprite of animation
	static unordered_map<string, LPANIMATION> all_animations; //save all animations
	static CAnimationSets animations_set; //save all the animation 
};