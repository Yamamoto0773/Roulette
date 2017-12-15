#pragma once

#include <windows.h>


typedef struct _Vtx {
	float x, y, z;
	float u, v;
}Vtx;


typedef struct _FONTSTATUS {
	int		iFontSize;		// �����̑傫��
	int		iFontWeight;	// �����̑���
	WCHAR	wcFontName[32];	// �t�H���g��
	int		iAntiAliasing;	// �A���`�G�C���A�X�̃��x��
	bool	bItalic;		// �C�^���b�N�̃t�H���g
	bool	bUnderline;		// �����t���t�H���g
	bool	bStruckOut;		// �ł��������t���t�H���g
}FONTSTATUS;


typedef struct _CHARDATA {
	int iWidth;			// �����̕�
	int iHeight;		// �����̍���
	int iAreaWidth;		// �����̕��@(�󔒂���
	int iAreaHeight;	// �����̍���(�󔒂���
	int iOriginX;		// �����̌��_��x���W
	int iOriginY;		// �����̌��_��y���W
}CHARDATA;


// �����̈ʒu�Ɋւ���w��
typedef enum _TEXTALIGN {
	TEXTALIGN_LEFT		= 0b00000001,	// ����
	TEXTALIGN_CENTERX	= 0b00000010,	// ������
	TEXTALIGN_RIGHT		= 0b00000100,	// �E��
	TEXTALIGN_CENTERXY	= 0b00001000,	// ���S�񂹁@��1�s�̂�
	TEXTALIGN_NONE		= 0b00000000,	// �w��Ȃ��@(�`��̈�͖�������A�܂�Ԃ�����܂���)
}TEXTALIGN;


// �����̊g��k���Ɋւ���w��
typedef enum _TEXTSCALE {
	TEXTSCALE_AUTOX		= 0b00010000,	// �`��̈�ɍ��킹�ĉ������ɏk�� (�͂ݏo���ꍇ�̂�) ��1�s�̂�
	TEXTSCALE_AUTOXY	= 0b00100000,	// �`��̈�ɍ��킹�ďc������ێ������܂܏k�� (�͂ݏo���ꍇ�̂�) ��1�s�̂�
	TEXTSCALE_NONE		= 0b00000000,	// �w��Ȃ�
}TEXTSCALE;