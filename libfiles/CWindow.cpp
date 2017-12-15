#pragma warning( disable : 4996 )
#include "CWindow.h"

///////////////////////////////////////////////////////////////////////////////////
// スタティック変数
///////////////////////////////////////////////////////////////////////////////////
HINSTANCE			CWindow::hInstance		= NULL;
HWND				CWindow::hWnd			= NULL;
BOOL				CWindow::bActive		= TRUE;

WCHAR				CWindow::mName[256]		= L"";
const WCHAR*		CWindow::cIconID		= IDI_APPLICATION;		// デフォルトのアイコン
HMENU				CWindow::hMenu			= NULL;
DWORD				CWindow::dwStyle		= WS_POPUP|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX;
DWORD				CWindow::dwExStyle		= 0;

LPONMSG				CWindow::mMsg			= NULL;
int					CWindow::iMsg			= 0;


///////////////////////////////////////////////////////////////////////////////////
// コンストラクタ
///////////////////////////////////////////////////////////////////////////////////
CWindow::CWindow(void)
{
}

///////////////////////////////////////////////////////////////////////////////////
// デストラクタ
///////////////////////////////////////////////////////////////////////////////////
CWindow::~CWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////
// メインウインドウのイベントハンドラ
///////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CWindow *win = NULL;

	switch( uMsg )
	{
	case WM_CREATE:
		win = (CWindow*)lParam;
		break;
	case WM_ACTIVATE:
		bActive = LOWORD(wParam)?TRUE:FALSE;				// アクティブ状態変更
		break;
	case WM_DESTROY:										// ALT+F4が押されたら
		PostQuitMessage( 0 );
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_SYSCOMMAND:
		switch( wParam )
		{
		case SC_CLOSE:
			PostQuitMessage(0);
			return 0;
		}
		break;
	case WM_IME_NOTIFY:
		switch( wParam )
		{
		case IMN_SETOPENSTATUS:
			HIMC hImc = ImmGetContext( hWnd );
            ImmSetOpenStatus( hImc,FALSE );
			break;
		}
		break;
    }

	// 特殊メッセージ処理
	int i;
	for( i=0;i<iMsg;i++ ) {
		if( uMsg==mMsg[i].uiMsg ) {
			return mMsg[i].cmdProc( hWnd,wParam,lParam );	// 特殊メッセージ操作完了なら
		}
	}

	return DefWindowProc( hWnd,uMsg,wParam,lParam );		// デフォルトを返す
}

///////////////////////////////////////////////////////////////////////////////////
// ウインドウを生成する
///////////////////////////////////////////////////////////////////////////////////
BOOL CWindow::Create( HINSTANCE hInst,const WCHAR *appName,BOOL show,DWORD w,DWORD h,HWND parent )
{
	WNDCLASS wc;
	DEVMODE dmMode;

	// 画面解像度をチェック
	EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dmMode);
	// 16bit以上の解像度じゃないと起動できない
	if( dmMode.dmBitsPerPel<16 ) {
		MessageBoxW( GetDesktopWindow(),L"16Bit以上の解像度にしてください",L"起動できません",MB_OK );
		return FALSE;
	}

	// セット
	hInstance = hInst;
	wcscpy( mName,appName );

	// ウインドウクラス登録
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
    wc.lpfnWndProc		= WindowProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= sizeof(DWORD);
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, cIconID );
    wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName		= MAKEINTRESOURCE( hMenu );
    wc.lpszClassName	= mName;
    if ( !RegisterClass(&wc) )
        return FALSE;

	// ウインドウ生成
    hWnd = CreateWindowExW(
		dwExStyle,
		wc.lpszClassName,		// Class
		mName,					// Title bar
		dwStyle,				// Style
		GetSystemMetrics(SM_CXSCREEN)/2-w/2,
		GetSystemMetrics(SM_CYSCREEN)/2-h/2,
		w,						// Init. x pos
		h,						// Init. y pos
		parent,					// Parent window
		NULL,					// Menu handle
		hInstance,				// Program handle
		this					// Create parms
	);
    if( !hWnd )
        return FALSE;			// 生成に失敗

	// フォントの設定
	HDC hdc = GetDC( hWnd );
	if( hdc ) {
		SetBkMode( hdc,TRANSPARENT );
		ReleaseDC( hWnd,hdc );
	}

	MoveClientWindowCenter( w,h );
	// ウインドウを表示
	if( show )
		::ShowWindow( hWnd,SW_SHOW );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// 明示的にウィンドウを削除する
///////////////////////////////////////////////////////////////////////////////////
void CWindow::Delete( void )
{
	if( hWnd ) {
		// 通常のウィンドウなら
		::DestroyWindow( hWnd );
		// 登録したクラス名を解除
		UnregisterClassW( mName,hInstance );
		ZeroMemory( &mName,sizeof(mName) );
		hWnd = NULL;
		hInstance = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// カーソルの表示・非表示
///////////////////////////////////////////////////////////////////////////////////
void CWindow::ShowCursor( BOOL bShow )
{
	if( bShow )
		while(::ShowCursor(TRUE)<0);
	else
		while(::ShowCursor(FALSE)>=0);
}

///////////////////////////////////////////////////////////////////////////////////
// ウインドウの表示・非表示
///////////////////////////////////////////////////////////////////////////////////
void CWindow::ShowWindow( BOOL bShow )
{
	// ウインドウの表示
	if( bShow )
		::ShowWindow( hWnd,SW_SHOW );
	else
		::ShowWindow( hWnd,SW_HIDE );
}

///////////////////////////////////////////////////////////////////////////////////
// アプリケーションのアイコンの変更
///////////////////////////////////////////////////////////////////////////////////
void CWindow::SetIcon( const WCHAR *icon )
{
	cIconID = icon;
}


// 特殊メッセージの追加
BOOL CWindow::AddMsgProc( UINT msg,ONCOMMAND proc )
{
	int i;
	// 既に存在していないかチェック
	for( i=0;i<iMsg;i++ ) {
		if( mMsg[i].uiMsg==msg ) {
			// あれば新しいアドレスに更新
			mMsg[i].cmdProc = proc;
			return TRUE;
		}
	}

	// 追加
	iMsg++;
	mMsg = (LPONMSG)realloc( mMsg,sizeof(ONMSG)*iMsg );
	ZeroMemory( &mMsg[iMsg-1],sizeof(ONMSG) );
	mMsg[iMsg-1].uiMsg = msg;
	mMsg[iMsg-1].cmdProc = proc;
	return TRUE;
}

// ウインドウスタイルの変更(動的に変更も可能)
BOOL CWindow::SetWindowStyle( DWORD style )
{
	dwStyle = style;
	if( hWnd ) {
		// すでにウインドウが存在する場合は即反映
		::SetWindowLong( hWnd,GWL_STYLE,style );
		::SetWindowPos( hWnd,0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER );
	}
	return TRUE;
}

// ウインドウの移動
void CWindow::Move( int x,int y )
{
	SetWindowPos( hWnd,0,x,y,0,0,SWP_NOSIZE|SWP_NOZORDER );
}

// ウインドウの移動(幅と高さも同時に変更)
void CWindow::Move( int x,int y,int w,int h )
{
	MoveWindow( hWnd,x,y,w,h,TRUE );
}

// 指定サイズがクライアント領域になるようにウインドウを配置
BOOL CWindow::MoveClientWindowCenter( int w,int h )
{
	RECT Win,Cli;
	GetWindowRect( hWnd,&Win );								// ウインドウの左上を取得
	GetClientRect( hWnd,&Cli );								// ウインドウ内のクライアント座標を取得
	int frame_w = (Win.right - Win.left) - Cli.right;		// フレームの幅
	int frame_h = (Win.bottom - Win.top) - Cli.bottom;		// フレームの高さ
	int scr_w	= GetSystemMetrics( SM_CXSCREEN );			// スクリーンの幅
	int scr_h	= GetSystemMetrics( SM_CYSCREEN );			// スクリーンの高さ
	SetWindowPos( hWnd,0,( scr_w - (frame_w/2+w) ) / 2,( scr_h - (frame_h/2+h) ) / 2,w+frame_w,h+frame_h,SWP_NOZORDER );

	return TRUE;
}

// メニューアイテムの変更
BOOL CWindow::SetMenuItem(int menuid,BOOL check,BOOL gray )
{
	HMENU menu = GetMenu( hWnd );
	if( menu ) {
		MENUITEMINFO miinfo;
		ZeroMemory( &miinfo,sizeof(miinfo) );
		miinfo.cbSize = sizeof(miinfo);
		miinfo.fMask = MIIM_STATE;
		if( check )
			miinfo.fState |= MFS_CHECKED;
		else
			miinfo.fState |= MFS_UNCHECKED;
		if( gray )
			miinfo.fState |= MFS_GRAYED;
		else
			miinfo.fState |= MFS_ENABLED;
		return SetMenuItemInfo( menu,menuid,FALSE,&miinfo );
	}
	return TRUE;
}

BOOL CWindow::TextOutW( int x,int y,const WCHAR *str,COLORREF col )
{
	HDC hdc = GetDC( hWnd );
	if( hdc ) {
		SetTextColor( hdc,col );
		::TextOutW( hdc,x,y,str,(int)wcslen(str) );
		ReleaseDC( hWnd,hdc );
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// メニューバーの変更(Createの前に必要）
///////////////////////////////////////////////////////////////////////////////////
void CWindow::SetMenu( HMENU menu )
{
	hMenu = menu;
}
