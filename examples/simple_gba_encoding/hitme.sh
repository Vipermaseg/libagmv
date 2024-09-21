#!/bin/bash

# typedef enum AGMV_IMG_TYPE{
# 	AGMV_IMG_BMP = 0x1,
# 	AGMV_IMG_TGA = 0x2,
# 	AGMV_IMG_TIM = 0x3,
# 	AGMV_IMG_PCX = 0x4,
# 	AGMV_IMG_LMP = 0x5,
# 	AGMV_IMG_PVR = 0x6,
# 	AGMV_IMG_GXT = 0x7,
# 	AGMV_IMG_BTI = 0x8,
# 	AGMV_IMG_3DF = 0x9,
# 	AGMV_IMG_PPM = 0x0A,
# 	AGMV_IMG_LBM = 0x0B,
# }AGMV_IMG_TYPE;

# typedef enum AGMV_OPT{
# 	AGMV_OPT_I       = 0x1,  /* 512 COLORS, BITSTREAM V1, HEAVY PDIFS */
# 	AGMV_OPT_II      = 0x2,  /* 256 COLORS, BITSTREAM V2, LIGHT PDIFS */
# 	AGMV_OPT_III     = 0x3,  /* 512 COLORS, BITSTREAM V1, LIGHT PDIFS */
# 	AGMV_OPT_ANIM    = 0x4,  /* 256 COLORS, BITSTREAM V2, HEAVY PDIFS */
# 	AGMV_OPT_GBA_I   = 0x5,  /* 512 COLORS, BITSTREAM V1, HEAVY PDIFS, REDUCED RES */
# 	AGMV_OPT_GBA_II  = 0x6,  /* 256 COLORS, BITSTREAM V2, HEAVY PDIFS, REDUCED RES */
# 	AGMV_OPT_GBA_III = 0x7,  /* 512 COLORS, BITSTREAM V2, LIGHT PDIFS, REDUCED RES */
# 	AGMV_OPT_NDS     = 0x8,  /* 512 COLORS, BITSTREAM V1, HEAVY PDIFS, REDUCED RES*/
# }AGMV_OPT;

# typedef enum AGMV_QUALITY{
# 	AGMV_HIGH_QUALITY = 0x1,
# 	AGMV_MID_QUALITY  = 0x2,
# 	AGMV_LOW_QUALITY  = 0x3,
# }AGMV_QUALITY;

# typedef enum AGMV_COMPRESSION{
# 	AGMV_LZSS_COMPRESSION = 0x1,
# 	AGMV_LZ77_COMPRESSION = 0x2,
# }AGMV_COMPRESSION;

# Loop through all mp4 files
cd raw_videos
for file in *.mp4; do
  # Extract the filename without the extension
  filename="${file%.*}"
  
  # Create a directory for the frames output
  mkdir -p ../output/"$filename"
  
  # Run ffmpeg to process the video
  ffmpeg -y -i "$file" \
    -filter:v "fps=8,scale=-1:160,crop=240:160" \
    -map 0:v -c:v bmp -pix_fmt pal8 ../output/"$filename"/frame_%d.bmp \
    -map 0:a -ac 1 -ar 16000 -acodec pcm_s8 -f s8 ../output/"$filename.raw"
  ../main 1724 240 160 8 ../output/"$filename.raw" 16000 ../output/"$filename.agmv" ../output/"$filename" frame_ 1 6 3 1
  mv -f GBA_GEN_AGMV.h ../output/"$filename.h"
done
cp ../output/"$filename.h" ../../simple_gba_video/include/GBA_GEN_AGMV.h
cd ../../simple_gba_video
make clean
make
mv simple_gba_video.gba ../simple_gba_encoding/output/"$filename.gba"