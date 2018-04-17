#include <Windows.h>
#include "CGame.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// main関数のWindows版
///////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain( HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdParam,int nCmdShow )
{

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
