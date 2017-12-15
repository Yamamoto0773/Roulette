#ifndef	__CWINDOW_H
#define __CWINDOW_H
///////////////////////////////////////////////////////////////////////////////////
// CWindow : ウィンドウ管理 v5.01(UNICODE)                                       //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>

#pragma comment(lib,"imm32.lib")


///////////////////////////////////////////////////////////////////////////////////
// コールバック関数定義
///////////////////////////////////////////////////////////////////////////////////
typedef LRESULT (CALLBACK *ONCOMMAND)( HWND hWnd,WPARAM wParam,LPARAM lParam );
typedef struct _ONMSG {
	UINT		uiMsg;									// メッセージ番号
	ONCOMMAND	cmdProc;								// コールバックアドレス
} ONMSG,*LPONMSG;


///////////////////////////////////////////////////////////////////////////////////
// ウインドウクラス
///////////////////////////////////////////////////////////////////////////////////
class CWindow {

public:
	// 使いやすいように外へ出す
	static HINSTANCE	hInstance;						// メインウインドウのインスタンス
	static HWND			hWnd;							// メインウインドウのハンドル
	static BOOL			bActive;						// アクティブか

protected:
	// ウインドウ関係
	static WCHAR		mName[256];						// アプリケーション名(クラス名としても使用)
	static const WCHAR	*cIconID;						// アイコンの種類
	static HMENU		hMenu;							// メニューハンドル
	static DWORD		dwStyle;						// ウインドウスタイル
	static DWORD		dwExStyle;						// 拡張スタイル
	static LPONMSG		mMsg;							// 追加メッセージデータ
	static int			iMsg;							// 追加メッセージの数

private:
	// ウインドウメッセージ処理コールバック
	static LRESULT CALLBACK WindowProc( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam );

public:
	CWindow();
	virtual ~CWindow();
	void SetIcon( const WCHAR *icon );																				// アイコンの変更(Createの前に必要）
	BOOL SetWindowStyle( DWORD style );																				// ウインドウのスタイルの変更(動的に変更も可能)
	void SetMenu( HMENU menu );
	BOOL Create( HINSTANCE hInst,const WCHAR *appName,BOOL show=TRUE,DWORD w=640,DWORD h=480,HWND parent=NULL );	// 内部で管理するウインドウを生成する
	void Delete( void );																							// 明示的にウィンドウを削除する

	BOOL AddMsgProc( UINT msg,ONCOMMAND proc );																		// 特殊メッセージの追加
	void ShowCursor( BOOL bShow );																					// マウスカーソルの表示、非表示
	void ShowWindow( BOOL bShow );																					// ウインドウの表示、非表示
	void Move( int x,int y );																						// ウインドウの移動
	void Move( int x,int y,int w,int h );																			// ウインドウの移動(幅と高さも同時に変更)

	BOOL MoveClientWindowCenter( int w,int h );																		// 指定サイズがクライアント領域になるようにウインドウを配置
	BOOL SetMenuItem( int menuid,BOOL check=FALSE,BOOL gray=FALSE );												// メニューアイテムの状態変更

	BOOL TextOutW( int x,int y,const WCHAR *str,COLORREF col=0 );													// 現在のフォントでテキスト表示
};

#endif
