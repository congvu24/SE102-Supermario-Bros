#include <d3dx9.h>
#include <algorithm>


#include "Utils.h"
#include "Textures.h"
#include "Game.h"
#include "GameObject.h"
#include "Sprites.h"


CGameObject::CGameObject()
{
	/*p.x = p.y = 0;
	v.x = v.y = 0;*/


	p = Vector(0, 0);
	v = Vector(0, 0);
	d = Vector(0, 0);
	nx = 1;


	//D3DXIMAGE_INFO info;
	//HRESULT result = D3DXGetImageInfoFromFile(L"assets/texture/bbox.png", &info);
	//if (result != D3D_OK)
	//{
	//	DebugOut(L"[ERROR] GetImageInfoFromFile failed: %s\n", L"assets/texture/bbox.png");
	//	return;
	//}

	//LPDIRECT3DDEVICE9 d3ddv = CGame::GetInstance()->GetDirect3DDevice();
	//LPDIRECT3DTEXTURE9 textu;

	//result = D3DXCreateTextureFromFileEx(
	//	d3ddv,								// Pointer to Direct3D device object
	//	L"assets/texture/bbox.png",							// Path to the image to load
	//	info.Width,							// Texture width
	//	info.Height,						// Texture height
	//	1,
	//	D3DUSAGE_DYNAMIC,
	//	D3DFMT_UNKNOWN,
	//	D3DPOOL_DEFAULT,
	//	D3DX_DEFAULT,
	//	D3DX_DEFAULT,
	//	D3DCOLOR_XRGB(255, 0, 255),		// transparentColor	
	//	&info,
	//	NULL,
	//	&textu);								// Created texture pointer

	//if (result != D3D_OK)
	//{
	//	OutputDebugString(L"[ERROR] CreateTextureFromFile failed\n");
	//	return;
	//}

	//bboxtex = textu;

}

void CGameObject::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	this->dt = dt;
	d = v * dt;
}

/*
	Extension of original SweptAABB to deal with two moving objects
*/
LPCOLLISIONEVENT CGameObject::SweptAABBEx(LPGAMEOBJECT coO)
{
	float sl, st, sr, sb;		// static object bbox
	float ml, mt, mr, mb;		// moving object bbox
	float t, nx, ny;

	coO->GetBoundingBox(sl, st, sr, sb);

	// deal with moving object: m speed = original m speed - collide object speed
	float svx = 0, svy = 0;
	coO->GetSpeed(svx, svy);

	float sdx = svx * dt;
	float sdy = svy * dt;

	// (rdx, rdy) is RELATIVE movement distance/velocity 
	float rdx = this->d.x - sdx;
	float rdy = this->d.y - sdy;

	GetBoundingBox(ml, mt, mr, mb);

	CGame::SweptAABB(
		ml, mt, mr, mb,
		rdx, rdy,
		sl, st, sr, sb,
		t, nx, ny
	);

	CCollisionEvent* e = new CCollisionEvent(t, nx, ny, rdx, rdy, coO);
	return e;
}

/*
	Calculate potential collisions with the list of colliable objects

	coObjects: the list of colliable objects
	coEvents: list of potential collisions
*/
void CGameObject::CalcPotentialCollisions(
	vector<LPGAMEOBJECT>* coObjects,
	vector<LPCOLLISIONEVENT>& coEvents)
{
	for (UINT i = 0; i < coObjects->size(); i++)
	{
		if (this != coObjects->at(i)) {
			LPCOLLISIONEVENT e = SweptAABBEx(coObjects->at(i));

			if (e->t > 0 && e->t <= 1.0f) {
				coEvents.push_back(e);
			}
			else {
				delete e;
			}
		}
	}

	std::sort(coEvents.begin(), coEvents.end(), CCollisionEvent::compare);
}

//filter collision with moving object 
void CGameObject::FilterCollision(
	vector<LPCOLLISIONEVENT>& coEvents,
	vector<LPCOLLISIONEVENT>& coEventsResult,
	float& min_tx, float& min_ty,
	float& nx, float& ny, float& rdx, float& rdy)
{
	min_tx = 1.0f;
	min_ty = 1.0f;
	int min_ix = -1;
	int min_iy = -1;

	nx = 0.0f;
	ny = 0.0f;

	coEventsResult.clear();

	for (UINT i = 0; i < coEvents.size(); i++)
	{
		LPCOLLISIONEVENT c = coEvents[i];

		if (c->t < min_tx && c->nx != 0) {
			min_tx = c->t; nx = c->nx; min_ix = i; rdx = c->dx;
		}

		if (c->t < min_ty && c->ny != 0) {
			min_ty = c->t; ny = c->ny; min_iy = i; rdy = c->dy;
		}
	}

	if (min_ix >= 0) coEventsResult.push_back(coEvents[min_ix]);
	if (min_iy >= 0) coEventsResult.push_back(coEvents[min_iy]);
}



void CGameObject::RenderBoundingBox()
{
	/*D3DXVECTOR3 p(p.x, p.y, 0);
	RECT rect;

	float l, t, r, b;

	GetBoundingBox(l, t, r, b);
	rect.left = 0;
	rect.top = 0;
	rect.right = (int)r - (int)l;
	rect.bottom = (int)b - (int)t;

	if (bboxtex != NULL)
		CGame::GetInstance()->Draw(p.x, p.y, bboxtex, rect.left, rect.top, rect.right, rect.bottom, 100);*/
}

CGameObject::~CGameObject()
{
}