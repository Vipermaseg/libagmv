#include "agmv_gba_multi.h"
#include "GBA_GEN_AGMV.h"  
#include "zelda_music_16K_mono.h"

#define VRAM_F  0x6000000 
#define VRAM_B	0x600A000
#define FPS 6

int IWRAM_DATA main(){
	
	SetVideoMode(AGMV_MODE_3);
	EnableTimers();
	
	int lastFr=-1; 
	int frame_counter=0; 
	
	REG_BG2PA = 128;
	REG_BG2PD = 128;

	u16* vram = (u16*)VRAM_F;
	
	File* file = (File*)malloc(sizeof(File));
	
	Open(file,GBA_AGMV_FILE,6753918);
	
	AGMV* agmv = AGMV_AllocResources(file);
	//AGMV* agmv = AGMV_AllocResources_Multi(file, FPS);
	
	//AGMV_DisableAllAudio(agmv);
    /* clear the sound control initially */
    *sound_control = 0;
	
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	//for (int i = 0; i < 224086; i++) {
	//	agmv->audio_chunk->sample[i] = zelda_music_16K_mono[i];
	//}
	//play_sound(agmv->audio_chunk->sample, zelda_music_16K_mono_bytes/4, 16000, 'A');
	//play_sound(zelda_music_16K_mono, zelda_music_16K_mono_bytes/4, 16000, 'A');

	while(1){
		
		VBlankIntrWait();
		scanKeys();
		
		
		if((REG_TM3CNT)!=lastFr){

			if(button_pressed(BUTTON_RIGHT)){
				AGMV_SkipForwards(file,agmv,10);
			}
			
			if(button_pressed(BUTTON_LEFT)){
				AGMV_SkipBackwards(file,agmv,10);
			}
			
			if(button_pressed(BUTTON_A)){
				AGMV_ResetVideo(file,agmv);
			}
			
			if(button_pressed(BUTTON_B)){
				if(agmv->audio_chunk->enable_audio == TRUE){
					agmv->audio_chunk->enable_audio = FALSE;
				}
				else{
					agmv->audio_chunk->enable_audio = TRUE;
				}
			}

			//AGMV_PlayAGMV(file,agmv);
			AGMV_PlayAGMV_E(file,agmv,FPS,frame_counter);
			//AGMV_PlayAudio_Multi(file,agmv,FPS,frame_counter);
			//AGMV_PlayAudio(file,agmv);
			
			frame_counter++;
			if(frame_counter == FPS){
				frame_counter = 0;
			}	
			AGMV_DisplayFrame(vram,240,80,agmv);
		   
		   if(AGMV_IsVideoDone(agmv)){ 
			   AGMV_ResetVideo(file,agmv);
		   }
		   
		   lastFr=(REG_TM3CNT);  

					   
		}		
	}	
	
	free(file);
	DestroyAGMV(agmv);
}
