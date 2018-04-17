#pragma once

#include "DirectXImage.hpp"
#include <time.h>

#define TEXTURECUNT (4)		// テクスチャの数
#define MAXEFFECTCNT (200)	// 一度に描画できるエフェクトの数

typedef struct _EFFECTPOS {
	float x, y;
}EFFECTPOS;

typedef struct _EFFECTINFO {
	unsigned int texID;
	EFFECTPOS pos;
	SIZE size;
	float rotate;
}EFFECTINFO;


class Effect {
private:
	EFFECTINFO info;

	EFFECTPOS mvSpeed;
	float rotateSpeed;
	

public:
	Effect(unsigned int texID, EFFECTPOS &pos, SIZE &size, EFFECTPOS &mvSpd, float rotateSpd);
	~Effect();
	
	void Move();
	
	const EFFECTPOS* GetPosition() const;
	const EFFECTINFO* GetInfo() const;
};


class EffectManager {
private:
	dx9::DirectXImage *dim;
	Effect *effect[MAXEFFECTCNT];

	unsigned int width, height;

	EffectManager(const EffectManager&) = delete;	// コピー不可
	const EffectManager& operator=(const EffectManager&) = delete; // 代入不可

public:
	EffectManager(dx9::DirectXImage *dim, unsigned int screenW, unsigned int screenH);
	~EffectManager();

	// 更新
	void Update();

	// 描画
	void Draw() const;

	unsigned int effectCnt;

private:
	void AddEffect();
	bool IsFrameout(unsigned int element);
	void DeleteEffect(unsigned int element);

};