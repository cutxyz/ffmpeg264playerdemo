/*
 * main.cpp
 *
 *  Created on: 2013-10-26
*      Author: 李镇城  （ cut / cutxyz） (e-mail: cut-12345@hotmail.com/501931049@qq.com)
 */
#include <stdint.h>
#include <version.h>
#include <avcodec.h>
#include <avformat.h>
//#include "include/libavutil/mem.h"
#include <Windows.h>
#include <swscale.h>
#pragma comment (lib,"bin/avcodec.lib")
#pragma comment (lib,"bin/avutil.lib")
#pragma comment (lib,"bin/swscale.lib")
#pragma comment (lib,"bin/avformat.lib")


#define  RETURN_(condition,result_) if ((condition)){return result_;}
#define  RETURN_BY_ACTION_(condition,actions,result_) if ((condition)){actions;return result_;}

typedef struct st_rgb_frame
{
	struct AVFrame * frame;
	uint8_t* bits;
}av_rgb_frame_t;


typedef struct st_av_decoder
{
	struct AVCodecContext* code_context;
	struct AVCodec* video_code;
}decoder_t;


// 创建解码器
int create_decoder (decoder_t& res_decoder, int decoder_id = CODEC_ID_H264)
{
	int result_;
	 res_decoder.video_code = avcodec_find_decoder((AVCodecID)decoder_id);
	 if (NULL == res_decoder.video_code)
	 {
		 result_ = -1;
		 res_decoder.code_context = NULL;
		 return result_;
	 }
	 res_decoder.code_context = avcodec_alloc_context3(res_decoder.video_code);

	 if (NULL == res_decoder.code_context)
	 {
		 result_ = -2;
		 return result_;
	 }

	 res_decoder.code_context->time_base.num = 1;
	 res_decoder.code_context->frame_number = 1;
	 res_decoder.code_context->codec_type = AVMEDIA_TYPE_VIDEO;
	 res_decoder.code_context->bit_rate = 0;
	 //res_decoder.code_context->width = 1280;
	 //res_decoder.code_context->height = 720;

	 result_ = avcodec_open2(res_decoder.code_context,res_decoder.video_code,NULL);
	 return result_;
}

//释放解码器
int dispose_decoder (decoder_t& res_decoder)
{
	res_decoder.video_code = NULL;
	if (NULL != res_decoder.code_context)
	{
		avcodec_close(res_decoder.code_context);
		av_free (res_decoder.code_context);
		res_decoder.code_context = NULL;
	}
	
	return 0;
}
// 获取一个AV帧
struct AVFrame* fetch_av_frame (decoder_t& decodec,const char* stream,int stream_length,int &status)
{
	AVPacket packet;
	int got_a_picture = 0;
	struct AVFrame* avframe = avcodec_alloc_frame();
	
	if (NULL == avframe)
	{
		return avframe;
	}

	packet.data = (uint8_t*)stream;
	packet.size = stream_length;

	status = avcodec_decode_video2 (decodec.code_context,avframe,&got_a_picture,&packet);
	if (got_a_picture)
	{
		return avframe;
	}
	return NULL;
}
// 释放av帧
int dispose_av_frame (struct AVFrame* pAVFrame)
{
	if (NULL == pAVFrame)
	{
		avcodec_free_frame(&pAVFrame);
	}
	return 0;
}


// 将av数据保存成bmp图片
static void save_as_bmp (const char* filenamepath,const char* pbits, int width, int height, int index, int bpp)   
{   
	BITMAPFILEHEADER bmpheader;   
	BITMAPINFOHEADER bmpinfo;   
	FILE *fp;   

	char *filename = new char[255];  

	sprintf_s(filename,255,"%sh264_%d.bmp",filenamepath,index);  
	if ( (fp=fopen(filename,"wb+")) == NULL )   
	{   
		printf ("open file failed!\n");   
		return;   
	}   

	bmpheader.bfType = 0x4d42;   
	bmpheader.bfReserved1 = 0;   
	bmpheader.bfReserved2 = 0;   
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);   
	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;   

	bmpinfo.biSize = sizeof(BITMAPINFOHEADER);   
	bmpinfo.biWidth = width;   
	bmpinfo.biHeight = height;   
	bmpinfo.biPlanes = 1;   
	bmpinfo.biBitCount = bpp;   
	bmpinfo.biCompression = BI_RGB;   
	bmpinfo.biSizeImage = (width*bpp+31)/32*4*height;   
	bmpinfo.biXPelsPerMeter = 100;   
	bmpinfo.biYPelsPerMeter = 100;   
	bmpinfo.biClrUsed = 0;   
	bmpinfo.biClrImportant = 0;   

	fwrite (&bmpheader, sizeof(bmpheader), 1, fp);   
	fwrite (&bmpinfo, sizeof(bmpinfo), 1, fp);   
	fwrite (pbits, width*height*bpp/8, 1, fp);   

	fclose(fp);   
}   

// YUV420 -> RGB24 bitmap输出
HBITMAP av_frame_to_hbitmap (HDC hdc, struct AVFrame* pAVFrame,int width,int height,int& result_)
{
	HBITMAP hbmp = NULL;

	static struct SwsContext* sws_context = NULL;
	int bytes_count = avpicture_get_size (PIX_FMT_RGB24,width,height);

	uint8_t* psrc_bmp_buffer = (uint8_t*) av_malloc(bytes_count * sizeof(uint8_t) + 
		sizeof (BITMAPINFOHEADER));
	uint8_t* psrc_buffer = psrc_bmp_buffer + sizeof (BITMAPFILEHEADER);//(uint8_t*) av_malloc(bytes_count * sizeof(uint8_t));

	if (NULL == psrc_buffer)
	{
		result_=-2;
		return NULL;
	}

	if (NULL == sws_context)
	{
		sws_context = sws_getContext (pAVFrame->width,pAVFrame->height,(AVPixelFormat)pAVFrame->format,width,height,
			PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL);
	}



	if (sws_context)
	{
		uint8_t *data[4] = {psrc_buffer, 0, 0, 0};
		int linesize[4] = {width*3, 0, 0, 0};

		sws_scale (sws_context,pAVFrame->data,pAVFrame->linesize,0,pAVFrame->height,data,linesize);
		
		static int count = 0;
		//save_as_bmp ("h264bmps\\",(char*)psrc_buffer,width,height,count++,24);
		LPVOID	lpDIBBits = psrc_buffer;
		int bpp = 24;

		BITMAPINFO*			pBMI    = (BITMAPINFO*) psrc_bmp_buffer;
		BITMAPINFOHEADER*	pHeader = &pBMI->bmiHeader;
		DWORD*				pColors = (DWORD*)&pBMI->bmiColors;   

		pHeader->biSize            = sizeof(BITMAPINFOHEADER);
		pHeader->biWidth           = width;
		pHeader->biHeight          = height;
		pHeader->biPlanes          = 1;
		pHeader->biBitCount        = bpp;
		pHeader->biCompression     = BI_RGB;//BI_BITFIELDS;
		pHeader->biSizeImage       =  (width*bpp+31)/32*4*height;   
		pHeader->biXPelsPerMeter   = 100;
		pHeader->biYPelsPerMeter   = 100;
		pHeader->biClrUsed         = 0;
		pHeader->biClrImportant    = 0;

		//FILE* file;
		//if ( (file=fopen(/*"kumax.bmp"*/"h264_0.bmp","r")) != NULL )   
		//{   
		//	long file_pos_;
		//	file_pos_ = ftell(file);
		//	fseek(file,0,SEEK_END);
		//	int filesize = ftell(file);
		//	fseek(file,file_pos_,SEEK_SET);
		//	static char* bmpbuffer = NULL;
		//	if (NULL == bmpbuffer)
		//	{ 
		//		bmpbuffer = new char[filesize+1];
		//	}
		//	int readsize = fread(bmpbuffer,1,filesize,file);
		//	  //if(readsize == filesize)
		//	  {
		//		  HBITMAP                 hShowBMP;
		//		  LPSTR                 hDIB,lpBuffer = bmpbuffer;
		//		  LPVOID                 lpDIBBits;
		//		  BITMAPFILEHEADER     bmfHeader;
		//		  DWORD                 bmfHeaderLen;

		//		  bmfHeaderLen = sizeof(bmfHeader);
		//		  strncpy((LPSTR)&bmfHeader,(LPSTR)lpBuffer,bmfHeaderLen);

		//		  if (bmfHeader.bfType != (*(WORD*)"BM")) return NULL;
		//		  hDIB = lpBuffer + bmfHeaderLen;
		//		  BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB ;
		//		  BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ;

		//		  lpDIBBits=(lpBuffer)+((BITMAPFILEHEADER *)lpBuffer)->bfOffBits;
		//		  hbmp = CreateDIBitmap(hdc,&bmiHeader,CBM_INIT,lpDIBBits,&bmInfo,DIB_RGB_COLORS);
		//		  result_ = GetLastError();
		//	  }
		//}   
		hbmp = ::CreateDIBitmap(hdc, pHeader , CBM_INIT,lpDIBBits, pBMI, DIB_RGB_COLORS);

		result_ = 0;
	}
	
	if (psrc_bmp_buffer)
	{
		av_free(psrc_bmp_buffer);
	}
	return hbmp;
}
#include <list>
using namespace std;
list < HBITMAP > m_av_frame_ls;


#define AV_FILE_NAME "stream_chn2.h264" 

#include "windowsform.h"

// 播放器
int play_h264_stream (simple_window_form* pform)
{
	struct AVFormatContext* format_context = NULL;
	int status;
	int first_stream_index = -1;

	// 注册解码组件
	av_register_all();
	avcodec_register_all();

	// 加载h264文件
	status = avformat_open_input (&format_context,AV_FILE_NAME,NULL,NULL);
	if (status < 0)
	{
		printf("avformat_open_input sb");
		return 1;
	}
	// AV流信息
	status = avformat_find_stream_info (format_context,NULL);
	if (status < 0)
	{
		printf("avformat_find_stream_info sb");
		return 1;
	}

	av_dump_format (format_context,-1,AV_FILE_NAME,0);

	for (int i=0;i<format_context->nb_streams;++i)
	{
		if (AVMEDIA_TYPE_VIDEO == format_context->streams[i]->codec->codec_type)
		{
			first_stream_index = i;
			break;
		}
	}

	if (-1 == first_stream_index)
	{
		printf("no strearm");
		return -2;
	}

	// 创建解码器
	decoder_t mydecoder;
	status = create_decoder(mydecoder);
	if (0 != status)
	{
		dispose_decoder(mydecoder);
		printf("create_decoder sb");
		return status;
	}

	int play_frame_count = 0;
	int play_frame_index = 0;

	printf("loading frame ..........\n");

	// window计时器
	LARGE_INTEGER ticker;
	QueryPerformanceFrequency(&ticker);
	double dfrequency = (double)ticker.QuadPart;
	dfrequency /= 1000.0f;

	QueryPerformanceCounter(&ticker);
	double start_tick = (double)ticker.QuadPart;
	double stop_tick = (double)ticker.QuadPart + 40.0f * dfrequency;
	double begin_tick = start_tick;

	//播放av
	do 
	{
		AVPacket packet;
		// 获取av包
		status = av_read_frame (format_context,&packet);
		if (status < 0)
		{
			printf("av_read_frame sb");
			break;
		}
		// 从av包里获取帧
		struct AVFrame* got_frame = fetch_av_frame(mydecoder,(char*)packet.data,packet.size,status);

		if (got_frame)
		{
			++play_frame_count;
			// 将帧由yuv420 -> rgb24保存
			HBITMAP hbmp = av_frame_to_hbitmap((HDC)pform->getdc(),got_frame,got_frame->width,got_frame->height,status);
			if (0 == status)
			{
				m_av_frame_ls.push_back(hbmp);
			}
			printf("load fin....%d\n",play_frame_count);
		}
		if (NULL == got_frame && packet.stream_index >= first_stream_index)
		{
			printf("load stream fin");
			break;
		}

		if (got_frame)
		{
			avcodec_free_frame(&got_frame);
		}
		av_free_packet(&packet);


		if (m_av_frame_ls.size() > 0 && (stop_tick - start_tick)/dfrequency > 35)
		{
			// 一边解码一边播，尼玛gdi图片现实真慢，demo忍了
			pform->set_bitmap((long)m_av_frame_ls.front());
			m_av_frame_ls.pop_front();
			printf("play index = %d,play.....%lfms\n",play_frame_index++,(stop_tick - start_tick)/dfrequency);
			start_tick = stop_tick;
		}

		QueryPerformanceCounter(&ticker);
		stop_tick = (double)ticker.QuadPart;
	} while (1);

	// 解码解完，播剩下没播的
	while (m_av_frame_ls.size() > 0)
	{
		if ((stop_tick - start_tick)/dfrequency > 35)
		{
			// 一边解码一边播，尼玛gdi图片现实真慢，demo忍了
			pform->set_bitmap((long)m_av_frame_ls.front());
			m_av_frame_ls.pop_front();
			printf("play index = %d,play.....%lfms\n",play_frame_index++,(stop_tick - start_tick)/dfrequency);
			start_tick = stop_tick;
			::Sleep(35);
		}

		QueryPerformanceCounter(&ticker);
		stop_tick = (double)ticker.QuadPart;
	}
	printf("play fin\n");

	dispose_decoder(mydecoder);
	return 0;
}

DWORD WINAPI play_thread_proc(LPVOID lpParams)
{
	return play_h264_stream((simple_window_form*)lpParams);
}
int main()
{
	simple_window_form form;
	init_windows_form_system();
	form.init_window(100,100,1280,720,L"h264 play test");
	DWORD threadid;
	HANDLE hthread = ::CreateThread(NULL,0,play_thread_proc,&form,0,&threadid);



	int  res = form.show();
	windows_program_loop();
	return 0;
}
