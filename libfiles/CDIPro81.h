#ifndef _CDIPRO81_H
#define _CDIPRO81_H
///////////////////////////////////////////////////////////////////////////////////
// CDIPro81 : DirectInput8管理クラス for おとゲマ v2.01                          //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>

#define DIRECTINPUT_VERSION	0x0800								// DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define JOYSTICK_DEPTH		1000								// アナログジョイスティックの稼動範囲(±値)

// ジョイスティック情報
typedef struct _CDIJOYSTICK {
	LPDIRECTINPUTDEVICE8	lpJoyDev;							// ジョイスティックデバイス
	WCHAR					mInstName[MAX_PATH];				// インスタンス名
	WCHAR					mProdName[MAX_PATH];				// 製品名
} CDIJOYSTICK,*LPCDIJOYSTICK;


class CDIPro81 {
protected:
	HINSTANCE				hInst;
	HWND					hWnd;
	LPDIRECTINPUT8			lpDI;								// DirectInputオブジェクト
	LPDIRECTINPUTDEVICE8	lpMouse;							// マウスデバイス
	LPDIRECTINPUTDEVICE8	lpKeyboard;							// キーボードデバイス
	int						iJoy;								// ジョイスティックの数
	LPCDIJOYSTICK			mJoy;								// ジョイスティック情報
private:
	static BOOL CALLBACK EnumJoyDeviceProc( LPDIDEVICEINSTANCE lpddi,LPVOID pvRef );

public:
	CDIPro81();
	virtual ~CDIPro81();
	BOOL Create( HWND hwnd,HINSTANCE hinst );					// 構築
	BOOL Delete( void );										// 終了

	BOOL CreateKeyboard( void );								// キーボード初期化
	BOOL GetKeyboard( LPBYTE key );								// キーボードの状態を返す（CreateKeyboardされていない場合はエミュレーション）

	BOOL CreateMouse( void );									// マウス初期化
	BOOL GetMouse( LPDIMOUSESTATE mouse );						// マウスの状態を取得

	BOOL CreateJoystick( void );								// 全てのジョイスティックを初期化
	BOOL GetJoystick( LPDIJOYSTATE2 js );						// 全てのジョイスティック情報を返す(ジョイスティック数分のDIJOYSTATE2を指定)
	BOOL GetJoystick( LPDIJOYSTATE2 js,int id );				// ジョイスティックの情報を1つ取得(1つ分のDIJOYSTATE2を指定)

public:
	const WCHAR *GetJoyInstName( int id );						// 指定IDのインスタンス名を返す
	const WCHAR *GetJoyProdName( int id );						// 指定IDの製品名を返す
	inline int GetJoyNum( void ) { return iJoy; }				// 接続されているジョイスティックの数を返す
};

typedef CDIPro81	CDIPro,CDIPRO,*LPCDIPRO;

#endif
