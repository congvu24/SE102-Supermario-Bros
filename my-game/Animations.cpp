#include "Animations.h"
#include "Utils.h"
#include "GameObject.h"

void CAnimation::Add(LPSPRITE sprite, DWORD time)
{
	int t = time;
	if (time == 0) t = this->defaultTime;


	//if (sprite == NULL)
	//{
	//	DebugOut(L"[ERROR] Sprite ID %d cannot be found!\n", spriteId);
	//}

	LPANIMATION_FRAME frame = new CAnimationFrame(sprite, t);
	frames.push_back(frame);
}

// NOTE: sometimes Animation object is NULL ??? HOW ??? 
void CAnimation::Render(float x, float y, int alpha, float& width, float& height, Vector scale)
{
	this->currentFrame = this->currentFrame;
	DWORD now = GetTickCount();
	if (currentFrame == -1)
	{
		currentFrame = 0;
		lastFrameTime = now;
	}
	else
	{
		DWORD t = frames[currentFrame]->GetTime();
		if (now - lastFrameTime > t)
		{
			currentFrame++;
			lastFrameTime = now;
			if (currentFrame == frames.size()) currentFrame = 0;
		}
	}
	LPSPRITE sprite = frames[currentFrame]->GetSprite();
	width = sprite->width;
	height = sprite->height;

	sprite->DrawWithScale(x, y, scale, alpha);
}


void CAnimation::GetSpriteSize( float& width, float& height)
{
	this->currentFrame = this->currentFrame;
	DWORD now = GetTickCount();
	if (currentFrame == -1)
	{
		currentFrame = 0;
		lastFrameTime = now;
	}
	else
	{
		DWORD t = frames[currentFrame]->GetTime();
		if (now - lastFrameTime > t)
		{
			currentFrame++;
			lastFrameTime = now;
			if (currentFrame == frames.size()) currentFrame = 0;
		}
	}
	LPSPRITE sprite = frames[currentFrame]->GetSprite();
	width = sprite->width;
	height = sprite->height;
}



void CAnimations::Add(string id, LPANIMATION ani)
{
	animations[id] = ani;
}

LPANIMATION CAnimations::Get(string id)
{
	LPANIMATION ani = animations[id];
	if (ani == NULL)
		DebugOut(L"[ERROR] Failed to find animation id: %d\n", id);
	return ani;
}

void CAnimations::Clear()
{
	for (auto x : animations)
	{
		LPANIMATION ani = x.second;
		delete ani;
	}

	animations.clear();
}

CAnimationSets::CAnimationSets()
{

}



LPANIMATION_SET CAnimationSets::Get(string id)
{
	LPANIMATION_SET ani_set = animation_sets[id];
	/*if (ani_set == NULL)
		DebugOut(L"[ERROR] Failed to find animation set id: %d\n", id);*/

	return ani_set;
}

void CAnimationSets::Add(string id, LPANIMATION_SET ani_set)
{
	animation_sets[id] = ani_set;
}

void CAnimationSets::Clear() {

	animation_sets.clear();
}