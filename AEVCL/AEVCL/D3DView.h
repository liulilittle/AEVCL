#pragma once

#include "afxdialogex.h"
#include "RtpClient.h"

class CD3DView :
	public CDialogEx
{
private:
	CRtpClient* g_ptrRtpClient;
	bool m_selected;
	bool m_fullscreen;

public:
	CD3DView();
	~CD3DView();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
	void Connect(const char* server, int port, const char* sim, int channel);
	void Play();
	void Stop();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	bool __declspec(property(get = get_Selected, put = set_Selected)) Selected;
	bool get_Selected();
	void set_Selected(bool value);
	bool __declspec(property(get = get_Fullscreen, put = set_Fullscreen)) Fullscreen;
	bool get_Fullscreen();
	void set_Fullscreen(bool value);
	void Move(int x, int y, int width, int height);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

