
// MEsayMvUnit.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMEsayMvUnitApp: 
// �йش����ʵ�֣������ MEsayMvUnit.cpp
//

class CApplication : public CWinApp
{
public:
	CApplication();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CApplication theApp;