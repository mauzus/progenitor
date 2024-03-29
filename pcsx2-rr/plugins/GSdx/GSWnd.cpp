/*
 *	Copyright (C) 2007-2009 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"
#include "GSdx.h"
#include "GSWnd.h"

GSWnd::GSWnd()
	: m_hWnd(NULL)
	, m_IsManaged(true)
	, m_HasFrame(true)
{
}

GSWnd::~GSWnd()
{
}

LRESULT CALLBACK GSWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GSWnd* wnd = NULL;

	if(message == WM_NCCREATE)
	{
		wnd = (GSWnd*)((LPCREATESTRUCT)lParam)->lpCreateParams;

		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)wnd);

		wnd->m_hWnd = hWnd;
	}
	else
	{
		wnd = (GSWnd*)GetWindowLongPtr(hWnd, GWL_USERDATA);
	}

	if(wnd == NULL)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return wnd->OnMessage(message, wParam, lParam);
}

LRESULT GSWnd::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CLOSE:
		Hide();
		// DestroyWindow(m_hWnd);
		return 0;
	case WM_DESTROY:
		// This kills the emulator when GS is closed, which *really* isn't desired behavior,
		// especially in STGS mode (worked in MTGS mode since it only quit the thread, but even
		// that wasn't needed).
		//PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(m_hWnd, message, wParam, lParam);
}

bool GSWnd::Create(const string& title, int w, int h)
{
	if(m_hWnd) return true;

	WNDCLASS wc;

	memset(&wc, 0, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = theApp.GetModuleHandle();
	// TODO: wc.hIcon = ;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = "GSWnd";

	if(!GetClassInfo(wc.hInstance, wc.lpszClassName, &wc))
	{
		if(!RegisterClass(&wc))
		{
			return false;
		}
	}

	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW | WS_BORDER;

	GSVector4i r;

	GetWindowRect(GetDesktopWindow(), r);

	bool remote = !!GetSystemMetrics(SM_REMOTESESSION);

	if(w <= 0 || h <= 0 || remote)
	{
		w = r.width() / 3;
		h = r.width() / 4;

		if(!remote)
		{
			w *= 2;
			h *= 2;
		}
	}

	r.left = (r.left + r.right - w) / 2;
	r.top = (r.top + r.bottom - h) / 2;
	r.right = r.left + w;
	r.bottom = r.top + h;

	AdjustWindowRect(r, style, FALSE);

	m_hWnd = CreateWindow(wc.lpszClassName, title.c_str(), style, r.left, r.top, r.width(), r.height(), NULL, NULL, wc.hInstance, (LPVOID)this);

	if(!m_hWnd)
	{
		return false;
	}

	return true;
}

bool GSWnd::Attach(HWND hWnd, bool isManaged)
{
	// TODO: subclass

	m_hWnd = hWnd;
	m_IsManaged = isManaged;

	return true;
}

void GSWnd::Detach()
{
	if(m_hWnd && m_IsManaged)
	{
		// close the window, since it's under GSdx care.  It's not taking messages anyway, and
		// that means its big, ugly, and in the way.
		DestroyWindow(m_hWnd);
	}
	m_hWnd = NULL;
	m_IsManaged = true;
}

GSVector4i GSWnd::GetClientRect()
{
	GSVector4i r;

	::GetClientRect(m_hWnd, r);

	return r;
}

// Returns FALSE if the window has no title, or if th window title is under the strict
// management of the emulator.
bool GSWnd::SetWindowText(const char* title)
{
	if( !m_IsManaged ) return false;

	::SetWindowText(m_hWnd, title);

	return m_HasFrame;
}

void GSWnd::Show()
{
	if( !m_IsManaged ) return;

	//SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	SetForegroundWindow(m_hWnd);

	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	UpdateWindow(m_hWnd);
}

void GSWnd::Hide()
{
	if( !m_IsManaged ) return;

	ShowWindow(m_hWnd, SW_HIDE);
}

void GSWnd::HideFrame()
{
	if( !m_IsManaged ) return;

	SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(WS_CAPTION|WS_THICKFRAME));

	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	SetMenu(m_hWnd, NULL);

	m_HasFrame = false;
}
