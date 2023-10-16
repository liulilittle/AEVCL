#pragma once

#include <vector>
#include "D3DView.h"

class CMainDialog : public CDialogEx
{
private:
	int m_nViewCount;
	std::vector<CD3DView*>* g_pViewVector;

// ����
public:
	CMainDialog(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MAIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public: 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedCreateButton();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//
	void Connect(int index, const char* server, const int port, const char* sim, int channel);
	void Play(int index);
	void Stop(int index);
	void SetViewCount(int count);
	int GetViewCount();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

private:
	void FillSolidRect(CDC* g, int width, int height);
	void FillSolidBorder(CDC* g, RECT rect, int size, COLORREF color);
	RECT GetViewRect(CD3DView* view);
	bool OnPaint(CDC* pDC);
	CD3DView* GetFullscreen();
	CD3DView* GetClickView();
};
