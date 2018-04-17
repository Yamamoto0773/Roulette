#include "Effect.hpp"

#define _USE_MATH_DEFINES
#include <math.h>  


Effect::Effect(unsigned int _texID, EFFECTPOS &pos, SIZE &size, EFFECTPOS &_mvSpd, float rorateSpd) {
	info.texID = _texID;
	info.pos = pos;
	info.size = size;
	info.rotate = 0.0f;
	mvSpeed = _mvSpd;
	rotateSpeed = rorateSpd;
}

Effect::~Effect() {
}

void Effect::Move() {
	info.pos.x += mvSpeed.x;
	info.pos.y += mvSpeed.y;
	info.rotate += rotateSpeed;
}

const EFFECTPOS* Effect::GetPosition() const {
	return &info.pos;
}

const EFFECTINFO* Effect::GetInfo() const {
	return &info;
}




EffectManager::EffectManager(dx9::DirectXImage *dim, unsigned int screenW, unsigned int screenH) {
	this->dim = dim;
	width = screenW;
	height = screenH;

	effectCnt  = 0;

	for (int i=0; i<MAXEFFECTCNT; i++) {
		effect[i] = NULL;
	}

	srand(unsigned(time(NULL)));
}

EffectManager::~EffectManager() {
	for (int i=0; i<MAXEFFECTCNT; i++) {
		delete effect[i];
	}
}

void EffectManager::Update() {
	for (int i=0; i<effectCnt; i++) {
		effect[i]->Move();
		if (IsFrameout(i) == true) {
			DeleteEffect(i);
		}
	}

	if (rand()%10 == 1) AddEffect();
}

void EffectManager::Draw() const {
	dim->SetBlendMode(dx9::BLENDMODE::NORMAL);

	for (int i=0; i<effectCnt; i++) {
		const EFFECTINFO *info = effect[i]->GetInfo();
	
		dim->Draw(info->texID + 1, 
			info->pos.x, 
			info->pos.y, 
			dx9::DrawTexCoord::CENTER,
			0.7f,
			(float)info->size.cx/dim->GetTexSize(info->texID+1).w,
			(float)info->size.cy/dim->GetTexSize(info->texID+1).h,
			info->rotate
			);

	}
}

void EffectManager::AddEffect() {
	if (effectCnt >= MAXEFFECTCNT) return;

	int t = 30 + rand()%10;
	SIZE size ={ t,t };
	EFFECTPOS pos ={ (float)(-t+rand()%(width+2*t)), -2.0f*t };
	float rotate = (float)(rand()%10/100.0-0.05);
	EFFECTPOS spd ={ rotate*10, 1.0 };

	effect[effectCnt] = new Effect(rand()%TEXTURECUNT, pos, size, spd, rotate*180/M_PI);
	effectCnt++;
}

bool EffectManager::IsFrameout(unsigned int element) {
	if (element < 0 || element >= MAXEFFECTCNT) return false;

	const EFFECTINFO *info = effect[element]->GetInfo();
	
	if (info->pos.y > (height + info->size.cy)) return true;

	return false;
}

void EffectManager::DeleteEffect(unsigned int element) {
	if (effectCnt <= 0) return;
	if (element < 0u || element >= effectCnt) return;

	delete effect[element];
	effectCnt--;

	effect[element] = effect[effectCnt];
	effect[effectCnt] = NULL;
}
