#pragma once
#include "GameObject.h"
#include "Character.h"
#include "Game.h"
#include "MiniPortal.h"
#include <iostream>


#define SmallMario 1
#define BigMario 2
#define RacconMario 4



enum class MarioAction {
	IDLE,
	WALK,
	RUN,
	JUMP,
	CROUCH,
	FLY,
	JUMP_HEIGHT,
	FALL,
	HOLD,
	ATTACK,
	KICK,
	DIE,
	GETTING_INTO_THE_HOLE,
	TRANSFORM,
	PICK_UP,
	RELEASE
};

class Test : public Character
{
public:
	Test();
	bool canJump = true;
	bool isReadyChangeState = true;
	float max_move_x;
	float max_move_y;
	float powerX = 0;
	int timeMaxPower = 0;
	int timeBeginAction = 0;
	bool isAllowCameraFollow = true;
	int life = 1;
	int point = 0;

	MarioAction action;
	unordered_map<int, bool> holdingKeys;
	MiniPortal* teleportDestination;
	LPGAMEOBJECT* holdObject;

	virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom);
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	virtual void UpdateAnimation(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	virtual void Render();
	virtual void SetState(string state);
	virtual void SetAction(MarioAction newAction, DWORD time = 0);
	virtual void HandleCollision(LPCOLLISIONEVENT e);
	virtual void HandleCollisionVertical(LPCOLLISIONEVENT e);
	virtual void HandleCollisionHorizontal(LPCOLLISIONEVENT e);
	virtual void Die();
	virtual void Transform(int marioType);
	virtual void ProcessKeyboard(KeyboardEvent kEvent);
	virtual void OnHadCollided(LPGAMEOBJECT obj, LPCOLLISIONEVENT event) {}
	virtual void IncreasePowerX();
	virtual void DecreasePowerX();
	virtual bool IsReadyToChangeAction();
	virtual void Teleport(MiniPortal* destination, int duration);
};