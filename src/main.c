#include <stdio.h>
#include <agidl.h>
#include <time.h>
#include <agmv.h>

int main(){
	float startTimeInterval = (float)clock() / CLOCKS_PER_SEC;
	
	AGMV_ExportAGMVToHeader("quick_export.agmv");
	//AGMV_EncodeVideo("agmv_splash.agmv",".", "quick_export_",AGMV_IMG_BMP,1,119,320,240,24,AGMV_OPT_GBA_I,AGMV_MID_QUALITY,AGMV_LZSS_COMPRESSION);
	
	float endTimeInterval = (float)clock() / CLOCKS_PER_SEC;
	
	printf("deltaTime = %.4f\n",endTimeInterval-startTimeInterval);

	return 0;
}
