/********************************************
*   Adaptive Graphics Motion Video
*
*   Copyright (c) 2024 Ryandracus Chapman
*
*   Library: libagmv
*   File: simple_video.c
*   Date: 6/7/2024
*   Version: 1.1
*   Updated: 6/13/2024
*   Author: Ryandracus Chapman
*
********************************************/
#include <agmv.h>

int main(){
	u32 num_of_frames = 1724;
	u32 width = 240;
	u32 height = 160;
	u32 fps = 8;
	
	AGMV* agmv = CreateAGMV(num_of_frames-1,width,height,fps);
	AGMV_RawSignedPCMToAudioTrack("videoplayback.raw",agmv,1,16000);
	AGMV_EncodeAGMV(agmv,"videoplaybackniAudioANIM.agmv","output/videoplayback","frame_",AGMV_IMG_BMP,1,num_of_frames,width,height,fps,AGMV_OPT_GBA_II,AGMV_LOW_QUALITY,AGMV_LZSS_COMPRESSION);
	
	return 0;
}