#ifndef _CGAME_H
#define _CGAME_H
////////////////////////////////////////////////////////////////////////////
// CGame : ゲームメインルーチン v2.00
// ・Win10動作確認済み
////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "libfiles/CDIPro81.h"
#include "libfiles/CTimer.h"
#include "libfiles/CWindow.h"
#include "libfiles/DirectXFigure.hpp"
#include "libfiles/DirectXFontAscii.hpp"
#include "libfiles/DirectXImage.hpp"
#include "libfiles/Effect.hpp"
#include "libfiles/LogManager.hpp"
#include "Lottery.hpp"

// ステート
enum STATE {
	INIT=0,			// ゲーム初期化
	RUN,			// ルーレット開始
	END				// 全てのゲーム終了処理
};


enum SETNUMBER {
	NONE,
	GIRL,
	BOY
};

class CGame {
public :
	static const int KEYCNT = 16;

private:
	// ライブラリなど
	CWindow					win;		// ウインドウ管理
	dx9::DirectXImage		dim;
	CDIPro81				di;			// DirectInput全般
	dx9::DirectXFontAscii	dt;			// テキスト関連
	dx9::DirectXFontAscii	dtsmall;
	dx9::DirectXFigure		df;
	
	LogFile					log;

	EffectManager	*ef;

	Lottery			lottery;

	STATE			eState;				// ゲームのステート状況

	size_t			iWiningNum;			// 現在の当選番号

	int				iRouletteState;		// ルーレットの各桁の状態


	float			bgScale;


	// 入力状態
	BOOL			bOnKey[KEYCNT];	// キーが押されているか

				
	const int KEYID[KEYCNT] ={		// キーのリスト
		DIK_SPACE, DIK_C, DIK_V, DIK_B, DIK_N, DIK_RETURN, DIK_1, DIK_2, DIK_3,DIK_4,DIK_5,
		DIK_6, DIK_7, DIK_8, DIK_9, DIK_0
	};



private:
	// 初期化
	BOOL Init( HINSTANCE hinst );		// 初期化＆ゲーム形成

	BOOL RunRoulette();
	BOOL SetNumber(std::vector<size_t> &group);		// 女子のくじを引く場合はoptionをtrueにする

public:
	// 公開関数
	CGame();
	virtual ~CGame();
	
	BOOL Run( HINSTANCE hinst );		// ゲームメインルーチン
};

#endif
