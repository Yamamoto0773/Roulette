#ifndef _CDDPRO90_H
#define _CDDPRO90_H
///////////////////////////////////////////////////////////////////////////////////
// CDDPro90 : Direct3D9管理クラス for おとゲマ v2.01                             //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include "CDDTexPro90.h"

#define CDDMAXTEXTURE		2048		// 追加可能なテクスチャ数
#define CDDMAXOBJECT		2048		// 設定可能な切り抜き数

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")


// 画像切り抜き情報
typedef struct _PUTOBJECT {
	BOOL	bFlag;								// オブジェクトの定義がされているか
	int		iTexID;								// 使用するテクスチャID
	POINT	mPos;								// 切り抜き左上の座標
	SIZE	mSize;								// 切り抜くサイズ
	POINT	mOffset;							// 原点位置(Put時のみ適用)
	float	fAlpha;								// 透明度
	float	fScaleX;							// Ｘの倍率
	float	fScaleY;							// Ｙの倍率
	float	fRot;								// 回転角（ラジアン）
} PUTOBJECT,*LPPUTOBJECT;



class CDDPro90 {
	LPDIRECT3D9				lpD3D;						// Direct3D9
	LPDIRECT3DDEVICE9		lpD3DDevice;				// Direct3DDevice9
	D3DCAPS9				mD3DCaps;					// Direct3D9Caps

	CDDTexPro90				tex[CDDMAXTEXTURE];			// テクスチャリスト
	PUTOBJECT				mObj[CDDMAXOBJECT];			// 切り抜きリスト

private:
	BOOL					bFullScreen;				// フルスクリーンか
	BOOL					bVSync;						// ウインドウモードでVSYNCを行うか
	BOOL					bRightHand;					// 右手系か
	DWORD					dwWidth;					// 幅
	DWORD					dwHeight;					// 高さ
	DWORD					dwBit;						// ビット深度
	DWORD					dwFrequency;				// リフレッシュレート
	D3DCOLOR				dwBackColor;				// 背景カラー
	D3DPRESENT_PARAMETERS	mParam;						// デバイスの設定
	BOOL					bBegin;						// 描画中か
	float					fOffset;					// テクスチャずれ
private:
	BOOL					bLost;						// ロスト中か
	BOOL					bLostFirst;					// 1回目のロスト処理が終わったか
private:
	void CalcScaleRotate( float *vx,float *vy,float xscale,float yscale,float rot=0 );	// 指定のベクトルに拡大縮小・回転を行う(回転角はラジアン)
	BOOL SetBackBufferFormat( void );
public:
	CDDPro90();
	virtual ~CDDPro90();
	BOOL Create( HWND hwnd,BOOL full=TRUE,int w=1024,int h=768,int bit=32,int freq=0,BOOL vsync=TRUE,DWORD vs=D3DVS_VERSION(0,0),DWORD ps=D3DPS_VERSION(0,0) );		// 構築
	BOOL Delete( void );																					// 終了
	BOOL Clear( void );																						// テクスチャと切り抜き情報をすべてクリア
	BOOL SetHand( BOOL right=TRUE );																		// 座標系のセット(TRUE=右手)
	void SetBackColor( D3DCOLOR col=0x000000 );																// 画面クリア時の色をセット
	BOOL SetPlugin( char *path,char *ext );																	// プラグインの検索条件の変更
public:
	// テクスチャ
	BOOL AddTexture( int id,char *file );																	// ファイルから作成
	BOOL AddTexture( int id,LPVOID buf,int size );															// メモリイメージから作成
	BOOL DelTexture( int id );																				// 開放

public:
	// 切り抜き・効果
	BOOL SetPutRange( int id,int tex_id,int x,int y,int w,int h,int ox=0,int oy=0 );						// 描画画像切抜き
	BOOL SetPutStatus( int id,float alpha=1.0f,float scale=1.0f,float rot=0.0f );							// 描画ステータスのセット
	BOOL SetPutStatusEx( int id,float alpha=1.0f,float scale_x=1.0f,float scale_y=1.0f,float rot=0.0f );	// 描画ステータスのセット2

public:
	// 描画
	BOOL CheckDevice( void );																				// ロスト・リストア自動処理(LOST時は必ず1回FALSEが返る)
	BOOL DrawBegin( BOOL clear=TRUE );																		// バックバッファへの描画開始(clear=TRUEなら画面をクリア)
	BOOL DrawEnd( void );																					// バックバッファへの描画終了で実際に画面に表示
	BOOL Put( int obj,float x,float y );																	// SetPutRangeで指定した画像を左上を原点にて表示
	BOOL Put2( int obj,float x,float y );																	// SetPutRangeで指定した画像の中心を原点にて表示
	BOOL PutEx( int obj,float x,float y,float mx,float my );												// SetPutRangeで指定した画像の指定位置を原点にて表示
	BOOL SetBlendOne( BOOL on=TRUE );																		// 加算合成にするか
	BOOL SetRenderState( D3DRENDERSTATETYPE type,DWORD flag );												// レンダリングステートを設定
	BOOL SetTextureStageState( DWORD st,D3DTEXTURESTAGESTATETYPE type,DWORD val );							// テクスチャにステートを設定
public:
	inline LPDIRECT3DDEVICE9 GetD3DDevice( void ) { return lpD3DDevice; }									// Direct3DDevice9を返す
	inline operator LPDIRECT3DDEVICE9( void ) { return lpD3DDevice; }										// Direct3DDevice9直接参照
	inline const D3DCAPS9 *GetD3DCaps( void ) { return &mD3DCaps; }											// デバイス情報を返す
	inline CDDTexPro90 *GetTexClass( int id ) { return &tex[id]; }											// テクスチャクラスを返す
};

typedef class CDDPro90 CDDPro,CDDPRO90,*LPCDDPRO90;

#endif
