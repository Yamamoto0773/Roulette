#pragma warning( disable : 4996 )
#include "CDDTexPro90.h"

#if USEPLUGIN
#include "CSusie.h"
#endif

#define DEBUGMODE
#include "DEBUG.H"

#define SAFE_RELEASE(x)			{ if(x) { x->Release(); x=NULL; } }
#define LINESIZE(width,bit)		((( (width) * (bit) )+31)/32*4)					// 横幅のバイトサイズを求める

/////////////////////////////////////////////////////////////////////////
// コンストラクタ
/////////////////////////////////////////////////////////////////////////
CDDTexPro90::CDDTexPro90()
{
	lpTex = NULL;
	dwWidth = 0;
	dwHeight = 0;
	bLock = FALSE;
#if USEPLUGIN
	strcpy( mPlgPath,SPIDEFPLGPATH );
	strcpy( mExt,SPIDEFPLGEXT );
#else
	ZeroMemory( mPlgPath,sizeof(mPlgPath) );
	ZeroMemory( mExt,sizeof(mExt) );
#endif
}

/////////////////////////////////////////////////////////////////////////
// デストラクタ
/////////////////////////////////////////////////////////////////////////
CDDTexPro90::~CDDTexPro90()
{
	Delete();
}

/////////////////////////////////////////////////////////////////////////0
// プラグインDLLの検索パスの変更
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::SetPluginPath( const char *path )
{
	strcpy( mPlgPath,path );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// プラグインの拡張子の変更
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::SetExt( const char *ext )
{
	if( !ext )
		strcpy( mExt,"spi" );
	else
		strcpy( mExt,ext );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 画像ファイルからテクスチャを作る
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Create( LPDIRECT3DDEVICE9 dev,const char *file )
{

	Delete();

	// 対応画像か
	if( strnicmp(&file[strlen(file)-4],".BMP",4)==0 ||
		strnicmp(&file[strlen(file)-4],".TGA",4)==0 ||
		strnicmp(&file[strlen(file)-4],".DDS",4)==0 ||
		strnicmp(&file[strlen(file)-4],".DIB",4)==0 ||
		strnicmp(&file[strlen(file)-4],".JPG",4)==0 ||
		strnicmp(&file[strlen(file)-4],".PNG",4)==0 ) {

		D3DXIMAGE_INFO imgInfo;
		ZeroMemory( &imgInfo,sizeof(D3DXIMAGE_INFO) );
		HRESULT ret = D3DXCreateTextureFromFileExA(
				dev,
				file,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				1,
				0,
				D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED,
				D3DX_FILTER_POINT,
				D3DX_FILTER_NONE,
				0,
				&imgInfo,
				NULL,
				&lpTex );
		if( FAILED(ret) ) {
			DEBUG( "[%s] 画像ロード失敗\n",file );
			return FALSE;
		}

		// サイズを取得
		dwWidth = imgInfo.Width;
		dwHeight = imgInfo.Height;

	} else {

		// 非対応なら

#if !USEPLUGIN
		return FALSE;
#else

		CSusie su;
		su.SetPath( mPlgPath );
		su.SetExt( mExt );
		if( !su.Load(file) ) {
			DEBUG( "[%s] プラグインロードエラー\n",file );
			return FALSE;
		}

		// テクスチャの作成
		if( !Create(dev,su.GetWidth(),su.GetHeight()) )
			return FALSE;

		// テクスチャに書き込む
		D3DLOCKED_RECT rect;
		if( !Lock(&rect) ) {
			DEBUG( "ロックエラー\n" );
			return FALSE;
		}
		ZeroMemory( (LPBYTE)rect.pBits,LINESIZE(dwWidth,32)*dwHeight );

		// コピー
		for( int y=0;y<(int)su.GetHeight();y++ ) {
			LPBYTE p1 = su.GetBuffer() + y*su.GetLineSize();
			LPBYTE p2 = (LPBYTE)rect.pBits + (su.GetHeight()-y-1)*rect.Pitch;
			memcpy( p2,p1,su.GetLineSize() );
		}
		Unlock();

#endif

	}

	DEBUG( "ロードOK [%s]\n",file );
	DEBUG( "幅       [%d]\n",dwWidth );
	DEBUG( "高さ     [%d]\n",dwHeight );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 指定のサイズのテクスチャを生成
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Create( LPDIRECT3DDEVICE9 dev,DWORD w,DWORD h )
{
	Delete();

	// 値の保存
	dwWidth = w;
	dwHeight = h;

	// テクスチャの生成
	if( FAILED(dev->CreateTexture( w,h,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&lpTex,NULL )) ) {
		DEBUG("テクスチャの生成に失敗\n" );
		return FALSE;
	}

	DEBUG( "テクスチャ作成OK\n" );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// メモリイメージからテクスチャを生成
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Create( LPDIRECT3DDEVICE9 dev,const LPVOID buf,int size )
{
	HRESULT ret;

	Delete();

	D3DXIMAGE_INFO imgInfo;
	ZeroMemory( &imgInfo,sizeof(D3DXIMAGE_INFO) );
	ret = D3DXCreateTextureFromFileInMemoryEx(
			dev,
			(LPCVOID)buf,
			size,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			1,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_FILTER_POINT,
			D3DX_FILTER_POINT,
			0,
			&imgInfo,
			NULL,
			&lpTex );
	if( FAILED(ret) ) {
		DEBUG( "ロード失敗\n" );
		return FALSE;
	}

	dwWidth = imgInfo.Width;
	dwHeight = imgInfo.Height;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// 消去
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Delete( void )
{
	SAFE_RELEASE( lpTex );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// テクスチャのロック
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Lock( D3DLOCKED_RECT *rect )
{
	if( bLock )
		return FALSE;
	if( FAILED(lpTex->LockRect(0,rect,NULL,0)) )
		return FALSE;

	bLock = TRUE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// ロック解除
/////////////////////////////////////////////////////////////////////////
BOOL CDDTexPro90::Unlock( void )
{
	if( !bLock )
		return FALSE;

	bLock = FALSE;

	if( FAILED(lpTex->UnlockRect(0)) )
		return FALSE;

	return TRUE;
}

