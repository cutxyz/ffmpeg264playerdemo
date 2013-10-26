/*
 * windowsform.h
 *
 *  Created on: 2013-10-26
*      Author: ¿Ó’Ú≥«  £® cut / cutxyz£© (e-mail: cut-12345@hotmail.com/501931049@qq.com)
 */
#pragma once

class Iwindow_form
{
public:
	virtual int init_window(int topX,int topY,int width,int height,const wchar_t* titlename) = 0;
	virtual int windows_message_process(unsigned int umessage,long wParam,long lParam) = 0;
	virtual int render_update() = 0;
	virtual int show(bool isdialag = false) = 0;
	virtual long getdc() = 0;
};

class simple_window_form : public Iwindow_form
{
private:
	long m_hwnd;
	long m_hdc;
	long m_hdcMem;
	long m_renderbitmap;
	long m_backbuffer_bitmap;
public:
	simple_window_form();
	~simple_window_form();

	int init_window(int topX,int topY,int width,int height,const wchar_t* titlename);
	int windows_message_process(unsigned int umessage,long wParam,long lParam);
	int render_update();
	int show(bool isdialag = false);
	long getdc();
public:
	int set_bitmap(long bitmaphandle);
};

int init_windows_form_system ();
int windows_program_loop();
