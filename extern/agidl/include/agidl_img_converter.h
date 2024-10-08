#ifndef AGIDL_IMG_CONVERTER_H
#define AGIDL_IMG_CONVERTER_H

/********************************************
*   Adaptive Graphics Image Display Library
*
*   Copyright (c) 2023-2024 Ryandracus Chapman
*
*   Library: libagidl
*   File: agidl_img_converter.h
*   Date: 11/3/2023
*   Version: 0.1b
*   Updated: 2/25/2024
*   Author: Ryandracus Chapman
*
********************************************/

#include <agidl_img_types.h>
#include <agidl_img_bmp.h>
#include <agidl_img_tga.h>
#include <agidl_img_tim.h>
#include <agidl_img_pcx.h>
#include <agidl_img_quake.h>
#include <agidl_img_pvr.h>
#include <agidl_img_gxt.h>
#include <agidl_img_bti.h>
#include <agidl_img_3df.h>
#include <agidl_img_ppm.h>
#include <agidl_img_lbm.h>

/* BMP -> AGIDL_IMG_TYPE */
AGIDL_TGA* AGIDL_ConvertBMP2TGA(AGIDL_BMP* bmp);
AGIDL_TIM* AGIDL_ConvertBMP2TIM(AGIDL_BMP* bmp);
AGIDL_PCX* AGIDL_ConvertBMP2PCX(AGIDL_BMP* bmp);
AGIDL_LMP* AGIDL_ConvertBMP2LMP(AGIDL_BMP* bmp);
AGIDL_PVR* AGIDL_ConvertBMP2PVR(AGIDL_BMP* bmp);
AGIDL_GXT* AGIDL_ConvertBMP2GXT(AGIDL_BMP* bmp);
AGIDL_BTI* AGIDL_ConvertBMP2BTI(AGIDL_BMP* bmp);
AGIDL_3DF* AGIDL_ConvertBMP23DF(AGIDL_BMP* bmp);
AGIDL_PPM* AGIDL_ConvertBMP2PPM(AGIDL_BMP* bmp);
AGIDL_LBM* AGIDL_ConvertBMP2LBM(AGIDL_BMP* bmp);
/* TGA -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertTGA2BMP(AGIDL_TGA* tga);
AGIDL_TIM* AGIDL_ConvertTGA2TIM(AGIDL_TGA* tga);
AGIDL_PCX* AGIDL_ConvertTGA2PCX(AGIDL_TGA* tga);
AGIDL_LMP* AGIDL_ConvertTGA2LMP(AGIDL_TGA* tga);
AGIDL_PVR* AGIDL_ConvertTGA2PVR(AGIDL_TGA* tga);
AGIDL_GXT* AGIDL_ConvertTGA2GXT(AGIDL_TGA* tga);
AGIDL_BTI* AGIDL_ConvertTGA2BTI(AGIDL_TGA* tga);
AGIDL_3DF* AGIDL_ConvertTGA23DF(AGIDL_TGA* tga);
AGIDL_PPM* AGIDL_ConvertTGA2PPM(AGIDL_TGA* tga);
AGIDL_LBM* AGIDL_ConvertTGA2LBM(AGIDL_TGA* tga);
/* TIM -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertTIM2BMP(AGIDL_TIM* tim);
AGIDL_TGA* AGIDL_ConvertTIM2TGA(AGIDL_TIM* tim);
AGIDL_PCX* AGIDL_ConvertTIM2PCX(AGIDL_TIM* tim);
AGIDL_LMP* AGIDL_ConvertTIM2LMP(AGIDL_TIM* tim);
AGIDL_PVR* AGIDL_ConvertTIM2PVR(AGIDL_TIM* tim);
AGIDL_GXT* AGIDL_ConvertTIM2GXT(AGIDL_TIM* tim);
AGIDL_BTI* AGIDL_ConvertTIM2BTI(AGIDL_TIM* tim);
AGIDL_3DF* AGIDL_ConvertTIM23DF(AGIDL_TIM* tim);
AGIDL_PPM* AGIDL_ConvertTIM2PPM(AGIDL_TIM* tim);
AGIDL_LBM* AGIDL_ConvertTIM2LBM(AGIDL_TIM* tim);
/* PCX -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertPCX2BMP(AGIDL_PCX* pcx);
AGIDL_TGA* AGIDL_ConvertPCX2TGA(AGIDL_PCX* pcx);
AGIDL_TIM* AGIDL_ConvertPCX2TIM(AGIDL_PCX* pcx);
AGIDL_LMP* AGIDL_ConvertPCX2LMP(AGIDL_PCX* pcx);
AGIDL_PVR* AGIDL_ConvertPCX2PVR(AGIDL_PCX* pcx);
AGIDL_GXT* AGIDL_ConvertPCX2GXT(AGIDL_PCX* pcx);
AGIDL_BTI* AGIDL_ConvertPCX2BTI(AGIDL_PCX* pcx);
AGIDL_3DF* AGIDL_ConvertPCX23DF(AGIDL_PCX* pcx);
AGIDL_PPM* AGIDL_ConvertPCX2PPM(AGIDL_PCX* pcx);
AGIDL_LBM* AGIDL_ConvertPCX2LBM(AGIDL_PCX* pcx);
/* LMP -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertLMP2BMP(AGIDL_LMP* lmp);
AGIDL_TGA* AGIDL_ConvertLMP2TGA(AGIDL_LMP* lmp);
AGIDL_TIM* AGIDL_ConvertLMP2TIM(AGIDL_LMP* lmp);
AGIDL_PCX* AGIDL_ConvertLMP2PCX(AGIDL_LMP* lmp);
AGIDL_PVR* AGIDL_ConvertLMP2PVR(AGIDL_LMP* lmp);
AGIDL_GXT* AGIDL_ConvertLMP2GXT(AGIDL_LMP* lmp);
AGIDL_BTI* AGIDL_ConvertLMP2BTI(AGIDL_LMP* lmp);
AGIDL_3DF* AGIDL_ConvertLMP23DF(AGIDL_LMP* lmp);
AGIDL_PPM* AGIDL_ConvertLMP2PPM(AGIDL_LMP* lmp);
AGIDL_LBM* AGIDL_ConvertLMP2LBM(AGIDL_LMP* lmp);
/* PVR -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertPVR2BMP(AGIDL_PVR* pvr);
AGIDL_TGA* AGIDL_ConvertPVR2TGA(AGIDL_PVR* pvr);
AGIDL_TIM* AGIDL_ConvertPVR2TIM(AGIDL_PVR* pvr);
AGIDL_PCX* AGIDL_ConvertPVR2PCX(AGIDL_PVR* pvr);
AGIDL_LMP* AGIDL_ConvertPVR2LMP(AGIDL_PVR* pvr);
AGIDL_GXT* AGIDL_ConvertPVR2GXT(AGIDL_PVR* pvr);
AGIDL_BTI* AGIDL_ConvertPVR2BTI(AGIDL_PVR* pvr);
AGIDL_3DF* AGIDL_ConvertPVR23DF(AGIDL_PVR* pvr);
AGIDL_PPM* AGIDL_ConvertPVR2PPM(AGIDL_PVR* pvr);
AGIDL_LBM* AGIDL_ConvertPVR2LBM(AGIDL_PVR* pvr);
/* GXT -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertGXT2BMP(AGIDL_GXT* gxt);
AGIDL_TGA* AGIDL_ConvertGXT2TGA(AGIDL_GXT* gxt);
AGIDL_TIM* AGIDL_ConvertGXT2TIM(AGIDL_GXT* gxt);
AGIDL_PCX* AGIDL_ConvertGXT2PCX(AGIDL_GXT* gxt);
AGIDL_LMP* AGIDL_ConvertGXT2LMP(AGIDL_GXT* gxt);
AGIDL_PVR* AGIDL_ConvertGXT2PVR(AGIDL_GXT* gxt);
AGIDL_BTI* AGIDL_ConvertGXT2BTI(AGIDL_GXT* gxt);
AGIDL_3DF* AGIDL_ConvertGXT23DF(AGIDL_GXT* gxt);
AGIDL_PPM* AGIDL_ConvertGXT2PPM(AGIDL_GXT* gxt);
AGIDL_LBM* AGIDL_ConvertGXT2LBM(AGIDL_GXT* gxt);
/*BTI -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertBTI2BMP(AGIDL_BTI* bti);
AGIDL_TGA* AGIDL_ConvertBTI2TGA(AGIDL_BTI* bti);
AGIDL_TIM* AGIDL_ConvertBTI2TIM(AGIDL_BTI* bti);
AGIDL_PCX* AGIDL_ConvertBTI2PCX(AGIDL_BTI* bti);
AGIDL_LMP* AGIDL_ConvertBTI2LMP(AGIDL_BTI* bti);
AGIDL_PVR* AGIDL_ConvertBTI2PVR(AGIDL_BTI* bti);
AGIDL_GXT* AGIDL_ConvertBTI2GXT(AGIDL_BTI* bti);
AGIDL_3DF* AGIDL_ConvertBTI23DF(AGIDL_BTI* bti);
AGIDL_PPM* AGIDL_ConvertBTI2PPM(AGIDL_BTI* bti);
AGIDL_LBM* AGIDL_ConvertBTI2LBM(AGIDL_BTI* bti);
/*3DF -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_Convert3DF2BMP(AGIDL_3DF* glide);
AGIDL_TGA* AGIDL_Convert3DF2TGA(AGIDL_3DF* glide);
AGIDL_TIM* AGIDL_Convert3DF2TIM(AGIDL_3DF* glide);
AGIDL_PCX* AGIDL_Convert3DF2PCX(AGIDL_3DF* glide);
AGIDL_LMP* AGIDL_Convert3DF2LMP(AGIDL_3DF* glide);
AGIDL_PVR* AGIDL_Convert3DF2PVR(AGIDL_3DF* glide);
AGIDL_GXT* AGIDL_Convert3DF2GXT(AGIDL_3DF* glide);
AGIDL_BTI* AGIDL_Convert3DF2BTI(AGIDL_3DF* glide);
AGIDL_PPM* AGIDL_Convert3DF2PPM(AGIDL_3DF* glide);
AGIDL_LBM* AGIDL_Convert3DF2LBM(AGIDL_3DF* glide);
/*PPM -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertPPM2BMP(AGIDL_PPM* ppm);
AGIDL_TGA* AGIDL_ConvertPPM2TGA(AGIDL_PPM* ppm);
AGIDL_TIM* AGIDL_ConvertPPM2TIM(AGIDL_PPM* ppm);
AGIDL_PCX* AGIDL_ConvertPPM2PCX(AGIDL_PPM* ppm);
AGIDL_LMP* AGIDL_ConvertPPM2LMP(AGIDL_PPM* ppm);
AGIDL_PVR* AGIDL_ConvertPPM2PVR(AGIDL_PPM* ppm);
AGIDL_GXT* AGIDL_ConvertPPM2GXT(AGIDL_PPM* ppm);
AGIDL_BTI* AGIDL_ConvertPPM2BTI(AGIDL_PPM* ppm);
AGIDL_3DF* AGIDL_ConvertPPM23DF(AGIDL_PPM* ppm);
AGIDL_LBM* AGIDL_ConvertPPM2LBM(AGIDL_PPM* ppm);
/*LBM -> AGIDL_IMG_TYPE */
AGIDL_BMP* AGIDL_ConvertLBM2BMP(AGIDL_LBM* lbm);
AGIDL_TGA* AGIDL_ConvertLBM2TGA(AGIDL_LBM* lbm);
AGIDL_TIM* AGIDL_ConvertLBM2TIM(AGIDL_LBM* lbm);
AGIDL_PCX* AGIDL_ConvertLBM2PCX(AGIDL_LBM* lbm);
AGIDL_LMP* AGIDL_ConvertLBM2LMP(AGIDL_LBM* lbm);
AGIDL_PVR* AGIDL_ConvertLBM2PVR(AGIDL_LBM* lbm);
AGIDL_GXT* AGIDL_ConvertLBM2GXT(AGIDL_LBM* lbm);
AGIDL_BTI* AGIDL_ConvertLBM2BTI(AGIDL_LBM* lbm);
AGIDL_3DF* AGIDL_ConvertLBM23DF(AGIDL_LBM* lbm);
AGIDL_PPM* AGIDL_ConvertLBM2PPM(AGIDL_LBM* lbm);
#endif