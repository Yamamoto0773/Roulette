#pragma warning( disable : 4995 )				// 警告を無視
#pragma warning( disable : 4996 )				// 警告を無視
#include "CGame.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define DEBUGMODE
#include "libfiles/DEBUG.H"
#include "./libfiles/MT.h"

#define STR(var) #var
#define SAFE_FREE(x) { if (x) { free(x); x=NULL; } }




/////////////////////////////////////////////////////////////////////////
// コンストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::CGame() {
	int i;
	bLostDevice		= FALSE;
	BOYLOTTERYCNT		= 0;
	eState			= INIT;
	pLottery		= NULL;
	iLastBoyLotteryCnt	= 0;
	iRouletteState	= 0b0000;
	GIRLLOTTERYCNT	= 0;
	iLastGirlLotteryCnt = 0;
	iWiningNum		= 0;

	ZeroMemory(&bOnKey, sizeof(bOnKey));
}


/////////////////////////////////////////////////////////////////////////
// デストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::~CGame() {
	Clear();
}


///////////////////////////////////////////////////////
// 初期化＆ゲーム形成
///////////////////////////////////////////////////////
BOOL CGame::Init(HINSTANCE hinst) {
	int i;

	int windowW = 960, windowH = 720;

	// ウインドウ生成
	win.SetWindowStyle(WS_OVERLAPPEDWINDOW);					// 枠無しウインドウ(フルスクリーン時はWS_POPUPのみ、ウィンドウモード時はさらにWS_CAPTION|WS_SYSMENUなどを付ける)
	if (!win.Create(hinst, L"Roulette", 1, windowW, windowH)) {	// ウィンドウサイズは720p
		DEBUG("Window create error\n");
		return FALSE;
	}
	ImmAssociateContext(win.hWnd, NULL);			// IMEを出さないようにする

	// Direct3D生成
	// フルスクリーンの1920*1080の32bitカラーにセットする。
	// ※2つ目の引数をFALSEにするとウインドウモードに出来る
	if (!dd.Create(win.hWnd, FALSE, windowW, windowH, 32, 0, TRUE)) {
		DEBUG("Direct3D create error\n");
		return FALSE;
	}

	// DirectInput生成
	if (!di.Create(win.hWnd, win.hInstance)) {
		DEBUG("DirectInput生成失敗\n");
		return FALSE;
	}

	// キーボードを使う
	if (!di.CreateKeyboard()) {
		DEBUG("キーボード使用不可\n");
		//		return FALSE;				// キーボードが使用できなくても起動可能とする
	}


	// DirectXText生成
	if (!dt.Init(dd.GetD3DDevice(), windowW, windowH)) {
		DEBUG("DirectXText生成失敗\n");
		return FALSE;
	}


	// 画像ファイル読み込み
	dd.AddTexture(0, "background.png");
	CDDTexPro90 *tex = dd.GetTexClass(0);
	dd.SetPutRange(0, 0, 0, 0, tex->GetWidth(), tex->GetHeight());
	dd.SetPutStatus(0, 0.9f, 0.7f, 0.0f);;

	dd.SetBackColor(0xffffff);



	// 数字のテクスチャを作成
	for (i=0; i<10; i++) {
		dt.SetFontStatus(i, 300, L"Century Gothic", 10, 10, 10, 255, FW_HEAVY);
		dt.SetString(i, "%d", i);
	}
	dt.SetFontStatus(10, 300, L"Century Gothic", 10, 10, 10, 255, FW_HEAVY);
	dt.SetString(10, "-");




	// ファイル読み込み
	FILE *fp;
	int max2, min2;
	int max1, min1;
	fp = fopen("DEFINE/BoyLottery.txt", "r");
	if (!fp) {
		DEBUG("ファイルエラー> \"DEFINE/BoyLottery.txt\"を開けません\n");
		return FALSE;
	}
	if (fscanf_s(fp, "%d,%d", &min1, &max1) != 2
		|| min1 <= 0 || max1 <= 0
		|| min1 >= 1000 || max1 >= 1000
		|| min1 > max1) {
		DEBUG("エラー> 男子くじの定義に失敗しました\n");
		fclose(fp);
		return FALSE;
	}
	fclose(fp);
	

	fp = fopen("DEFINE/GirlLottery.txt", "r");
	if (!fp) {
		DEBUG("ファイルエラー> \"DEFINE/GirlLottery\"を開けません\n");
		return FALSE;
	}
	if (fscanf_s(fp, "%d,%d", &min2, &max2) != 2
		|| min2 <= 0 || max2 <= 0
		|| min2 >= 1000 || max2 >= 1000
		|| min2 > max2) {
		DEBUG("エラー> 女子くじの定義に失敗しました\n");
		fclose(fp);
		return FALSE;
	}
	fclose(fp);

	if (min1<=min2 && max1>=min2 ||
		max2>=min1 && max2<=min1) {
		DEBUG("エラー> 女子のくじと男子のくじの番号が重複しています\n");
		return FALSE;
	}

	DEBUG("女子くじ :%3d - %3d\n", min2, max2);
	DEBUG("男子くじ :%3d - %3d\n", min1, max1);


	// くじの定義
	BOYLOTTERYCNT = max1 - min1 +1;
	GIRLLOTTERYCNT = max2 - min2 +1;
	iLastBoyLotteryCnt = BOYLOTTERYCNT;
	iLastGirlLotteryCnt = GIRLLOTTERYCNT;

	pLottery = (int*)malloc(sizeof(int)*(GIRLLOTTERYCNT+BOYLOTTERYCNT));
	if (!pLottery) {
		DEBUG("エラー> pLottery　メモリの確保に失敗しました\n");
		return FALSE;
	}

	for (i=0; i<GIRLLOTTERYCNT; i++) {
		pLottery[i] = min2 + i;
	}
	for (; i<GIRLLOTTERYCNT+BOYLOTTERYCNT; i++) {
		pLottery[i] = min1 + i-GIRLLOTTERYCNT;
	}


	// 乱数の種の設定
	init_genrand((unsigned)time(NULL));

	ShowWindow(win.hWnd, SW_SHOW);


	return TRUE;
}


///////////////////////////////////////////////////////
// ロード済みデータの全開放
///////////////////////////////////////////////////////
BOOL CGame::Clear(void) {
	int i;

	dd.Clear();

	return TRUE;
}


///////////////////////////////////////////////////////
// ルーレット実行関数
///////////////////////////////////////////////////////
BOOL CGame::RunRoulette() {
	// テンポラリ変数
	int i, j, k;

	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// ESCキーで終了
	if (key[DIK_ESCAPE]&0x80)
		return -1;

	// 仮想入力ハードウェア
	BOOL press[MAXKEYCNT];						// 押した瞬間にtrueになる配列
	ZeroMemory(&press, sizeof(press));

	// キーボードの処理
	static const int KEYID[MAXKEYCNT] ={		// キーのリスト
		DIK_SPACE, DIK_RETURN, DIK_BACK, DIK_RSHIFT, DIK_V, DIK_B, DIK_N
	};

	for (i=0; i<MAXKEYCNT; i++) {
		if ((key[KEYID[i]]&0x80)) {
			// キーボード入力があった場合
			if (!bOnKey[i]) {
				// まだ押されていなければ押された瞬間とする
				press[i] = TRUE;
				bOnKey[i] = TRUE;
			}
		}
		else {
			// 押されていなければフラグをリセット
			bOnKey[i] = FALSE;
		}
	}



	///////////////////////////////////////////////////
	// 入力処理
	///////////////////////////////////////////////////
	if (press[0]) {
		// スペースキーでルーレット停止
		iRouletteState &= 0b0000;
	}
	if (press[1]) {
		if (iRouletteState == 0b0000) {
			// ルーレットスタート
			if (SetNumber(NONE))
				iRouletteState |= 0b0111;
		}
	}
	else if (press[2]) {
		if (iRouletteState == 0b0000) {
			// ルーレットスタート　(女子くじのみ
			if (SetNumber(GIRL))
				iRouletteState |= 0b0111;
		}
	}
	else if (press[3]) {
		if (iRouletteState == 0b0000) {
			// ルーレットスタート　(男子くじのみ
			if (SetNumber(BOY))
				iRouletteState |= 0b0111;
		}
	}


	if (press[4]) {
		iRouletteState &= 0b1011;
	}
	if (press[5]) {
		iRouletteState &= 0b1101;
	}
	if (press[6]) {
		iRouletteState &= 0b1110;
	}



	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 0;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}


	////////////////////////////////////////////////////////////////////////////////////
	// 描画処理
	////////////////////////////////////////////////////////////////////////////////////
	dd.DrawBegin();

	dd.Put2(0, 960/2, 720/2);

	// 当選番号の描画
	int width = 160;
	int textX = 570;
	int textY = 180;
	int num = iWiningNum;
	for (i=0; i<3; i++) {
		if ((iRouletteState >> i)&0b0001) {
			// ルーレット回転中
			dt.Put(rand()%10, textX-width*i, textY);
		}
		else {
			// ルーレット停止中
			if (iWiningNum == 0) {
				// くじがなくなったとき
				dt.Put(10, textX-width*i, textY);
			}
			else {
				dt.Put(num%10, textX-width*i, textY);

			}
		}
		num/=10;
	}

	dd.DrawEnd();

	// 継続
	return 0;
}


BOOL CGame::SetNumber(SETNUMBER mode) {
	// 当選番号取得
	int i;
	if (mode == GIRL) {
		if (iLastGirlLotteryCnt < 1) {
			// くじがないときは終了
			return FALSE;
		}

		i = genrand_int32()%iLastGirlLotteryCnt;
		i += (GIRLLOTTERYCNT-iLastGirlLotteryCnt);
	}
	else if (mode == BOY) {
		if (iLastBoyLotteryCnt < 1) {
			// くじがない場合は終了
			return FALSE;
		}

		i = genrand_int32()%iLastBoyLotteryCnt;
		i += GIRLLOTTERYCNT;
	}
	else {
		if (iLastBoyLotteryCnt+iLastGirlLotteryCnt < 1) {
			// くじがないときは終了
			return FALSE;
		}

		i = genrand_int32()%(iLastBoyLotteryCnt+iLastGirlLotteryCnt);
		i += (GIRLLOTTERYCNT-iLastGirlLotteryCnt);
	}
	iWiningNum = pLottery[i];



	// くじの並びの最適化
	if (i < GIRLLOTTERYCNT) {
		// 女子のくじの並び替え
		pLottery[i] = pLottery[GIRLLOTTERYCNT-iLastGirlLotteryCnt];
		pLottery[GIRLLOTTERYCNT-iLastGirlLotteryCnt] = 0;

		iLastGirlLotteryCnt--;
	}
	else {
		// 男子のくじの並び替え
		pLottery[i] = pLottery[GIRLLOTTERYCNT+iLastBoyLotteryCnt-1];
		pLottery[GIRLLOTTERYCNT+iLastBoyLotteryCnt-1] = 0;

		iLastBoyLotteryCnt--;
	}


	if (mode == GIRL) {
		DEBUG("No.%3d: %4d <GIRL>\n", GIRLLOTTERYCNT+BOYLOTTERYCNT-iLastBoyLotteryCnt-iLastGirlLotteryCnt, iWiningNum);
	}
	else if (mode == BOY) {
		DEBUG("No.%3d: %4d <BOY>\n",  GIRLLOTTERYCNT+BOYLOTTERYCNT-iLastBoyLotteryCnt-iLastGirlLotteryCnt, iWiningNum);
	}
	else {
		DEBUG("No.%3d: %4d\n",  GIRLLOTTERYCNT+BOYLOTTERYCNT-iLastBoyLotteryCnt-iLastGirlLotteryCnt, iWiningNum);

	}

	return TRUE;
}




/////////////////////////////////////////////////////////////////////////
// ゲームメインルーチン
/////////////////////////////////////////////////////////////////////////
BOOL CGame::Run(HINSTANCE hinst) {
	// ゲームメインループ
	MSG msg;
	BOOL bLoop=TRUE;
	while (bLoop) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message==WM_QUIT) {
				bLoop = FALSE;
				DEBUG("WM_QUIT\n");
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// メインゲーム処理分け
		switch (eState) {
		case INIT:
			// 初期化
			if (!Init(hinst)) {
				// 失敗
				eState = END;
			}
			else {
				// 成功
				eState = RUN;
			}
			break;

		case RUN:
			switch (RunRoulette()) {
			case 0:
				eState = RUN;
				break;
			case -1:
				eState = END;
				break;
			}
			break;

		case END:
			// 終了処理
			Clear();
			bLoop = FALSE;
			break;

		default:
			// 未定義のステート
			DEBUG("異常終了\n");
			return FALSE;
		}
		Sleep(10);
	}

	win.Delete();

	// プログラム正常終了
	return TRUE;
}
