#pragma warning( disable : 4995 )				// 警告を無視
#pragma warning( disable : 4996 )				// 警告を無視
#include "CGame.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>


#include "./libfiles/CTimer.h"

/////////////////////////////////////////////////////////////////////////
// コンストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::CGame() {
	eState			= INIT;
	iRouletteState	= 0b0000;
	iWiningNum		= 0;

	ZeroMemory(&bOnKey, sizeof(bOnKey));
}


/////////////////////////////////////////////////////////////////////////
// デストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::~CGame() {

}


///////////////////////////////////////////////////////
// 初期化＆ゲーム形成
///////////////////////////////////////////////////////
BOOL CGame::Init(HINSTANCE hinst) {
	const int windowW = 960, windowH = 720;

	log.init("DEBUG.txt");


	// ウインドウ生成
	win.SetWindowStyle(WS_OVERLAPPEDWINDOW);					// 枠無しウインドウ(フルスクリーン時はWS_POPUPのみ、ウィンドウモード時はさらにWS_CAPTION|WS_SYSMENUなどを付ける)
	if (!win.Create(hinst, L"Roulette", 1, windowW, windowH)) {	// ウィンドウサイズは720p
		log.tlnwrite("Window create error\n");
		return FALSE;
	}
	ImmAssociateContext(win.hWnd, NULL);			// IMEを出さないようにする

	dx9::DXDrawManager::SetLogWriteDest(&log);

	// Direct3D生成
	if (!dim.CreateWind(win.hWnd, windowW, windowH)) {
		log.tlnwrite("Failed to Create Direct3D9 Resources");
	}


	// DirectInput生成
	if (!di.Create(win.hWnd, win.hInstance)) {
		log.tlnwrite("DirectInput生成失敗\n");
		return FALSE;
	}

	// キーボードを使う
	if (!di.CreateKeyboard()) {
		log.tlnwrite("キーボード使用不可\n");
		return FALSE;
	}

	if (!df.Create()) {
		return FALSE;
	}


	// DirectXText生成
	if (!dt.Create("Century Gothic", 360, dx9::FontWeight::SEMIBOLD) ||
		!dtsmall.Create("Century Gothic", 20, dx9::FontWeight::NORMAL)) {
		log.tlnwrite("DirectXText生成失敗\n");
		return FALSE;
	}

	ef = new EffectManager(&dim, (unsigned)windowW, (unsigned)windowH);


	// 画像ファイル読み込み
	wchar_t *filename[] = {
		L"resource/background.jpg",
		L"resource/effect1.png",
		L"resource/effect2.png",
		L"resource/effect3.png",
		L"resource/effect4.png"
	};

	for (int i=0; i< (1+TEXTURECUNT); i++) {
		if (!dim.AddTexture((unsigned)i, filename[i]))
			return false;
	}



	// 背景画像の縮尺設定
	dx9::Size bgSize = dim.GetTexSize(0);
	if (bgSize.w/bgSize.h < windowW/windowH) {
		bgScale = (float)windowW/(float)bgSize.w;
	}
	else {
		bgScale = (float)windowH/(float)bgSize.h;
	}



	dim.SetBackGroundColor(0xffffff);



	// くじ定義ファイル読み込み
	if (!lottery.registerLottery(L"DEFINE/Lottery.txt")) {
		log.tlnwrite("くじの登録に失敗しました\n");
		return FALSE;
	}

	FILE *fp;
	fp = fopen("DEFINE/magnification.txt", "r");
	if (fp == NULL) {
		log.tlnwrite("\"DEFINE/magnification.txt\"を開けませんでした");
		return FALSE;
	}

	char tmp[128];
	int i;
	int grade, mag;
	while (fgets(tmp, 128, fp) != NULL) {
		i=0, grade=0, mag=0;

		while (tmp[i] >= '0' && tmp[i] <= '9') {
			grade *= 10;
			grade += tmp[i]-'0';
			i++;
		}

		if (grade < 1 || grade > 10) {
			log.tlnwrite("範囲外のグループの確率を指定しました");
			return FALSE;
		}

		while (tmp[i] == ' ' || tmp[i] == ',' || tmp[i] == '\t') i++;	// 区切り文字は飛ばす

		while (tmp[i] >= '0' && tmp[i] <= '9') {
			mag *= 10;
			mag += tmp[i]-'0';
			i++;
		}

		// 倍率の登録をおこなう
		lottery.setMagnification(grade, mag);

	}


	// くじの定義を出力
	log.lnwrite("\n -- Lottery define --");
	log.lnwrite("group  number");

	auto lot = lottery.getLottery();

	for (size_t group=0; group<lot.size(); group++) {
		for (auto num : lot[group]) {
			log.lnwrite("%5d  %6d", group+1, num);
		}
	}

	log.lnwrite("\n -- Magnification define --");
	log.lnwrite("group  mag");

	auto lotMag = lottery.getMagnification();

	i=1;
	for (auto m : lotMag) {
		if (m == 0)
			log.lnwrite("%5d  <undef>", i);
		else
			log.lnwrite("%5d  %3d", i, m);

		i++;
	}

	log.lnwrite("");




	ShowWindow(win.hWnd, SW_SHOW);


	return TRUE;
}



///////////////////////////////////////////////////////
// ルーレット実行関数
///////////////////////////////////////////////////////
BOOL CGame::RunRoulette() {
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// ESCキーで終了
	if (key[DIK_ESCAPE]&0x80)
		return -1;

	// 仮想入力ハードウェア
	BOOL press[KEYCNT];						// 押した瞬間にtrueになる配列
	ZeroMemory(&press, sizeof(press));

	for (int i=0; i<KEYCNT; i++) {
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

	// ルーレットスタート
	if (press[5]) {
		if (iRouletteState == 0b0000) {
			std::vector<size_t> v;

			for (size_t i=1; i<=10; i++) {
				if (bOnKey[5+i]) {
					v.push_back(i);
				}
			}

			if (SetNumber(v)) {
				iRouletteState |= 0b0111;
			}

		}
	}


	// ルーレット停止
	if (press[1]) {
		iRouletteState &= 0b0111;
	}
	if (press[2]) {
		iRouletteState &= 0b1011;
	}
	if (press[3]) {
		iRouletteState &= 0b1101;
	}
	if (press[4]) {
		iRouletteState &= 0b1110;
	}



	ef->Update();


	////////////////////////////////////////////////////////////////////////////////////
	// 描画処理
	////////////////////////////////////////////////////////////////////////////////////
	dim.ClearBackGround();
	dim.DrawBegin();


	dim.Draw(0, 960/2, 720/2, dx9::DrawTexCoord::CENTER, 1.0f, bgScale, bgScale);	// 背景

	dtsmall.Draw(750, 700, 0x7fffffff, "(c)2018, Nanami Yamamoto");	// 署名

	ef->Draw();	// エフェクト

	dim.SetBlendMode(dx9::BLENDMODE::NORMAL);

	df.DrawRect(180, 200, 600, 260, 0x64ffffff);	// ボックス

	// 当選番号の描画
	int width = 180;
	int textX = 580;
	int textY = 150;
	int num = iWiningNum;
	DWORD color = 0xB0EC1140;

	for (int i=0; i<3; i++) {
		if ((iRouletteState >> i)&0b0001) {
			// ルーレット回転中
			dt.Draw(textX-width*i, textY, color, "%d", rand()%10);
		}
		else {
			// ルーレット停止中
			if (iWiningNum == 0) {
				// くじがなくなったとき
				dt.Draw(textX-width*i, textY, color, "-");
			}
			else {
				dt.Draw(textX-width*i, textY, color, "%d", num%10);
			}
		}
		num/=10;
	}



	dim.DrawEnd();

	// 継続
	return 0;
}


BOOL CGame::SetNumber(std::vector<size_t> &group) {
	// 当選番号取得
	bool res;

	if (group.size() == 0) {
		res = lottery.getNumber(iWiningNum);
	}
	else {
		res = lottery.getNumber(iWiningNum, group);
	}

	if (!res) {
		return false;
	}


	if (group.size() > 0) {
		std::stringstream str;
		str << "No." << iWiningNum << "  group > ";

		for (size_t g : group) {
			str << g << ", ";
		}
		
		log.tlnwrite("%s", str.str().c_str());
	}
	else {
		log.tlnwrite("No.%d", iWiningNum);
	}

	return true;
}




/////////////////////////////////////////////////////////////////////////
// ゲームメインルーチン
/////////////////////////////////////////////////////////////////////////
BOOL CGame::Run(HINSTANCE hinst) {
	// ゲームメインループ
	MSG msg;
	BOOL bLoop=TRUE;

	CTimer timer;
	timer.Start(60);	// 60fpsで実行

	while (bLoop) {

		int frame = timer.Run();
		for (int i=0; i<frame; i++) {

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message==WM_QUIT) {
					bLoop = FALSE;
					log.tlnwrite("WM_QUIT\n");
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
					bLoop = FALSE;
					break;

				default:
					// 未定義のステート
					log.tlnwrite("異常終了\n");
					return FALSE;
			}



		}

	}

	win.Delete();

	// プログラム正常終了
	return TRUE;
}
