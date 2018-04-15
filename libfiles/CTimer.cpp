#include "CTimer.h"

CTimer::CTimer()
{
	if( QueryPerformanceFrequency(&mFreq) ) {
		// 高解像度カウンターに対応
		bPerf	= TRUE;
	} else {
		// 高解像度カウンターに非対応
		mFreq.QuadPart	= 1000LL;		// timeGetTime()で代用
		bPerf	= FALSE;
	}
	iFps = 60;
}

CTimer::~CTimer()
{

}

BOOL CTimer::Start( int fps )
{
	iFps	= fps;
	iCount	= 0;

	if( bPerf ) {
		QueryPerformanceCounter( &mStart );
	} else {
		mStart.QuadPart = (LONGLONG)timeGetTime();
	}
	return TRUE;
}

int CTimer::Run( void )
{
	LARGE_INTEGER now;
	if( bPerf ) {
		QueryPerformanceCounter( &now );
	} else {
		now.QuadPart = (LONGLONG)timeGetTime();
	}

	int count = (int)((now.QuadPart - mStart.QuadPart) / (mFreq.QuadPart/iFps) );
	if( count!=iCount ) {
		int ret = count - iCount;
		iCount = count;
		return ret;
	}

	return 0;
}
