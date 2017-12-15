#include "DXTextANSI.h"

DXTextANSI::DXTextANSI() {
	lpDev = NULL;
	lpVertexBuffer = NULL;
	lpEffect = NULL;
	lpDecl = NULL;

	iDrawWidth = 640;
	iDrawHeight = 480;
	iFontSize = 0;

	ZeroMemory(mCharData, sizeof(mCharData));
	for (int i=0; i<CHARACTERKIND; i++) {
		lpFontTex[i] = NULL;
	}

}

DXTextANSI::~DXTextANSI() {
	Clear();

	SAFE_RELEASE(lpVertexBuffer);
	SAFE_RELEASE(lpEffect);
	SAFE_RELEASE(lpDecl);
}


BOOL DXTextANSI::Init(LPDIRECT3DDEVICE9 dev, int drawWidth, int drawHeight) {
	if (!dev)
		return FALSE;
	if (drawWidth<=0 || drawHeight<=0)
		return FALSE;

	// ���P�[������{�ɐݒ�
	setlocale(LC_CTYPE, "jpn");

	lpDev		= dev;
	iDrawWidth	= drawWidth;
	iDrawHeight	= drawHeight;


	// �|���S������
	HRESULT hr;
	hr = lpDev->CreateVertexBuffer(sizeof(Vtx)*4, 0, 0, D3DPOOL_MANAGED, &lpVertexBuffer, 0);
	if (FAILED(hr))
		return FALSE;

	// �P�ʃt�H���g�|���S���쐬
	Vtx vtx[4] ={
		{ 0.0f, -1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f,  0.0f, 1.0f, 0.0f, 0.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f,  0.0f, 1.0f, 1.0f, 0.0f },
	};
	Vtx *p = NULL;

	lpVertexBuffer->Lock(0, sizeof(Vtx)*4, (void**)&p, 0);
	memcpy(p, vtx, sizeof(Vtx)*4);
	lpVertexBuffer->Unlock();


	// �V�F�[�_�쐬
	ID3DXBuffer	*error=NULL;
	if (FAILED(D3DXCreateEffectFromFile(lpDev, L"libfiles/sprite.fx", 0, 0, 0, 0, &lpEffect, &error))) {
		OutputDebugStringA((const char*)error->GetBufferPointer());
		return FALSE;
	}


	// ���_�錾�쐬
	D3DVERTEXELEMENT9 elems[] ={
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},				// ���_�ʒu (x, y, z)
		{0, sizeof(float)*3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},	// �e�N�X�`�����W
		D3DDECL_END()
	};
	dev->CreateVertexDeclaration(elems, &lpDecl);
	if (!lpDecl)
		return FALSE;


	return TRUE;
}


BOOL DXTextANSI::Create(int fontSize, int fontWeight, WCHAR * fontName, bool italic) {
	if (!lpDev)
		return FALSE;

	Clear();

	// �t�H���g�̐���
	LOGFONT	lf;
	lf.lfHeight				= fontSize;						// �����̍���
	lf.lfWidth				= 0;							// ������
	lf.lfEscapement			= 0;							// ����������X���Ƃ̊p�x
	lf.lfOrientation		= 0;							// �e������X���Ƃ̊p�x
	lf.lfWeight				= fontWeight;					// ����
	lf.lfItalic				= italic;						// �C�^���b�N��
	lf.lfUnderline			= FALSE;						// ����
	lf.lfStrikeOut			= FALSE;						// �ł�������
	lf.lfCharSet			= ANSI_CHARSET;					// �L�����N�^�Z�b�g
	lf.lfOutPrecision		= OUT_DEFAULT_PRECIS;			// �o�͐��x
	lf.lfClipPrecision		= CLIP_DEFAULT_PRECIS;			// �N���b�s���O�̐��x
	lf.lfQuality			= PROOF_QUALITY;				// �o�͕i��
	lf.lfPitchAndFamily		= DEFAULT_PITCH | FF_MODERN;	// �s�b�`�ƃt�@�~��
	StringCchCopy(lf.lfFaceName, 32, fontName);	// �t�H���g��

	HFONT hFont = CreateFontIndirect(&lf);
	if (hFont == NULL) {
		DeleteObject(hFont);
		return FALSE;
	}

	iFontSize = fontSize;


	// �f�o�C�X�ɑI�������t�H���g��ݒ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);


	///// �����e�N�X�`���쐬 //////
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	CONST MAT2 mat ={ { 0,1 },{ 0,0 },{ 0,0 },{ 0,1 } };


	for (int i=0; i<CHARACTERKIND; i++) {
		DWORD size = 0;
		UINT code = 0x20+i;
		GLYPHMETRICS gm;


		// �����̃O���t�r�b�g�}�b�v���擾
		if (0x20+i == ' ') code = '0';	// �󔒕����͐��������̑傫���ɂ���(�ϕ��t�H���g�΍�)

		if ((size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &gm, 0, NULL, &mat)) == GDI_ERROR) {
			SelectObject(hdc, oldFont);			//  ���̃t�H���g�ɖ߂�
			ReleaseDC(NULL, hdc);
			DeleteObject(hFont);
			Clear();
			return FALSE;
		}
		BYTE *pMono = new BYTE[size];
		GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &gm, size, pMono, &mat);

		if (0x20+i == ' ') code = ' ';


		// ��������ۑ�
		mCharData[i].iWidth			= (gm.gmBlackBoxX + 3) / 4 * 4;
		mCharData[i].iHeight		= gm.gmBlackBoxY;
		mCharData[i].iAreaWidth		= gm.gmCellIncX;
		mCharData[i].iAreaHeight	= tm.tmHeight;
		mCharData[i].iOriginX		= gm.gmptGlyphOrigin.x;
		mCharData[i].iOriginY		= gm.gmptGlyphOrigin.y-tm.tmAscent;		// ���㌴�_

		// �e�N�X�`������
		lpDev->CreateTexture(mCharData[i].iWidth, mCharData[i].iHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &lpFontTex[i], NULL);

		// �e�N�X�`���Ƀt�H���g�r�b�g�}�b�v����������
		D3DLOCKED_RECT lockedRect;
		lpFontTex[i]->LockRect(0, &lockedRect, NULL, 0);  // ���b�N
		DWORD *texBuf = (DWORD*)lockedRect.pBits;   // �e�N�X�`���������ւ̃|�C���^

		for (int y = 0; y < mCharData[i].iHeight; y++) {
			for (int x = 0; x < mCharData[i].iWidth; x++) {
				DWORD alpha;
				if (code == ' ')	// �X�y�[�X�̏ꍇ�͉����`�悵�Ȃ�
					alpha = 0;
				else
					alpha = pMono[y * mCharData[i].iWidth + x] * 255 / 16;	// 16�K����256�K���ɕϊ�

				texBuf[y * mCharData[i].iWidth + x] = (alpha << 24) & 0xff000000;
			}
		}

		lpFontTex[i]->UnlockRect(0);  // �A�����b�N


		delete[] pMono;
	}

	SelectObject(hdc, oldFont);			// ���̃t�H���g�ɖ߂�
	ReleaseDC(NULL, hdc);
	DeleteObject(hFont);				// ����̃t�H���g������

	return TRUE;
}


BOOL DXTextANSI::Draw(int x, int y, int fontSize, int charInterval, DWORD color, const char * s, ...) {
	BOOL result;
	RECT rect ={ x, y, 0, 0 };

	va_list list;
	va_start(list, s);
	result = DrawTEXT(&rect, fontSize, charInterval, TEXTALIGN_NONE|TEXTSCALE_NONE, color, s, list);
	va_end(list);

	return result;
}


BOOL DXTextANSI::DrawInRect(RECT * rect, int fontSize, int charInterval, DWORD format, DWORD color, const char * s, ...) {
	BOOL result;

	va_list list;
	va_start(list, s);
	result = DrawTEXT(rect, fontSize, charInterval, format, color, s, list);
	va_end(list);

	return result;
}


BOOL DXTextANSI::Clear() {

	ZeroMemory(mCharData, sizeof(mCharData));
	for (int i=0; i<CHARACTERKIND; i++) {
		SAFE_RELEASE(lpFontTex[i]);
	}

	return TRUE;
}


DWORD DXTextANSI::ConvertFromRGBA(int red, int green, int blue, int alpha) {
	if (red > 255)		red = 255;
	if (red < 0)		red = 0;
	if (green > 255)	green = 255;
	if (green < 0)		green = 0;
	if (blue > 255)		blue = 255;
	if (blue < 0)		blue = 0;
	if (alpha > 255)	alpha = 255;
	if (alpha < 0)		alpha = 0;

	DWORD color = 0x00000000;

	color = alpha << 24 | red << 16 | green << 8 | blue;

	return color;
}




int DXTextANSI::CalcTextPosition(RECT * rect, float inScale, int charInterval, DWORD format, const char * s, POINT * pt, float *outScale) {
	if (!pt)
		return FALSE;
	if (!s)
		return FALSE;
	if (!rect)
		return FALSE;
	if (!outScale)
		return FALSE;

	// �t�H�[�}�b�g�̑g�ݍ��킹���`�F�b�N
	if ((format&0x0F) == TEXTALIGN_NONE) {
		if ((format&0xF0) == TEXTSCALE_AUTOX || (format&0xF0) == TEXTSCALE_AUTOXY)
			return FALSE;
	}

	int charCnt = 0;

	// �̈�̏c���̒��������߂�
	int rectX = rect->right - rect->left;
	int rectY = rect->bottom - rect->top;


	int lineHead = 0;	// �v�Z�Ώۂł���s�́A�擪�����̔ԍ�
	int lineEnd = 0;	// �v�Z�Ώۂł���s�́A�Ō�̕����̔ԍ�
	float offsetX = 0;
	float offsetY = 0;
	float scaleX = 1.0f;
	float scaleY = 1.0f;


	while (lineHead < strlen(s)) {
		float lineLen = 0.0f;
		float lineHeight = mCharData[0].iAreaHeight*inScale;

		// �c�����ɂ͂ݏo������
		if (offsetY + lineHeight > rectY) {
			if ((format&0x0F) != TEXTALIGN_NONE && (format&0xF0) != TEXTSCALE_AUTOXY) {
				break;
			}
		}


		//// �s�̒����̎Z�o ////
		bool canPut = false;
		int i = -1;
		while ((lineHead+ ++i) < strlen(s)) {

			// �����R�[�h�擾
			UINT code = (UINT)s[lineHead +i];

			// ���s�Ȃ�s�̏I���Ƃ���
			if (code == (UINT)'\n') {
				i++;
				canPut = true;
				break;
			}

			// �������ɂ͂ݏo������
			if (lineLen + mCharData[code-0x20].iAreaWidth*inScale > rectX) {
				if ((format&0x0F) != TEXTALIGN_NONE && (format&0xF0) == TEXTSCALE_NONE) {
					break;
				}
			}

			lineLen += mCharData[code-0x20].iAreaWidth*inScale + charInterval;
			canPut = true;
		}

		if (!canPut)	// 1�������z�u�ł��Ȃ��ꍇ�͏I��
			break;

		lineEnd = lineHead + i-1;
		lineLen -= charInterval;	// �s���̋󔒂��폜


									// �k���{���v�Z
		if ((format&0xF0) != TEXTSCALE_NONE) {
			if (lineLen > rectX)
				scaleX = rectX/lineLen;
			if (lineHeight > rectY)
				scaleY = rectY/lineHeight;

			if ((format&0xF0) == TEXTSCALE_AUTOXY)
				*outScale = (scaleX < scaleY) ? scaleX : scaleY; // ���������ɍ��킹�ďc������ێ�
			else
				*outScale = scaleX;

			lineLen *= (*outScale);
			lineHeight *= (*outScale);
		}
		else {
			*outScale = 1.0f;
		}


		// �s���̕`��ʒu�̎Z�o
		switch ((format&0x0F)) {
			case TEXTALIGN_CENTERX:
				offsetX = (rectX - lineLen)/2;
				break;
			case TEXTALIGN_CENTERXY:
				offsetX = (rectX - lineLen)/2;
				offsetY = (rectY - lineHeight)/2;
				break;
			case TEXTALIGN_RIGHT:
				offsetX = rectX - lineLen;
				break;
			case TEXTALIGN_LEFT:
			case TEXTALIGN_NONE:
				offsetX = 0;
		}


		// �`��ʒu�̕ۑ�
		for (charCnt=lineHead; charCnt<=lineEnd; charCnt++) {
			// �����R�[�h�擾
			UINT code = (UINT)s[charCnt];

			if (code == (UINT)'\n')
				continue;

			pt[charCnt].x = rect->left + (int)offsetX;
			pt[charCnt].y = rect->top + (int)offsetY;

			offsetX += (mCharData[code-0x20].iAreaWidth*inScale + charInterval)*(*outScale);
		}


		// ���s����
		if ((format&0xF0) == TEXTSCALE_AUTOX ||
			(format&0xF0) == TEXTSCALE_AUTOXY ||
			(format&0x0F) == TEXTALIGN_CENTERXY)		// 1�s�݂̂̃t�H�[�}�b�g�͂����ŏI��
			break;

		offsetX = 0;
		offsetY += lineHeight;

		lineHead = lineEnd+1;	// �s���������X�V
	}

	return charCnt;
}



int DXTextANSI::DrawTEXT(RECT * rect, int fontSize, int charInterval, DWORD format, DWORD color, const char * s, va_list arg) {
	if (!lpDev)
		return FALSE;
	if (!lpVertexBuffer)
		return FALSE;
	if (!lpEffect)
		return FALSE;
	if (!lpDecl)
		return FALSE;
	if (!rect)
		return FALSE;
	if (!lpFontTex[0])
		return FALSE;


	// �ϒ��������X�g�ɏ]���Ēʏ�̕�����ɕϊ�
	int length;
	char tmp[MAXCHARACTER+1];
	vsnprintf(tmp, MAXCHARACTER+1, s, arg);

	// ������̍œK��
	char str[MAXCHARACTER+1];
	length = OptimizeString(str, tmp);


	// �`��ʒu�̎Z�o
	POINT *pt = NULL;
	pt = (POINT*)malloc(sizeof(POINT)*(length));
	if (!pt)
		return FALSE;

	float tmpScale = (float)fontSize/iFontSize;
	float scale;
	int charCnt = CalcTextPosition(rect, tmpScale, charInterval, format, str, pt, &scale);
	if (charCnt < 0)
		return FALSE;


	// �`��{���ݒ�
	float scaleX, scaleY;
	if ((format&0xF0) == TEXTSCALE_AUTOX)
		scaleX = tmpScale*scale, scaleY = tmpScale;
	else if ((format&0xF0) == TEXTSCALE_AUTOXY)
		scaleX = tmpScale*scale, scaleY = tmpScale*scale;
	else
		scaleX = tmpScale, scaleY = tmpScale;


	// �w��F��float�^�z��ɕϊ�
	float colorRGBA[4]={
		(color>>16 & 0x000000ff) / 255.0f,
		(color>>8 & 0x000000ff) /255.0f,
		(color & 0x000000ff) / 255.0f,
		(color>>24 & 0x000000ff) / 255.0f
	};


	//-------------------�`��ݒ�---------------------------------------
	// ���C�g�̓I�t��
	lpDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	// ���u�����h�ݒ�
	lpDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	lpDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	lpDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//�e�N�X�`���̃A���t�@�𓧖��x���g�p����ݒ�
	lpDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	lpDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	lpDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	lpDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	lpDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	lpDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);					// ����n

	lpDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);			// �e�N�X�`�����͂ݏo�����ɕ\�����Ȃ��ɂ���
	lpDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	lpDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	lpDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//-------------------------------------------------------------------


	D3DXMATRIX worldOffset;						// �|���S���̔z�u���W
	D3DXMATRIX localScale;						// �|���S���̑傫�� (�g��{��
	D3DXMATRIX localOffset;						// �|���S���̌��_���W
	D3DXMATRIX localMat;
	D3DXMATRIX world;

	// ������W�n���ˉe�s���ݒ�
	D3DXMATRIX ortho;
	D3DXMatrixOrthoLH(&ortho, (float)iDrawWidth, (float)iDrawHeight, 0.0f, 1000.0f);
	lpDev->SetTransform(D3DTS_PROJECTION, &ortho);


	//// �����`�� /////
	lpDev->SetVertexDeclaration(lpDecl);
	lpDev->SetStreamSource(0, lpVertexBuffer, 0, sizeof(Vtx));
	lpEffect->SetTechnique("BasicTech");

	// �V�F�[�_�J�n
	UINT numPass = 0;
	lpEffect->Begin(&numPass, 0);

	for (int i=0; i<charCnt; i++) {

		// �`�悷�镶���̕����R�[�h�擾
		UINT code = (UINT)str[i];

		if (code == (UINT)'\n')
			continue;

		if (lpFontTex[code-0x20] == NULL)
			continue;	// �e�N�X�`�������݂��Ȃ��Ȃ�X�L�b�v

		// �X�N���[���̍���������_�Ƃ������W�ɕϊ�
		pt[i].x -= iDrawWidth / 2;
		pt[i].y -= iDrawHeight / 2;
		pt[i].y *= -1;


		// ���[���h�r���[�ˉe�ϊ��s����쐬
		D3DXMatrixScaling(&localScale, (float)mCharData[code-0x20].iWidth*scaleX, (float)mCharData[code-0x20].iHeight*scaleY, 1.0f);				// �|���S���𕶎��̑傫���ɂ���
		D3DXMatrixTranslation(&localOffset, (float)mCharData[code-0x20].iOriginX*scaleX, (float)mCharData[code-0x20].iOriginY*scaleY, 0.0f);		// �|���S���𕶎��̌��_�Ɉړ�
		D3DXMATRIX localMat = localScale*localOffset;
		D3DXMatrixTranslation(&worldOffset, (float)pt[i].x -0.5f, (float)pt[i].y +0.5f, 0.0f);
		world = localMat * worldOffset;
		D3DXMATRIX matWorldViewProj = world*ortho;

		// �V�F�[�_�萔�ݒ�
		lpEffect->SetMatrix("matWorldViewProj", &matWorldViewProj);	// ���[���h�r���[�ˉe�ϊ��s���ݒ�
		lpEffect->SetFloatArray("color", colorRGBA, 4);				// �F�w��
		lpEffect->SetTexture("tex", lpFontTex[code-0x20]);			// �e�N�X�`���w��

	    // �`��J�n
		lpEffect->BeginPass(0);
		lpDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);			// �`��
		lpEffect->EndPass();
		

	}
	// �V�F�[�_�I��
	lpEffect->End();

	free(pt);


	return TRUE;
}

int DXTextANSI::OptimizeString(char * dst, const char * src) {
	if (!src)
		return -1;
	if (!dst)
		return -1;

	int cnt = 0;

	for (int i=0; i<strlen(src); i++) {
		int code = (UINT)src[i];

		if ((0x20 <= code && code <= 0x7E) || code == '\n' || code == '\t')
			dst[cnt++] = src[i];
	}

	dst[cnt] = '\0';	// NULL����

	return cnt;
}
