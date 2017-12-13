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
#include "./libfiles/DirectXText.h"


#define MAXKEYCNT 7


typedef struct _Point {
	float x, y;
}Point;

// ステート
enum STATE {
	INIT=0,			// ゲーム初期化
	RUN,	// ルーレット開始
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
	DirectXText		dt;					// テキスト関連

	STATE			eState;				// ゲームのステート状況

	BOOL			bLostDevice;		// D3Dデバイスロスト状態フラグ

	int				*pLottery;			// くじを保存するポインタ
	int				BOYLOTTERYCNT;		// 当選する番号の最大数
	int				iLastBoyLotteryCnt;	// くじの残り数(合計
	int				GIRLLOTTERYCNT;		// 女子のくじの最大数
	int				iLastGirlLotteryCnt;// 女子のくじの残り数

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
