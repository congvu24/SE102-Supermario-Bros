#include "Mario.h"
#include <iostream>
#include "Quadtree.h"
#include "MiniPortal.h"
#include "PlayScence.h"
#include "Game.h"
#include "Effect.h"


Mario::Mario()
{
	name = "player";
	renderOrder = DEFAULT_RENDER_ORDER;
	g = Vector(0, DEFAULT_GY);
	isUniversal = true;
	SetState("idle");
}


void Mario::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	if (untouchableTime > 0) {
		untouchableTime -= dt;
	}

	if (action == MarioAction::TRANSFORM && timeBeginAction >= 0) {
		UpdateTimeAction();
		ax = 0;
		v.x = 0;
		return;
	}
	if (action == MarioAction::GETTING_INTO_THE_HOLE && timeBeginAction >= 0) {
		UpdateAnimation(dt, coObjects);
		return;
	}

	if (abs(v.x) < MAX_VX)
		v.x += ax * dt;
	if (abs(v.x) >= MAX_VX && canJump == false) v.x = ((v.x) / abs(v.x)) * MAX_VX;



	CGameObject::Update(dt, coObjects);
	UpdateTimeAction();

	if (action != MarioAction::FLY)
		v.y = v.y + g.y * (float)dt * 1.8;
	if (v.y >= 0.8) v.y = 0.8;

	if (v.x * nx < 0 && abs(v.x) >= VX_TO_SKID && canJump == true) {
		SetAction(MarioAction::SKID, 500);
	}
	else if (v.x * nx < 0) {
		v.x = 0;
	}

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;
	vector<LPGAMEOBJECT>* checkObjects = new vector<LPGAMEOBJECT>();
	coEvents.clear();

	for (auto i = coObjects->begin(); i != coObjects->end(); i++)
	{
		if (state != "die") {
			if ((*i)->isBlockPlayer == true) {
				checkObjects->push_back((*i));
			}
		}
		else {
			if ((*i)->name == "Death") {
				checkObjects->push_back((*i));
			}
		}
	}

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

		if (nx != 0) v.x = 0;
		if (ny != 0) v.y = 0;

		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			if (ny < 0 && canJump == false) powerX = 0;
			if (ny < 0) canJump = true;
			HandleCollision(coEventsResult[i]);
			coEventsResult[i]->obj->OnHadCollided(this, coEventsResult[i]);
		}

		for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];

		p.x += min_tx * d.x + nx * 0.4f;
		p.y += min_ty * d.y + ny * 0.4f;

		coEventsResult.clear();
		coEvents.clear();
	}

	checkObjects->clear();
	delete checkObjects;

	ax = 0;
}

void Mario::Render()
{
	if (untouchableTime > 0) {
		isRender = !isRender;
	}
	else {
		isRender = true;
	}

	if (action == MarioAction::TRANSFORM && timeBeginAction >= 100) {
		return;
	}
	if (isRender) {
		float w = width;
		float h = height;
		Vector scale = Vector((float)nx, 1.0f);

		animations_set.Get(type).at(state)->GetSpriteSize(w, h);

		if (action != MarioAction::ATTACK) {
			if (abs(h - this->height) >= MIN_DIFF_UPDATE) {
				p.y = p.y - (h - this->height);
				height = h;
			}
			if (abs(w - this->width) >= MIN_DIFF_UPDATE) {
				p.x = p.x - (w - this->width);
				width = w;
			}
		}
		animations_set.Get(type).at(state)->Render(p.x, p.y, 255, w, h, scale);

		RenderBoundingBox();
	}
}

void Mario::SetState(string state)
{
	if (isReadyChangeState == true) {

		if (action == MarioAction::HOLD) {
			if (state == "running") {
				CGameObject::SetState("running-hold");
			}
			else if (state == "idle") {
				CGameObject::SetState("idle-hold");
			}
			return;
		}
		if (state == "idle") {
			v.x = 0;
			CGameObject::SetState("idle");

		}
		else if (state == "jump-height") {
			CGameObject::SetState("jumping");
		}
		else if (state == "fly-up" && canJump == false) {
			CGameObject::SetState("flying-speed");
		}
		else if (state == "fall-down" && canJump == false) {
			CGameObject::SetState("jumping");
		}
		else if (state == "flying" && canJump == false) {
			if (abs(powerX) == MAX_POWER)
				CGameObject::SetState("flying-speed");
			else
				CGameObject::SetState("flying");
		}
		else
			CGameObject::SetState(state);
	}

}

bool Mario::IsReadyToChangeAction() {
	if (timeBeginAction <= 0) return true;
	return false;
}

void Mario::SetAction(MarioAction newAction, DWORD time) {

	if (newAction == MarioAction::DIE) {
		action = newAction;
		return;
	}

	if (newAction == MarioAction::TRANSFORM) {
		action = newAction;
		timeBeginAction = time;
		LPGAMEOBJECT smoke = new Effect("smoke", time);
		smoke->p = Vector((p.x + width / 2), p.y + height / 2);
		CGame::GetInstance()->GetCurrentScene()->addObject(smoke);
	}

	if (newAction == MarioAction::RELEASE) {
		action = newAction;
		timeBeginAction = 200;
		SetState("kick");
	}

	if (action == MarioAction::HOLD && newAction == MarioAction::IDLE) {
		v.x = 0;
		SetState("idle");
		return;
	}
	else if (action == MarioAction::HOLD && newAction == MarioAction::RUN) {
		SetState("running");
		return;
	}
	else if (action == MarioAction::HOLD) {
		return;
	}

	if (newAction == MarioAction::HOLD) {
		action = newAction;
	}

	if (IsReadyToChangeAction() == false) return;

	switch (newAction)
	{
	case MarioAction::IDLE:
		if (canJump == true) {
			SetState("idle");
			action = newAction;
			timeBeginAction = time;
		}
		else if (action == MarioAction::FLY && canJump == false) {
			SetAction(MarioAction::JUMP_HEIGHT);
		}
		break;
	case MarioAction::WALK:
		action = newAction;
		timeBeginAction = 0;
		break;
	case MarioAction::RUN:
		SetState("running");
		action = newAction;
		timeBeginAction = 0;
		break;
	case MarioAction::JUMP:
		if (abs(powerX) == MAX_POWER && stoi(type) == RacconMario) {
			SetAction(MarioAction::JUMP_HEIGHT);
			break;
		}
		else {
			if (canJump == true) {
				v.y = -JUMP_VY;
				canJump = false;
				SetState("jumping");
				action = newAction;
				timeBeginAction = time;
			}
		}
		break;
	case MarioAction::CROUCH:
		v.x = 0;
		v.y = 0;
		SetState("crouch");
		break;
	case MarioAction::FLY:
		if (abs(powerX) == MAX_POWER && (action == MarioAction::FLY || action == MarioAction::JUMP_HEIGHT)) {
			v.y = -FLY_VY;
			SetState("fly-up");
			action = MarioAction::FLY;
			/*ax = 0;
			v.x = 0;*/
			timeBeginAction = time;
		}
		else if ((action == MarioAction::JUMP || action == MarioAction::JUMP_HEIGHT) && stoi(type) == RacconMario && canJump == false) {
			v.y = -0.1f;
			SetState("flying");
			action = MarioAction::JUMP;
			timeBeginAction = time;
		}
		break;
	case MarioAction::JUMP_HEIGHT:
		if (action == MarioAction::FLY) {
			action = MarioAction::JUMP_HEIGHT;
			timeBeginAction = time;
		}
		else if (canJump == true && abs(powerX) == MAX_POWER) {
			v.y = -FLY_VY;
			canJump = false;
			SetState("jump-height");
			action = newAction;
			timeBeginAction = time;
		}
		else if (action == MarioAction::JUMP && v.y > -JUMP_VY * 0.6f && v.y < -JUMP_VY * 0.55 && powerX < MAX_POWER) {
			v.y = -JUMP_VY;
			SetState("jumping");
			action = newAction;
			timeBeginAction = time;
		}
		break;
	case MarioAction::FALL:
		if (holdingKeys[DIK_S] == false && action == MarioAction::FLY) {
			SetState("fall-down");
			action = newAction;
			timeBeginAction = time;
		}
		break;
	case MarioAction::ATTACK:
		if (stoi(type) == RacconMario && canJump == true) {
			SetState("attack");
			action = newAction;
			timeBeginAction = time;
		}
		break;
	case MarioAction::KICK:
		SetState("kick");
		action = newAction;
		timeBeginAction = time;
		break;
	case MarioAction::DIE:
		action = newAction;
		break;
	case MarioAction::SKID:
		SetState("skid");
		action = newAction;
		v.x = -nx * VX_AFTER_SKID;
		timeBeginAction = time;
		break;
	case MarioAction::GETTING_INTO_THE_HOLE:
		SetState("into-hole");
		v.x = 0;
		action = newAction;
		canJump = false;
		timeBeginAction = time;
		break;
	case MarioAction::PICK_UP:
		action = newAction;
		timeBeginAction = time;
		break;
	default:
		break;
	}

}

void Mario::HandleCollision(LPCOLLISIONEVENT e) {
	LPGAMEOBJECT obj = e->obj;
	if (e->nx != 0) {
		HandleCollisionHorizontal(e);
	}

	if (e->ny != 0) {
		HandleCollisionVertical(e);
	}

	if (obj->name == "Death") {
		((CPlayScene*)CGame::GetInstance()->GetCurrentScene())->GameOver();
	}

}

void Mario::Die() {
	if (untouchableTime > 0) return;
	if (IsReadyToChangeAction() == true) {
		v.y = -JUMP_VY;
		v.x = 0;
		life = life - 1;

		if (life == 0) {
			SetState("die");
			SetAction(MarioAction::DIE);
			isReadyChangeState = false;
			isAllowCollision = false;
		}
		else if (stoi(type) == RacconMario) {
			Transform(BigMario);
		}
		else if (stoi(type) == BigMario) {
			Transform(SmallMario);
		}

	}
}

void Mario::Transform(int marioType) {

	if (marioType < stoi(type)) {
		untouchableTime = UNTOUCHABLE_TIME;
	}
	else {
		SetAction(MarioAction::TRANSFORM, 500);
	}
	switch (marioType)
	{
	case SmallMario:
		this->type = to_string(marioType);
		life = SMALL_LIFE;
		break;
	case BigMario:
		this->type = to_string(marioType);
		life = BIG_LIFE;
		break;
	case RacconMario:
		this->type = to_string(marioType);
		life = RACCON_LIFE;
		break;
	default:
		break;
	}
	SetState("jumping");
}

void Mario::ProcessKeyboard(KeyboardEvent kEvent)
{
	if (action == MarioAction::TRANSFORM || action == MarioAction::GETTING_INTO_THE_HOLE || state == "die") return;

	if (kEvent.isKeyUp == true) {
		holdingKeys[kEvent.key] = false;
	}

	if (kEvent.isHolding == true) holdingKeys[kEvent.key] = true;

	switch (kEvent.key) {
	case DIK_RIGHT:
		ax = A_X;
		nx = 1;

		if (action == MarioAction::FLY && stoi(type) == RacconMario && canJump == false) {
			if (holdingKeys[DIK_S] == false) {
				SetState("flying");

			}
		}
		else if (action == MarioAction::JUMP_HEIGHT && stoi(type) == RacconMario && canJump == false) {
			if (holdingKeys[DIK_S] == false) {
				SetState("flying");
			}
		}
		else if (action == MarioAction::JUMP && stoi(type) == RacconMario && canJump == false) {
			SetState("flying");
		}
		else if (canJump == false) {
			SetState("jumping");
		}
		else {
			//SetState("running");
			SetAction(MarioAction::RUN, 0);
		}
		break;
	case DIK_LEFT:
		ax = -A_X;
		nx = -1;

		if (action == MarioAction::FLY && stoi(type) == RacconMario && canJump == false) {
			if (holdingKeys[DIK_S] == false) {
				SetState("flying");
			}
		}
		else if (action == MarioAction::JUMP_HEIGHT && stoi(type) == RacconMario && canJump == false) {
			if (holdingKeys[DIK_S] == false) {
				SetState("flying");
			}
		}
		else if (action == MarioAction::JUMP && stoi(type) == RacconMario && canJump == false) {
			SetState("flying");
		}
		else if (canJump == false) {
			SetState("jumping");
		}
		else {
			//SetState("running");
			SetAction(MarioAction::RUN, 0);
		}
		break;
	case DIK_S:

		if (stoi(type) == RacconMario && canJump == false ) {
			if (!kEvent.isHolding && !kEvent.isKeyUp) {
				SetAction(MarioAction::FLY);
			}
			if (!kEvent.isHolding && !kEvent.isKeyUp) {
				SetAction(MarioAction::FALL);
			}
			//return;
		}


		if (action != MarioAction::FLY || action != MarioAction::JUMP_HEIGHT) {
			if (!kEvent.isHolding) {
				if (!kEvent.isKeyUp) SetAction(MarioAction::JUMP);
			}
			else SetAction(MarioAction::JUMP_HEIGHT);
		}


		break;
	case DIK_A:
		if (kEvent.isKeyUp == true) {
			if (action == MarioAction::HOLD) {
				SetAction(MarioAction::RELEASE);
			}
		}
		if (action != MarioAction::HOLD) {
			if (!kEvent.isHolding && !kEvent.isKeyUp)
				SetAction(MarioAction::ATTACK, 300);
		}
		break;
	case DIK_DOWN:
		if (action == MarioAction::IDLE && canJump == true && stoi(type) > SmallMario) {
			SetAction(MarioAction::CROUCH);
		}
		break;
	case DIK_SPACE:

		break;
	case DIK_1:
		Transform(SmallMario);
		break;
	case DIK_2:
		Transform(BigMario);
		break;
	case DIK_3:
		Transform(RacconMario);
		break;
	default:
		break;
	}

}


void Mario::HandleCollisionVertical(LPCOLLISIONEVENT e) {
	if (this->nx != powerX / abs(powerX)) {
		powerX = 0;
	}

	if (holdingKeys[DIK_A] == true) {
		if (v.x != 0) {
			IncreasePowerX();
		}
	}
}
void Mario::HandleCollisionHorizontal(LPCOLLISIONEVENT e) {

}


void Mario::IncreasePowerX() {
	if (abs(powerX + ASC_POWER * this->nx) <= MAX_POWER) {
		powerX = powerX + ASC_POWER * this->nx;
		timeMaxPower = 0;
		if (abs(powerX) == MAX_POWER) {
			timeMaxPower = TIME_MAX_POWER;
		}
	}
}

void Mario::DecreasePowerX() {
	if (powerX >= 0)
		if (powerX - DESC_POWER >= 0)
			powerX = powerX - DESC_POWER;
		else powerX = 0;
	else if (powerX <= 0) {
		if (powerX + DESC_POWER <= 0)
			powerX = powerX + DESC_POWER;
		else powerX = 0;
	}
}


void Mario::Teleport(MiniPortal* destination, int duration) {
	if (teleportDestination == NULL) {
		teleportDestination = destination;
		renderOrder = 0;
		if (destination->direction == "DOWN") {
			d.y = 0.5f;
		}
		else if (destination->direction == "UPP") {
			d.y = -10;
		}
		else {
			d.y = -0.5f;
		}
		SetAction(MarioAction::GETTING_INTO_THE_HOLE, duration);
	}
}

void Mario::UpdateAnimation(DWORD dt, vector<LPGAMEOBJECT>* coObjects) {
	if (action == MarioAction::GETTING_INTO_THE_HOLE && timeBeginAction >= 100) {
		p.y = p.y + d.y;
		timeBeginAction = timeBeginAction - dt;
	}
	else if (action == MarioAction::GETTING_INTO_THE_HOLE && timeBeginAction < 100 && teleportDestination != NULL) {
		timeBeginAction = 0;
		Camera* camera = CGame::GetInstance()->GetCurrentScene()->getCamera();
		camera->isCameraMoving = !camera->isCameraMoving;
		camera->cam_top_limit = teleportDestination->camera_top_limit;
		camera->cam_right_limit = teleportDestination->camera_right_limit;
		camera->cam_left_limit = teleportDestination->camera_left_limit;
		camera->cam_bottom_limit = teleportDestination->camera_bottom_limit;

		p = teleportDestination->p;
		teleportDestination = NULL;
		nx = 1;
		v.x = 0;
		canJump = true;
		SetAction(MarioAction::IDLE);
		SetState("jumping");
		canJump = false;
		renderOrder = DEFAULT_RENDER_ORDER;
	}

	if (action == MarioAction::ATTACK && timeBeginAction >= 0) {
		timeBeginAction = timeBeginAction - dt;
	}
}

void Mario::HandleAfterCreated() {
	switch (stoi(type))
	{
	case SmallMario:
		life = SMALL_LIFE;
		break;
	case BigMario:
		life = BIG_LIFE;
		break;
	case RacconMario:
		life = RACCON_LIFE;
		break;
	default:
		break;
	}
}


void Mario::UpdateTimeAction() {
	if (timeBeginAction >= 0) {
		timeBeginAction = timeBeginAction - dt;
	}
	if (abs(powerX) == MAX_POWER && timeMaxPower >= 0) {
		timeMaxPower = timeMaxPower - dt;
	}
	if (powerX != 0 && (timeMaxPower < 0)) {
		DecreasePowerX();
	}
}