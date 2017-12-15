#ifndef _DXTEXTANSI_H
#define _DXTEXTANSI_H

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <stdio.h>
#include <strsafe.h>
#include <locale.h>


#include "DirectXTextClass.h"

#define CHARACTERKIND		(0x7E - 0x20 +1)
#define MAXCHARACTER		256


#define SAFE_FREE(x)		{ if(x){free(x); x=NULL;} }
#define SAFE_RELEASE(x)		{ if(x){x->Release(); x=NULL;} }


class DXTextANSI {
private:
	LPDIRECT3DDEVICE9		lpDev;

	IDirect3DVertexBuffer9	*lpVertexBuffer;
	ID3DXEffect				*lpEffect;			// �V�F�[�_�Ǘ��p
	IDirect3DVertexDeclaration9	*lpDecl;		// ���_�錾�쐬

	int						iDrawWidth;
	int						iDrawHeight;
	int						iFontSize;

	
	IDirect3DTexture9		*lpFontTex[CHARACTERKIND];

	CHARDATA				mCharData[CHARACTERKIND];

public:
	DXTextANSI();
	~DXTextANSI();

	// DXTextANSI�������@���`��O�ɕK���Ăяo��
	BOOL	Init(LPDIRECT3DDEVICE9 dev, int drawWidth, int drawHeight);
	// �e�N�X�`�������@ANSI�����̃e�N�X�`�����쐬�@���`��O�ɌĂяo��
	BOOL	Create(int fontSize, int fontWeight, WCHAR *fontName, bool italic);
	// �����`��
	BOOL	Draw(int x, int y, int fontSize, int charInterval, DWORD color, const char *s, ...);
	// �^����ꂽ�̈���֕����`��
	BOOL	DrawInRect(RECT *rect, int fontSize, int charInterval, DWORD format, DWORD color, const char *s, ...);
	// �t�H���g�e�N�X�`�����J������@(�f�X�g���N�^�Ŏ����I�ɌĂяo����܂��B�����I�ɌĂяo���K�v�͂���܂���
	BOOL	Clear();


	//�@RGBA��DWORD�^�ɕϊ�����֐��B�F��4�����Ŏw�肵�����Ƃ��Ɏg���܂�
	DWORD	ConvertFromRGBA(int red, int green, int blue, int alpha = 255);

private:

	// �^����ꂽ�`��̈�ƃt�H�[�}�b�g����A�ŏI�I�ȕ`����W���v�Z
	int	CalcTextPosition(RECT *rect, float inScale, int charInterval, DWORD format, const char *s, POINT *pt, float *outScale);
	// ���ۂɕ`���S������֐�
	int		DrawTEXT(RECT *rect, int fontSize, int charInterval, DWORD format, DWORD color, const char *s, va_list arg);
	// ��������œK������ (�߂�l�͍œK����̕�����)
	int		OptimizeString(char *dst, const char *src);

};


#endif // !_DIRECTXFONT_H