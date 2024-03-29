﻿#include "Koopas.h"
#include "Vector.h"
#include "Mario.h"
#include "PlayScence.h"
#include "RedGoomba.h"
#include "GoldenBrick.h"
#include "BoomerangBrother.h"
#include <iostream>


LPDIRECT3DTEXTURE9 Koopas::texture = NULL;
unordered_map<string, LPSPRITE> Koopas::sprites; //save all sprite of animation
unordered_map<string, LPANIMATION> Koopas::all_animations; //save all animations
CAnimationSets Koopas::animations_set; //save all the animation sets
json Koopas::data = NULL;
json Koopas::spriteData = NULL;



#define VX_KICK 0.5f

Koopas::Koopas()
{
	SetState("running");
	v = Vector(-0.05f, 0);
	isBlockPlayer = true;
	useLimit = true;
}

void Koopas::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	if (revieTime >= 0 && state == "die" && isHitted == false) {
		revieTime -= dt;
	}
	else if (revieTime < 0 && state == "die" && isHitted == false) {
		SetState("running");
		isHolded = false;
		holdedBy = NULL;
	}

	if (isHolded == true && ((Mario*)holdedBy)->action == MarioAction::HOLD) {
		p.x = holdedBy->nx < 0 ? holdedBy->p.x - holdedBy->width : holdedBy->p.x + holdedBy->width;
		p.y = holdedBy->p.y;
		nx = holdedBy->nx;
		return;
	}
	else if (isHolded == true && ((Mario*)holdedBy)->action != MarioAction::HOLD) {
		isHolded = false;
		isUniversal = true;
		this->v.x = VX_KICK * holdedBy->nx;
		isHitted = true;
		isBlockPlayer = true;
		useLimit = false;
	}


	Enemy::Update(dt, coObjects);
	CGameObject::Update(dt, coObjects);
	Enemy::CheckToChangeDirection();

	v.y = v.y + g.y * (float)dt;
	
	if (abs(v.x) > VX_KICK) v.x = VX_KICK * (v.x / abs(v.x));

	if (v.y > MAX_VY) v.y = MAX_VY;
	if (v.x > 0) nx = 1; else nx = -1;

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	vector<LPGAMEOBJECT>* checkObjects = new vector<LPGAMEOBJECT>();

	coEvents.clear();


	for (auto i = coObjects->begin(); i != coObjects->end(); i++)
	{
		if ((*i)->isAllowCollision == true && !CPlayScene::IsPlayer(*i)) {
			checkObjects->push_back((*i));
		}
	}

	CalcPotentialCollisions(checkObjects, coEvents);


	if (coEvents.size() == 0) {

		p = p + d;
	}
	else {
		p.x = p.x + d.x;

		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;

		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);


		/*	if (nx != 0) v.x = -v.x;*/
		if (ny != 0) v.y = 0;

		for (UINT i = 0; i < coEventsResult.size(); i++) {
			HandleCollision(coEventsResult[i]);
		}

		for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
	}
	coEvents.clear();
	coEventsResult.clear();
	checkObjects->clear();
	delete checkObjects;
}


void Koopas::SetState(string state)
{
	if (state == "die") {
		v = Vector(0, 0);
	}
	else if (state == "running") {
		v.x = -0.05f;
	}
	CGameObject::SetState(state);
}

void Koopas::HandleCollision(LPCOLLISIONEVENT e) {
	Enemy::HandleCollision(e);
	if (e->obj->name == "RectCollision") {
		useLimit = false;
	}

	if (state == "die" && isHitted == true && e->nx != 0) {
		if (Goomba* obj = dynamic_cast<Goomba*>(e->obj)) {
			obj->BeingKill();
		}
		if (RedGoomba* obj = dynamic_cast<RedGoomba*>(e->obj)) {
			obj->BeingKill();
		}
		if (BoomerangBrother* obj = dynamic_cast<BoomerangBrother*>(e->obj)) {
			obj->BeingKill();
		}
		if (MisteryBox* obj = dynamic_cast<MisteryBox*>(e->obj)) {
			obj->GiveReward();
		}
		if (GoldenBrick* obj = dynamic_cast<GoldenBrick*>(e->obj)) {
			if (obj->type == "Break") obj->Explore();
		}
	}
}


void Koopas::OnHadCollided(LPGAMEOBJECT obj, LPCOLLISIONEVENT event) {
	string stateBefore = state;
	Enemy::OnHadCollided(obj, event);
	if (Mario* player = dynamic_cast<Mario*>(obj)) {
		if (stateBefore != state) {
			revieTime = 7000;
			player->SetAction(MarioAction::JUMP, 500);
			return;
		}

		if (state == "die") {
			if (isHitted == false && player->holdingKeys[DIK_A] == false && isHolded == false && player->action != MarioAction::ATTACK) {
				player->SetAction(MarioAction::KICK, 200);
				isUniversal = true;
				this->v.x = VX_KICK * event->nx != 0 ? -event->nx : player->nx;
				isHitted = true;
				isBlockPlayer = true;
				useLimit = false;
				return;
			}
			else if (isHitted == false && event->nx != 0 && isHolded == false && player->holdingKeys[DIK_A] == true) {
				player->SetAction(MarioAction::HOLD);
				holdedBy = player;
				isHolded = true;
			}
			else if (isHitted == true && player->action != MarioAction::KICK) {
				KillPlayer(player);
				this->v.x = VX_KICK * event->nx != 0 ? -event->nx : nx;
			}

		}
	}
}
