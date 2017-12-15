#pragma warning( disable : 4996 )
#include "CDIPro81.h"

#define SAFE_RELEASE(x)		{ if(x) { x->Release(); x=NULL; } }
#define SAFE_FREE(x)		{ if(x) { free(x); x=NULL; } }

#define DEBUGMODE
#include "DEBUG.H"

/////////////////////////////////////////////////////////////////////////
// コンストラクタ
/////////////////////////////////////////////////////////////////////////
CDIPro81::CDIPro81(void)
{
	hInst			= NULL;
	hWnd			= NULL;
	lpDI			= NULL;
	lpMouse			= NULL;
	lpKeyboard		= NULL;
	iJoy			= 0;
	mJoy			= NULL;
}

/////////////////////////////////////////////////////////////////////////
// デストラクタ
/////////////////////////////////////////////////////////////////////////
CDIPro81::~CDIPro81()
{
	Delete();
}

/////////////////////////////////////////////////////////////////////////
// ジョイスティック用コールバック
/////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CDIPro81::EnumJoyDeviceProc( LPDIDEVICEINSTANCE lpddi, LPVOID pvRef )
{
	CDIPro81 *p = (CDIPro81*)pvRef;

	// デバイス追加
	p->iJoy++;
	p->mJoy = (LPCDIJOYSTICK)realloc( p->mJoy,sizeof(CDIJOYSTICK)*p->iJoy );						// CDIJOYSTICKの配列を確保
	ZeroMemory( &p->mJoy[p->iJoy-1],sizeof(CDIJOYSTICK) );											// 初期化
	wcscpy( p->mJoy[p->iJoy-1].mInstName,lpddi->tszInstanceName );								// インスタンス名コピー
	wcscpy( p->mJoy[p->iJoy-1].mProdName,lpddi->tszProductName );								// 製品名コピー
//	DEBUGW( L"インスタンス名 [%s]\n",p->mJoy[p->iJoy-1].mInstName );
//	DEBUGW( L"製品名         [%s]\n",p->mJoy[p->iJoy-1].mProdName );

	// デバイス構築
	HRESULT ret = p->lpDI->CreateDevice(lpddi->guidInstance,&p->mJoy[p->iJoy-1].lpJoyDev,NULL );
	if( FAILED(ret) ) {
		// 失敗なら次のデバイスへ
		DEBUG( "デバイス構築失敗\n" );
		return DIENUM_CONTINUE;
	}

	// フォーマットのセット
	ret = p->mJoy[p->iJoy-1].lpJoyDev->SetDataFormat( &c_dfDIJoystick2 );
	if( FAILED(ret) ) {
		// 失敗なら次のデバイスへ
		DEBUG( "フォーマット設定失敗\n" );
		return DIENUM_CONTINUE;
	}

	// 入力範囲の設定
	DIPROPRANGE	diprg;
	diprg.diph.dwSize		= sizeof(diprg);
	diprg.diph.dwHeaderSize	= sizeof(diprg.diph);
	diprg.diph.dwHow		= DIPH_BYOFFSET;
	diprg.lMax				=  JOYSTICK_DEPTH;
	diprg.lMin				= -JOYSTICK_DEPTH;
	diprg.diph.dwObj		= DIJOFS_X;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );
	diprg.diph.dwObj		= DIJOFS_Y;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );
	diprg.diph.dwObj		= DIJOFS_Z;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );
	diprg.diph.dwObj		= DIJOFS_RX;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );
	diprg.diph.dwObj		= DIJOFS_RY;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );
	diprg.diph.dwObj		= DIJOFS_RZ;
	p->mJoy[p->iJoy-1].lpJoyDev->SetProperty( DIPROP_RANGE,&diprg.diph );

	// 起動準備
	p->mJoy[p->iJoy-1].lpJoyDev->Poll();
	p->mJoy[p->iJoy-1].lpJoyDev->Acquire();

	return DIENUM_CONTINUE;
}

/////////////////////////////////////////////////////////////////////////
// DirectInputオブジェクトの生成
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::Create( HWND hwnd,HINSTANCE hinst )
{
	hWnd = hwnd;
	hInst = hinst;
	HRESULT ret = DirectInput8Create( hInst,DIRECTINPUT_VERSION ,IID_IDirectInput8,(void**)&lpDI,NULL );
	if( FAILED(ret) ) {
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 明示的開放
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::Delete( void )
{
	int i;
	for( i=0;i<iJoy;i++ ) {
		if( mJoy[i].lpJoyDev ) {
			mJoy[i].lpJoyDev->Unacquire();	// 動作停止
			mJoy[i].lpJoyDev->Release();
			mJoy[i].lpJoyDev = NULL;
		}
	}
	SAFE_FREE( mJoy );
	iJoy = 0;

	if( lpKeyboard )
		lpKeyboard->Unacquire();	// 動作停止
	SAFE_RELEASE( lpKeyboard );

	if( lpMouse )
		lpMouse->Unacquire();		// 動作停止
	SAFE_RELEASE( lpMouse );

	SAFE_RELEASE( lpDI );

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// キーボードオブジェクトを生成しマウントする
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::CreateKeyboard( void )
{
	if( !lpDI )
		return TRUE;		// エミュレーションモードで動作

	HRESULT ret = lpDI->CreateDevice( GUID_SysKeyboard,&lpKeyboard,NULL );
	if( FAILED(ret) ) {
		DEBUG( "キーボードデバイスの生成に失敗\n" );
		return FALSE;
	}
	ret = lpKeyboard->SetDataFormat( &c_dfDIKeyboard );
	if( FAILED(ret) ) {
		DEBUG( "データフォーマットの設定に失敗\n" );
		return FALSE;
	}

	// 協調モードの設定
	ret = lpKeyboard->SetCooperativeLevel( hWnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY );
	if( FAILED(ret) ) {
		DEBUG( "協調モードの設定に失敗\n" );
		return FALSE;
	}

	// 動作開始
	lpKeyboard->Acquire();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// キーボードの情報を取得
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::GetKeyboard( LPBYTE key )
{
	ZeroMemory( key,256 );

	if( lpKeyboard ) {
		// デバイスが構築されている場合
		HRESULT ret = lpKeyboard->GetDeviceState( 256,key );
		if( FAILED(ret) ) {
			lpKeyboard->Acquire();
			ret = lpKeyboard->GetDeviceState( 256,key );
			if( FAILED(ret) ) {
//				DEBUG( "入力エラー %08X\n",ret );
				return FALSE;
			}
		}
	} else {
		// エミュレーション
		BYTE tmp[256];
		GetKeyboardState( tmp );
		static const BYTE vk[256] = {
			NULL,VK_ESCAPE,'1','2','3','4','5','6','7','8','9','0',0xbd,NULL,VK_BACK,VK_TAB,
			'Q','W','E','R','T','Y','U','I','O','P',NULL,NULL,VK_RETURN,VK_LCONTROL,'A','S',
			'D','F','G','H','J','K','L',VK_OEM_PLUS,NULL,NULL,VK_LSHIFT,VK_OEM_102,'Z','X','C','V',
			'B','N','M',VK_OEM_COMMA,VK_OEM_PERIOD,VK_OEM_2,VK_RSHIFT,VK_MULTIPLY,VK_LMENU,VK_SPACE,NULL,VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,
			VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,VK_NUMLOCK,VK_SCROLL,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9,VK_SUBTRACT,VK_NUMPAD4,VK_NUMPAD5,VK_NUMPAD6,VK_ADD,VK_NUMPAD1,
			VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD0,VK_DECIMAL,NULL,NULL,NULL,VK_F11,VK_F12,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,VK_F13,VK_F14,VK_F15,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,VK_CONVERT,NULL,VK_NONCONVERT,NULL,VK_OEM_5,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,VK_OEM_1,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,VK_RETURN,VK_RCONTROL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,VK_OEM_COMMA,NULL,VK_DIVIDE,NULL,NULL,VK_RMENU,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,VK_UP,VK_PRIOR,NULL,VK_LEFT,NULL,VK_RIGHT,NULL,VK_END,
			VK_DOWN,VK_NEXT,VK_INSERT,VK_DELETE,NULL,NULL,NULL,NULL,NULL,NULL,NULL,VK_LWIN,VK_RWIN,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		};

		// VK→DIK
		int i;
		for( i=0;i<256;i++ ) {
			key[i] = tmp[vk[i]];
		}
	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// マウスオブジェクトを生成（相対モード）
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::CreateMouse( void )
{
	if( !lpDI )
		return FALSE;

	HRESULT ret = lpDI->CreateDevice( GUID_SysMouse,&lpMouse,NULL );
	if( FAILED(ret) ) {
		DEBUG( "マウスデバイスの生成に失敗\n" );
		return FALSE;
	}
	ret = lpMouse->SetDataFormat( &c_dfDIMouse );
	if( FAILED(ret) ) {
		DEBUG( "データフォーマットの設定に失敗\n" );
		return FALSE;
	}

	// 動作開始
	lpMouse->Acquire();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// マウスの情報を取得
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::GetMouse( LPDIMOUSESTATE mouse )
{
	ZeroMemory( mouse,sizeof(DIMOUSESTATE) );
	if( !lpMouse )
		return FALSE;

	HRESULT ret = lpMouse->GetDeviceState( sizeof(DIMOUSESTATE),mouse );
	if( FAILED(ret) ) {
		lpMouse->Acquire();
		ret = lpMouse->GetDeviceState( sizeof(DIMOUSESTATE),mouse );
		if( FAILED(ret) )
			return FALSE;
	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// 全てのジョイスティックを生成しマウントする
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::CreateJoystick( void )
{
	if( !lpDI )
		return FALSE;

	HRESULT ret;
	ret = lpDI->EnumDevices( DI8DEVCLASS_GAMECTRL,(LPDIENUMDEVICESCALLBACK)EnumJoyDeviceProc,this,DIEDFL_ATTACHEDONLY );
	if( FAILED(ret) )
		return FALSE;

	// ジョイスティックが見つからない場合
	if( iJoy<1 )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 全てのJoyデータ配列を返す
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::GetJoystick( LPDIJOYSTATE2 js )
{
	if( !lpDI )
		return FALSE;
	ZeroMemory( js,sizeof(DIJOYSTATE2)*iJoy );

	int i;
	for( i=0;i<iJoy;i++ ) {
		GetJoystick( &js[i],i );
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 1つのJoystickデータを取得
/////////////////////////////////////////////////////////////////////////
BOOL CDIPro81::GetJoystick( LPDIJOYSTATE2 js,int id )
{
	if( !lpDI )
		return FALSE;

	// 初期化
	ZeroMemory( js,sizeof(DIJOYSTATE2) );
	js->rgdwPOV[0] = js->rgdwPOV[1] = js->rgdwPOV[2] = js->rgdwPOV[3] = 0xFFFFFFFF;

	if( id<0 || id>iJoy-1 )
		return FALSE;

	mJoy[id].lpJoyDev->Poll();
	HRESULT ret = mJoy[id].lpJoyDev->GetDeviceState( sizeof(DIJOYSTATE2),js );
	if( FAILED(ret) ) {
		mJoy[id].lpJoyDev->Acquire();
		mJoy[id].lpJoyDev->Poll();
		ret = mJoy[id].lpJoyDev->GetDeviceState( sizeof(DIJOYSTATE2),js );
		if( FAILED(ret) )
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 指定IDのインスタンス名を返す
/////////////////////////////////////////////////////////////////////////
const WCHAR *CDIPro81::GetJoyInstName( int id )
{
	if( id<0 || id>iJoy-1 ) {
		DEBUG( "JoystickのIDが無効 [%d]\n",id );
		return NULL;
	}
	return mJoy[id].mInstName;
}

/////////////////////////////////////////////////////////////////////////
// 指定IDの製品名を返す
/////////////////////////////////////////////////////////////////////////
const WCHAR *CDIPro81::GetJoyProdName( int id )
{
	if( id<0 || id>iJoy-1 ) {
		DEBUG( "JoystickのIDが無効 [%d]\n",id );
		return NULL;
	}
	return mJoy[id].mProdName;
}
