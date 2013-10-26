/*
 * windowsform.cpp
 *
 *  Created on: 2013-10-26
*      Author: 李镇城  （ cut / cutxyz） (e-mail: cut-12345@hotmail.com/501931049@qq.com)
 */
#include "windowsform.h"

#include <Windows.h>
#include <stdio.h>
static CRITICAL_SECTION g_cslock;
static CRITICAL_SECTION g_cslock_paint;
int init_windows_form_system ()
{
	InitializeCriticalSection (&g_cslock);
	InitializeCriticalSection (&g_cslock_paint);
	return 0;
}

int windows_program_loop()
{
	MSG msg;
	int bRet;

	while( (bRet = ::GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			break;
		}
		else
		{
			::TranslateMessage(&msg); 
			::DispatchMessage(&msg); 
		}
	} 

	return 0;
}

static LRESULT WINAPI WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (NULL == hwnd)
	{
		return 0;
	}

	if (WM_CREATE == message)
	{	
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)(((LPCREATESTRUCT)lParam)->lpCreateParams));
	}

	Iwindow_form* pWinForm = (Iwindow_form*)(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (NULL != pWinForm)
	{
		LRESULT res = pWinForm->windows_message_process(message,wParam,lParam);
		pWinForm->render_update();
		return res;
	}

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

simple_window_form::simple_window_form() : m_hwnd (NULL),m_hdc(NULL),
	m_hdcMem(NULL),
	m_renderbitmap(NULL),
	m_backbuffer_bitmap(NULL)
{

}

simple_window_form::~simple_window_form()
{
	::SendMessage ((HWND)m_hwnd,WM_DESTROY,0,0);
}


int simple_window_form::init_window(int topX,int topY,int width,int height,const wchar_t* titlename)
{
	WNDCLASS window_class;
	int res;

	memset(&window_class, 0, sizeof(WNDCLASS));
	window_class.lpszClassName = titlename;
	window_class.lpfnWndProc = WndProc;
	window_class.style =   CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	window_class.hIcon = ::LoadIcon (NULL,IDI_APPLICATION);
	window_class.hInstance = ::GetModuleHandle(NULL);
	RECT window_rect;
	::SetRect(&window_rect, 
		topX,
		topY,
		topX + width,
		topY + height);

	if (0 == ::RegisterClass(&window_class))
	{
		return -1;
	}

	res = ::AdjustWindowRect(&window_rect, WS_BORDER | WS_SYSMENU, FALSE);

	m_hwnd = (long)::CreateWindowEx(
		WS_EX_APPWINDOW,
		titlename,
		titlename,
		WS_BORDER | WS_SYSMENU,
		window_rect.left,	// x
		window_rect.top,	// y
		window_rect.right - window_rect.left,	// width
		window_rect.bottom - window_rect.top,	// height
		NULL,
		NULL,
		window_class.hInstance,
		this
		);

	m_hdc = (long) ::GetDC((HWND)m_hwnd);
	res = GetLastError();
	return res;
}
static LARGE_INTEGER ticker;
static double dfrequency;
static double start_tick;
static double stop_tick; 
int simple_window_form::windows_message_process(unsigned int umessage,long wParam,long lParam)
{
	int result_ = 0;

		
	switch(umessage)
	{
	case WM_CREATE:
		
		QueryPerformanceFrequency(&ticker);
		dfrequency = (double)ticker.QuadPart;
		dfrequency /= 1000.0f;

		QueryPerformanceCounter(&ticker);
		 start_tick = (double)ticker.QuadPart;
		 stop_tick = (double)ticker.QuadPart + 1.0f;

		break;
	case WM_CLOSE:
		{
			ReleaseDC((HWND)m_hwnd, (HDC)m_hdc);
			DeleteDC((HDC)m_hdcMem);
			::DestroyWindow((HWND)m_hwnd);

		}
		break;
	case WM_PAINT:
		{
			//尼玛将图片放这画，就卡滴1B
			//PAINTSTRUCT ps; 
			//::BeginPaint ((HWND)m_hwnd,&ps);
			//EnterCriticalSection (&g_cslock_paint);
			//HBITMAP hbitmap = (HBITMAP)m_renderbitmap;
			//if (NULL == m_hdcMem)
			//{
			//	m_hdcMem = (long)::CreateCompatibleDC((HDC)m_hdc);
			//}
			//
			//HBITMAP holdbmp = (HBITMAP)::SelectObject((HDC)m_hdcMem,(HBITMAP)hbitmap);
			//RECT rect_;
			//::GetClientRect ((HWND)m_hwnd,&rect_);
			//StretchBlt ((HDC)m_hdc,0,0,rect_.right - rect_.left,rect_.bottom-rect_.top,
			//	(HDC)m_hdcMem,0,0,rect_.right - rect_.left,rect_.bottom-rect_.top,SRCCOPY);
			//::SelectObject((HDC)m_hdcMem,holdbmp);
			//::DeleteDC(m_hdcMem);
			
			//QueryPerformanceCounter(&ticker);
			//stop_tick = (double)ticker.QuadPart;
			//double fps = 1000.0 / (stop_tick-start_tick);


			//start_tick = stop_tick;
			//char fpstext[256];
			//int fps_str_size = sprintf (fpstext,"fps = %f",fps);
			//TextOutA((HDC)m_hdc,100,100,fpstext,fps_str_size);
			
			//LeaveCriticalSection (&g_cslock_paint);
			//::EndPaint ((HWND)m_hwnd,&ps);
			::Sleep(10);
			break;
		}

	case WM_SIZE:
		break;
	case WM_DESTROY:
		{
			m_hwnd = NULL;
			::PostQuitMessage(0);
		}
		break;
	default:
		result_ = ::DefWindowProc((HWND)m_hwnd, umessage, wParam, lParam);;
		break;
	}
	return result_;
	
}

int simple_window_form::render_update()
{
	if (NULL != m_hwnd && NULL != m_hdc)
	{
		EnterCriticalSection (&g_cslock);
		if (m_backbuffer_bitmap)
		{
			long tempbmp = m_renderbitmap;
			m_renderbitmap = m_backbuffer_bitmap;
			m_backbuffer_bitmap = NULL;
			LeaveCriticalSection (&g_cslock);
			::DeleteObject((HBITMAP)tempbmp);

			HBITMAP hbitmap = (HBITMAP)m_renderbitmap;
			if (NULL == m_hdcMem)
			{
				m_hdcMem = (long)::CreateCompatibleDC((HDC)m_hdc);
			}

			HBITMAP holdbmp = (HBITMAP)::SelectObject((HDC)m_hdcMem,(HBITMAP)hbitmap);
			RECT rect_;
			::GetClientRect ((HWND)m_hwnd,&rect_);
			StretchBlt ((HDC)m_hdc,0,0,rect_.right - rect_.left,rect_.bottom-rect_.top,
				(HDC)m_hdcMem,0,0,rect_.right - rect_.left,rect_.bottom-rect_.top,SRCCOPY);
			::SelectObject((HDC)m_hdcMem,holdbmp);

			QueryPerformanceCounter(&ticker);
			stop_tick = (double)ticker.QuadPart;
			double render_times =  (stop_tick-start_tick)/dfrequency;


			start_tick = stop_tick;
			char fpstext[256];
			int fps_str_size = sprintf (fpstext,"FPS = %lf,渲染时间 = %lfms",1000.0/render_times,render_times);
			TextOutA((HDC)m_hdc,100,100,fpstext,fps_str_size);

			LeaveCriticalSection (&g_cslock_paint);
		}
		else
		{
			LeaveCriticalSection (&g_cslock);
		}
	}

	

	
	return 0;
}

long simple_window_form::getdc()
{
	//return (long)::GetDC((HWND)m_hwnd);
	return m_hdc;
}

int simple_window_form::show(bool isdialag) 
{
	return ::ShowWindow((HWND)m_hwnd,SW_NORMAL);
}

int simple_window_form::set_bitmap(long bitmaphandle)
{
	EnterCriticalSection (&g_cslock);
	m_backbuffer_bitmap = bitmaphandle;
	LeaveCriticalSection(&g_cslock);
	return 0;
}


