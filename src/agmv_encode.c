/********************************************
*   Adaptive Graphics Motion Video
*
*   Copyright (c) 2024 Ryandracus Chapman
*
*   Library: libagmv
*   File: agmv_encode.c
*   Date: 5/17/2024
*   Version: 1.1
*   Updated: 6/14/2024
*   Author: Ryandracus Chapman
*
********************************************/
#include <math.h>
#include <agidl.h>
#include <stdlib.h>
#include <string.h>
#include <agmv_encode.h>
#include <agmv_utils.h>

void AGMV_EncodeHeader(FILE* file, AGMV* agmv){
	u32 i;
	u8  r, g, b;

	AGMV_OPT opt;
	AGMV_COMPRESSION compression;
	
	opt = AGMV_GetOPT(agmv);
	compression = AGMV_GetCompression(agmv);

	AGMV_WriteFourCC(file,'A','G','M','V');
	AGMV_WriteLong(file,AGMV_GetNumberOfFrames(agmv));
	AGMV_WriteLong(file,AGMV_GetWidth(agmv));
	AGMV_WriteLong(file,AGMV_GetHeight(agmv));
	AGMV_WriteByte(file,1);

	if(opt != AGMV_OPT_II && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_II){
		
		AGMV_WriteByte(file,AGMV_GetVersionFromOPT(opt,compression));	
		
		AGMV_WriteLong(file,AGMV_GetFramesPerSecond(agmv));
		AGMV_WriteLong(file,AGMV_GetTotalAudioDuration(agmv));
		AGMV_WriteLong(file,AGMV_GetSampleRate(agmv));
		AGMV_WriteLong(file,AGMV_GetAudioSize(agmv));
		AGMV_WriteShort(file,AGMV_GetNumberOfChannels(agmv));
		AGMV_WriteShort(file,AGMV_GetBitsPerSample(agmv));

		for(i = 0; i < 256; i++){
			u32 color = agmv->header.palette0[i];

			r = AGMV_GetR(color);
			g = AGMV_GetG(color);
			b = AGMV_GetB(color);

			AGMV_WriteByte(file,r);
			AGMV_WriteByte(file,g);
			AGMV_WriteByte(file,b);
		}

		for(i = 0; i < 256; i++){
			u32 color = agmv->header.palette1[i];

			r = AGMV_GetR(color);
			g = AGMV_GetG(color);
			b = AGMV_GetB(color);

			AGMV_WriteByte(file,r);
			AGMV_WriteByte(file,g);
			AGMV_WriteByte(file,b);
		}
	}
	else{
		AGMV_WriteByte(file,AGMV_GetVersionFromOPT(opt,compression));

		AGMV_WriteLong(file,AGMV_GetFramesPerSecond(agmv));
		AGMV_WriteLong(file,AGMV_GetTotalAudioDuration(agmv));
		AGMV_WriteLong(file,AGMV_GetSampleRate(agmv));
		AGMV_WriteLong(file,AGMV_GetAudioSize(agmv));
		AGMV_WriteShort(file,AGMV_GetNumberOfChannels(agmv));
		AGMV_WriteShort(file,AGMV_GetBitsPerSample(agmv));

		for(i = 0; i < 256; i++){
			u32 color = agmv->header.palette0[i];

			r = AGMV_GetR(color);
			g = AGMV_GetG(color);
			b = AGMV_GetB(color);

			AGMV_WriteByte(file,r);
			AGMV_WriteByte(file,g);
			AGMV_WriteByte(file,b);
		}
	}
}

/*
==================
LZSS
==================
*/
#define	BACK_WINDOW		65535
#define	BACK_BITS		16
#define	FRONT_WINDOW	15
#define	FRONT_BITS		4

u32 AGMV_LZSS (FILE* file, AGMV_BITSTREAM* in)
{
	int		i;
	int		val;
	int		j, start, max;
	int		bestlength, beststart;
	int		outbits = 0;
	int     pos = in->pos;
	u8*     data = in->data;

	outbits = 0;
	for (i=0 ; i<pos ; )
	{
		val = data[i];

		max = FRONT_WINDOW;
		if (i + max > pos)
			max = pos - i;

		start = i - BACK_WINDOW;
		if (start < 0)
			start = 0;
		bestlength = 0;
		beststart = 0;
		for ( ; start < i ; start++)
		{
			if (data[start] != val)
				continue;
			// count match length
			for (j=0 ; j<max ; j++)
				if (data[start+j] != data[i+j])
					break;
			if (j > bestlength)
			{
				bestlength = j;
				beststart = start;
			}
		}
		
		beststart = BACK_WINDOW - (i-beststart);

		if (bestlength < 3)
		{	/* output a single char */
			bestlength = 1;

			AGMV_WriteBits(file,1,1);
			AGMV_WriteBits(file,val,8);
			
			outbits += 9;
		}
		else
		{
			AGMV_WriteBits(file,0,1);
			if(BACK_WINDOW-beststart < 65536){
				AGMV_WriteBits(file,BACK_WINDOW-beststart,16);
			}
			else{
				AGMV_WriteBits(file,65535,16);
			}
			AGMV_WriteBits(file,bestlength,4);
			
			outbits += 21;
		}

		while (bestlength--)
		{
			i++;
		}
	}
	
	return outbits / 8.0f;
}

u32 AGMV_LZ77(FILE* file, AGMV_BITSTREAM* in)
{
	int		i;
	int		val;
	int		j, start, max;
	int		bestlength, beststart;
	int		outbits = 0;
	int     pos = in->pos;
	u8*     data = in->data;

	outbits = 0;
	for (i=0 ; i<pos ; )
	{
		val = data[i];

		max = 255;
		if (i + max > pos)
			max = in->pos - i;

		start = i - BACK_WINDOW;
		if (start < 0)
			start = 0;
		bestlength = 0;
		beststart = 0;
		for ( ; start < i ; start++)
		{
			if (data[start] != val)
				continue;
			// count match length
			for (j=0 ; j<max ; j++)
				if (data[start+j] != data[i+j])
					break;
			if (j > bestlength)
			{
				bestlength = j;
				beststart = i - start;
			}
		}

		if (bestlength > 0)
		{	
			AGMV_WriteShort(file,beststart);
			AGMV_WriteByte(file,bestlength);
			AGMV_WriteByte(file,in->data[i+bestlength]);
			i += bestlength + 1;
		}
		else
		{
			
			AGMV_WriteShort(file,0);
			AGMV_WriteByte(file,0);
			AGMV_WriteByte(file,in->data[i]);
			i++;
		}
		
		outbits += 32;
	}
	
	return outbits / 8.0f;
}

u8 AGMV_ComparePFrameBlock(AGMV* agmv, u32 x, u32 y, AGMV_ENTRY* entry){
	u32 i, j, width, color1, color2;
	int r1, g1, b1, r2, g2, b2, rdiff, gdiff, bdiff;
	u8 count;
	
	width = agmv->frame->width;
	count = 0;
	
	AGMV_ENTRY* iframe_entries = agmv->iframe_entries;
	
	for(j = 0; j < 4; j++){
		for(i = 0; i < 4; i++){
			AGMV_ENTRY ent1 = entry[(x+i)+(y+j)*width];
			AGMV_ENTRY ent2 = iframe_entries[(x+i)+(y+j)*width];
			
			if(ent1.pal_num == 0){
				color1 = agmv->header.palette0[ent1.index];
			}
			else{
				color1 = agmv->header.palette1[ent1.index];
			}
			
			if(ent2.pal_num == 0){
				color2 = agmv->header.palette0[ent2.index];
			}
			else{
				color2 = agmv->header.palette1[ent2.index];
			}
			
			r1 = AGMV_GetR(color1);
			g1 = AGMV_GetG(color1);
			b1 = AGMV_GetB(color1);
			
			r2 = AGMV_GetR(color2);
			g2 = AGMV_GetG(color2);
			b2 = AGMV_GetB(color2);
			
			rdiff = r1-r2;
			gdiff = g1-g2;
			bdiff = b1-b2;
			
			if(rdiff < 0){
				rdiff = AGIDL_Abs(rdiff);
			}
			
			if(gdiff < 0){
				gdiff = AGIDL_Abs(gdiff);
			}
			
			if(bdiff < 0){
				bdiff = AGIDL_Abs(bdiff);
			}
			
			if(rdiff <= 2 && gdiff <= 2 && bdiff <= 2){
				count++;
			}
		}
	}
	
	return count;
}

u8 AGMV_CompareIFrameBlock(AGMV* agmv, u32 x, u32 y, u32 color, AGMV_ENTRY* img_entry){
	u32 i, j, width;
	int r1, g1, b1, r2, g2, b2, rdiff, gdiff, bdiff;
	u8 count;
	
	width = agmv->frame->width;
	count = 0;
	
	for(j = 0; j < 4; j++){
		for(i = 0; i < 4; i++){
			AGMV_ENTRY entry = img_entry[(x+i)+(y+j)*width];
			u32 bc;
			if(entry.pal_num == 0){
				bc = agmv->header.palette0[entry.index];
			}
			else{
				bc = agmv->header.palette1[entry.index];
			}

			r1 = AGMV_GetR(color);
			g1 = AGMV_GetG(color);
			b1 = AGMV_GetB(color);
			
			r2 = AGMV_GetR(bc);
			g2 = AGMV_GetG(bc);
			b2 = AGMV_GetB(bc);
			
			rdiff = r1-r2;
			gdiff = g1-g2;
			bdiff = b1-b2;
			
			if(rdiff < 0){
				rdiff = AGIDL_Abs(rdiff);
			}
			
			if(gdiff < 0){
				gdiff = AGIDL_Abs(gdiff);
			}
			
			if(bdiff < 0){
				bdiff = AGIDL_Abs(bdiff);
			}
			
			if(rdiff <= 2 && gdiff <= 2 && bdiff <= 2){
				count++;
			}
		}
	}
	
	return count;
}

void AGMV_AssembleIFrameBitstream(AGMV* agmv, AGMV_ENTRY* img_entry){
	AGMV_OPT opt;
	u32 width, height, x, y, i, j;
	u8* data = agmv->bitstream->data;			
	
	width = agmv->frame->width;
	height = agmv->frame->height;
	
	opt = AGMV_GetOPT(agmv);
	
	if(opt != AGMV_OPT_II && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_II){
		for(y = 0; y < height; y += 4){
			for(x = 0; x < width; x += 4){
				u8 count;
				u32 color;
				AGMV_ENTRY entry = img_entry[x+y*width];
	
				if(entry.pal_num == 0){
					color = agmv->header.palette0[entry.index];
				}
				else{
					color = agmv->header.palette1[entry.index];
				}
				
				count = AGMV_CompareIFrameBlock(agmv,x,y,color,img_entry);
				
				if(count >= AGMV_FILL_COUNT){
					data[agmv->bitstream->pos++] = AGMV_FILL_FLAG;
					if(entry.index < 127){
						data[agmv->bitstream->pos++] = entry.pal_num << 7 | entry.index;
					}
					else{
						data[agmv->bitstream->pos++] = entry.pal_num << 7 | 127;
						data[agmv->bitstream->pos++] = entry.index;
					}
				}
				else{
					data[agmv->bitstream->pos++] = AGMV_NORMAL_FLAG;
					for(j = 0; j < 4; j++){
						for(i = 0; i < 4; i++){
							AGMV_ENTRY norm = img_entry[(x+i)+(y+j)*width];
							if(norm.index < 127){
								data[agmv->bitstream->pos++] = norm.pal_num << 7 | norm.index;
							}
							else{
								data[agmv->bitstream->pos++] = norm.pal_num << 7 | 127;
								data[agmv->bitstream->pos++] = norm.index;
							}
						}
					}
				}
			}
		}
	}
	else{
		for(y = 0; y < height; y += 4){
			for(x = 0; x < width; x += 4){
				
				u8 count;
				u32 color;
				AGMV_ENTRY entry = img_entry[x+y*width];
	
				color = agmv->header.palette0[entry.index];
				count = AGMV_CompareIFrameBlock(agmv,x,y,color,img_entry);
				
				if(count >= AGMV_FILL_COUNT){
					data[agmv->bitstream->pos++] = AGMV_FILL_FLAG;
					data[agmv->bitstream->pos++] = entry.index;
				}
				else{
					data[agmv->bitstream->pos++] = AGMV_NORMAL_FLAG;
	
					for(j = 0; j < 4; j++){
						for(i = 0; i < 4; i++){
							AGMV_ENTRY entry = img_entry[(x+i)+(y+j)*width];
							data[agmv->bitstream->pos++] = entry.index;
						}
					}
				}
			}
		}
	}
}

void AGMV_AssemblePFrameBitstream(AGMV* agmv, AGMV_ENTRY* img_entry){
	AGMV_OPT opt;
	u32 width, height, x, y, i, j;
	u8* data = agmv->bitstream->data;			
	
	width = agmv->frame->width;
	height = agmv->frame->height;
	
	opt = AGMV_GetOPT(agmv);
	
	if(opt != AGMV_OPT_II && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_II){
		for(y = 0; y < height; y += 4){
			for(x = 0; x < width; x += 4){
				u8 count1,count2;
				u32 color;
				AGMV_ENTRY entry = img_entry[x+y*width];
	
				if(entry.pal_num == 0){
					color = agmv->header.palette0[entry.index];
				}
				else{
					color = agmv->header.palette1[entry.index];
				}
				
				count1 = AGMV_CompareIFrameBlock(agmv,x,y,color,img_entry);
				count2 = AGMV_ComparePFrameBlock(agmv,x,y,img_entry);
				
				if(count2 >= AGMV_COPY_COUNT){
					data[agmv->bitstream->pos++] = AGMV_COPY_FLAG;
				}
				else if(count1 >= AGMV_FILL_COUNT){
					data[agmv->bitstream->pos++] = AGMV_FILL_FLAG;
					if(entry.index < 127){
						data[agmv->bitstream->pos++] = entry.pal_num << 7 | entry.index;
					}
					else{
						data[agmv->bitstream->pos++] = entry.pal_num << 7 | 127;
						data[agmv->bitstream->pos++] = entry.index;
					}
				}
				else{
					data[agmv->bitstream->pos++] = AGMV_NORMAL_FLAG;
					for(j = 0; j < 4; j++){
						for(i = 0; i < 4; i++){
							AGMV_ENTRY norm = img_entry[(x+i)+(y+j)*width];
							if(norm.index < 127){
								data[agmv->bitstream->pos++] = norm.pal_num << 7 | norm.index;
							}
							else{
								data[agmv->bitstream->pos++] = norm.pal_num << 7 | 127;
								data[agmv->bitstream->pos++] = norm.index;
							}
						}
					}
				}
			}
		}
	}
	else{
		for(y = 0; y < height; y += 4){
			for(x = 0; x < width; x += 4){
				u8 count1,count2;
				u32 color;
				AGMV_ENTRY entry = img_entry[x+y*width];
	
				color  = agmv->header.palette0[entry.index];
				count1 = AGMV_CompareIFrameBlock(agmv,x,y,color,img_entry);
				count2 = AGMV_ComparePFrameBlock(agmv,x,y,img_entry);
				
				if(count2 >= AGMV_COPY_COUNT){
					data[agmv->bitstream->pos++] = AGMV_COPY_FLAG;
				}
				else if(count1 >= AGMV_FILL_COUNT){
					data[agmv->bitstream->pos++] = AGMV_FILL_FLAG;
					data[agmv->bitstream->pos++] = entry.index;
				}
				else{
					data[agmv->bitstream->pos++] = AGMV_NORMAL_FLAG;
	
					for(j = 0; j < 4; j++){
						for(i = 0; i < 4; i++){
							AGMV_ENTRY entry = img_entry[(x+i)+(y+j)*width];
							data[agmv->bitstream->pos++] = entry.index;
						}
					}
				}
			}
		}
	}
}

void AGMV_EncodeFrame(FILE* file, AGMV* agmv, u32* img_data){
	AGMV_OPT opt = AGMV_GetOPT(agmv);
	AGMV_COMPRESSION compression = AGMV_GetCompression(agmv);
	AGMV_ENTRY* iframe_entries, *img_entry;

	int i, csize, pos, size, max_size;
	u32 palette0[256], palette1[256];
	
	for(i = 0; i < 256; i++){
		palette0[i] = agmv->header.palette0[i];
		palette1[i] = agmv->header.palette1[i];
	}
	
	size     = AGMV_GetWidth(agmv)*AGMV_GetHeight(agmv);
	max_size = size + (size * 0.08f);
	
	iframe_entries = agmv->iframe_entries;
	img_entry = (AGMV_ENTRY*)malloc(sizeof(AGMV_ENTRY)*size);

	AGMV_SyncFrameAndImage(agmv,img_data);
	AGMV_WriteFourCC(file,'A','G','F','C');
	AGMV_WriteLong(file,agmv->frame_count+1);

	agmv->bitstream->pos = 0;

	if(opt != AGMV_OPT_II && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_II){
		
		for(i = 0; i < size; i++){
			img_entry[i] = AGMV_FindNearestEntry(palette0,palette1,img_data[i]);
		}
		
		if(agmv->frame_count % 4 == 0){
			AGMV_AssembleIFrameBitstream(agmv,img_entry);
		}
		else{
			AGMV_AssemblePFrameBitstream(agmv,img_entry);
		}

		AGMV_WriteLong(file,agmv->bitstream->pos);
		AGMV_WriteLong(file,0);

		pos = ftell(file);
		
		if(AGMV_GetCompression(agmv) == AGMV_LZSS_COMPRESSION){
			csize = AGMV_LZSS(file,agmv->bitstream);
		}
		else{
			csize = AGMV_LZ77(file,agmv->bitstream);
		}
		
		AGMV_FlushWriteBits(file);
		
		fseek(file,pos-4,SEEK_SET);
		
		AGMV_WriteLong(file,csize);

		fseek(file,csize,SEEK_CUR);
		
	}
	else{
		for(i = 0; i < size; i++){
			img_entry[i].index = AGMV_FindNearestColor(palette0,img_data[i]);
			img_entry[i].pal_num = 0;
		}
		
		if(agmv->frame_count % 4 == 0){
			AGMV_AssembleIFrameBitstream(agmv,img_entry);
		}
		else{
			AGMV_AssemblePFrameBitstream(agmv,img_entry);
		}

		AGMV_WriteLong(file,agmv->bitstream->pos);
		AGMV_WriteLong(file,0);
		
		pos = ftell(file);

		if(AGMV_GetCompression(agmv) == AGMV_LZSS_COMPRESSION){
			csize = AGMV_LZSS(file,agmv->bitstream);
		}
		else{
			csize = AGMV_LZ77(file,agmv->bitstream);
		}
		
		AGMV_FlushWriteBits(file);
		
		fseek(file,pos-4,SEEK_SET);
		
		AGMV_WriteLong(file,csize);

		fseek(file,csize,SEEK_CUR);
	}
	
	for(i = 0; i < 8; i++){
		AGMV_WriteByte(file,0xff);
	}
	
	if(agmv->frame_count % 4 == 0){
		for(i = 0; i < size; i++){
			iframe_entries[i] = img_entry[i];
		}
	}
		
	free(img_entry);
	agmv->frame_count++;
}

void roundUpEven(u8* num){
	u8 temp = *num;
	while(temp % 2 != 0){
		temp++;
	}
	*num = temp;
}

void roundUpOdd(u8* num){
	u8 temp = *num;
	while(temp % 2 == 0){
		temp++;
	}
	*num = temp;
}

f32 AGMV_Round(f32 x) {
    if (x >= 0.0)
        return floor(x + 0.5);
    else
        return ceil(x - 0.5);
}

void AGMV_CompressAudio(AGMV* agmv){
	int i, resamp1, resamp2, resamp3, size = AGMV_GetAudioSize(agmv);
	u32 dist1, dist2, dist3, dist;
	u8 ssqrt1, ssqrt2, shift, *atsample = agmv->audio_chunk->atsample;
	u16* pcm = agmv->audio_track->pcm;
	u8* pcm8 = agmv->audio_track->pcm8;
	
	if(AGMV_GetBitsPerSample(agmv) == 16){
		for(i = 0; i < size; i++){
			int samp = pcm[i];
			
			ssqrt1 = sqrt(samp);
			ssqrt2 = AGMV_Round(sqrt(samp));
			shift = samp >> 8;
			
			roundUpEven(&ssqrt1);
			roundUpEven(&ssqrt2);
			roundUpOdd(&shift);
			
			resamp1 = ssqrt1 * ssqrt1;
			resamp2 = ssqrt2 * ssqrt2;
			resamp3 = shift << 8;
			
			dist1 = AGMV_Abs(resamp1-samp);
			dist2 = AGMV_Abs(resamp2-samp);
			dist3 = AGMV_Abs(resamp3-samp);	    
			
			dist = AGMV_Min(dist1,dist2);
			dist = AGMV_Min(dist1,dist3);	    
			
			if(dist == dist1){
				atsample[i] = ssqrt1;
			}
			else if(dist == dist2){
				atsample[i] = ssqrt2;
			}
			else{
				atsample[i] = shift;
			}
		}
	}
	else{
		for(i = 0; i < size; i++){
			atsample[i] = pcm8[i];
		}
	}
}

void AGMV_EncodeAudioChunk(FILE* file, AGMV* agmv){
	int i, size = agmv->audio_chunk->size;
	u8* atsample = agmv->audio_chunk->atsample;
		
	AGMV_WriteFourCC(file,'A','G','A','C');
	AGMV_WriteLong(file,agmv->audio_chunk->size);
		
	for(i = 0; i < size; i++){
		AGMV_WriteByte(file,atsample[agmv->audio_track->start_point++]);
	}
}

void AGMV_EncodeVideo(const char* filename, const char* dir, const char* basename, u8 img_type, u32 start_frame, u32 end_frame, u32 width, u32 height, u32 frames_per_second, AGMV_OPT opt, AGMV_QUALITY quality, AGMV_COMPRESSION compression){
	u32 i, palette0[256], palette1[256], n, count = 0, num_of_frames_encoded = 0, w, h, num_of_pix, max_clr, size = width*height;
	u32 pal[512];
	
	AGMV* agmv = CreateAGMV(end_frame-start_frame,width,height,frames_per_second);
	AGMV_SetOPT(agmv,opt);
	AGMV_SetCompression(agmv,compression);
	
	switch(quality){
		case AGMV_HIGH_QUALITY:{
			max_clr = AGMV_MAX_CLR;
		}break;
		case AGMV_MID_QUALITY:{
			max_clr = 131071;
		}break;
		case AGMV_LOW_QUALITY:{
			max_clr = 65535;
		}break;
		default:{
			max_clr = AGMV_MAX_CLR;
		}break;
	}
	
	u32* colorgram = (u32*)malloc(sizeof(u32)*max_clr+5);
	u32* histogram = (u32*)malloc(sizeof(u32)*max_clr+5);
	
	switch(opt){
		case AGMV_OPT_I:{
			AGMV_SetLeniency(agmv,0.2282);
		}break;
		case AGMV_OPT_II:{
			AGMV_SetLeniency(agmv,0.1282);
		}break;
		case AGMV_OPT_III:{
			AGMV_SetLeniency(agmv,0.2282);
		}break;
		case AGMV_OPT_ANIM:{
			AGMV_SetLeniency(agmv,0.2282);
		}break;
		case AGMV_OPT_GBA_I:{
			AGMV_SetLeniency(agmv,0.0);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
		}break;
		case AGMV_OPT_GBA_II:{
			AGMV_SetLeniency(agmv,0.0);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
		}break;
		case AGMV_OPT_GBA_III:{
			AGMV_SetLeniency(agmv,0.0f);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
		}break;
		case AGMV_OPT_NDS:{
			AGMV_SetLeniency(agmv,0.2282);
			
			AGMV_SetWidth(agmv,AGMV_NDS_W);
			AGMV_SetHeight(agmv,AGMV_NDS_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_NDS_W*AGMV_NDS_H);
		}break;
		default:{
			AGMV_SetLeniency(agmv,0.2282);
		}break;
	}
	
	for(i = 0; i < 512; i++){
		if(i < 256){
			palette0[i] = 0;
			palette1[i] = 0;
		}
		
		pal[i] = 0;
	}
	
	for(i = 0; i < max_clr; i++){
		histogram[i] = 1;
		colorgram[i] = i;
	}
	
	char* ext = AGIDL_GetImgExtension(img_type);
	
	for(i = start_frame; i <= end_frame; i++){
		char filename[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename,"%s/%s%ld%s",dir,basename,i,ext);
		}
		else{
			sprintf(filename,"%s%ld%s",basename,i,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				u32* pixels;
				
				AGIDL_BMP* bmp = AGIDL_LoadBMP(filename);
				AGIDL_ColorConvertBMP(bmp,AGIDL_RGB_888);
				
				pixels = bmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBMP(bmp);
			}break;
			case AGIDL_IMG_TGA:{
				u32* pixels;
				
				AGIDL_TGA* tga = AGIDL_LoadTGA(filename);
				AGIDL_ColorConvertTGA(tga,AGIDL_RGB_888);
				
				pixels = tga->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTGA(tga);
			}break;
			case AGIDL_IMG_TIM:{
				u32* pixels;
				
				AGIDL_TIM* tim = AGIDL_LoadTIM(filename);
				
				pixels = tim->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTIM(tim);
			}break;
			case AGIDL_IMG_PCX:{
				u32* pixels;
				
				AGIDL_PCX* pcx = AGIDL_LoadPCX(filename);
				AGIDL_ColorConvertPCX(pcx,AGIDL_RGB_888);
				
				pixels = pcx->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePCX(pcx);
			}break;
			case AGIDL_IMG_LMP:{
				u32* pixels;
				
				AGIDL_LMP* lmp = AGIDL_LoadLMP(filename);
				AGIDL_ColorConvertLMP(lmp,AGIDL_RGB_888);
				
				pixels = lmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLMP(lmp);
			}break;
			case AGIDL_IMG_PVR:{
				u32* pixels;
				
				AGIDL_PVR* pvr = AGIDL_LoadPVR(filename);
				AGIDL_ColorConvertPVR(pvr,AGIDL_RGB_888);
				
				pixels = pvr->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePVR(pvr);
			}break;
			case AGIDL_IMG_GXT:{
				u32* pixels;
				
				AGIDL_GXT* gxt = AGIDL_LoadGXT(filename);
				AGIDL_ColorConvertGXT(gxt,AGIDL_RGB_888);
				
				pixels = gxt->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeGXT(gxt);
			}break;
			case AGIDL_IMG_BTI:{
				u32* pixels;
				
				AGIDL_BTI* bti = AGIDL_LoadBTI(filename);
				AGIDL_ColorConvertBTI(bti,AGIDL_RGB_888);
				
				pixels = bti->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBTI(bti);
			}break;
			case AGIDL_IMG_3DF:{
				u32* pixels;
				
				AGIDL_3DF* glide = AGIDL_Load3DF(filename);
				AGIDL_ColorConvert3DF(glide,AGIDL_RGB_888);
				
				pixels = glide->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_Free3DF(glide);
			}break;
			case AGIDL_IMG_PPM:{
				u32* pixels;
				
				AGIDL_PPM* ppm = AGIDL_LoadPPM(filename);
				AGIDL_ColorConvertPPM(ppm,AGIDL_RGB_888);
				
				pixels = ppm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePPM(ppm);
			}break;
			case AGIDL_IMG_LBM:{
				u32* pixels;
				
				AGIDL_LBM* lbm = AGIDL_LoadLBM(filename);
				AGIDL_ColorConvertLBM(lbm,AGIDL_RGB_888);
				
				pixels = lbm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLBM(lbm);
			}break;
		}
	}

	AGMV_BubbleSort(histogram,colorgram,max_clr);
	
	for(n = max_clr; n > 0; n--){
		Bool skip = FALSE;
			
		u32 clr = colorgram[n];
		
		int r = AGMV_GetQuantizedR(clr,quality);
		int g = AGMV_GetQuantizedG(clr,quality);
		int b = AGMV_GetQuantizedB(clr,quality);
		
		int j;
		for(j = 0; j < 512; j++){
			u32 palclr = pal[j];
			
			int palr = AGMV_GetQuantizedR(palclr,quality);
			int palg = AGMV_GetQuantizedG(palclr,quality);
			int palb = AGMV_GetQuantizedB(palclr,quality);
			
			int rdiff = r-palr;
			int gdiff = g-palg;
			int bdiff = b-palb;
			
			if(rdiff < 0){
				rdiff = AGIDL_Abs(rdiff);
			}
			
			if(gdiff < 0){
				gdiff = AGIDL_Abs(gdiff);
			}
			
			if(bdiff < 0){
				bdiff = AGIDL_Abs(bdiff);
			}
			
			if(quality == AGMV_HIGH_QUALITY){
				if(rdiff <= 2 && gdiff <= 2 && bdiff <= 3){
					skip = TRUE;
				}
			}
			else{
				if(rdiff <= 1 && gdiff <= 1 && bdiff <= 1){
					skip = TRUE;
				}
			}
		}
		
		if(skip == FALSE){
			pal[count] = clr;
			count++;
		}
		
		if(count >= 512){
			break;
		}
	}
	
	if(opt == AGMV_OPT_I || opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_III || opt == AGMV_OPT_GBA_III || opt == AGMV_OPT_NDS){
		for(n = 0; n < 512; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			if(n < 126){
				palette0[n] = invclr;
			}
			else if(n >= 126 && n <= 252){
				palette1[n-126] = invclr;
			}
			
			if(n > 252 && n <= 381){
				palette0[n-126] = invclr;
			}
			
			if(n > 381 && (n-255) < 256){
				palette1[n-255] = invclr;
			}
		}
	}
	
	if(opt == AGMV_OPT_II || opt == AGMV_OPT_GBA_II|| opt == AGMV_OPT_ANIM){
		for(n = 0; n < 256; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			palette0[n] = invclr;
		}
	}
	
	free(colorgram);
	free(histogram);
	
	FILE* file = fopen(filename,"wb");
	
	AGMV_SetICP0(agmv,palette0);
	AGMV_SetICP1(agmv,palette1);
	
	printf("Encoding AGMV Header...\n");
	AGMV_EncodeHeader(file,agmv);
	printf("Encoded AGMV Header...\n");
	
	for(i = start_frame; i <= end_frame;){
		char filename1[60],filename2[60],filename3[60],filename4[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename1,"%s/%s%ld%s",dir,basename,i,ext);
			sprintf(filename2,"%s/%s%ld%s",dir,basename,i+1,ext);
			sprintf(filename3,"%s/%s%ld%s",dir,basename,i+2,ext);
			sprintf(filename4,"%s/%s%ld%s",dir,basename,i+3,ext);
		}
		else{
			sprintf(filename1,"%s%ld%s",basename,i,ext);
			sprintf(filename2,"%s%ld%s",basename,i+1,ext);
			sprintf(filename3,"%s%ld%s",basename,i+2,ext);
			sprintf(filename4,"%s%ld%s",basename,i+3,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_BMP* frame1 = AGIDL_LoadBMP(filename1);
				AGIDL_BMP* frame2 = AGIDL_LoadBMP(filename2);
				AGIDL_BMP* frame3 = AGIDL_LoadBMP(filename3);
				AGIDL_BMP* frame4 = AGIDL_LoadBMP(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertBMP(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_BMPGetWidth(frame2), h = AGIDL_BMPGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
				    printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeBMP(frame1);
				AGIDL_FreeBMP(frame2);
				AGIDL_FreeBMP(frame3);
				AGIDL_FreeBMP(frame4);
			}break;
			case AGIDL_IMG_TGA:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_TGA* frame1 = AGIDL_LoadTGA(filename1);
				AGIDL_TGA* frame2 = AGIDL_LoadTGA(filename2);
				AGIDL_TGA* frame3 = AGIDL_LoadTGA(filename3);
				AGIDL_TGA* frame4 = AGIDL_LoadTGA(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertTGA(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_TGAGetWidth(frame2), h = AGIDL_TGAGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeTGA(frame1);
				AGIDL_FreeTGA(frame2);
				AGIDL_FreeTGA(frame3);
				AGIDL_FreeTGA(frame4);
			}break;
			case AGIDL_IMG_TIM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_TIM* frame1 = AGIDL_LoadTIM(filename1);
				AGIDL_TIM* frame2 = AGIDL_LoadTIM(filename2);
				AGIDL_TIM* frame3 = AGIDL_LoadTIM(filename3);
				AGIDL_TIM* frame4 = AGIDL_LoadTIM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertTIM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_TIMGetWidth(frame2), h = AGIDL_TIMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeTIM(frame1);
				AGIDL_FreeTIM(frame2);
				AGIDL_FreeTIM(frame3);
				AGIDL_FreeTIM(frame4);
			}break;
			case AGIDL_IMG_PCX:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PCX* frame1 = AGIDL_LoadPCX(filename1);
				AGIDL_PCX* frame2 = AGIDL_LoadPCX(filename2);
				AGIDL_PCX* frame3 = AGIDL_LoadPCX(filename3);
				AGIDL_PCX* frame4 = AGIDL_LoadPCX(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPCX(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_PCXGetWidth(frame2), h = AGIDL_PCXGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePCX(frame1);
				AGIDL_FreePCX(frame2);
				AGIDL_FreePCX(frame3);
				AGIDL_FreePCX(frame4);
			}break;
			case AGIDL_IMG_LMP:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_LMP* frame1 = AGIDL_LoadLMP(filename1);
				AGIDL_LMP* frame2 = AGIDL_LoadLMP(filename2);
				AGIDL_LMP* frame3 = AGIDL_LoadLMP(filename3);
				AGIDL_LMP* frame4 = AGIDL_LoadLMP(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertLMP(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_LMPGetWidth(frame2), h = AGIDL_LMPGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeLMP(frame1);
				AGIDL_FreeLMP(frame2);
				AGIDL_FreeLMP(frame3);
				AGIDL_FreeLMP(frame4);
			}break;
			case AGIDL_IMG_PVR:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PVR* frame1 = AGIDL_LoadPVR(filename1);
				AGIDL_PVR* frame2 = AGIDL_LoadPVR(filename2);
				AGIDL_PVR* frame3 = AGIDL_LoadPVR(filename3);
				AGIDL_PVR* frame4 = AGIDL_LoadPVR(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPVR(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_PVRGetWidth(frame2), h = AGIDL_PVRGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePVR(frame1);
				AGIDL_FreePVR(frame2);
				AGIDL_FreePVR(frame3);
				AGIDL_FreePVR(frame4);
			}break;
			case AGIDL_IMG_GXT:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_GXT* frame1 = AGIDL_LoadGXT(filename1);
				AGIDL_GXT* frame2 = AGIDL_LoadGXT(filename2);
				AGIDL_GXT* frame3 = AGIDL_LoadGXT(filename3);
				AGIDL_GXT* frame4 = AGIDL_LoadGXT(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertGXT(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_GXTGetWidth(frame2), h = AGIDL_GXTGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeGXT(frame1);
				AGIDL_FreeGXT(frame2);
				AGIDL_FreeGXT(frame3);
				AGIDL_FreeGXT(frame4);
			}break;
			case AGIDL_IMG_BTI:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_BTI* frame1 = AGIDL_LoadBTI(filename1);
				AGIDL_BTI* frame2 = AGIDL_LoadBTI(filename2);
				AGIDL_BTI* frame3 = AGIDL_LoadBTI(filename3);
				AGIDL_BTI* frame4 = AGIDL_LoadBTI(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertBTI(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_BTIGetWidth(frame2), h = AGIDL_BTIGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeBTI(frame1);
				AGIDL_FreeBTI(frame2);
				AGIDL_FreeBTI(frame3);
				AGIDL_FreeBTI(frame4);
			}break;
			case AGIDL_IMG_3DF:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_3DF* frame1 = AGIDL_Load3DF(filename1);
				AGIDL_3DF* frame2 = AGIDL_Load3DF(filename2);
				AGIDL_3DF* frame3 = AGIDL_Load3DF(filename3);
				AGIDL_3DF* frame4 = AGIDL_Load3DF(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvert3DF(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_3DFGetWidth(frame2), h = AGIDL_3DFGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_Free3DF(frame1);
				AGIDL_Free3DF(frame2);
				AGIDL_Free3DF(frame3);
				AGIDL_Free3DF(frame4);
			}break;
			case AGIDL_IMG_PPM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PPM* frame1 = AGIDL_LoadPPM(filename1);
				AGIDL_PPM* frame2 = AGIDL_LoadPPM(filename2);
				AGIDL_PPM* frame3 = AGIDL_LoadPPM(filename3);
				AGIDL_PPM* frame4 = AGIDL_LoadPPM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPPM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_PPMGetWidth(frame2), h = AGIDL_PPMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePPM(frame1);
				AGIDL_FreePPM(frame2);
				AGIDL_FreePPM(frame3);
				AGIDL_FreePPM(frame4);
			}break;
			case AGIDL_IMG_LBM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_LBM* frame1 = AGIDL_LoadLBM(filename1);
				AGIDL_LBM* frame2 = AGIDL_LoadLBM(filename2);
				AGIDL_LBM* frame3 = AGIDL_LoadLBM(filename3);
				AGIDL_LBM* frame4 = AGIDL_LoadLBM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertLBM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_LBMGetWidth(frame2), h = AGIDL_LBMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
						
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
						
						num_of_frames_encoded += 3; i += 4;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					f32 ratio = AGMV_CompareFrameSimilarity(frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					if(ratio >= AGMV_GetLeniency(agmv)){
						u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

						AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
						
						printf("Encoding Interpolated Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,interp);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);

						num_of_frames_encoded++; i += 2;
						
						free(interp);
					}
					else{
						printf("Encoding AGIDL Image Frame - %ld...\n",i);
						AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
						printf("Encoded AGIDL Image Frame - %ld...\n",i);
						num_of_frames_encoded++; i++;
					}
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeLBM(frame1);
				AGIDL_FreeLBM(frame2);
				AGIDL_FreeLBM(frame3);
				AGIDL_FreeLBM(frame4);
			}break;
		}
		
		if(i + 4 >= end_frame){
			break;
		}
	}
	
	fseek(file,4,SEEK_SET);
	AGIDL_WriteLong(file,num_of_frames_encoded);

	fseek(file,18,SEEK_SET);
	f32 rate = (f32)num_of_frames_encoded/AGMV_GetNumberOfFrames(agmv);
	AGIDL_WriteLong(file,round(AGMV_GetFramesPerSecond(agmv)*rate));
		
	fclose(file);
	
	free(ext);
	DestroyAGMV(agmv); 
	
	if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
		FILE* file = fopen(filename,"rb");
		fseek(file,0,SEEK_END);
		u32 file_size = ftell(file);
		fseek(file,0,SEEK_SET);
		u8* data = (u8*)malloc(sizeof(u8)*file_size);
		fread(data,1,file_size,file);
		fclose(file);
		
		FILE* out = fopen("GBA_GEN_AGMV.h","w");
		
		fprintf(out,"#ifndef GBA_GEN_AGMV_H\n");
		fprintf(out,"#define GBA_GEN_AGMV_H\n\n");
		fprintf(out,"const unsigned char GBA_AGMV_FILE[%ld] = {\n",file_size);
		
		int i;
		for(i = 0; i < file_size; i++){
			if(i != 0 && i % 4000 == 0){
				fprintf(out,"\n");
			}
			
			fprintf(out,"%d,",data[i]);
		}
		
		fprintf(out,"};\n\n");
		fprintf(out,"#endif");
		
		free(data);
		fclose(out);		
	}
}

void AGMV_EncodeAGMV(AGMV* agmv, const char* filename, const char* dir, const char* basename, u8 img_type, u32 start_frame, u32 end_frame, u32 width, u32 height, u32 frames_per_second, AGMV_OPT opt, AGMV_QUALITY quality, AGMV_COMPRESSION compression){
	u32 i, palette0[256], palette1[256], n, count = 0, num_of_frames_encoded = 0, w, h, num_of_pix, max_clr, size = width*height;
	u32 sample_size, adjusted_num_of_frames = end_frame-start_frame;
	u32 pal[512];

	AGMV_SetOPT(agmv,opt);
	AGMV_SetCompression(agmv,compression);
	
	switch(quality){
		case AGMV_HIGH_QUALITY:{
			max_clr = AGMV_MAX_CLR;
		}break;
		case AGMV_MID_QUALITY:{
			max_clr = 131071;
		}break;
		case AGMV_LOW_QUALITY:{
			max_clr = 65535;
		}break;
		default:{
			max_clr = AGMV_MAX_CLR;
		}break;
	}
	
	u32* colorgram = (u32*)malloc(sizeof(u32)*max_clr+5);
	u32* histogram = (u32*)malloc(sizeof(u32)*max_clr+5);
	
	switch(opt){
		case AGMV_OPT_I:{
			AGMV_SetLeniency(agmv,0); adjusted_num_of_frames /= 2;
		}break;
		case AGMV_OPT_II:{
			AGMV_SetLeniency(agmv,0); adjusted_num_of_frames *= 0.75;
		}break;
		case AGMV_OPT_III:{
			AGMV_SetLeniency(agmv,0); adjusted_num_of_frames *= 0.75;
		}break;
		case AGMV_OPT_ANIM:{
			AGMV_SetLeniency(agmv,0); adjusted_num_of_frames /= 2;
		}break;
		case AGMV_OPT_GBA_I:{
			AGMV_SetLeniency(agmv,0);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
			
			adjusted_num_of_frames /= 2;
		}break;
		case AGMV_OPT_GBA_II:{
			AGMV_SetLeniency(agmv,0);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
			
			adjusted_num_of_frames /= 2;
		}break;
		case AGMV_OPT_GBA_III:{
			AGMV_SetLeniency(agmv,0);
			
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
			
			adjusted_num_of_frames *= 0.75f;
		}break;
		case AGMV_OPT_NDS:{
			AGMV_SetLeniency(agmv,0);
			
			AGMV_SetWidth(agmv,AGMV_NDS_W);
			AGMV_SetHeight(agmv,AGMV_NDS_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_NDS_W*AGMV_NDS_H);
			
			adjusted_num_of_frames *= 0.75;
		}break;
	}

	for(i = 0; i < 512; i++){
		if(i < 256){
			palette0[i] = 0;
			palette1[i] = 0;
		}
		
		pal[i] = 0;
	}
	
	for(i = 0; i < max_clr; i++){
		histogram[i] = 1;
		colorgram[i] = i;
	}
	
	char* ext = AGIDL_GetImgExtension(img_type);
	
	for(i = start_frame; i <= end_frame; i++){
		char filename[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename,"%s/%s%ld%s",dir,basename,i,ext);
		}
		else{
			sprintf(filename,"%s%ld%s",basename,i,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				u32* pixels;
				
				AGIDL_BMP* bmp = AGIDL_LoadBMP(filename);
				AGIDL_ColorConvertBMP(bmp,AGIDL_RGB_888);
				
				pixels = bmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBMP(bmp);
			}break;
			case AGIDL_IMG_TGA:{
				u32* pixels;
				
				AGIDL_TGA* tga = AGIDL_LoadTGA(filename);
				AGIDL_ColorConvertTGA(tga,AGIDL_RGB_888);
				
				pixels = tga->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTGA(tga);
			}break;
			case AGIDL_IMG_TIM:{
				u32* pixels;
				
				AGIDL_TIM* tim = AGIDL_LoadTIM(filename);
				
				pixels = tim->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTIM(tim);
			}break;
			case AGIDL_IMG_PCX:{
				u32* pixels;
				
				AGIDL_PCX* pcx = AGIDL_LoadPCX(filename);
				AGIDL_ColorConvertPCX(pcx,AGIDL_RGB_888);
				
				pixels = pcx->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePCX(pcx);
			}break;
			case AGIDL_IMG_LMP:{
				u32* pixels;
				
				AGIDL_LMP* lmp = AGIDL_LoadLMP(filename);
				AGIDL_ColorConvertLMP(lmp,AGIDL_RGB_888);
				
				pixels = lmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLMP(lmp);
			}break;
			case AGIDL_IMG_PVR:{
				u32* pixels;
				
				AGIDL_PVR* pvr = AGIDL_LoadPVR(filename);
				AGIDL_ColorConvertPVR(pvr,AGIDL_RGB_888);
				
				pixels = pvr->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePVR(pvr);
			}break;
			case AGIDL_IMG_GXT:{
				u32* pixels;
				
				AGIDL_GXT* gxt = AGIDL_LoadGXT(filename);
				AGIDL_ColorConvertGXT(gxt,AGIDL_RGB_888);
				
				pixels = gxt->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeGXT(gxt);
			}break;
			case AGIDL_IMG_BTI:{
				u32* pixels;
				
				AGIDL_BTI* bti = AGIDL_LoadBTI(filename);
				AGIDL_ColorConvertBTI(bti,AGIDL_RGB_888);
				
				pixels = bti->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBTI(bti);
			}break;
			case AGIDL_IMG_3DF:{
				u32* pixels;
				
				AGIDL_3DF* glide = AGIDL_Load3DF(filename);
				AGIDL_ColorConvert3DF(glide,AGIDL_RGB_888);
				
				pixels = glide->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_Free3DF(glide);
			}break;
			case AGIDL_IMG_PPM:{
				u32* pixels;
				
				AGIDL_PPM* ppm = AGIDL_LoadPPM(filename);
				AGIDL_ColorConvertPPM(ppm,AGIDL_RGB_888);
				
				pixels = ppm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePPM(ppm);
			}break;
			case AGIDL_IMG_LBM:{
				u32* pixels;
				
				AGIDL_LBM* lbm = AGIDL_LoadLBM(filename);
				AGIDL_ColorConvertLBM(lbm,AGIDL_RGB_888);
				
				pixels = lbm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLBM(lbm);
			}break;
		}
	}
	
	AGMV_BubbleSort(histogram,colorgram,max_clr);
	
	for(n = max_clr; n > 0; n--){
		Bool skip = FALSE;
			
		u32 clr = colorgram[n];
		
		int r = AGMV_GetQuantizedR(clr,quality);
		int g = AGMV_GetQuantizedG(clr,quality);
		int b = AGMV_GetQuantizedB(clr,quality);
		
		int j;
		for(j = 0; j < 512; j++){
			u32 palclr = pal[j];
			
			int palr = AGMV_GetQuantizedR(palclr,quality);
			int palg = AGMV_GetQuantizedG(palclr,quality);
			int palb = AGMV_GetQuantizedB(palclr,quality);
			
			int rdiff = r-palr;
			int gdiff = g-palg;
			int bdiff = b-palb;
			
			if(rdiff < 0){
				rdiff = AGIDL_Abs(rdiff);
			}
			
			if(gdiff < 0){
				gdiff = AGIDL_Abs(gdiff);
			}
			
			if(bdiff < 0){
				bdiff = AGIDL_Abs(bdiff);
			}
			
			if(quality == AGMV_HIGH_QUALITY){
				if(rdiff <= 2 && gdiff <= 2 && bdiff <= 3){
					skip = TRUE;
				}
			}
			else{
				if(rdiff <= 1 && gdiff <= 1 && bdiff <= 1){
					skip = TRUE;
				}
			}
		}
		
		if(skip == FALSE){
			pal[count] = clr;
			count++;
		}
		
		if(count >= 512){
			break;
		}
	}
	
	if(opt == AGMV_OPT_I || opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_III || opt == AGMV_OPT_GBA_III || opt == AGMV_OPT_NDS){
		for(n = 0; n < 512; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			if(n < 126){
				palette0[n] = invclr;
			}
			else if(n >= 126 && n <= 252){
				palette1[n-126] = invclr;
			}
			
			if(n > 252 && n <= 381){
				palette0[n-126] = invclr;
			}
			
			if(n > 381 && (n-255) < 256){
				palette1[n-255] = invclr;
			}
		}
	}
	
	if(opt == AGMV_OPT_II || opt == AGMV_OPT_GBA_II|| opt == AGMV_OPT_ANIM){
		for(n = 0; n < 256; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			palette0[n] = invclr;
		}
	}
	
	free(colorgram);
	free(histogram);
	
	sample_size = agmv->header.audio_size / (f32)adjusted_num_of_frames;
	
	agmv->audio_chunk->size = sample_size;
	agmv->audio_chunk->atsample = (u8*)malloc(sizeof(u8)*agmv->header.audio_size);
	agmv->audio_track->start_point = 0;
		
	AGMV_CompressAudio(agmv);

	FILE* file = fopen(filename,"wb");
	
	AGMV_SetICP0(agmv,palette0);
	AGMV_SetICP1(agmv,palette1);
	
	printf("Encoding AGMV Header...\n");
	AGMV_EncodeHeader(file,agmv);
	printf("Encoded AGMV Header...\n");
	
	for(i = start_frame; i <= end_frame;){
		char filename1[60],filename2[60],filename3[60],filename4[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename1,"%s/%s%ld%s",dir,basename,i,ext);
			sprintf(filename2,"%s/%s%ld%s",dir,basename,i+1,ext);
			sprintf(filename3,"%s/%s%ld%s",dir,basename,i+2,ext);
			sprintf(filename4,"%s/%s%ld%s",dir,basename,i+3,ext);
		}
		else{
			sprintf(filename1,"%s%ld%s",basename,i,ext);
			sprintf(filename2,"%s%ld%s",basename,i+1,ext);
			sprintf(filename3,"%s%ld%s",basename,i+2,ext);
			sprintf(filename4,"%s%ld%s",basename,i+3,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_BMP* frame1 = AGIDL_LoadBMP(filename1);
				AGIDL_BMP* frame2 = AGIDL_LoadBMP(filename2);
				AGIDL_BMP* frame3 = AGIDL_LoadBMP(filename3);
				AGIDL_BMP* frame4 = AGIDL_LoadBMP(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertBMP(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertBMP(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBMP(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_BMPGetWidth(frame2), h = AGIDL_BMPGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);

					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp); 
				
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeBMP(frame1);
				AGIDL_FreeBMP(frame2);
				AGIDL_FreeBMP(frame3);
				AGIDL_FreeBMP(frame4);
			}break;
			case AGIDL_IMG_TGA:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_TGA* frame1 = AGIDL_LoadTGA(filename1);
				AGIDL_TGA* frame2 = AGIDL_LoadTGA(filename2);
				AGIDL_TGA* frame3 = AGIDL_LoadTGA(filename3);
				AGIDL_TGA* frame4 = AGIDL_LoadTGA(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertTGA(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertTGA(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTGA(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_TGAGetWidth(frame2), h = AGIDL_TGAGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);

					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeTGA(frame1);
				AGIDL_FreeTGA(frame2);
				AGIDL_FreeTGA(frame3);
				AGIDL_FreeTGA(frame4);
			}break;
			case AGIDL_IMG_TIM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_TIM* frame1 = AGIDL_LoadTIM(filename1);
				AGIDL_TIM* frame2 = AGIDL_LoadTIM(filename2);
				AGIDL_TIM* frame3 = AGIDL_LoadTIM(filename3);
				AGIDL_TIM* frame4 = AGIDL_LoadTIM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertTIM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertTIM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleTIM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_TIMGetWidth(frame2), h = AGIDL_TIMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeTIM(frame1);
				AGIDL_FreeTIM(frame2);
				AGIDL_FreeTIM(frame3);
				AGIDL_FreeTIM(frame4);
			}break;
			case AGIDL_IMG_PCX:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PCX* frame1 = AGIDL_LoadPCX(filename1);
				AGIDL_PCX* frame2 = AGIDL_LoadPCX(filename2);
				AGIDL_PCX* frame3 = AGIDL_LoadPCX(filename3);
				AGIDL_PCX* frame4 = AGIDL_LoadPCX(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPCX(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPCX(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePCX(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				
				w = AGIDL_PCXGetWidth(frame2), h = AGIDL_PCXGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePCX(frame1);
				AGIDL_FreePCX(frame2);
				AGIDL_FreePCX(frame3);
				AGIDL_FreePCX(frame4);
			}break;
			case AGIDL_IMG_LMP:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_LMP* frame1 = AGIDL_LoadLMP(filename1);
				AGIDL_LMP* frame2 = AGIDL_LoadLMP(filename2);
				AGIDL_LMP* frame3 = AGIDL_LoadLMP(filename3);
				AGIDL_LMP* frame4 = AGIDL_LoadLMP(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertLMP(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertLMP(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLMP(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_LMPGetWidth(frame2), h = AGIDL_LMPGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
						
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeLMP(frame1);
				AGIDL_FreeLMP(frame2);
				AGIDL_FreeLMP(frame3);
				AGIDL_FreeLMP(frame4);
			}break;
			case AGIDL_IMG_PVR:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PVR* frame1 = AGIDL_LoadPVR(filename1);
				AGIDL_PVR* frame2 = AGIDL_LoadPVR(filename2);
				AGIDL_PVR* frame3 = AGIDL_LoadPVR(filename3);
				AGIDL_PVR* frame4 = AGIDL_LoadPVR(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPVR(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPVR(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePVR(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_PVRGetWidth(frame2), h = AGIDL_PVRGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePVR(frame1);
				AGIDL_FreePVR(frame2);
				AGIDL_FreePVR(frame3);
				AGIDL_FreePVR(frame4);
			}break;
			case AGIDL_IMG_GXT:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_GXT* frame1 = AGIDL_LoadGXT(filename1);
				AGIDL_GXT* frame2 = AGIDL_LoadGXT(filename2);
				AGIDL_GXT* frame3 = AGIDL_LoadGXT(filename3);
				AGIDL_GXT* frame4 = AGIDL_LoadGXT(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertGXT(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertGXT(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleGXT(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_GXTGetWidth(frame2), h = AGIDL_GXTGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);
					AGMV_EncodeAudioChunk(file,agmv);					
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeGXT(frame1);
				AGIDL_FreeGXT(frame2);
				AGIDL_FreeGXT(frame3);
				AGIDL_FreeGXT(frame4);
			}break;
			case AGIDL_IMG_BTI:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_BTI* frame1 = AGIDL_LoadBTI(filename1);
				AGIDL_BTI* frame2 = AGIDL_LoadBTI(filename2);
				AGIDL_BTI* frame3 = AGIDL_LoadBTI(filename3);
				AGIDL_BTI* frame4 = AGIDL_LoadBTI(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertBTI(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertBTI(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleBTI(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_BTIGetWidth(frame2), h = AGIDL_BTIGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeBTI(frame1);
				AGIDL_FreeBTI(frame2);
				AGIDL_FreeBTI(frame3);
				AGIDL_FreeBTI(frame4);
			}break;
			case AGIDL_IMG_3DF:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_3DF* frame1 = AGIDL_Load3DF(filename1);
				AGIDL_3DF* frame2 = AGIDL_Load3DF(filename2);
				AGIDL_3DF* frame3 = AGIDL_Load3DF(filename3);
				AGIDL_3DF* frame4 = AGIDL_Load3DF(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvert3DF(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvert3DF(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScale3DF(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_3DFGetWidth(frame2), h = AGIDL_3DFGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);

					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_Free3DF(frame1);
				AGIDL_Free3DF(frame2);
				AGIDL_Free3DF(frame3);
				AGIDL_Free3DF(frame4);
			}break;
			case AGIDL_IMG_PPM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_PPM* frame1 = AGIDL_LoadPPM(filename1);
				AGIDL_PPM* frame2 = AGIDL_LoadPPM(filename2);
				AGIDL_PPM* frame3 = AGIDL_LoadPPM(filename3);
				AGIDL_PPM* frame4 = AGIDL_LoadPPM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertPPM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertPPM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScalePPM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_PPMGetWidth(frame2), h = AGIDL_PPMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
	
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
					AGMV_EncodeAudioChunk(file,agmv);					
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreePPM(frame1);
				AGIDL_FreePPM(frame2);
				AGIDL_FreePPM(frame3);
				AGIDL_FreePPM(frame4);
			}break;
			case AGIDL_IMG_LBM:{
				printf("Loading Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				AGIDL_LBM* frame1 = AGIDL_LoadLBM(filename1);
				AGIDL_LBM* frame2 = AGIDL_LoadLBM(filename2);
				AGIDL_LBM* frame3 = AGIDL_LoadLBM(filename3);
				AGIDL_LBM* frame4 = AGIDL_LoadLBM(filename4);
				printf("Loaded Group of AGIDL Image Frames - %ld - %ld...\n",i,i+3);
				
				AGIDL_ColorConvertLBM(frame1,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame2,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame3,AGIDL_RGB_888);
				AGIDL_ColorConvertLBM(frame4,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame2,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame3,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame4,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame2,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame3,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
					AGIDL_FastScaleLBM(frame4,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				w = AGIDL_LBMGetWidth(frame2), h = AGIDL_LBMGetHeight(frame2);
				
				num_of_pix = w*h;
				
				if(opt != AGMV_OPT_I && opt != AGMV_OPT_ANIM && opt != AGMV_OPT_GBA_I && opt != AGMV_OPT_GBA_II && opt != AGMV_OPT_NDS){

					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);

					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame2->pixels.pix32,frame3->pixels.pix32,w,h);
					
					printf("Encoding AGIDL Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
					AGMV_EncodeAudioChunk(file,agmv);					
					printf("Encoded AGIDL Image Frame - %ld...\n",i);
					printf("Encoding Interpolated Image Frame - %ld...\n",i+1);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					AGMV_EncodeFrame(file,agmv,frame4->pixels.pix32);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i+3);
					
					num_of_frames_encoded += 3; i += 4;
					
					free(interp);
					
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+3);
				}
				else{
					printf("Performing Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
					
					u32* interp = (u32*)malloc(sizeof(u32)*num_of_pix);

					AGMV_InterpFrame(interp,frame1->pixels.pix32,frame2->pixels.pix32,w,h);
					
					printf("Encoding Interpolated Image Frame - %ld...\n",i);
					AGMV_EncodeFrame(file,agmv,interp);	
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGIDL Image Frame - %ld...\n",i);

					num_of_frames_encoded++; i += 2;
					
					free(interp);
						
					printf("Performed Progressive Differential Interpolated Frame Skipping - %ld - %ld...\n",i,i+1);
				}

				AGIDL_FreeLBM(frame1);
				AGIDL_FreeLBM(frame2);
				AGIDL_FreeLBM(frame3);
				AGIDL_FreeLBM(frame4);
			}break;
		}
		
		if(i + 4 >= end_frame){
			break;
		}
	}
	
	fseek(file,4,SEEK_SET);
	AGIDL_WriteLong(file,num_of_frames_encoded);

	fseek(file,18,SEEK_SET);
	f32 rate = (f32)adjusted_num_of_frames/(AGMV_GetNumberOfFrames(agmv)+1);
	AGIDL_WriteLong(file,round(AGMV_GetFramesPerSecond(agmv)*rate));

	fclose(file);
	
	free(ext);
	DestroyAGMV(agmv); 
	
	if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
		FILE* file = fopen(filename,"rb");
		fseek(file,0,SEEK_END);
		u32 file_size = ftell(file);
		fseek(file,0,SEEK_SET);
		u8* data = (u8*)malloc(sizeof(u8)*file_size);
		fread(data,1,file_size,file);
		fclose(file);
		
		FILE* out = fopen("GBA_GEN_AGMV.h","w");
		
		fprintf(out,"#ifndef GBA_GEN_AGMV_H\n");
		fprintf(out,"#define GBA_GEN_AGMV_H\n\n");
		fprintf(out,"const unsigned char GBA_AGMV_FILE[%ld] = {\n",file_size);
		
		int i;
		for(i = 0; i < file_size; i++){
			if(i != 0 && i % 4000 == 0){
				fprintf(out,"\n");
			}
			
			fprintf(out,"%d,",data[i]);
		}
		
		fprintf(out,"};\n\n");
		fprintf(out,"#endif");
		
		free(data);
		fclose(out);		
	}
}

void AGMV_EncodeFullAGMV(AGMV* agmv, const char* filename, const char* dir, const char* basename, u8 img_type, u32 start_frame, u32 end_frame, u32 width, u32 height, u32 frames_per_second, AGMV_OPT opt, AGMV_QUALITY quality, AGMV_COMPRESSION compression){
	u32 i, palette0[256], palette1[256], n, count = 0, max_clr, size = width*height;
	u32 sample_size;
	u32 pal[512];

	AGMV_SetOPT(agmv,opt);
	AGMV_SetCompression(agmv,compression);
	
	switch(quality){
		case AGMV_HIGH_QUALITY:{
			max_clr = 524287;
		}break;
		case AGMV_MID_QUALITY:{
			max_clr = 131071;
		}break;
		case AGMV_LOW_QUALITY:{
			max_clr = 65535;
		}break;
		default:{
			max_clr = 524287; 
		}break;
	}
	
	u32* colorgram = (u32*)malloc(sizeof(u32)*max_clr+5);
	u32* histogram = (u32*)malloc(sizeof(u32)*max_clr+5);
	
	switch(opt){
		case AGMV_OPT_GBA_I:{
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);

		}break;
		case AGMV_OPT_GBA_II:{
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
		}break;
		case AGMV_OPT_GBA_III:{
			AGMV_SetWidth(agmv,AGMV_GBA_W);
			AGMV_SetHeight(agmv,AGMV_GBA_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_GBA_W*AGMV_GBA_H);
		}break;
		case AGMV_OPT_NDS:{
			AGMV_SetWidth(agmv,AGMV_NDS_W);
			AGMV_SetHeight(agmv,AGMV_NDS_H);
			
			free(agmv->frame->img_data);
			agmv->frame->img_data = (u32*)malloc(sizeof(u32)*AGMV_NDS_W*AGMV_NDS_H);
		}break;
	}

	for(i = 0; i < 512; i++){
		if(i < 256){
			palette0[i] = 0;
			palette1[i] = 0;
		}
		
		pal[i] = 0;
	}
	
	for(i = 0; i < max_clr; i++){
		histogram[i] = 1;
		colorgram[i] = i;
	}
	
	char* ext = AGIDL_GetImgExtension(img_type);
	
	for(i = start_frame; i <= end_frame; i++){
		char filename[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename,"%s/%s%ld%s",dir,basename,i,ext);
		}
		else{
			sprintf(filename,"%s%ld%s",basename,i,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				u32* pixels;
				
				AGIDL_BMP* bmp = AGIDL_LoadBMP(filename);
				AGIDL_ColorConvertBMP(bmp,AGIDL_RGB_888);
				
				pixels = bmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBMP(bmp);
			}break;
			case AGIDL_IMG_TGA:{
				u32* pixels;
				
				AGIDL_TGA* tga = AGIDL_LoadTGA(filename);
				AGIDL_ColorConvertTGA(tga,AGIDL_RGB_888);
				
				pixels = tga->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTGA(tga);
			}break;
			case AGIDL_IMG_TIM:{
				u32* pixels;
				
				AGIDL_TIM* tim = AGIDL_LoadTIM(filename);
				
				pixels = tim->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeTIM(tim);
			}break;
			case AGIDL_IMG_PCX:{
				u32* pixels;
				
				AGIDL_PCX* pcx = AGIDL_LoadPCX(filename);
				AGIDL_ColorConvertPCX(pcx,AGIDL_RGB_888);
				
				pixels = pcx->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePCX(pcx);
			}break;
			case AGIDL_IMG_LMP:{
				u32* pixels;
				
				AGIDL_LMP* lmp = AGIDL_LoadLMP(filename);
				AGIDL_ColorConvertLMP(lmp,AGIDL_RGB_888);
				
				pixels = lmp->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLMP(lmp);
			}break;
			case AGIDL_IMG_PVR:{
				u32* pixels;
				
				AGIDL_PVR* pvr = AGIDL_LoadPVR(filename);
				AGIDL_ColorConvertPVR(pvr,AGIDL_RGB_888);
				
				pixels = pvr->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePVR(pvr);
			}break;
			case AGIDL_IMG_GXT:{
				u32* pixels;
				
				AGIDL_GXT* gxt = AGIDL_LoadGXT(filename);
				AGIDL_ColorConvertGXT(gxt,AGIDL_RGB_888);
				
				pixels = gxt->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeGXT(gxt);
			}break;
			case AGIDL_IMG_BTI:{
				u32* pixels;
				
				AGIDL_BTI* bti = AGIDL_LoadBTI(filename);
				AGIDL_ColorConvertBTI(bti,AGIDL_RGB_888);
				
				pixels = bti->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeBTI(bti);
			}break;
			case AGIDL_IMG_3DF:{
				u32* pixels;
				
				AGIDL_3DF* glide = AGIDL_Load3DF(filename);
				AGIDL_ColorConvert3DF(glide,AGIDL_RGB_888);
				
				pixels = glide->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_Free3DF(glide);
			}break;
			case AGIDL_IMG_PPM:{
				u32* pixels;
				
				AGIDL_PPM* ppm = AGIDL_LoadPPM(filename);
				AGIDL_ColorConvertPPM(ppm,AGIDL_RGB_888);
				
				pixels = ppm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreePPM(ppm);
			}break;
			case AGIDL_IMG_LBM:{
				u32* pixels;
				
				AGIDL_LBM* lbm = AGIDL_LoadLBM(filename);
				AGIDL_ColorConvertLBM(lbm,AGIDL_RGB_888);
				
				pixels = lbm->pixels.pix32;
				
				int n;
				for(n = 0; n < size; n++){
					u32 color = pixels[n];
					u32 hcolor = AGMV_QuantizeColor(color,quality);
					histogram[hcolor] = histogram[hcolor] + 1;
				}
						
				AGIDL_FreeLBM(lbm);
			}break;
		}
	}
	
	AGMV_BubbleSort(histogram,colorgram,max_clr);
	
	for(n = max_clr; n > 0; n--){
		Bool skip = FALSE;
			
		u32 clr = colorgram[n];
		
		int r = AGMV_GetQuantizedR(clr,quality);
		int g = AGMV_GetQuantizedG(clr,quality);
		int b = AGMV_GetQuantizedB(clr,quality);
		
		int j;
		for(j = 0; j < 512; j++){
			u32 palclr = pal[j];
			
			int palr = AGMV_GetQuantizedR(palclr,quality);
			int palg = AGMV_GetQuantizedG(palclr,quality);
			int palb = AGMV_GetQuantizedB(palclr,quality);
			
			int rdiff = r-palr;
			int gdiff = g-palg;
			int bdiff = b-palb;
			
			if(rdiff < 0){
				rdiff = AGIDL_Abs(rdiff);
			}
			
			if(gdiff < 0){
				gdiff = AGIDL_Abs(gdiff);
			}
			
			if(bdiff < 0){
				bdiff = AGIDL_Abs(bdiff);
			}
			
			if(quality == AGMV_HIGH_QUALITY){
				if(rdiff <= 2 && gdiff <= 2 && bdiff <= 3){
					skip = TRUE;
				}
			}
			else{
				if(rdiff <= 1 && gdiff <= 1 && bdiff <= 1){
					skip = TRUE;
				}
			}
		}
		
		if(skip == FALSE){
			pal[count] = clr;
			count++;
		}
		
		if(count >= 512){
			break;
		}
	}
	
	if(opt == AGMV_OPT_I || opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_III || opt == AGMV_OPT_GBA_III || opt == AGMV_OPT_NDS){
		for(n = 0; n < 512; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			if(n < 126){
				palette0[n] = invclr;
			}
			else if(n >= 126 && n <= 252){
				palette1[n-126] = invclr;
			}
			
			if(n > 252 && n <= 381){
				palette0[n-126] = invclr;
			}
			
			if(n > 381 && (n-255) < 256){
				palette1[n-255] = invclr;
			}
		}
	}
	
	if(opt == AGMV_OPT_II || opt == AGMV_OPT_GBA_II|| opt == AGMV_OPT_ANIM){
		for(n = 0; n < 256; n++){
			u32 clr = pal[n];
			u32 invclr = AGMV_ReverseQuantizeColor(clr,quality);
			
			palette0[n] = invclr;
		}
	}
	
	free(colorgram);
	free(histogram);
	
	if(AGMV_GetTotalAudioDuration(agmv) != 0){
		sample_size = agmv->header.audio_size / (f32)(end_frame-start_frame);
		agmv->audio_chunk->size = sample_size;
		agmv->audio_chunk->atsample = (u8*)malloc(sizeof(u8)*agmv->header.audio_size);
		agmv->audio_track->start_point = 0;
			
		AGMV_CompressAudio(agmv);
	}
	
	FILE* file = fopen(filename,"wb");
	
	AGMV_SetICP0(agmv,palette0);
	AGMV_SetICP1(agmv,palette1);
	
	printf("Encoding AGMV Header...\n");
	AGMV_EncodeHeader(file,agmv);
	printf("Encoded AGMV Header...\n");
	
	for(i = start_frame; i <= end_frame; i++){
		char filename1[60];
		if(dir[0] != 'c' || dir[1] != 'u' || dir[2] != 'r'){
			sprintf(filename1,"%s/%s%ld%s",dir,basename,i,ext);
		}
		else{
			sprintf(filename1,"%s%ld%s",basename,i,ext);
		}
		
		switch(img_type){
			case AGIDL_IMG_BMP:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_BMP* frame1 = AGIDL_LoadBMP(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertBMP(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BMPGetWidth(frame1), h = AGIDL_BMPGetHeight(frame1);
					AGIDL_FastScaleBMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeBMP(frame1);
			}break;
			case AGIDL_IMG_TGA:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_TGA* frame1 = AGIDL_LoadTGA(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertTGA(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TGAGetWidth(frame1), h = AGIDL_TGAGetHeight(frame1);
					AGIDL_FastScaleTGA(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeTGA(frame1);
			}break;
			case AGIDL_IMG_TIM:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_TIM* frame1 = AGIDL_LoadTIM(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertTIM(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_TIMGetWidth(frame1), h = AGIDL_TIMGetHeight(frame1);
					AGIDL_FastScaleTIM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeTIM(frame1);
			}break;
			case AGIDL_IMG_PCX:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_PCX* frame1 = AGIDL_LoadPCX(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertPCX(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PCXGetWidth(frame1), h = AGIDL_PCXGetHeight(frame1);
					AGIDL_FastScalePCX(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreePCX(frame1);
			}break;
			case AGIDL_IMG_LMP:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_LMP* frame1 = AGIDL_LoadLMP(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertLMP(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LMPGetWidth(frame1), h = AGIDL_LMPGetHeight(frame1);
					AGIDL_FastScaleLMP(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeLMP(frame1);
			}break;
			case AGIDL_IMG_PVR:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_PVR* frame1 = AGIDL_LoadPVR(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertPVR(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PVRGetWidth(frame1), h = AGIDL_PVRGetHeight(frame1);
					AGIDL_FastScalePVR(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreePVR(frame1);
			}break;
			case AGIDL_IMG_GXT:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_GXT* frame1 = AGIDL_LoadGXT(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertGXT(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_GXTGetWidth(frame1), h = AGIDL_GXTGetHeight(frame1);
					AGIDL_FastScaleGXT(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}

				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeGXT(frame1);
			}break;
			case AGIDL_IMG_BTI:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_BTI* frame1 = AGIDL_LoadBTI(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertBTI(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_BTIGetWidth(frame1), h = AGIDL_BTIGetHeight(frame1);
					AGIDL_FastScaleBTI(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
	
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);	
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				AGIDL_FreeBTI(frame1);
			}break;
			case AGIDL_IMG_3DF:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_3DF* frame1 = AGIDL_Load3DF(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvert3DF(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_3DFGetWidth(frame1), h = AGIDL_3DFGetHeight(frame1);
					AGIDL_FastScale3DF(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_Free3DF(frame1);
			}break;
			case AGIDL_IMG_PPM:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_PPM* frame1 = AGIDL_LoadPPM(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertPPM(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_PPMGetWidth(frame1), h = AGIDL_PPMGetHeight(frame1);
					AGIDL_FastScalePPM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
					
				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreePPM(frame1);
			}break;
			case AGIDL_IMG_LBM:{
				printf("Loading AGIDL Image Frame - %ld\n",i);
				AGIDL_LBM* frame1 = AGIDL_LoadLBM(filename1);
				printf("Loaded AGIDL Image Frame - %ld\n",i);
				
				AGIDL_ColorConvertLBM(frame1,AGIDL_RGB_888);
				
				if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_GBA_W/w)+0.001f,((f32)AGMV_GBA_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}
				
				if(opt == AGMV_OPT_NDS){
					u32 w = AGIDL_LBMGetWidth(frame1), h = AGIDL_LBMGetHeight(frame1);
					AGIDL_FastScaleLBM(frame1,((f32)AGMV_NDS_W/w)+0.001f,((f32)AGMV_NDS_H/h)+0.001f,AGIDL_SCALE_NEAREST);
				}

				printf("Encoding AGIDL Image Frame - %ld...\n",i);
				AGMV_EncodeFrame(file,agmv,frame1->pixels.pix32);
				printf("Encoded AGIDL Image Frame - %ld...\n",i);
				
				if(AGMV_GetTotalAudioDuration(agmv) != 0){
					printf("Encoding AGMV Audio Chunk - %ld...\n",i);					
					AGMV_EncodeAudioChunk(file,agmv);
					printf("Encoded AGMV Audio Chunk - %ld...\n",i);	
				}
				
				AGIDL_FreeLBM(frame1);
			}break;
		}
	}

	fclose(file);
	
	free(ext);
	DestroyAGMV(agmv); 
	
	if(opt == AGMV_OPT_GBA_I || opt == AGMV_OPT_GBA_II || opt == AGMV_OPT_GBA_III){
		FILE* file = fopen(filename,"rb");
		fseek(file,0,SEEK_END);
		u32 file_size = ftell(file);
		fseek(file,0,SEEK_SET);
		u8* data = (u8*)malloc(sizeof(u8)*file_size);
		fread(data,1,file_size,file);
		fclose(file);
		
		FILE* out = fopen("GBA_GEN_AGMV.h","w");
		
		fprintf(out,"#ifndef GBA_GEN_AGMV_H\n");
		fprintf(out,"#define GBA_GEN_AGMV_H\n\n");
		fprintf(out,"const unsigned char GBA_AGMV_FILE[%ld] = {\n",file_size);
		
		int i;
		for(i = 0; i < file_size; i++){
			if(i != 0 && i % 4000 == 0){
				fprintf(out,"\n");
			}
			
			fprintf(out,"%d,",data[i]);
		}
		
		fprintf(out,"};\n\n");
		fprintf(out,"#endif");
		
		free(data);
		fclose(out);		
	}
}
