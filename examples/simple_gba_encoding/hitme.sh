#!/bin/bash

# Loop through all mp4 files
cd raw_videos
for file in *.mp4; do
  # Extract the filename without the extension
  filename="${file%.*}"
  
  # Create a directory for the frames output
  mkdir -p ../output/"$filename"
  
  # Run ffmpeg to process the video
  ffmpeg -i "$file" -filter:v "fps=8,scale=-1:160,crop=240:160" -map 0:v -c:v bmp -pix_fmt pal8 ../output/"$filename"/frame_%d.bmp -map 0:a ../output/"$filename.wav"
done