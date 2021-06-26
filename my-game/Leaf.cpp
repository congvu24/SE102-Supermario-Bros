#include "Leaf.h"
#include "Vector.h"
#include "Test.h"
#include <iostream>


LPDIRECT3DTEXTURE9 Leaf::texture = NULL;
unordered_map<string, LPSPRITE> Leaf::sprites; //save all sprite of animation
unordered_map<string, LPANIMATION> Leaf::all_animations; //save all animations
CAnimationSets Leaf::animations_set; //save all the animation sets
json Leaf::data = NULL;

Leaf::Leaf()
{
	SetState("fromMisteryBox");
	isBlockPlayer = false;
	isAllowCollision = false;
}

void Leaf::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	v = v + g * dt;
	if (v.y > 0.15f) v.y = 0.15f;

	CGameObject::Update(dt, coObjects);

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;


	coEvents.clear();
	for (auto i = coObjects->begin(); i != coObjects->end(); i++)
	{
		if (Test* player = dynamic_cast<Test*>((*i))) {
			LPCOLLISIONEVENT e = SweptAABBEx(*i);

			if (e->t > 0 && e->t <= 1.0f) {
				coEvents.push_back(e);
			}
			else {
				delete e;
			}
		}

	}
	float time = GetTickCount64() - beginFalling;
	p.y = p.y + d.y;
	if (v.y > 0)
		p.x = oldP.x + LEAF_AMPLITUDE * cos(LEAF_SPEED * time * 1.0f - PI / 2);

	if (LEAF_AMPLITUDE * cos(LEAF_SPEED * time * 1.0f - PI / 2) > 0) nx = 1;
	else nx = - 1;

	float min_tx, min_ty, nx = 0, ny;
	float rdx = 0;
	float rdy = 0;

	FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);


	for (UINT i = 0; i < coEventsResult.size(); i++) {

		HandleCollision(coEventsResult[i]);
	}

	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];

}

void Leaf::SetState(string state)
{
	if (state == "running") {
		v.x = 0;
		v.y = 0;
		g.y = 0;
	}
	else if (state == "fromMisteryBox") {
		p.y = p.y - 20;
		oldP = p;
	}
	else if (state == "hidden") {

	}

	CGameObject::SetState(state);

}

void Leaf::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = p.x;
	top = p.y;
	right = p.x + width;
	bottom = p.y + height;
}


void Leaf::HandleCollision(LPCOLLISIONEVENT e) {
	LPGAMEOBJECT obj = e->obj;

	if (Test* player = dynamic_cast<Test*>(e->obj)) {
		player->Transform();
		SetState("hidden");
	}
}