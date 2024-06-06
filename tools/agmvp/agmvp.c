/********************************************
*   Adaptive Graphics Motion Video
*
*   Copyright (c) 2024 Ryandracus Chapman
*
*   Library: libagmv
*   Program: agmvp.exe
*   File: agmvp.h
*   Date: 3/29/2024
*   Version: 1.0
*   Updated: 6/4/2024
*   Author: Ryandracus Chapman
*
********************************************/
#include <windows.h>
#include <mmsystem.h>
#include <Shlwapi.h>
#include <agidl.h>
#include <agmv.h>
#include "screen.h"

/*Game-loop termiating variable*/
static AGIDL_Bool quit;
/*WinAPI Structures Setup*/
static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;
/*WinAPI callback function designed to process window display info such as graphics and keyboard input*/
LRESULT CALLBACK WindowProcessMessage(HWND, UINT, WPARAM, LPARAM);

/* CONTAINS PRIMIARY ATTRIBUTES ABOUT OUR WINDOW AND VIDEO MEMORY*/
Screen* screen; f32 dist = 550.0f;

/* PRIMARY CONTAINER FOR AN AGMV VIDEO FILE */
AGMV* agmv; FILE* file; float fps; float drawInterval;
AGIDL_Bool once = TRUE; 

/* HUD MENU BUTTONS AND ATTRIBUTES*/
AGIDL_BMP* play, *pause, *stop, *skip, *open, *inc_dec, *volume_on, *volume_off;
AGIDL_ATTR hplay, hpause, hstop, hskipf, hskipb, hopen, hinc_dec, hvolume_on, hvolume_off;

WAVEFORMATEX wfx;
HWAVEOUT hWaveOut;
WAVEHDR waveHdr;

void Init(){
	screen = AGMV_CreateScreen(200,200,640,480,AGIDL_RGB_888);
	
	agmv = (AGMV*)malloc(sizeof(AGMV));

	agmv->frame_chunk = (AGMV_FRAME_CHUNK*)malloc(sizeof(AGMV_FRAME_CHUNK));
	agmv->audio_chunk = (AGMV_AUDIO_CHUNK*)malloc(sizeof(AGMV_AUDIO_CHUNK));
	agmv->bitstream = (AGMV_BITSTREAM*)malloc(sizeof(AGMV_BITSTREAM));
	agmv->frame = (AGMV_FRAME*)malloc(sizeof(AGMV_FRAME));
	agmv->iframe = (AGMV_FRAME*)malloc(sizeof(AGMV_FRAME));
	agmv->audio_track = (AGMV_AUDIO_TRACK*)malloc(sizeof(AGMV_AUDIO_TRACK));
	
	play = AGIDL_LoadBMP("res/play.bmp");
	AGIDL_BMPBGR2RGB(play);
	pause = AGIDL_LoadBMP("res/pause.bmp");
	AGIDL_BMPBGR2RGB(pause);
	stop = AGIDL_LoadBMP("res/stop.bmp");
	AGIDL_BMPBGR2RGB(stop);
	skip = AGIDL_LoadBMP("res/skip.bmp");
	AGIDL_BMPBGR2RGB(skip);
	open = AGIDL_LoadBMP("res/open.bmp");
	AGIDL_BMPBGR2RGB(open);
	volume_on = AGIDL_LoadBMP("res/volume_on.bmp");
	AGIDL_BMPBGR2RGB(volume_on);
	volume_off = AGIDL_LoadBMP("res/volume_off.bmp");
	AGIDL_BMPBGR2RGB(volume_off);
	
	inc_dec = AGIDL_LoadBMP("res/inc_dec.bmp");
	AGIDL_BMPBGR2RGB(inc_dec);
}

void InitAttr(){
	AGMV_InitHUD(&hplay,260+15,80,0.09f,0.09f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hpause,275+15,95,0.045f,0.045f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hstop,75+15,97,0.045f,0.045f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hskipf,385+15,80,0.09f,0.09f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hskipb,130+15,78,0.09f,0.09f,TRUE,TRUE,0xFFFFFF,FALSE,TRUE);
	AGMV_InitHUD(&hopen,40,390,0.25f,0.25f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hinc_dec,500+15,94,0.09f,0.09f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hvolume_on,20,94,0.10f,0.10f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
	AGMV_InitHUD(&hvolume_off,20,94,0.10f,0.10f,TRUE,TRUE,0xFFFFFF,FALSE,FALSE);
}

void Cleanup(){
	AGMV_FreeScreen(screen);
	AGIDL_FreeBMP(play);
	AGIDL_FreeBMP(pause);
	AGIDL_FreeBMP(stop);
	AGIDL_FreeBMP(skip);
	AGIDL_FreeBMP(open);
	AGIDL_FreeBMP(inc_dec);
	AGIDL_FreeBMP(volume_on);
	AGIDL_FreeBMP(volume_off);
	DestroyAGMV(agmv);
}

/*Varibles to hold current system time in miliseconds*/
LARGE_INTEGER gTimerFreq;
LARGE_INTEGER gTimerStart;

void timerInit()
{
    QueryPerformanceFrequency(&gTimerFreq);
    QueryPerformanceCounter(&gTimerStart);
}

long osGetSystemTimeMS()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count); 
    return (long)((count.QuadPart - gTimerStart.QuadPart) * 1000L / gTimerFreq.QuadPart);
}

void BlitToScreen(HWND window_handle){
	static HDC device_context;
	device_context = GetDC(window_handle);
	RECT window_rect;
	GetClientRect(window_handle, &window_rect);
	StretchBlt(device_context, 0, 0, window_rect.right, window_rect.bottom, frame_device_context, 0, 0, 624, 441, SRCCOPY);
  //  BitBlt(device_context,0,0,640,480,frame_device_context,0,0,SRCCOPY);
	ReleaseDC(window_handle,device_context);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow){
	
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathRemoveFileSpec(szPath); // Remove the executable filename, leaving the directory
	SetCurrentDirectory(szPath);
	
	LPWSTR lpCmdLineW = GetCommandLineW(); // Get command line as wide string
    LPWSTR *argv;
    int argc;

    // Parse command-line arguments
    argv = CommandLineToArgvW(lpCmdLineW, &argc);
	
	const wchar_t window_class_name[] = L"My Window Class";
	static WNDCLASS window_class = {0};
	window_class.lpfnWndProc = WindowProcessMessage;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = (PCSTR)window_class_name;
	RegisterClass(&window_class);
	
	timerInit();
	
	/*Initialize Bitmap Header Info*/
	frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
	frame_bitmap_info.bmiHeader.biPlanes = 1;
	frame_bitmap_info.bmiHeader.biBitCount = 32;
	frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
	frame_device_context = CreateCompatibleDC(0);
	
	Init();
	InitAttr();
	
	static HWND window_handle;
	window_handle = CreateWindow((PCSTR)window_class_name, "Adaptive Graphics Motion Video Player (AGMVP)", WS_OVERLAPPEDWINDOW |
        CW_USEDEFAULT | CW_USEDEFAULT | WS_VISIBLE, screen->x, screen->y,
	screen->width, screen->height, NULL, NULL, hInstance, NULL);
	
	
	HICON hIcon = (HICON)LoadImage(NULL, "res/agmvp.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	SendMessage(window_handle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(window_handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	
	if(window_handle == NULL){
		return -1;
	}
	
	frame_bitmap_info.bmiHeader.biWidth  = screen->width;
    frame_bitmap_info.bmiHeader.biHeight = screen->height;

    if(frame_bitmap) DeleteObject(frame_bitmap);
    frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&screen->bitmap->vram, 0, 0);
    SelectObject(frame_device_context, frame_bitmap);
	
	fps = 60.0f;
	drawInterval = 1000.00f/fps;
	
    float delta = 0.0f;
    long lastTime = osGetSystemTimeMS();
    long currentTime;
    int timer = 0;
    int drawCount = 0;

	if(argc > 1){

		file = _wfopen(argv[1],L"rb");
						
		if(file == NULL){
			printf("%s\n",AGMV_Error2Str(FILE_NOT_FOUND_ERR));
			screen->HasVideo = FALSE;
		}
		else{
			int error = AGMV_DecodeHeader(file,agmv);
			
			printf("version = %d\n",AGMV_GetVersion(agmv));
	   
			printf("%s\n",AGMV_Error2Str(error));
		   
			if(error != NO_IMG_ERROR){
			   fclose(file);
			   screen->HasVideo = FALSE;
			}
			else{
			   screen->HasVideo = TRUE;
			   fps = AGMV_GetFramesPerSecond(agmv);
			   drawInterval = 1000.0f / fps;
			   agmv->frame_count = 0;
			   
			    agmv->frame->width = agmv->header.width;
			    agmv->frame->height = agmv->header.height;
				agmv->frame->img_data = (u32*)AGIDL_AllocImgDataMMU(agmv->frame->width,agmv->frame->height,AGIDL_RGB_888);
				
				agmv->iframe->width = agmv->header.width;
			    agmv->iframe->height = agmv->header.height;
				agmv->iframe->img_data = (u32*)AGIDL_AllocImgDataMMU(agmv->frame->width,agmv->frame->height,AGIDL_RGB_888);
				
				agmv->bitstream->len = agmv->frame->width*agmv->frame->height*2;
				agmv->bitstream->pos = 0;
				agmv->bitstream->data = (u8*)malloc(sizeof(u8)*agmv->bitstream->len);
			   
			   if(agmv->header.total_audio_duration != 0){	   
				   AGMV_SetAudioState(agmv,TRUE);
				   AGMV_SetVolume(agmv,1.0f);
				   
				   agmv->audio_track->start_point = 0;
				   agmv->audio_track->pcm = (u16*)malloc(sizeof(u16)*agmv->header.audio_size);
				
				   agmv->audio_chunk->size = agmv->header.audio_size / (f32)agmv->header.num_of_frames;
				   
				   AGMV_ParseAGMV(file,agmv);
				   
				   agmv->frame_count = 0;
				   
				    wfx.wFormatTag = WAVE_FORMAT_PCM;
					wfx.nChannels = agmv->header.num_of_channels;
					wfx.nSamplesPerSec = agmv->header.sample_rate;
					wfx.wBitsPerSample = 16;  // Adjust as per your audio data
					wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
					wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
					wfx.cbSize = 0;
					
					// Open the audio device
					waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
					
					// Prepare headers for audio playback
					waveHdr.lpData = agmv->audio_track->pcm;
					waveHdr.dwBufferLength = agmv->header.audio_size*2;
					waveHdr.dwFlags = 0;

					// Prepare and play audio
					waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
					waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

					// Wait for audio playback to finish
					waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
			   }
			   else{	   
				   AGMV_ParseAGMV(file,agmv);
				   agmv->frame_count = 0;
				}
			    
			   AGMV_ResetVideo(file,agmv);
			}
		}
	}
	
	while(!quit){
		static MSG message = {0};
		while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)){
			DispatchMessage(&message);
		}
		
		currentTime = osGetSystemTimeMS();
            
        delta += (currentTime - lastTime)/drawInterval;
        timer += (currentTime - lastTime);
        
        lastTime = currentTime;
        
      //  printf("%d\n",drawCount);
        
        if(delta >= 1){
			
			AGMV_FillScreen(screen,0);

			if(screen->HasVideo == TRUE && screen->IsPlay == TRUE){
				if(AGMV_GetAudioState(agmv) == TRUE){
					waveOutRestart(hWaveOut);
				}
				
				AGIDL_ATTR table = {
					0,0,
					screen->width/(f32)agmv->header.width,screen->height/(f32)agmv->header.height,
					TRUE,
					FALSE,
					0,
					FALSE,
					FALSE
				};
				AGMV_PlayAGMV(file,agmv);
				AGIDL_DispImgData(screen->bitmap->vram,screen->width,screen->height,table,agmv->frame->img_data,agmv->header.width,agmv->header.height,
				agmv->header.fmt);
			}
			else if(screen->HasVideo == TRUE && screen->IsPlay == FALSE){
				waveOutPause(hWaveOut);
				
				if(once == TRUE){
					once = FALSE;
					AGMV_PlayAGMV(file,agmv);
				}
				
				AGIDL_ATTR table = {
					0,0,
					screen->width/(f32)agmv->header.width,screen->height/(f32)agmv->header.height,
					TRUE,
					FALSE,
					0,
					FALSE,
					FALSE
				};
				AGIDL_DispImgData(screen->bitmap->vram,screen->width,screen->height,table,agmv->frame->img_data,agmv->header.width,agmv->header.height,
				agmv->header.fmt);
			}
			
			if(screen->IsHidingHUD == FALSE){
				if(screen->IsPlay == TRUE){
					AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hplay,play);
				}
				else{
					AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hpause,pause);
				}
				AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hstop,stop);
				AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hskipf,skip);
				AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hskipb,skip);
				AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hopen,open);
				AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hinc_dec,inc_dec);
				
				if(AGMV_GetAudioState(agmv) == TRUE){
					AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hvolume_on,volume_on);
				}
				else{
					AGIDL_DispBMP(screen->bitmap->vram,screen->width,screen->height,hvolume_off,volume_off);
				}
			}
			
			if(screen->HasVideo == TRUE && screen->IsHidingHUD == FALSE){
				f32 ratio = agmv->frame_count/(f32)AGMV_GetNumberOfFrames(agmv);
				f32 w = 550 * ratio;
				AGMV_FillRect(screen,25,160,575,10,AGIDL_Color3f(0.87f,0.87f,0.87f,AGIDL_RGB_888));
				AGMV_FillRect(screen,25,162,w,6,BLUE_RGB_888);
			}
			
			if(agmv->frame_count >= agmv->header.num_of_frames || agmv->frame_count < 0){
        		agmv->frame_count = 0;
				
				if(agmv->header.total_audio_duration != 0){
				
					waveOutReset(hWaveOut);
					waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
					waveOutClose(hWaveOut);
					
					waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
				// Prepare headers for audio playback
					waveHdr.lpData = agmv->audio_track->pcm;
					waveHdr.dwBufferLength = agmv->header.audio_size*2;
					waveHdr.dwFlags = 0;
					
					if(AGMV_GetAudioState(agmv) == TRUE){
						// Prepare and play audio
						waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

						// Wait for audio playback to finish
						waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
					}
				
				}
				
				AGMV_ResetVideo(file,agmv);
        	}
			
			if(screen->OpenFile == TRUE){
				OPENFILENAME ofn;
				char szFile[MAX_PATH] = "";
				char szFilter[MAX_PATH] = "AGMV Videos (*.agmv)\0*.agmv\0All Files (*.*)\0*.*\0";
				char szDefaultExt[] = "agmv";

				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = szFilter;
				ofn.lpstrDefExt = szDefaultExt;
				ofn.nFilterIndex = 1;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				// Display the Open dialog box
				if (GetOpenFileName(&ofn) == TRUE) {
					// User selected a file
					printf("Selected file: %s\n", szFile);
					
					if(file != NULL){
						once = TRUE;
						fclose(file);
					}
					
					file = fopen(ofn.lpstrFile,"rb");
					
					if(file == NULL){
						printf("%s\n",AGMV_Error2Str(FILE_NOT_FOUND_ERR));
						screen->HasVideo = FALSE;
					}
					else{
						waveOutPause(hWaveOut);
						
						if(agmv->frame->img_data != NULL){
							free(agmv->frame->img_data);
							free(agmv->iframe->img_data);
							agmv->frame->img_data = NULL;
							agmv->iframe->img_data = NULL;
						}
					    int error = AGMV_DecodeHeader(file,agmv);
				   
					    printf("%s\n",AGMV_Error2Str(error));
					   
					    if(error != NO_IMG_ERROR){
						   fclose(file);
						   screen->HasVideo = FALSE;
					    }
					    else{
						   screen->HasVideo = TRUE;
						   fps = AGMV_GetFramesPerSecond(agmv);
						   drawInterval = 1000.0f / fps;
						   agmv->frame_count = 0;
						   
						   agmv->frame->width = agmv->header.width;
						   agmv->frame->height = agmv->header.height;
						   
						   agmv->frame->img_data = (u32*)malloc(sizeof(u32)*agmv->frame->width*agmv->frame->height);
						   
						   agmv->iframe->width = agmv->header.width;
						   agmv->iframe->height = agmv->header.height;
						   
						   agmv->iframe->img_data = (u32*)malloc(sizeof(u32)*agmv->frame->width*agmv->frame->height);
						   
						   if(agmv->bitstream->data != NULL){
							   free(agmv->bitstream->data);
							   agmv->bitstream->data = NULL;
						   }
						   
						   agmv->bitstream->len = agmv->frame->width*agmv->frame->height*2;
						   agmv->bitstream->pos = 0;
						   agmv->bitstream->data = (u8*)malloc(sizeof(u8)*agmv->bitstream->len);
						   
						   if(agmv->header.total_audio_duration != 0){
							    AGMV_SetAudioState(agmv,TRUE);
								AGMV_SetVolume(agmv,1.0f);
								
							   if(agmv->audio_track->pcm != NULL){
								   free(agmv->audio_track->pcm);
							   }
								agmv->audio_track->start_point = 0;
								agmv->audio_track->pcm = (u16*)malloc(sizeof(u16)*AGMV_GetAudioSize(agmv));

								agmv->audio_chunk->size = agmv->header.audio_size / (f32)AGMV_GetNumberOfFrames(agmv);

							    AGMV_ParseAGMV(file,agmv);
						   
								agmv->frame_count = 0;
							   
								wfx.wFormatTag = WAVE_FORMAT_PCM;
								wfx.nChannels = agmv->header.num_of_channels;
								wfx.nSamplesPerSec = agmv->header.sample_rate;
								wfx.wBitsPerSample = 16;  // Adjust as per your audio data
								wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
								wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
								wfx.cbSize = 0;
								
								waveOutReset(hWaveOut);
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutClose(hWaveOut);
								
								// Open the audio device
								waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
								
								// Prepare headers for audio playback
								waveHdr.lpData = agmv->audio_track->pcm;
								waveHdr.dwBufferLength = agmv->header.audio_size*2;
								waveHdr.dwFlags = 0;

								if(AGMV_GetAudioState(agmv) == TRUE){
									// Prepare and play audio
									waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
									waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

									// Wait for audio playback to finish
									waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								}
						   }
						   else{
							   AGMV_ParseAGMV(file,agmv);
							   agmv->frame_count = 0;
							}
						   
						   AGMV_ResetVideo(file,agmv);
					    }
					}
					
				} else {
					// Check for specific error
					DWORD dwError = CommDlgExtendedError();
					if (dwError != 0) {
						// Error occurred
						printf("Open dialog error: %lu\n", dwError);
					} else {
						// User canceled
						printf("Open dialog canceled.\n");
					}
				}
			}
        	
            BlitToScreen(window_handle);
			
			screen->OpenFile = FALSE;
	   		
   			drawCount++;
			delta--;
        }      
   
        if(timer >= 1000){
            drawCount = 0;          	                   
            timer = 0;
        }
    
	}

	Cleanup();
	
	fclose(file);
	
	return 0;
}

DWORD savedStyle;
RECT savedRect;

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		case WM_QUIT:
		case WM_DESTROY:{
			quit = TRUE;
		}break;
		case WM_KEYDOWN:{
			if(wParam == H_PRESSED){
				if(screen->IsHidingHUD == TRUE){
					screen->IsHidingHUD = FALSE;
				}
				else{
					screen->IsHidingHUD = TRUE;
				}
			}
			
			if(wParam == SPACE_PRESSED){
				if(screen->IsPlay == TRUE){
					screen->IsPlay = FALSE;
				}
				else{
					screen->IsPlay = TRUE;
				}
			}
			
			if(wParam == M_PRESSED){
				if(AGMV_GetAudioState(agmv) == TRUE){
					AGMV_SetAudioState(agmv,FALSE);
					waveOutPause(hWaveOut);
				}
				else{
					AGMV_SetAudioState(agmv,TRUE);
					waveOutReset(hWaveOut);
					waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
					waveOutClose(hWaveOut);
					
					waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
				// Prepare headers for audio playback
					
					f32 start = agmv->frame_count / fps;
					
					int startbyteaddr = (start * agmv->header.sample_rate) * 2;
					int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

					waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
					waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
					waveHdr.dwFlags = 0;

					// Prepare and play audio
					waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
					waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

					// Wait for audio playback to finish
					waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
				}
			}
			
			if(wParam == D_PRESSED){
				if(screen->HasVideo == TRUE){
					if(agmv->header.total_audio_duration != 0){
						AGMV_SkipForwards(file,agmv,30);
						waveOutReset(hWaveOut);
						waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						waveOutClose(hWaveOut);
						
						waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
					// Prepare headers for audio playback
						
						f32 start = agmv->frame_count / fps;
						
						int startbyteaddr = (start * agmv->header.sample_rate) * 2;
						int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

						waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
						waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
						waveHdr.dwFlags = 0;

						if(AGMV_GetAudioState(agmv) == TRUE){
							// Prepare and play audio
							waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

							// Wait for audio playback to finish
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						}
					}
					else{
						AGMV_SkipForwards(file,agmv,30);
					}
				}
			}
			
			if(wParam == A_PRESSED){
				if(screen->HasVideo == TRUE){
					AGMV_SkipBackwards(file,agmv,30);
					if(agmv->header.total_audio_duration != 0){
						waveOutReset(hWaveOut);
						waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						waveOutClose(hWaveOut);
						
						waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
					// Prepare headers for audio playback
						
						f32 start = agmv->frame_count / fps;
						
						int startbyteaddr = (start * agmv->header.sample_rate) * 2;
						int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

						waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
						waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
						waveHdr.dwFlags = 0;

						if(AGMV_GetAudioState(agmv) == TRUE){
							// Prepare and play audio
							waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

							// Wait for audio playback to finish
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						}
					}
				}
			}
			
			if(wParam == ENTER_PRESSED){
				if(screen->IsFullscreen == TRUE){
					screen->IsFullscreen = FALSE;
				}
				else{
					screen->IsFullscreen = TRUE;
				}
				
				if (GetWindowLong(window_handle, GWL_STYLE) & WS_OVERLAPPEDWINDOW) {
                    savedStyle = GetWindowLong(window_handle, GWL_STYLE);

                    // Get the dimensions of the primary monitor
                    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

                    SetWindowLong(window_handle, GWL_STYLE, savedStyle & ~WS_OVERLAPPEDWINDOW);
                    SetWindowPos(window_handle, HWND_TOP, 0, 0, screenWidth, screenHeight,
                                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                } else {
                    SetWindowLong(window_handle, GWL_STYLE, savedStyle);
                    SetWindowPos(window_handle, HWND_TOP, 200, 200,
                                640, 480,
                                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                }
			}
			
			if(wParam == ARROW_UP_PRESSED){
				f32 volume = AGMV_GetVolume(agmv);
				
				volume += 0.05f;
				
				AGMV_SetVolume(agmv,volume);
				
				DWORD dwVolume = (DWORD)(AGMV_GetVolume(agmv) * 0xFFFF);
				dwVolume = (dwVolume & 0xFFFF) | (dwVolume << 16); // Set left and right channel

				// Set the volume
				MMRESULT result = waveOutSetVolume(hWaveOut, dwVolume);
			}
			
			if(wParam == ARROW_DOWN_PRESSED){
				f32 volume = AGMV_GetVolume(agmv);
				
				volume -= 0.05f;
				
				AGMV_SetVolume(agmv,volume);
				
				DWORD dwVolume = (DWORD)(AGMV_GetVolume(agmv) * 0xFFFF);
				dwVolume = (dwVolume & 0xFFFF) | (dwVolume << 16); // Set left and right channel

				// Set the volume
				MMRESULT result = waveOutSetVolume(hWaveOut, dwVolume);
			}
        }break;
		case WM_KILLFOCUS:{
			
		} break;
		case WM_CREATE:{
	   }break;

		case WM_SETFOCUS:

		case WM_LBUTTONDOWN:{
			int x = LOWORD(lParam);
			int y = screen->height - HIWORD(lParam);
			
			if(screen->IsFullscreen == FALSE){
				if(screen->HasVideo == TRUE){
					if(x >= 25 && y >= 160 && x <= 600 && y <= 170){
						f32 ratio = x/(f32)600;
						u32 skip = agmv->header.num_of_frames * ratio;
						AGMV_SkipTo(file,agmv,skip);
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;
							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}
			else{
				if(screen->HasVideo == TRUE){
					if(x >= 54 && y >= -175 && x <= 1316 && y <= -58){
						f32 ratio = x/(f32)1316;
						u32 skip = agmv->header.num_of_frames * ratio;
						AGMV_SkipTo(file,agmv,skip);
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}

			if(screen->IsFullscreen == FALSE){
				if(x >= 42 && y >= 394 && x <= 90 && y <= 440){
					screen->OpenFile = TRUE;
				}
			}
			else{
				if(x >= 93 && y >= 233 && x <= 201 && y <= 420){
					screen->OpenFile = TRUE;
				}
			}
			
			if(screen->IsFullscreen == FALSE){
				if(x >= 296 && y >= 105 && x <= 336 && y <= 142){
					if(screen->IsPlay == TRUE){
						screen->IsPlay = FALSE;
					}
					else{
						screen->IsPlay = TRUE;
					}
				}
			}
			else{
				if(x >= 649 && y >= -172 && x <= 727 && y <= -102){
					if(screen->IsPlay == TRUE){
						screen->IsPlay = FALSE;
					}
					else{
						screen->IsPlay = TRUE;
					}
				}
			}
			
			if(screen->IsFullscreen == FALSE){
				if(x >= 97 && y >= 109 && x <= 126 && y <= 135){
					if(screen->HasVideo == TRUE){
						AGMV_ResetVideo(file,agmv);
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							waveHdr.lpData = agmv->audio_track->pcm;
							waveHdr.dwBufferLength = agmv->header.audio_size*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}
			else{
				if(x >= 211 && y >= -170 && x <= 277 && y <= -118){
					if(screen->HasVideo == TRUE){
						AGMV_ResetVideo(file,agmv);
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							waveHdr.lpData = agmv->audio_track->pcm;
							waveHdr.dwBufferLength = agmv->header.audio_size*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}
			
			if(screen->IsFullscreen == FALSE){		
				if(x >= 164 && y >= 108 && x <= 215 && y <= 137){
					if(screen->HasVideo == TRUE){

						AGMV_SkipBackwards(file,agmv,5 * AGMV_GetFramesPerSecond(agmv));
						
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}
			else{
				if(x >= 360 && y >= -170 && x <= 462 && y <= -112){
					if(screen->HasVideo == TRUE){
						
						AGMV_SkipBackwards(file,agmv,5 * AGMV_GetFramesPerSecond(agmv));
						
						if(agmv->header.total_audio_duration != 0){
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
				}
			}
			
			if(screen->IsFullscreen == FALSE){
				if(x >= 418 && y >= 108 && x <= 471 && y <= 142){
					if(screen->HasVideo == TRUE){
						if(agmv->header.total_audio_duration != 0){
							
							AGMV_SkipForwards(file,agmv,10);
							
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
					}
					else{
						AGMV_SkipForwards(file,agmv,10);
					}
				}
			}
			else{
				if(x >= 919 && y >= -163 && x <= 1017 && y <= -107){
					if(screen->HasVideo == TRUE){
						if(agmv->header.total_audio_duration != 0){
							AGMV_SkipForwards(file,agmv,10);
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							if(AGMV_GetAudioState(agmv) == TRUE){
								// Prepare and play audio
								waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
								waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

								// Wait for audio playback to finish
								waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							}
						}
						else{
							AGMV_SkipForwards(file,agmv,10);
						}
					}
				}
			}
			
			if(screen->HasVideo == TRUE){
				if(screen->IsFullscreen == FALSE){
					if(x >= 30 && y >= 107 && x <= 65 && y <= 138){
						if(AGMV_GetAudioState(agmv) == TRUE){
							AGMV_SetAudioState(agmv,FALSE);
							waveOutPause(hWaveOut);
						}
						else{
							AGMV_SetAudioState(agmv,TRUE);
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							// Prepare and play audio
							waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

							// Wait for audio playback to finish
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						}
					}
				}
				else{
					if(x >= 65 && y >= -176 && x <= 140 && y <= -113){
						if(AGMV_GetAudioState(agmv) == TRUE){
							AGMV_SetAudioState(agmv,FALSE);
							waveOutPause(hWaveOut);
						}
						else{
							AGMV_SetAudioState(agmv,TRUE);
							waveOutReset(hWaveOut);
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutClose(hWaveOut);
							
							waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
						// Prepare headers for audio playback
							
							f32 start = agmv->frame_count / fps;
							
							int startbyteaddr = (start * agmv->header.sample_rate) * 2;
							int endbyteaddr = (agmv->header.total_audio_duration * agmv->header.sample_rate) * 2;

							waveHdr.lpData = agmv->audio_track->pcm + startbyteaddr;
							waveHdr.dwBufferLength = (endbyteaddr-startbyteaddr)*2;
							waveHdr.dwFlags = 0;

							// Prepare and play audio
							waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
							waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

							// Wait for audio playback to finish
							waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
						}
					}
				}
			}
			else{
				if(screen->IsFullscreen == FALSE){
					if(x >= 30 && y >= 107 && x <= 65 && y <= 138){
						if(AGMV_GetAudioState(agmv) == TRUE){
							AGMV_SetAudioState(agmv,FALSE);
						}
						else{
							AGMV_SetAudioState(agmv,TRUE);
						}
					}
				}
				else{
					if(x >= 65 && y >= -176 && x <= 140 && y <= -113){
						if(AGMV_GetAudioState(agmv) == TRUE){
							AGMV_SetAudioState(agmv,FALSE);
						}
						else{
							AGMV_SetAudioState(agmv,TRUE);
						}
					}
				}
			}
			
			if(x >= 519 && y >= 129 && x <= 557 && y <=153){
				f32 volume = AGMV_GetVolume(agmv);
				
				volume += 0.05f;
				
				AGMV_SetVolume(agmv,volume);
				
				DWORD dwVolume = (DWORD)(AGMV_GetVolume(agmv) * 0xFFFF);
				dwVolume = (dwVolume & 0xFFFF) | (dwVolume << 16); // Set left and right channel

				// Set the volume
				MMRESULT result = waveOutSetVolume(hWaveOut, dwVolume);
			}
			
			if(x >= 519 && y <= 123 && x <= 558 && y >= 97){
				f32 volume = AGMV_GetVolume(agmv);
				
				volume -= 0.05f;
				
				AGMV_SetVolume(agmv,volume);
				
				DWORD dwVolume = (DWORD)(AGMV_GetVolume(agmv) * 0xFFFF);
				dwVolume = (dwVolume & 0xFFFF) | (dwVolume << 16); // Set left and right channel

				// Set the volume
				MMRESULT result = waveOutSetVolume(hWaveOut, dwVolume);
			}
			
		}break;
		case WM_SYSKEYUP:
        default:{
            return DefWindowProc(window_handle, message, wParam, lParam);
        }break;
    }
	return 0;
}