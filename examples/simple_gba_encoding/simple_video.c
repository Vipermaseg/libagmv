/********************************************
 * baseline
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
********************************************/
#include <agmv.h>
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* atoi */

int main(int argc, char *argv[]) {
    // Check if all required arguments are provided
    if (argc != 14) {
        fprintf(stderr, "Usage: %s <num_of_frames> <width> <height> <fps> <input_audio_file> <audio_sample_rate> <output_agmv_file>  <output_frame_directory> <output_frame_prefix> <img_format> <gba_option> <quality_option> <compression_option>\n", argv[0]);
        return 1;
    }

    u32 num_of_frames = atoi(argv[1]);
    u32 width = atoi(argv[2]);
    u32 height = atoi(argv[3]);
    u32 fps = atoi(argv[4]);
    char *input_audio_file = argv[5];
    int audio_sample_rate = atoi(argv[6]);
    char *output_agmv_file = argv[7];
    char *output_frame_directory = argv[8];
    char *output_frame_prefix = argv[9];
    int img_format = atoi(argv[10]);
    int gba_option = atoi(argv[11]);
    int quality_option = atoi(argv[12]);
    int compression_option = atoi(argv[13]);

    AGMV* agmv = CreateAGMV(num_of_frames - 1, width, height, fps);
    AGMV_RawSignedPCMToAudioTrack(input_audio_file, agmv, 1, audio_sample_rate);
    AGMV_EncodeAGMV(agmv, output_agmv_file, output_frame_directory, output_frame_prefix, img_format, 1, num_of_frames, width, height, fps, gba_option, quality_option, compression_option);

    return 0;
}
