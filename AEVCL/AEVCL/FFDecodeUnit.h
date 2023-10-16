#pragma once

#include "stdafx.h"
#include "AppDefinition.h"
#include "ffdecoder/FFDecoderAPI.h"

class CDecodeUnit
{
public:
	static void ZeroYuvFrame(YUV_FRAME_INFO* ptr, int len);
	static void Delay(int begin, int delay, int end);

	static DECODER_OBJ* GetDecoder(RTSP_PLAY_OBJ_T* _pPlayThread, unsigned int mediaType, MEDIA_FRAME_INFO *_frameinfo)
	{
		if (NULL == _pPlayThread || NULL == _frameinfo)	{
			return NULL;
		}
		int iIdx = -1;
		int iExist = -1;
		for (int i = 0; i < MAX_DECODER_NUM; i++) {
			if (MEDIA_TYPE_VIDEO == mediaType) {
				if (_frameinfo->width < 1 || _frameinfo->height < 1 || _frameinfo->codec < 1)		 {
					return NULL;
				}
				if (_pPlayThread->decoderObj[i].codec.vidCodec == 0x00 &&
					_pPlayThread->decoderObj[i].codec.width == 0x00 &&
					_pPlayThread->decoderObj[i].codec.height == 0x00 && iIdx == -1) {
					iIdx = i;
				}
				if (_pPlayThread->decoderObj[i].codec.vidCodec == _frameinfo->codec &&
					_pPlayThread->decoderObj[i].codec.width == _frameinfo->width &&
					_pPlayThread->decoderObj[i].codec.height == _frameinfo->height) {
					if (NULL == _pPlayThread->decoderObj[i].ffDecoder) {
						iExist = i;
						int nDecoder = OUTPUT_PIX_FMT_YUV420P;
						if (_pPlayThread->renderFormat == GDI_FORMAT_RGB24) {
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, _pPlayThread->renderFormat, &_pPlayThread->decoderObj[i].yuv_size);
						}
						else {
#ifdef __ENABLE_SSE
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, DISPLAY_FORMAT_YV12, &_pPlayThread->decoderObj[i].yuv_size);
#else
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, _pPlayThread->renderFormat, &_pPlayThread->decoderObj[i].yuv_size);
#endif
						}
						FFD_Init(&_pPlayThread->decoderObj[i].ffDecoder);
						FFD_SetVideoDecoderParam(_pPlayThread->decoderObj[i].ffDecoder, _frameinfo->width, _frameinfo->height, _frameinfo->codec, nDecoder);
					}
					return &_pPlayThread->decoderObj[i];
				}
				if (iIdx >= 0) {
					if (NULL == _pPlayThread->decoderObj[iIdx].ffDecoder) {
						int nDecoder = OUTPUT_PIX_FMT_YUV420P;
						if (_pPlayThread->renderFormat == GDI_FORMAT_RGB24) {
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, _pPlayThread->renderFormat, &_pPlayThread->decoderObj[iIdx].yuv_size);
						}
						else {
#ifdef __ENABLE_SSE
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, DISPLAY_FORMAT_YV12, &_pPlayThread->decoderObj[iIdx].yuv_size);
#else
							CDecodeUnit::ParseDecoder2Render(&nDecoder, _frameinfo->width, _frameinfo->height, _pPlayThread->renderFormat, &_pPlayThread->decoderObj[iIdx].yuv_size);
#endif
						}
						FFD_Init(&_pPlayThread->decoderObj[iIdx].ffDecoder);
						FFD_SetVideoDecoderParam(_pPlayThread->decoderObj[iIdx].ffDecoder, _frameinfo->width, _frameinfo->height, _frameinfo->codec, nDecoder);
						if (NULL != _pPlayThread->decoderObj[iIdx].ffDecoder) {
							EnterCriticalSection(&_pPlayThread->crit);
							_pPlayThread->resetD3d = true;
							LeaveCriticalSection(&_pPlayThread->crit);
							_pPlayThread->decoderObj[iIdx].codec.vidCodec = _frameinfo->codec;
							_pPlayThread->decoderObj[iIdx].codec.width = _frameinfo->width;
							_pPlayThread->decoderObj[iIdx].codec.height = _frameinfo->height;
							return &_pPlayThread->decoderObj[iIdx];
						}
						else {
							return NULL;
						}
					}
				}
			}
			else if (MEDIA_TYPE_AUDIO == mediaType) {
				if (_frameinfo->sample_rate < 1 || _frameinfo->channels < 1 || _frameinfo->codec < 1) {
					return NULL;
				}
				if (_pPlayThread->decoderObj[i].codec.audCodec == 0x00 &&
					_pPlayThread->decoderObj[i].codec.samplerate == 0x00 &&
					_pPlayThread->decoderObj[i].codec.channels == 0x00 && iIdx == -1) {
					iIdx = i;
				}
				if (_pPlayThread->decoderObj[i].codec.audCodec == _frameinfo->codec &&
					_pPlayThread->decoderObj[i].codec.samplerate == _frameinfo->sample_rate &&
					_pPlayThread->decoderObj[i].codec.channels == _frameinfo->channels) {
					if (NULL == _pPlayThread->decoderObj[i].ffDecoder)
					{
						iExist = i;
						FFD_Init(&_pPlayThread->decoderObj[i].ffDecoder);
						FFD_SetAudioDecoderParam(_pPlayThread->decoderObj[i].ffDecoder, _frameinfo->channels, _frameinfo->sample_rate, _frameinfo->codec);
					}
					return &_pPlayThread->decoderObj[i];
				}
				if (iIdx >= 0) {
					if (NULL == _pPlayThread->decoderObj[iIdx].ffDecoder) {
						FFD_Init(&_pPlayThread->decoderObj[iIdx].ffDecoder);
						FFD_SetAudioDecoderParam(_pPlayThread->decoderObj[i].ffDecoder, _frameinfo->channels, _frameinfo->sample_rate, _frameinfo->codec);
						if (NULL != _pPlayThread->decoderObj[iIdx].ffDecoder) {
							_pPlayThread->decoderObj[iIdx].codec.audCodec = _frameinfo->codec;
							_pPlayThread->decoderObj[iIdx].codec.samplerate = _frameinfo->sample_rate;
							_pPlayThread->decoderObj[iIdx].codec.channels = _frameinfo->channels;
							return &_pPlayThread->decoderObj[iIdx];
						}
						else {
							return NULL;
						}
					}
					else {
						FFD_SetAudioDecoderParam(_pPlayThread->decoderObj[i].ffDecoder, _frameinfo->channels, _frameinfo->sample_rate, _frameinfo->codec);
						_pPlayThread->decoderObj[iIdx].codec.audCodec = _frameinfo->codec;
						_pPlayThread->decoderObj[iIdx].codec.samplerate = _frameinfo->sample_rate;
						_pPlayThread->decoderObj[iIdx].codec.channels = _frameinfo->channels;
						return &_pPlayThread->decoderObj[iIdx];
					}
				}
			}
		}
		return NULL;
	}

	static void ParseDecoder2Render(int *_decoder, int _width, int _height, int renderFormat, int *yuvsize)
	{
		if (NULL == _decoder) {
			return;
		}
		int nDecoder = 0x00;
		int nYUVSize = 0x00;
		if (renderFormat == D3D_FORMAT_UYVY) {
			nDecoder = 17;	//PIX_FMT_UYVY422
			nYUVSize = _width*_height * 2 + 1;
		}
		else if (renderFormat == D3D_FORMAT_YV12) {
			nDecoder = 0;//OUTPUT_PIX_FMT_YUV420P;
			nYUVSize = _width*_height * 3 / 2 + 1;
		}
		else if (renderFormat == D3D_FORMAT_YUY2) {
			nDecoder = 1;//OUTPUT_PIX_FMT_YUV422;
			nYUVSize = _width*_height * 2 + 1;
		}
		else if (renderFormat == D3D_FORMAT_RGB565)	{			//23 ok
			nDecoder = 44;
			nYUVSize = _width*_height * 2 + 1;
		}
		else if (renderFormat == D3D_FORMAT_RGB555)	{			//23 ok
			nDecoder = 46;
			nYUVSize = _width*_height * 2 + 1;
		}
		else if (renderFormat == D3D_FORMAT_X8R8G8B8) {
			nDecoder = 30;
			nYUVSize = _width*_height * 4 + 1;
		}
		else if (renderFormat == D3D_FORMAT_A8R8G8B8) {			//21 ok
			nDecoder = 30;
			nYUVSize = _width*_height * 4 + 1;
		}
		else if (renderFormat == (D3D_SUPPORT_FORMAT)22) {		//22	ok
			nDecoder = 28;
			nYUVSize = _width*_height * 4 + 1;
		}
		else if (renderFormat == (D3D_SUPPORT_FORMAT)1498831189) {
			nDecoder = 17;
			nYUVSize = _width*_height * 4 + 1;
		}
		else if (renderFormat == (D3D_SUPPORT_FORMAT)36) {
			nDecoder = 32;
			nYUVSize = _width*_height * 4 + 1;
		}
		else if (renderFormat == (D3D_SUPPORT_FORMAT)20) {
			nDecoder = 3;
			nYUVSize = _width*_height * 3 + 1;
		}
		else if (renderFormat == GDI_FORMAT_RGB24) {
			nDecoder = 3;
			nYUVSize = _width*_height * 3 + 1;
		}
		else {
			nDecoder = 28;
			nYUVSize = _width*_height * 4 + 1;
		}
		if (NULL != _decoder) {
			*_decoder = nDecoder;
		}
		if (NULL != yuvsize){
			*yuvsize = nYUVSize;
		}
	}
};

