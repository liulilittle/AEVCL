#include "stdafx.h"
#include "D3DView.h"
#include "Resource.h"
#include <time.h>

BEGIN_MESSAGE_MAP(CD3DView, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

CD3DView::CD3DView() : CDialogEx(IDD_VIEW_DIALOG)
{
	g_ptrRtpClient = new CRtpClient;
	m_selected = false;
	m_fullscreen = false;
}

CD3DView::~CD3DView()
{
	delete g_ptrRtpClient;
}

BOOL CD3DView::OnEraseBkgnd(CDC* pDC)
{
	CBrush brush(RGB(0, 0, 0));
	RECT rect;
	pDC->GetClipBox(&rect);
	pDC->FillRect(&rect, &brush);
	return TRUE;
}

void CD3DView::Connect(const char* server, int port, const char* sim, int channel)
{
	timeBeginPeriod(1);
	unsigned int seed = timeGetTime();
	srand(seed);
	timeEndPeriod(1);
	g_ptrRtpClient->Connect(rand(), server, port, sim, channel);
}

void CD3DView::Play()
{
	HWND hWnd = GetSafeHwnd();
	g_ptrRtpClient->Play(hWnd);
	g_ptrRtpClient->Enable(TRUE);
}

void CD3DView::Stop()
{
	g_ptrRtpClient->Enable(FALSE);
}

BOOL CD3DView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(LoadCursor(NULL, IDC_HAND));
	return TRUE;
}

void CD3DView::set_Selected(bool value)
{
	m_selected = value;
}

bool CD3DView::get_Fullscreen()
{
	return m_fullscreen;
}

void CD3DView::set_Fullscreen(bool value)
{
	m_fullscreen = value;
}

void CD3DView::Move(int x, int y, int width, int height)
{
	BOOL refresh = TRUE;
	if (g_ptrRtpClient->Reading())
		refresh = FALSE;
	this->MoveWindow(x, y, width, height, refresh);
	CWnd* parent = GetParent();
	if (refresh && parent != NULL)
		parent->Invalidate(TRUE);
}

bool CD3DView::get_Selected()
{
	return m_selected;
}

void CD3DView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* parent = GetParent();
	if (parent != NULL)
		parent->PostMessage(WM_LBUTTONDOWN, (WPARAM)this);
}

void CD3DView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd* parent = GetParent();
	if (parent != NULL)
		parent->PostMessage(WM_LBUTTONDBLCLK, (WPARAM)this);
}
