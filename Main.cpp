#include <Windows.h>
#include "CGame.h"

#define DEBUGMODE
#include "libfiles/DEBUG.H"

///////////////////////////////////////////////////////////////////////////////////////////////////
// main関数のWindows版
///////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain( HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdParam,int nCmdShow )
{
	// デバッグライブラリ初期化
	INITDEBUG();
	CLEARDEBUG();

	// COMの初期化
	// ※DirectSoundが内部で使用しているので、呼び出しておかないとVC上でワーニングが出る
	CoInitialize( NULL );

	// ゲームメイン
	CGame *game = new CGame();			// newでないとオーバーフローする
	game->Run( hInstance );
	delete game;

	// COMの終了
	CoUninitialize();

	return 0;
}
