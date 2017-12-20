#ifndef _CGAME_H
#define _CGAME_H
////////////////////////////////////////////////////////////////////////////
// CGame : ゲームメインルーチン v2.00
// ・Win10動作確認済み
////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "./libfiles/CDDPro90.h"
#include "./libfiles/CDIPro81.h"
#include "./libfiles/CWindow.h"
#include "./libfiles/DXTextANSI.h"
#include "./libfiles/Effect.hpp"
#include "./libfiles/DirectXFigure.h"

#define MAXKEYCNT 7


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
	// ライブラリなど
	CWindow			win;				// ウインドウ管理
	CDDPro90		dd;					// Direct3D管理クラス
	CDIPro81		di;					// DirectInput全般
	DXTextANSI		dt;					// テキスト関連
	DXTextANSI		dtsmall;
	DirectXFigure	df;

	EffectManager	*ef;

	STATE			eState;				// ゲームのステート状況

	BOOL			bLostDevice;		// D3Dデバイスロスト状態フラグ

	int				*pLottery;			// くじを保存するポインタ
	int				BOYCNT;				// 男子くじの数
	int				iRestBoyCnt;		// 男子くじの残り数
	int				GIRLCNT;			// 女子のくじの数
	int				iRestGirlCnt;		// 女子のくじの残り数

	int				iWiningNum;			// 現在の当選番号

	int				iRouletteState;		// ルーレットの各桁の状態

	// 入力状態
	BOOL			bOnKey[MAXKEYCNT];	// キーが押されているか

private:
	// 初期化
	BOOL Init( HINSTANCE hinst );		// 初期化＆ゲーム形成
	BOOL Clear( void );					// ロード済みデータの開放

	BOOL RunRoulette();
	BOOL SetNumber(SETNUMBER mode);		// 女子のくじを引く場合はoptionをtrueにする

public:
	// 公開関数
	CGame();
	virtual ~CGame();
	
	BOOL Run( HINSTANCE hinst );		// ゲームメインルーチン
};

#endif
