#include "Mushroom.h"
#include "Vector.h"
#include "Mario.h"
#include "Effect.h"
#include "PlayScence.h"
#include <iostream>


LPDIRECT3DTEXTURE9 Mushroom::texture = NULL;
unordered_map<string, LPSPRITE> Mushroom::sprites; //save all sprite of animation
unordered_map<string, LPANIMATION> Mushroom::all_animations; //save all animations
CAnimationSets Mushroom::animations_set; //save all the animation sets
json Mushroom::data = NULL;
json Mushroom::spriteData = NULL;
Mushroom::Mushroom()
{
	SetState("fromMisteryBox");
	isBlockPlayer = false;
	isAllowCollision = false;
	point = 400;
	isUniversal = true;
}

Mushroom::Mushroom(string type)
{
	SetState("fromMisteryBox");
	isBlockPlayer = false;
	isAllowCollision = false;
	this->type = type;
	point = 400;
	isUniversal = true;
}

void Mushroom::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	v = v + g * (float)dt;
	if (v.y > 0.15f) v.y = 0.15f;

	

	CGameObject::Update(dt, coObjects);
	if (p.y + height <= oldP.y - 20) {
		SetState("running");
	}


	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;
	vector<LPGAMEOBJECT>* checkObjects = new vector<LPGAMEOBJECT>();

	coEvents.clear();
	for (auto i = coObjects->begin(); i != coObjects->end(); i++)
	{
		if (CPlayScene::IsPlayer(*i) || (*i)->isAllowCollision == true) {
			checkObjects->push_back(*i);
		}
	}

	
	coEvents.clear();
	CalcPotentialCollisions(checkObjects, coEvents);

	if (coEvents.size() == 0) {
		p = p + d;
	}
	else {
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;

		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);

		if (rdx != 0 && rdx != d.x)
			p.x += nx * abs(rdx);

		if (nx != 0) v.x = -v.x;
		if (ny != 0) v.y = 0;


		for (UINT i = 0; i < coEventsResult.size(); i++) {

			HandleCollision(coEventsResult[i]);
		}

		p.x += min_tx * d.x + nx * 0.4f;
		p.y += min_ty * d.y + ny * 0.4f;

		for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];

	}
}


void Mushroom::SetState(string state)
{
	if (state == "running") {
		v.x = -0.05f;
		v.y = 0;
		g.y = 0.001f;
	}
	else if (state == "fromMisteryBox") {
		oldP = p;
		v = Vector(0, -0.15f);
		g = Vector(0, 0);
	}
	else if (state == "hidden") {

	}

	CGameObject::SetState(state);

}

void Mushroom::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = p.x;
	top = p.y;
	right = p.x + width;
	bottom = p.y + height;
}


void Mushroom::HandleCollision(LPCOLLISIONEVENT e) {
	if (Mario* obj = dynamic_cast<Mario*>(e->obj)) {
		GiveReward();
		SetState("hidden");
	}
}

void Mushroom::OnHadCollided(LPGAMEOBJECT obj, LPCOLLISIONEVENT event) {
	if (Mario* obj = dynamic_cast<Mario*>(event->obj)) {
		GiveReward();
		SetState("hidden");
	}
}

void Mushroom::GiveReward() {
	if (type == "green") {
		Effect* pointEffect = new Effect("1up", 300);
		pointEffect->v.y = -0.05f;
		pointEffect->p = p;
		CGame::GetInstance()->GetCurrentScene()->addObject(pointEffect);
		CGame::GetInstance()->GetCurrentScene()->AddPoint(1000);
		LPGAMEOBJECT player = ((CPlayScene*)CGame::GetInstance()->GetCurrentScene())->player;
		((Mario*)player)->life++;
	}
	else {
		Effect* pointEffect = new Effect(to_string(point), 300);
		pointEffect->v.y = -0.05f;
		pointEffect->p = p;

		LPGAMEOBJECT player = ((CPlayScene*)CGame::GetInstance()->GetCurrentScene())->player;
		if (stoi(player->type) < BigMario) {
			((Mario*)player)->Transform(BigMario);
		}
		else {
			CGame::GetInstance()->GetCurrentScene()->addObject(pointEffect);
		}
		CGame::GetInstance()->GetCurrentScene()->AddPoint(point);
	}
}