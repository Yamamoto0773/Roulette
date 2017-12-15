#pragma warning( disable : 4996 )
#include "DEBUG.H"
#include <stdio.h>


// 同期オブジェクト
class CDbgCritialSection {
	CRITICAL_SECTION mCS;
public:
	CDbgCritialSection() {
		InitializeCriticalSection( &mCS );
	}
	virtual ~CDbgCritialSection() {
		DeleteCriticalSection( &mCS );
	}
	void Enter( void ) {
		EnterCriticalSection( &mCS );
	}
	void Leave( void ) {
		LeaveCriticalSection( &mCS );
	}
};



static CDbgCritialSection	cs;
static BOOL					bInit = FALSE;
static char					mFileName[MAX_PATH] = "";




// 現在の日時を文字列で返す(同期必須)
const char *GetNow( void )
{
	static char mNowStr[256] = "";
	SYSTEMTIME	st;
	GetLocalTime( &st );
	sprintf( mNowStr,"%04d/%02d/%02d %02d:%02d:%02d.%03d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds );
	return mNowStr;
}


void dbgInitA( const char *file )
{
	cs.Enter();
	if( !bInit ) {
		// 未初期化なら初期化
		strcpy( mFileName,file );
		DeleteFileA( mFileName );
		bInit = TRUE;
	}
	cs.Leave();
}

void dbgDebugA( const char *s,... )
{
	cs.Enter();

	if( bInit ) {
		char *str = NULL;
		va_list ap;
		va_start( ap,s );

		int len = _vscprintf( s,ap );
		str = (CHAR*)malloc( sizeof(CHAR)*(len+1) );
		if( str ) {
			vsprintf( str,s,ap );
			// ファイル書き出し
			FILE *fp;
			fp = fopen( mFileName,"a" );
			if( fp ) {
				fprintf( fp,"[%s] %s",GetNow(),str );
				fclose( fp );
			}
			free( str );
		}
		va_end( ap );
	}

	cs.Leave();
}


void dbgClear( void )
{
	cs.Enter();

	if( bInit ) {
		char s[1024];
		ZeroMemory( s,sizeof(s) );
		sprintf( s,"[%s] << Log Clear >>\n",GetNow() );

		// ファイル書き出し
		FILE *fp;
		fp = fopen( mFileName,"a" );
		if( fp ) {
			fputs( s,fp );
			fclose( fp );
		}
	}

	cs.Leave();
}

void dbgOutputDebugStringA( const char *s,... )
{
	cs.Enter();

	va_list ap;
	va_start( ap,s );

	int len = _vscprintf( s,ap );
	char *str = (CHAR*)malloc( sizeof(char)*(len+1) );
	if( str ) {
		vsprintf( str,s,ap );
		size_t len2 = strlen(str);
		if( len2>=DEBUG_MAXTEXTBUF ) {
			str[DEBUG_MAXTEXTBUF-4] = NULL;
			strcat( str,"...\n" );
		}
		OutputDebugStringA( str );
		free( str );
	}
	va_end( ap );

	cs.Leave();
}
