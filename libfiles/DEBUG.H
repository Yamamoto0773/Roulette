#ifndef __DEBUG_H
#define __DEBUG_H
///////////////////////////////////////////////////////////////////////////////////
// DEBUG : デバッグ出力ライブラリ v2.01                                          //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>

//#undef DEBUGMODE						// コメントをはずすと全ての出力をキャンセル

#define DEBUG_MAXTEXTBUF		2048	// ODSX()で出力される最大サイズ(これ以上は"..."となって切り捨て)


#undef DEBUG

#ifdef DEBUGMODE
#define INITDEBUG()			dbgInitA()
#define DEBUG(x,...)		dbgDebugA(x,__VA_ARGS__)
#define ODS(x,...)			dbgOutputDebugStringA(x,__VA_ARGS__)
#define CLEARDEBUG()		dbgClear()
#else
#define INITDEBUG(...)
#define DEBUG(x,...)
#define ODS(x,...)
#define CLEARDEBUG()
#endif



#ifdef __cplusplus
extern "C" {
#endif

// 公開関数
extern void dbgInitA( const char *file="DEBUG.txt" );
extern void dbgDebugA( const char *s,... );
extern void dbgClear( void );
extern void dbgOutputDebugStringA( const char *s,... );

#ifdef __cplusplus
}
#endif

#endif

