#include "stdafx.h"
#include "Application.h"
#include "MainDialog.h"
#include "afxdialogex.h"
#include "ChannelService.h"
#include "D3DView.h"
#include <vector>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef MAX_RTP_CLIENT_VIEW_DIALOG
#define MAX_RTP_CLIENT_VIEW_DIALOG 36
#endif

#ifndef RTP_CLIENT_VIEW_DIALOG_BORDER
#define RTP_CLIENT_VIEW_DIALOG_BORDER 4
#endif

CMainDialog::CMainDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nViewCount = 36;
	g_pViewVector = NULL;
}

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	if (g_pViewVector == NULL) {
		g_pViewVector = new std::vector<CD3DView*>();
		for (int i = 0; i < MAX_RTP_CLIENT_VIEW_DIALOG; i++) { // 构建最大流媒体客户端视图会话（D3D）
			CD3DView* dlg = new CD3DView;
			dlg->Create(IDD_VIEW_DIALOG, this);
			dlg->SetParent(this);
			dlg->ShowWindow(SW_NORMAL);
			g_pViewVector->push_back(dlg);
		}
		this->OnSize(-1, -1, -1);
		for (int i = 0; i < MAX_RTP_CLIENT_VIEW_DIALOG; i++) {
			CD3DView* dlg = g_pViewVector->at(i);
			dlg->Connect("192.168.100.114", 6813, "123456", 5);
			/*dlg->Connect("192.168.100.234", 7138, "");*/
			dlg->Play();
		}
	}
}

BEGIN_MESSAGE_MAP(CMainDialog, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

BOOL CMainDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	return TRUE;
}

void CMainDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CMainDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDialog::FillSolidBorder(CDC* g, RECT rect, int size, COLORREF color)
{
	g->FillSolidRect(rect.left, rect.top, rect.right + size, size, color);
	g->FillSolidRect(rect.left, rect.top + rect.bottom, rect.right + size, size, color);
	g->FillSolidRect(rect.left, rect.top + size, size, rect.bottom, color);
	g->FillSolidRect(rect.left + rect.right, rect.top + size, size, rect.bottom, color);
}

void CMainDialog::FillSolidRect(CDC* g, int width, int height)
{
	//g->FillSolidRect(0, 0, width, height, RGB(0, 0, 0));
	int size = RTP_CLIENT_VIEW_DIALOG_BORDER;
	CD3DView* dlg = NULL;
	for (int i = 0; i < m_nViewCount; i++) {
		CD3DView* view = g_pViewVector->at(i);
		if (view->Selected)
			dlg = view; 
		else 
			this->FillSolidBorder(g, this->GetViewRect(view), size, RGB(81, 81, 81));
	}
	if (dlg != NULL) 
		this->FillSolidBorder(g, this->GetViewRect(dlg), size, RGB(92, 218, 204));
}

RECT CMainDialog::GetViewRect(CD3DView* view)
{
	int size = RTP_CLIENT_VIEW_DIALOG_BORDER;
	RECT rect;
	view->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.bottom += size;
	rect.right += size;
	rect.top -= size;
	rect.left -= size;
	return rect;
}

bool CMainDialog::OnPaint(CDC* pDC)
{
	if (!this->IsWindowVisible() || this->IsIconic())
		return FALSE;
	if (pDC == NULL) 
		pDC = this->GetDC();
	pDC->SetMapMode(MM_ANISOTROPIC);
	RECT rect;
	pDC->GetClipBox(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	//CBitmap bmp;
	//CDC g;
	//g.CreateCompatibleDC(pDC);
	//bmp.CreateCompatibleBitmap(pDC, width, height);
	//CBitmap* select = g.SelectObject(&bmp);
	this->FillSolidRect(pDC, width, height);
	//pDC->BitBlt(0, 0, width, height, &g, 0, 0, SRCCOPY);
	//g.SelectObject(select);
	//bmp.DeleteObject();
	return  ReleaseDC(pDC); // ReleaseDC(&g) &&
}

CD3DView* CMainDialog::GetFullscreen()
{
	for (int i = 0; i < m_nViewCount; i++) {
		CD3DView* view = g_pViewVector->at(i);
		if (view->Fullscreen)
			return view;
	}
	return NULL;
}

CD3DView* CMainDialog::GetClickView()
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	RECT rect;
	CD3DView* select = NULL;
	for (int i = 0; i < m_nViewCount; i++) {
		CD3DView* view = g_pViewVector->at(i);
		view->GetWindowRect(&rect);
		ScreenToClient(&rect);
		if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom)
			select = view;
		else {
			view->Selected = false;
			view->Fullscreen = false;
		}
	}
	return select;
}

BOOL CMainDialog::OnEraseBkgnd(CDC* pDC)
{
	return this->OnPaint(pDC);
}

void ssqrt(int n, int* min)
{
	if (n == 1) {
		min[0] = 1;
		min[1] = 1;
	}
	if (n == 2 || n== 3) {
		min[0] = 1;
		min[1] = n;
	}
	if (n == 4 || n == 6) {
		min[0] = 2;
		min[1] = n / 2;
	}
	if (n == 9 || n == 12) {
		min[0] = 3;
		min[1] = n / 3;
	}
	if (n == 16 || n == 20 || n == 32) {
		min[0] = 4;
		min[1] = n / 4;
	}
	if (n == 24) {
		min[0] = 6;
		min[1] = n / 4;
	}
	if (n == 28) {
		min[0] = 7;
		min[1] = n / 7;
	}
	if (n == 36) {
		min[0] = 6;
		min[1] = n / 6;
	}
	if (n == 64) {
		min[0] = 8;
		min[1] = n / 8;
	}
}

void CMainDialog::OnSize(UINT nType, int cx, int cy)
{
	if (g_pViewVector != NULL) {
		int size = RTP_CLIENT_VIEW_DIALOG_BORDER;
		int min[4] = { 0, 0, 0, 0 };
		RECT rect = { 0, 0, 0, 0 };
		GetClientRect(&rect);
		ssqrt(m_nViewCount, min); // 水平分量
		for (int y = 0; y < min[0]; y++) {
			for (int x = 0; x < min[1]; x++) {
				int offset = (y * min[1]) + x;
				CD3DView* dlg = g_pViewVector->at(offset);
				int width = rect.right - rect.left;
				int top = 0;
				int left = 0;
				int height = rect.bottom - rect.top;
				if (!dlg->Fullscreen && m_nViewCount > 1) {
					height -= size;
					width -= size;
					height /= min[0];
					width /= min[1];
					left = (x * width) + size;
					top = (y * height) + size;
					width -= size;
					height -= size;
					if (x == (min[1] - 1)) { // 水平差值修正
						min[2] = (min[1] * width) + (min[1] * size);
						min[3] = rect.right - rect.left;
						if (min[3] > min[2])
							width += (min[3] - min[2]) - size;
					}
					if (y == (min[0] - 1)) { // 垂直差值修正
						min[2] = (min[0] * height) + (min[0] * size);
						min[3] = rect.bottom - rect.top;
						if (min[3] > min[2])
							height += (min[3] - min[2]) - size;
					}
				}
				dlg->Move(left, top, width, height);
			}
		}
		this->OnPaint(NULL);
	}
}

void CMainDialog::Connect(int index, const char* server, const int port, const char* sim, int channel)
{
	if (g_pViewVector != NULL && index >= 0 && index < g_pViewVector->size()) {
		CD3DView* dlg = g_pViewVector->at(index);
		dlg->Connect(server, port, sim, channel);
	}
}

void CMainDialog::Play(int index)
{
	if (g_pViewVector != NULL && index >= 0 && index < g_pViewVector->size()) {
		CD3DView* dlg = g_pViewVector->at(index);
		dlg->Play();
	}
}

void CMainDialog::Stop(int index)
{
	if (g_pViewVector != NULL && index >= 0 && index < g_pViewVector->size()) {
		CD3DView* dlg = g_pViewVector->at(index);
		dlg->Stop();
	}
}

void CMainDialog::SetViewCount(int count)
{
	m_nViewCount = count;
	this->OnSize(-1, -1, -1);
}

int CMainDialog::GetViewCount()
{
	return m_nViewCount;
}

void CMainDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CD3DView* dlg = this->GetClickView();
	bool refresh = true;
	if (dlg != NULL) {
		refresh = !dlg->Selected;
		dlg->Selected = true;
	}
	if (refresh)
		this->OnPaint(NULL);
}

void CMainDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CD3DView* dlg = this->GetFullscreen();
	if (dlg == NULL)
		dlg = this->GetClickView();
	if (dlg != NULL) 
		dlg->Fullscreen = !dlg->Fullscreen;
	this->OnSize(-1, -1, -1);
}