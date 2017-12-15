#ifndef _CDSTEXPRO90_H
#define _CDSTEXPRO90_H
///////////////////////////////////////////////////////////////////////////////////
// CDDTexPro90 : DirectX9テクスチャ管理クラス for おとゲマ v2.00                 //
// ※このクラスはCDDPro90で使用されるため直接構築する必要はありません。          //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <d3dx9.h>

// Susieプラグインを利用してテクスチャ化をする場合は1にする(要CSusieライブラリ)
#define USEPLUGIN		0


class CDDTexPro90 {
	LPDIRECT3DTEXTURE9	lpTex;										// サーフェスオブジェクト
	DWORD				dwWidth;									// 画像の幅
	DWORD				dwHeight;									// 画像の高さ
	BOOL				bLock;										// テクスチャロック中か
	char				mPlgPath[MAX_PATH];							// プラグインの存在するフォルダ
	char				mExt[256];									// プラグインの拡張子
public:
	CDDTexPro90();
	virtual ~CDDTexPro90();
	BOOL SetPluginPath( const char *path );							// SusieプラグインDLLの検索パスの変更
	BOOL SetExt( const char *ext );									// Susieプラグインの拡張子の変更
	BOOL Create( LPDIRECT3DDEVICE9 dev,const char *file );			// ファイルからサーフェスを生成
	BOOL Create( LPDIRECT3DDEVICE9 dev,DWORD w,DWORD h );			// 指定のサイズのテクスチャを生成
	BOOL Create( LPDIRECT3DDEVICE9 dev,const LPVOID buf,int size );	// メモリイメージからテクスチャを生成
	BOOL Delete( void );											// 消去
	BOOL Lock( D3DLOCKED_RECT *rect );								// テクスチャのロック
	BOOL Unlock( void );											// ロック解除
public:
	inline LPDIRECT3DTEXTURE9 GetTexture( void ) { return lpTex; }	// テクスチャを返す
	inline DWORD GetWidth( void ) { return dwWidth; }				// テクスチャの幅を返す
	inline DWORD GetHeight( void ) { return dwHeight; }				// テクスチャの高さを返す
};

#endif
