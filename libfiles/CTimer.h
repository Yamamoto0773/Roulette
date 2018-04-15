#ifndef _CTIMER_H
#define _CTIMER_H
///////////////////////////////////////////////////////////////////////////////////
// CTimer : タイマー処理クラス v2.00                                             //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>


class CTimer {
protected:
	BOOL			bPerf;			// 高解像度タイマーがサポートされているか
	LARGE_INTEGER	mFreq;			// システム周波数
	LARGE_INTEGER	mStart;			// 開始時間
	int				iFps;			// 割り込みタイミング
	int				iCount;			// 割り込みカウント数
public:
	CTimer();
	virtual ~CTimer();
	BOOL Start( int fps=60 );		// 指定のFPSで割り込みスタート
	int Run( void );				// 割り込みがあれば1以上を返す（割り込み回数）
};

#endif
