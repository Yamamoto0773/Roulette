#include "CDDPro90.h"
#include <math.h>

#define SAFE_RELEASE(x)		{ if(x) { x->Release(); x=NULL; } }


#define DEBUGMODE
#include "DEBUG.H"


// 描画頂点
typedef struct _CDDTLVERTEX {
	float x,y,z,rhw;
	DWORD color;
	float tu,tv;
} CDDTLVERTEX,*LPCDDTLVERTEX;
#define CDDFVF_TLVERTEX		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)


CDDPro90::CDDPro90()
{
	lpD3D = NULL;
	lpD3DDevice = NULL;
	ZeroMemory( &mD3DCaps,sizeof(mD3DCaps) );

	ZeroMemory(&mObj, sizeof(mObj));

	// スクリーンモード初期化
	bFullScreen		= TRUE;				// フルスクリーン
	bVSync			= TRUE;				// VSYNC
	bRightHand		= FALSE;
	dwWidth			= 640;
	dwHeight		= 480;
	dwBit			= 16;
	dwFrequency		= 0;				// デフォルト周波数
	dwBackColor		= 0x00000000;
	ZeroMemory( &mParam,sizeof(mParam) );
	bBegin			= FALSE;
	fOffset			= -0.5f;

	// ロスト処理
	bLost			= FALSE;
	bLostFirst		= FALSE;

	Clear();
}

CDDPro90::~CDDPro90()
{
	Delete();
}

// 指定のベクトルに拡大縮小・回転を行う(回転角はラジアン)
void CDDPro90::CalcScaleRotate( float *vx,float *vy,float xscale,float yscale,float rot )
{
	float lx = *vx * xscale;
	float ly = *vy * yscale;
	*vx = (float)(lx * cos(rot) - ly * sin(rot));
	*vy = (float)(lx * sin(rot) + ly * cos(rot));
}

BOOL CDDPro90::SetBackBufferFormat( void )
{
	if( !lpD3D )
		return FALSE;

	// 現在のディスプレイモードを得る
	if( bFullScreen ) {
		// フルスクリーンなら
		if( dwBit==16 )
			mParam.BackBufferFormat		= D3DFMT_R5G6B5;
		else
			mParam.BackBufferFormat		= D3DFMT_X8R8G8B8;
	} else {
		D3DDISPLAYMODE mDispMode;					// 画面モード
		HRESULT ret = lpD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT,&mDispMode );	// DEFAULT指定でプライマリアダプタを選択
		if( FAILED(ret) ) {
			DEBUG( "GetAdapterDisplayModeエラー\n" );
			return FALSE;
		}
		mParam.BackBufferFormat			= mDispMode.Format;								// バックサーフェスのフォーマットをコピーして使用する
		DEBUG( "WIDTH  [%d]\n",mDispMode.Width );
		DEBUG( "HEIGHT [%d]\n",mDispMode.Height );
		DEBUG( "FREQ   [%d]\n",mDispMode.RefreshRate );
		DEBUG( "FORMAT [%d]\n",mDispMode.Format );
	}
	return TRUE;
}

BOOL CDDPro90::Create( HWND hwnd,BOOL full,int w,int h,int bit,int freq,BOOL vsync,DWORD vs,DWORD ps )
{
	Delete();

	HRESULT ret;

	bFullScreen	= full;
	dwWidth		= w;
	dwHeight	= h;
	dwBit		= bit;
	dwFrequency	= freq;
	bVSync		= vsync;

	// Direct3D9オブジェクトの取得
	lpD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if( !lpD3D )
		return FALSE;

	ret = lpD3D->GetDeviceCaps( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&mD3DCaps );
	if( FAILED(ret) ) {
		DEBUG( "ハードウェアの情報取得失敗\n" );
		return FALSE;
	}


	// Caps
/*	DEBUG( "----- Caps -----\n" );
	DEBUG( "[%s] ディスプレイハードウェアは現在の走査線を返すことができる\n",(mD3DCaps.Caps&D3DCAPS_READ_SCANLINE)?"YES":"NO " );

	// Caps2
	DEBUG( "----- Caps2 -----\n" );
	DEBUG( "[%s] ドライバはミップマップを自動生成できる\n",(mD3DCaps.Caps2&D3DCAPS2_CANAUTOGENMIPMAP)?"YES":"NO " );
	DEBUG( "[%s] ガンマランプを調整できる口径測定器がインストールされている\n",(mD3DCaps.Caps2&D3DCAPS2_CANCALIBRATEGAMMA)?"YES":"NO " );
	DEBUG( "[%s] ドライバはリソースを管理できる\n",(mD3DCaps.Caps2&D3DCAPS2_CANMANAGERESOURCE)?"YES":"NO " );
	DEBUG( "[%s] ドライバは動的テクスチャをサポートしている\n",(mD3DCaps.Caps2&D3DCAPS2_DYNAMICTEXTURES)?"YES":"NO " );
	DEBUG( "[%s] フルスクリーンモードでのダイナミックガンマランプ調整をサポートしている\n",(mD3DCaps.Caps2&D3DCAPS2_FULLSCREENGAMMA)?"YES":"NO " );

	// Caps3
	DEBUG( "----- Caps3 -----\n" );
	DEBUG( "[%s] FLIPまたはDISCARDスワップ効果を使う一方で、フルスクリーンモードでのD3DRS_ALPHABLENDENABLEレンダーステートを尊重することを示す\n",(mD3DCaps.Caps3&D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)?"YES":"NO " );
	DEBUG( "[%s] システムメモリからローカルビデオメモリへのメモリコピーを高速化できる\n",(mD3DCaps.Caps3&D3DCAPS3_COPY_TO_VIDMEM)?"YES":"NO " );
	DEBUG( "[%s] ローカルビデオメモリからシステムメモリへのメモリコピーを高速化できる\n",(mD3DCaps.Caps3&D3DCAPS3_COPY_TO_SYSTEMMEM)?"YES":"NO " );
	DEBUG( "[%s] ウィンドウモードのバックバッファからsRGBデスクトップへのガンマ補正を実行できる\n",(mD3DCaps.Caps3&D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION)?"YES":"NO " );

	// PresentationIntervals
	DEBUG( "----- PresentationIntervals -----\n" );
	DEBUG( "[%s] VSYNCを待たずにフリップ可能\n",(mD3DCaps.PresentationIntervals&D3DPRESENT_INTERVAL_IMMEDIATE)?"YES":"NO " );
	DEBUG( "[%s] １回目のスクリーンフレッシュごとにフリップ可能\n",(mD3DCaps.PresentationIntervals&D3DPRESENT_INTERVAL_ONE)?"YES":"NO " );
	DEBUG( "[%s] ２回目のスクリーンフレッシュごとにフリップ可能\n",(mD3DCaps.PresentationIntervals&D3DPRESENT_INTERVAL_TWO)?"YES":"NO " );
	DEBUG( "[%s] ３回目のスクリーンフレッシュごとにフリップ可能\n",(mD3DCaps.PresentationIntervals&D3DPRESENT_INTERVAL_THREE )?"YES":"NO " );
	DEBUG( "[%s] ４回目のスクリーンフレッシュごとにフリップ可能\n",(mD3DCaps.PresentationIntervals&D3DPRESENT_INTERVAL_FOUR)?"YES":"NO " );

	// CursorCaps
	DEBUG( "----- CursorCaps -----\n" );
	DEBUG( "[%s] 高解像度モードでフルカラーカーソルをサポート\n",(mD3DCaps.CursorCaps&D3DCURSORCAPS_COLOR)?"YES":"NO " );
	DEBUG( "[%s] 高解像度モードと低解像度モードでフルカラーカーソルをサポート\n",(mD3DCaps.CursorCaps&D3DCURSORCAPS_LOWRES)?"YES":"NO " );

	// DevCaps
	DEBUG( "----- DevCaps -----\n" );
	DEBUG( "[%s] システムメモリテクスチャから非ローカルビデオメモリテクスチャへのブリットをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_CANBLTSYSTONONLOCAL)?"YES":"NO " );
	DEBUG( "[%s] ページフリップの後で、レンダリングコマンドをキューに入れることができる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_CANRENDERAFTERFLIP)?"YES":"NO " );
	DEBUG( "[%s] 少なくともDirectX5.0準拠のドライバをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_DRAWPRIMITIVES2)?"YES":"NO " );
	DEBUG( "[%s] 少なくともDirectX7.0準拠のドライバをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_DRAWPRIMITIVES2EX)?"YES":"NO " );
	DEBUG( "[%s] DrawPrimitive 対応のHALをエクスポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_DRAWPRIMTLVERTEX)?"YES":"NO " );
	DEBUG( "[%s] システムメモリにある実行バッファを使用できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_EXECUTESYSTEMMEMORY)?"YES":"NO " );
	DEBUG( "[%s] ビデオメモリにある実行バッファを使用できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_EXECUTEVIDEOMEMORY)?"YES":"NO " );
	DEBUG( "[%s] シーンのラスタ化用のハードウェアアクセラレーションがある\n",(mD3DCaps.DevCaps&D3DDEVCAPS_HWRASTERIZATION)?"YES":"NO " );
	DEBUG( "[%s] ハードウェアでトランスフォーム＆ライティングをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)?"YES":"NO " );
	DEBUG( "[%s] Nパッチをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_NPATCHES)?"YES":"NO " );
	DEBUG( "[%s] ハードウェアでラスタ化、トランスフォーム、ライティング、およびシェーディングをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_PUREDEVICE)?"YES":"NO " );
	DEBUG( "[%s] 5次ベジェおよびBスプラインをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_QUINTICRTPATCHES)?"YES":"NO " );
	DEBUG( "[%s] 矩形および三角形パッチをサポート\n",(mD3DCaps.DevCaps&D3DDEVCAPS_RTPATCHES)?"YES":"NO " );
	DEBUG( "[%s] ハードウェアアーキテクチャで情報をキャッシュする必要がない\n",(mD3DCaps.DevCaps&D3DDEVCAPS_RTPATCHHANDLEZERO)?"YES":"NO " );
	DEBUG( "[%s] 独立したメモリプールからテクスチャ処理を行っている\n",(mD3DCaps.DevCaps&D3DDEVCAPS_SEPARATETEXTUREMEMORIES)?"YES":"NO " );
	DEBUG( "[%s] 非ローカルビデオメモリからテクスチャを取得できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_TEXTURENONLOCALVIDMEM)?"YES":"NO " );
	DEBUG( "[%s] システムメモリからテクスチャを取得できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_TEXTURESYSTEMMEMORY)?"YES":"NO " );
	DEBUG( "[%s] デバイスメモリからテクスチャを取得できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_TEXTUREVIDEOMEMORY)?"YES":"NO " );
	DEBUG( "[%s] トランスフォーム済みライティング済みの頂点用に、システムメモリのバッファを使用できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)?"YES":"NO " );
	DEBUG( "[%s] トランスフォーム済みライティング済みの頂点用に、ビデオメモリのバッファを使用できる\n",(mD3DCaps.DevCaps&D3DDEVCAPS_TLVERTEXVIDEOMEMORY)?"YES":"NO " );

	// PrimitiveMiscCaps
	DEBUG( "----- PrimitiveMiscCaps -----\n" );
	DEBUG( "[%s] ピクセル処理における深度バッファの変更の有効/無効を切り替えることができる\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_MASKZ)?"YES":"NO " );
	DEBUG( "[%s] D3DCULL_NONEが指定可能\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_CULLNONE)?"YES":"NO " );
	DEBUG( "[%s] 時計回りの三角形カリングD3DCULL_CWが指定可能\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_CULLCW)?"YES":"NO " );
	DEBUG( "[%s] 反時計回りの三角形カリングD3DCULL_CCWが指定可能\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_CULLCCW)?"YES":"NO " );
	DEBUG( "[%s] レンダーターゲットのカラーバッファへのチャンネルごとの書き込みをサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_COLORWRITEENABLE)?"YES":"NO " );
	DEBUG( "[%s] 1.0より大きいサイズのスケーリングされたポイントを正確にクリップできる\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_CLIPPLANESCALEDPOINTS)?"YES":"NO " );
	DEBUG( "[%s] 座標変換後の頂点プリミティブをクリップできる\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_CLIPTLVERTS)?"YES":"NO " );
	DEBUG( "[%s] 一時レジスタ用のD3DTAをサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_TSSARGTEMP)?"YES":"NO " );
	DEBUG( "[%s] D3DBLENDOP_ADD以外のアルファブレンディング処理をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_BLENDOP)?"YES":"NO " );
	DEBUG( "[%s] レンダリングしないリファレンスデバイス\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_NULLREFERENCE)?"YES":"NO " );
	DEBUG( "[%s] 複数要素のテクスチャまたは複数レンダーターゲットで個別の書き込みマスクの使用をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_INDEPENDENTWRITEMASKS)?"YES":"NO " );
	DEBUG( "[%s] ステージごとの定数をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_PERSTAGECONSTANT)?"YES":"NO " );
	DEBUG( "[%s] アルファチャンネルに対する個別のブレンド設定をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_SEPARATEALPHABLEND)?"YES":"NO " );
	DEBUG( "[%s] 複数レンダーターゲットに対して異なるビット深度をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS)?"YES":"NO " );
	DEBUG( "[%s] 複数レンダーターゲットに対するピクセルシェーダ後の処理をサポート\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING)?"YES":"NO " );
	DEBUG( "[%s] 頂点ごとのフォグブレンド係数を制限\n",(mD3DCaps.PrimitiveMiscCaps&D3DPMISCCAPS_FOGVERTEXCLAMPED)?"YES":"NO " );

	// RasterCaps
	DEBUG( "----- RasterCaps -----\n" );
	DEBUG( "[%s] 異方性フィルタリングをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_ANISOTROPY)?"YES":"NO " );
	DEBUG( "[%s] 色の遠近を正しく補間\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_COLORPERSPECTIVE)?"YES":"NO " );
	DEBUG( "[%s] ディザによって色解像度を向上させることができる\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_DITHER)?"YES":"NO " );
	DEBUG( "[%s] 旧来の深度バイアスをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_FOGRANGE)?"YES":"NO " );
	DEBUG( "[%s] 特定のピクセルの深度に対してインデックス付けされたフォグ値が含まれる参照テーブルに照会し、フォグ値を計算\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_FOGTABLE)?"YES":"NO " );
	DEBUG( "[%s] ライティング処理の最中にフォグ値を計算し、ラスタ化の最中にフォグ値を補間\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_FOGVERTEX)?"YES":"NO " );
	DEBUG( "[%s] 詳細レベル (LOD) バイアスの調整をサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_MIPMAPLODBIAS)?"YES":"NO " );
	DEBUG( "[%s] BeginSceneとEndSceneの間でマルチサンプリングのオンとオフの切り替えをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_MULTISAMPLE_TOGGLE)?"YES":"NO " );
	DEBUG( "[%s] シザーテストをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_SCISSORTEST)?"YES":"NO " );
	DEBUG( "[%s] 真の勾配スケールベースの深度バイアスを実行\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS)?"YES":"NO " );
	DEBUG( "[%s] wを使った深度バッファリングをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_WBUFFER)?"YES":"NO " );
	DEBUG( "[%s] wベースのフォグをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_WFOG)?"YES":"NO " );
	DEBUG( "[%s] アプリケーションによるポリゴンのソートや深度バッファの割り当てを必要とせずに隠面消去を実行できる\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_ZBUFFERLESSHSR)?"YES":"NO " );
	DEBUG( "[%s] zベースのフォグをサポート\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_ZFOG)?"YES":"NO " );
	DEBUG( "[%s] zテスト処理を実行できる\n",(mD3DCaps.RasterCaps&D3DPRASTERCAPS_ZTEST)?"YES":"NO " );

	// SrcBlendCaps
	DEBUG( "----- SrcBlendCaps -----\n" );
	DEBUG( "[%s] D3DBLEND_BLENDFACTORとD3DBLEND_INVBLENDFACTOR\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_BLENDFACTOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHINVSRCALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_BOTHINVSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHSRCALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_BOTHSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_DESTALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_DESTALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_DESTCOLOR\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_DESTCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVDESTALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_INVDESTALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVDESTCOLOR\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_INVDESTCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVSRCALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_INVSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVSRCCOLOR\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_INVSRCCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_ONE\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_ONE)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHSRCALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_SRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_SRCALPHA\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_SRCALPHASAT)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_SRCCOLOR\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_SRCCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_ZERO\n",(mD3DCaps.SrcBlendCaps&D3DPBLENDCAPS_ZERO)?"YES":"NO " );

	// DestBlendCaps
	DEBUG( "----- DestBlendCaps -----\n" );
	DEBUG( "[%s] D3DBLEND_BLENDFACTORとD3DBLEND_INVBLENDFACTOR\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_BLENDFACTOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHINVSRCALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_BOTHINVSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHSRCALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_BOTHSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_DESTALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_DESTALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_DESTCOLOR\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_DESTCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVDESTALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_INVDESTALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVDESTCOLOR\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_INVDESTCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVSRCALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_INVSRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_INVSRCCOLOR\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_INVSRCCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_ONE\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_ONE)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_BOTHSRCALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_SRCALPHA)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_SRCALPHA\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_SRCALPHASAT)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_SRCCOLOR\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_SRCCOLOR)?"YES":"NO " );
	DEBUG( "[%s] D3DBLEND_ZERO\n",(mD3DCaps.DestBlendCaps&D3DPBLENDCAPS_ZERO)?"YES":"NO " );

	// ShadeCaps
	DEBUG( "----- ShadeCaps -----\n" );
	DEBUG( "[%s] グーローブレンドされた透明度のアルファ成分をサポート\n",(mD3DCaps.ShadeCaps&D3DPSHADECAPS_ALPHAGOURAUDBLEND)?"YES":"NO " );
	DEBUG( "[%s] 色付きのグーローシェーディングをサポート\n",(mD3DCaps.ShadeCaps&D3DPSHADECAPS_COLORGOURAUDRGB)?"YES":"NO " );
	DEBUG( "[%s] グーローシェーディングモードでフォグをサポート\n",(mD3DCaps.ShadeCaps&D3DPSHADECAPS_FOGGOURAUD)?"YES":"NO " );
	DEBUG( "[%s] スペキュラハイライトのグーローシェーディングをサポート\n",(mD3DCaps.ShadeCaps&D3DPSHADECAPS_SPECULARGOURAUDRGB)?"YES":"NO " );

	// TextureCaps
	DEBUG( "----- TextureCaps -----\n" );
	DEBUG( "[%s] テクスチャピクセルでアルファがサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_ALPHA)?"YES":"NO " );
	DEBUG( "[%s] テクスチャパレットからアルファを描画できる\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_ALPHAPALETTE)?"YES":"NO " );
	DEBUG( "[%s] キューブテクスチャをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP)?"YES":"NO " );
	DEBUG( "[%s] キューブテクスチャマップのディメンジョンが2の累乗で指定されている必要がある\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP_POW2)?"YES":"NO " );
	DEBUG( "[%s] ミップマップ化キューブテクスチャをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_MIPCUBEMAP)?"YES":"NO " );
	DEBUG( "[%s] ミップマップ化テクスチャをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_MIPMAP)?"YES":"NO " );
	DEBUG( "[%s] ミップマップ化ボリュームテクスチャをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_MIPVOLUMEMAP)?"YES":"NO " );
	DEBUG( "[%s] ディメンジョンが2の累乗でない2Dテクスチャの使用を条件付きでサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_NONPOW2CONDITIONAL)?"YES":"NO " );
	DEBUG( "[%s] プログラマブルシェーダおよび固定機能シェーダでの射影バンプ環境参照処理をサポートしない\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_NOPROJECTEDBUMPENV)?"YES":"NO " );
	DEBUG( "[%s] 遠近補正テクスチャ処理をサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_PERSPECTIVE)?"YES":"NO " );
	DEBUG( "[%s] すべてのテクスチャの幅と高さを2の累乗で指定する必要がある\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_POW2)?"YES":"NO " );
	DEBUG( "[%s] D3DTTFF_PROJECTEDテクスチャ座標変換フラグをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_PROJECTED)?"YES":"NO " );
	DEBUG( "[%s] すべてのテクスチャが正方形でなければならない\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY)?"YES":"NO " );
	DEBUG( "[%s] 補間の前にテクスチャインデックスがテクスチャサイズに合わせてスケーリングされない\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE)?"YES":"NO " );
	DEBUG( "[%s] ボリュームテクスチャをサポート\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_VOLUMEMAP)?"YES":"NO " );
	DEBUG( "[%s] ボリュームテクスチャマップのディメンジョンが2の累乗で指定されている必要がある\n",(mD3DCaps.TextureCaps&D3DPTEXTURECAPS_VOLUMEMAP_POW2)?"YES":"NO " );
/**/
	// その他
/*	DEBUG( "----- その他 -----\n" );
	DEBUG( "[%d] テクスチャの幅の最大値\n",mD3DCaps.MaxTextureWidth );
	DEBUG( "[%d] テクスチャの高さの最大値\n",mD3DCaps.MaxTextureHeight );
	DEBUG( "[%d] ボリュームテクスチャの3つのいずれかのディメンジョン (幅、高さ、深度) の最大値\n",mD3DCaps.MaxVolumeExtent );
	DEBUG( "[%d] ハードウェアでサポートされるテクスチャの最大アスペクト比\n",mD3DCaps.MaxTextureAspectRatio );
	DEBUG( "[%d] 固定機能パイプラインでサポートされるテクスチャブレンディングステージの最大数\n",mD3DCaps.MaxTextureBlendStages );
	DEBUG( "[%d] 同時にアクティブにできるライトの最大数\n",mD3DCaps.MaxActiveLights );
	DEBUG( "[%d] ハードウェア頂点処理でサポートされるインデックスの最大サイズ\n",mD3DCaps.MaxVertexIndex );
	DEBUG( "[%d] SetStreamSourceの同時データストリームの最大数\n",mD3DCaps.MaxStreams );
	DEBUG( "[%d] SetStreamSourceの最大ストライド\n",mD3DCaps.MaxStreamStride );
	DEBUG( "[%04X] 頂点シェーダーバージョン\n",mD3DCaps.VertexShaderVersion&0xFFFF );
	DEBUG( "[%04X] ピクセルシェーダーバージョン\n",mD3DCaps.PixelShaderVersion&0xFFFF );
	DEBUG( "[%d] 実行できる頂点シェーダ命令の最大数\n",mD3DCaps.MaxVShaderInstructionsExecuted );
	DEBUG( "[%d] 実行できるピクセルシェーダ命令の最大数\n",mD3DCaps.MaxPShaderInstructionsExecuted );
	DEBUG( "----------------------------------------------------------------------------------------\n" );/**/


	// バックサーフェースのフォーマットをコピーして使用する
	ZeroMemory(&mParam,sizeof(mParam));
	mParam.BackBufferWidth = dwWidth;
	mParam.BackBufferHeight = dwHeight;
	mParam.BackBufferCount = 1;
	mParam.MultiSampleType = D3DMULTISAMPLE_NONE;
	mParam.SwapEffect = D3DSWAPEFFECT_DISCARD;		// 垂直同期でフリップ
	mParam.Windowed						= !bFullScreen;					// ウィンドウモードか
	mParam.EnableAutoDepthStencil = TRUE;
	mParam.AutoDepthStencilFormat = D3DFMT_D24S8;
//	mParam.AutoDepthStencilFormat		= D3DFMT_D16;
	mParam.FullScreen_RefreshRateInHz = 0;							// リフレッシュレート
	
	if( !vsync )
		mParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// バックバッファフォーマットのセット
	SetBackBufferFormat();


	// デバイスの最適な設定を行う
	DWORD dwHalOrRef		= D3DDEVTYPE_HAL;
	DWORD dwHardOrSoftFlag	= D3DCREATE_SOFTWARE_VERTEXPROCESSING;			// ハード＆ソフト頂点処理

	// T&L対応なら
	if( mD3DCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		DEBUG( "T&Lに対応\n" );
		dwHardOrSoftFlag &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;			// 解除
		dwHardOrSoftFlag |= D3DCREATE_HARDWARE_VERTEXPROCESSING;			// ハードウェアに変更
	}

	if( vs>D3DVS_VERSION(0,0) ) {
		// 頂点シェーダーのバージョン指定時
		if( mD3DCaps.VertexShaderVersion<vs ) {
			DEBUG( "頂点シェーダー%d.%dに対応していない\n",(vs&0xFF00)>>8,vs&0xFF );
			dwHardOrSoftFlag &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			dwHardOrSoftFlag |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;		// ハード＆ソフト頂点処理に変更
		}
	}

	if( ps>(DWORD)D3DPS_VERSION(0,0) ) {
		// ピクセルシェーダーのバージョン指定時
		if( mD3DCaps.PixelShaderVersion<ps ) {
			DEBUG( "ピクセルシェーダー%d.%dに対応していない\n",(ps&0xFF00)>>8,ps&0xFF );
			dwHalOrRef = D3DDEVTYPE_REF;
		}
	}

	// 全オペレーションが可能なら
/*	if( mD3DCaps.DevCaps&D3DDEVCAPS_PUREDEVICE ) {
		DEBUG( "PUREDEVICEを使用\n" );
		dwHardOrSoftFlag |= D3DCREATE_PUREDEVICE;
	}/**/

	// マルチスレッド対応
	dwHardOrSoftFlag |= D3DCREATE_MULTITHREADED;

	// デバイスの作成
	ret = lpD3D->CreateDevice( D3DADAPTER_DEFAULT,(D3DDEVTYPE)dwHalOrRef,hwnd,dwHardOrSoftFlag,&mParam,&lpD3DDevice );
	if( FAILED(ret) ) {
		DEBUG( "Direct3Dデバイスの作成に失敗\n" );
		return FALSE;
	}


	// デフォルトステートのセット
	lpD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );							// ライティング無効
	lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);					// アルファブレンド使用可能
	lpD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );				// 転送元のブレンド設定
	lpD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );			// 転送先ブレンド設定
	if( bRightHand )
		lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );					// 右回りを消去(右手系)
	else
		lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );					// 左回りを消去(左手系)

	lpD3DDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
	lpD3DDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );


	lpD3DDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_MODULATE );
	lpD3DDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_TEXTURE );
	lpD3DDevice->SetTextureStageState( 0,D3DTSS_COLORARG2,D3DTA_DIFFUSE );
	lpD3DDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	lpD3DDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
	lpD3DDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE );

	return TRUE;
}

BOOL CDDPro90::Delete( void )
{
	Clear();
	SAFE_RELEASE( lpD3DDevice );
	SAFE_RELEASE( lpD3D );

	ZeroMemory( &mObj,sizeof(mObj) );
	bRightHand	= TRUE;
	bBegin		= FALSE;
	bLost		= FALSE;
	bLostFirst	= FALSE;
	return TRUE;
}

BOOL CDDPro90::Clear( void )
{
	int i;
	for( i=0;i<CDDMAXTEXTURE;i++ ) {
		tex[i].Delete();
	}
	ZeroMemory( &mObj,sizeof(mObj) );
	return TRUE;
}

BOOL CDDPro90::SetHand( BOOL right )
{
	bRightHand = right;
	return TRUE;
}

void CDDPro90::SetBackColor( D3DCOLOR col )
{
	dwBackColor = col & 0xffffff;
}

BOOL CDDPro90::SetPlugin( char *path,char *ext )
{
	int i;
	for( i=0;i<CDDMAXTEXTURE;i++ ) {
		tex[i].SetPluginPath( path );
		tex[i].SetExt( ext );
	}
	return TRUE;
}

BOOL CDDPro90::AddTexture( int id,char *file )
{
	if( id<0 || id>CDDMAXTEXTURE-1 )
		return FALSE;

	return tex[id].Create( lpD3DDevice,file );
}

BOOL CDDPro90::AddTexture( int id,LPVOID buf,int size )
{
	if( id<0 || id>CDDMAXTEXTURE-1 )
		return FALSE;

	return tex[id].Create( lpD3DDevice,buf,size );
}

BOOL CDDPro90::DelTexture( int id )
{
	if( id<0 || id>CDDMAXTEXTURE-1 )
		return FALSE;

	// 該当するPUT情報があれば使用不可とする
	for( int i=0;i<CDDMAXOBJECT;i++ ) {
		if( mObj[i].bFlag ) {
			if( mObj[i].iTexID==id )
				mObj[i].bFlag = FALSE;
		}
	}

	return tex[id].Delete();
}



BOOL CDDPro90::SetPutRange( int id,int tex_id,int x,int y,int w,int h,int ox,int oy )
{
	if( id<0 || id>CDDMAXOBJECT-1 )
		return FALSE;
	if( tex_id<0 || tex_id>CDDMAXTEXTURE-1 )
		return FALSE;

	// セット
	mObj[id].bFlag		= TRUE;
	mObj[id].iTexID		= tex_id;
	mObj[id].mPos.x		= x;
	mObj[id].mPos.y		= y;
	mObj[id].mSize.cx	= w;
	mObj[id].mSize.cy	= h;
	mObj[id].mOffset.x	= ox;
	mObj[id].mOffset.y	= oy;
	mObj[id].fAlpha		= 1.0f;
	mObj[id].fScaleX	= 1.0f;
	mObj[id].fScaleY	= 1.0f;
	mObj[id].fRot		= 0.0f;
	return TRUE;
}

BOOL CDDPro90::SetPutStatus( int id,float alpha,float scale,float rot )
{
	if( id<0 || id>CDDMAXOBJECT-1 )
		return FALSE;

	// セット
	mObj[id].fAlpha		= alpha;
	mObj[id].fScaleX	= scale;
	mObj[id].fScaleY	= scale;
	mObj[id].fRot		= rot;
	return TRUE;
}

BOOL CDDPro90::SetPutStatusEx( int id,float alpha,float scale_x,float scale_y,float rot )
{
	if( id<0 || id>CDDMAXOBJECT-1 )
		return FALSE;

	// セット
	mObj[id].fAlpha		= alpha;
	mObj[id].fScaleX	= scale_x;
	mObj[id].fScaleY	= scale_y;
	mObj[id].fRot		= rot;
	return TRUE;
}


BOOL CDDPro90::DrawBegin( BOOL clear )
{
	// ロスト中なら
	if( bLost )
		return FALSE;

	if( !lpD3DDevice )
		return FALSE;

	if( !bBegin ) {
		if( clear )
			lpD3DDevice->Clear( 0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,dwBackColor,1.0f,0 );
		if( FAILED(lpD3DDevice->BeginScene()) ) {
			DEBUG( "BeginScene失敗\n" );
			return FALSE;
		}
		bBegin = TRUE;
	}

	// ライトはオフで
	lpD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);				// アルファブレンド使用可能
	lpD3DDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );		// 転送元のブレンド設定
	lpD3DDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );	// 転送先ブレンド設定
	lpD3DDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );
	lpD3DDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );

	if( bRightHand )
		lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );			// 右回りを消去(右手系)
	else
		lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );			// 左回りを消去(左手系)

	lpD3DDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );
	return TRUE;
}

BOOL CDDPro90::DrawEnd( void )
{
	if( !bBegin )
		return FALSE;

	// シーン終了
	lpD3DDevice->EndScene();
	bBegin = FALSE;

	// ウインドウモードでVSYNCチェックが出来るハードなら
	if( !bFullScreen && bVSync && mD3DCaps.Caps&D3DCAPS_READ_SCANLINE ) {
		// Vsync開始待ち
		D3DRASTER_STATUS rs;
		while( SUCCEEDED(lpD3DDevice->GetRasterStatus(0,&rs)) ) {
			if( rs.InVBlank )
				break;				// 垂直帰線消去中なら抜ける
			Sleep(0);
		}
	}

	// フリップ
	HRESULT ret = lpD3DDevice->Present( NULL,NULL,NULL,NULL );
	if( FAILED(ret) ) {
		DEBUG( "ロスト\n" );
		bLost = TRUE;
		bLostFirst = FALSE;
		return FALSE;
	}

	// ウインドウモードでVSYNCチェックが出来るハードなら
	// ※超高性能なグラフィックボードではVSYNC中に何フレームも描画出来てしまうため、
	//   ここでは1フレーム分のVSYNCが確実に終わるのを待つ
	if( !bFullScreen && bVSync && mD3DCaps.Caps&D3DCAPS_READ_SCANLINE ) {
		// Vsync終了待ち
		D3DRASTER_STATUS rs;
		while( SUCCEEDED(lpD3DDevice->GetRasterStatus(0,&rs)) ) {
			if( !rs.InVBlank )
				break;				// 垂直帰線消去終了なら抜ける
			Sleep(0);
		}
	}

	return TRUE;
}

BOOL CDDPro90::CheckDevice( void )
{
	HRESULT ret = lpD3DDevice->TestCooperativeLevel();
	switch( ret )
	{
	case S_OK:
		break;

	case D3DERR_DEVICELOST:
		DEBUG( "D3DERR_DEVICELOST\n" );
		bLost = TRUE;
		if( !bLostFirst ) {
			// １度はLostとする
			DEBUG( "初回ロスト\n" );
			bLostFirst = TRUE;
			break;
		}
		break;

	case D3DERR_DEVICENOTRESET:
		DEBUG( "D3DERR_DEVICENOTRESET\n" );
		bLost = TRUE;
		if( !bLostFirst ) {
			// １度はLostとする
			DEBUG( "初回ロスト\n" );
			bLostFirst = TRUE;
			break;
		}

		// バックバッファフォーマットのセット
		SetBackBufferFormat();

		// リセット
		ret = lpD3DDevice->Reset( &mParam );
		if( FAILED(ret) ) {
			switch( ret )
			{
			case D3DERR_DEVICELOST:
				DEBUG( "リセット失敗 (D3DERR_DEVICELOST)\n" );
				break;
			case D3DERR_DEVICENOTRESET:
				DEBUG( "リセット失敗 (D3DERR_DEVICENOTRESET)\n" );
				break;
			case D3DERR_NOTAVAILABLE:
				DEBUG( "リセット失敗 (D3DERR_NOTAVAILABLE)\n" );
				break;
			default:
				DEBUG( "リセット失敗 (%08X)\n",ret );
				break;
			}
			break;
		}

		// 標準のレンダーステートとテクスチャステートをセット
		lpD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );							// ライティング無効
		lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);					// アルファブレンド使用可能
		lpD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );				// 転送元のブレンド設定
		lpD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );			// 転送先ブレンド設定

																						//テクスチャのアルファに、ポリゴン色をアルファとして加味する設定
		lpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		lpD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		lpD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);								// 2D用にZバッファを使用しない

		//if( bRightHand )
		//	lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );					// 右回りを消去(右手系)
		//else
		//	lpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );					// 左回りを消去(左手系)

		// 再構築完了なら
		bLost		= FALSE;
		bLostFirst	= FALSE;
		DEBUG( "リストア完了\n" );
		break;

	case D3DERR_NOTAVAILABLE:
		DEBUG( "D3DERR_NOTAVAILABLE\n" );
		bLost = TRUE;
		break;

	default:
		DEBUG( "TestCooperativeLevel RESULT=[%08X]\n",ret );
		break;
	}

	// ロストなら
	if( bLost )
		return FALSE;

	return TRUE;
}

BOOL CDDPro90::Put( int obj,float x,float y )
{
	return PutEx( obj,
				  x,
				  y,
				  (float)mObj[obj].mOffset.x,
				  (float)mObj[obj].mOffset.y );
}

BOOL CDDPro90::Put2( int obj,float x,float y )
{
	return PutEx( obj,
				  x,
				  y,
				  (float)mObj[obj].mSize.cx / 2,			// 中心X座標
				  (float)mObj[obj].mSize.cy / 2 );			// 中心Y座標
}

BOOL CDDPro90::PutEx( int obj,float x,float y,float mx,float my )
{
	if( bLost )
		return FALSE;
	if( !lpD3DDevice )
		return FALSE;
	if( obj<0 || obj>CDDMAXOBJECT-1 )
		return FALSE;
	if( !mObj[obj].bFlag )
		return FALSE;

	// シーンがスタートしてなければする
	if( !bBegin ) {
		if( !DrawBegin() )
			return FALSE;
	}

	// テクスチャ変更
	lpD3DDevice->SetTexture( 0,tex[mObj[obj].iTexID].GetTexture() );

	float w,h;
	w = (float)mObj[obj].mSize.cx;
	h = (float)mObj[obj].mSize.cy;

	int i;
	CDDTLVERTEX v[4];
	ZeroMemory( &v,sizeof(v) );
	if( bRightHand ) {
		v[0].x = v[1].x		= -mx;
		v[2].x = v[3].x		= v[0].x + w;
		v[0].y = v[2].y		= -my;
		v[1].y = v[3].y		= v[0].y + h;
		v[0].tu = v[1].tu	= (mObj[obj].mPos.x                    )   / (float)tex[ mObj[obj].iTexID ].GetWidth();
		v[2].tu = v[3].tu	= (mObj[obj].mPos.x + mObj[obj].mSize.cx)  / (float)tex[ mObj[obj].iTexID ].GetWidth();
		v[0].tv = v[2].tv	= (mObj[obj].mPos.y                    )   / (float)tex[ mObj[obj].iTexID ].GetHeight();
		v[1].tv = v[3].tv	= (mObj[obj].mPos.y + mObj[obj].mSize.cy)  / (float)tex[ mObj[obj].iTexID ].GetHeight();
	} else {
		v[0].x = v[2].x		= -mx;
		v[1].x = v[3].x		= v[0].x + w;
		v[0].y = v[1].y		= -my;
		v[2].y = v[3].y		= v[0].y + h;
		v[0].tu = v[2].tu	= (mObj[obj].mPos.x                    )   / (float)tex[ mObj[obj].iTexID ].GetWidth();
		v[1].tu = v[3].tu	= (mObj[obj].mPos.x + mObj[obj].mSize.cx)  / (float)tex[ mObj[obj].iTexID ].GetWidth();
		v[0].tv = v[1].tv	= (mObj[obj].mPos.y                    )   / (float)tex[ mObj[obj].iTexID ].GetHeight();
		v[2].tv = v[3].tv	= (mObj[obj].mPos.y + mObj[obj].mSize.cy)  / (float)tex[ mObj[obj].iTexID ].GetHeight();
	}

	for( i=0;i<4;i++ ) {
		v[i].color = 0x00ffffff | ((DWORD)(mObj[obj].fAlpha*255)<<24);
		v[i].rhw = 1.0f;
		CalcScaleRotate( &v[i].x,&v[i].y,mObj[obj].fScaleX,mObj[obj].fScaleY,-mObj[obj].fRot );
		v[i].x += x + fOffset;
		v[i].y += y + fOffset;
	}

	// デフォルトステートのセット
	lpD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);							// ライティング無効
	lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);					// アルファブレンド使用可能
	lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);				// 転送元のブレンド設定
	lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);			// 転送先ブレンド設定
	if (bRightHand)
		lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);					// 右回りを消去(右手系)
	else
		lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);					// 左回りを消去(左手系)

	lpD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	lpD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


	lpD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	lpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	lpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	lpD3DDevice->SetFVF( CDDFVF_TLVERTEX );
	lpD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP,2,v,sizeof(CDDTLVERTEX) );

	return TRUE;
}

BOOL CDDPro90::SetBlendOne( BOOL on )
{
	if( !lpD3DDevice )
		return FALSE;

	if( on ) {
		lpD3DDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ONE );
	} else {
		lpD3DDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	}

	return TRUE;
}

BOOL CDDPro90::SetRenderState( D3DRENDERSTATETYPE type,DWORD flag )
{
	if( !lpD3DDevice )
		return FALSE;
	if( FAILED(lpD3DDevice->SetRenderState(type,flag)) )
		return FALSE;

	return TRUE;
}

BOOL CDDPro90::SetTextureStageState( DWORD st,D3DTEXTURESTAGESTATETYPE type,DWORD val )
{
	if( !lpD3DDevice )
		return FALSE;
	if( FAILED(lpD3DDevice->SetTextureStageState(st,type,val)) )
		return FALSE;

	return TRUE;
}
