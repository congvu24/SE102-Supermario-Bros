#include "MisteryBox.h"
#include "Mushroom.h"
#include "Vector.h"
#include "PlayScence.h"
#include <iostream>


LPDIRECT3DTEXTURE9 MisteryBox::texture = NULL;
unordered_map<string, LPSPRITE> MisteryBox::sprites; //save all sprite of animation
unordered_map<string, LPANIMATION> MisteryBox::all_animations; //save all animations
CAnimationSets MisteryBox::animations_set; //save all the animation sets
json MisteryBox::data = NULL;
json MisteryBox::spriteData = NULL;
MisteryBox::MisteryBox()
{
	SetState("running");
	oldP = Vector(0, 0);
	d = Vector(0, 0);
	v = Vector(0, 0);
	g = Vector(0, 0);
	nx = -1;
	isAllowCollision = true;
}

void MisteryBox::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	Box::Update(dt, coObjects);
}


void MisteryBox::SetState(string state)
{
	CGameObject::SetState(state);
}

void MisteryBox::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = p.x;
	top = p.y;
	right = p.x + width;
	bottom = p.y + height;
}

void MisteryBox::OnHadCollided(LPGAMEOBJECT obj, LPCOLLISIONEVENT event) {

	Box::OnHadCollided(obj, event);

	if (CPlayScene::IsPlayer(obj)) {
		if (isHitted == false && event->ny > 0 && event->nx == 0) {
			GiveReward();
		}
	}
}

void MisteryBox::GiveReward() {
	if (isHitted == false) {
		SetState("hitted");
		isHitted = true;
		LPGAMEOBJECT reward = NULL;
		if (name == "QuestionBox_Coin") {
			reward = new Coin();
			reward->name = "Coin";
			reward->ParseFromOwnJson();
			reward->isAllowCollision = false;
		}
		else if (name == "QuestionBox_Mushroom") {
			reward = new Mushroom();
			reward->ParseFromOwnJson();
			reward->name = "Mushroom";
			reward->isAllowCollision = true;
		}
		else {
			reward = new Leaf();
			reward->ParseFromOwnJson();
			reward->name = "Leaf";
			reward->isAllowCollision = true;
		}
		reward->height = height;
		reward->width = width;
		reward->p = p;
		reward->p.y = p.y - height;
		CGame::GetInstance()->GetCurrentScene()->addObject(reward);
		reward->SetState("fromMisteryBox");
	}

}

