CC = gcc
INCLUDES = -I"$(CURDIR)/extern/agidl/include" -I"$(CURDIR)/include"
CFLAGS = -Wall -O2 $(INCLUDES)
LDFLAGS = -L"$(CURDIR)/extern/agidl/lib" -lagidl -lm
DEPS = include/agmv_utils.h \
		include/agmv_encode.h \
		include/agmv_decode.h \
		include/agmv_playback.h \
		include/agmv_defines.h \
		include/agmv.h
		
OBJFILES = src/agmv_utils.o \
		src/agmv_encode.o \
		src/agmv_decode.o \
		src/agmv_playback.o \
		extern/agidl/src/agidl_math_utils.o \
		extern/agidl/src/agidl_cc_manager.o \
		extern/agidl/src/agidl_cc_converter.o \
		extern/agidl/src/agidl_cc_mixer.o \
		extern/agidl/src/agidl_cc_chromascan.o \
		extern/agidl/src/agidl_img_types.o \
		extern/agidl/src/agidl_imgp_grayscale.o \
		extern/agidl/src/agidl_imgp_rotate.o \
		extern/agidl/src/agidl_imgp_scale.o \
		extern/agidl/src/agidl_imgp_filter.o \
		extern/agidl/src/agidl_imgp_lightmap.o \
		extern/agidl/src/agidl_imgp_mirror.o \
		extern/agidl/src/agidl_imgp_blend.o \
		extern/agidl/src/agidl_imgp_wrap.o \
		extern/agidl/src/agidl_imgp_font.o \
		extern/agidl/src/agidl_imgp_mipmap.o \
		extern/agidl/src/agidl_imgp_halftone.o \
		extern/agidl/src/agidl_imgp_impl.o \
		extern/agidl/src/agidl_img_bmp.o \
		extern/agidl/src/agidl_img_tga.o \
		extern/agidl/src/agidl_img_tim.o \
		extern/agidl/src/agidl_img_pcx.o \
		extern/agidl/src/agidl_img_quake.o \
		extern/agidl/src/agidl_img_pvr.o \
		extern/agidl/src/agidl_img_gxt.o \
		extern/agidl/src/agidl_img_bti.o \
		extern/agidl/src/agidl_img_3df.o \
		extern/agidl/src/agidl_img_ppm.o \
		extern/agidl/src/agidl_img_lbm.o \
		extern/agidl/src/agidl_img_converter.o \
		extern/agidl/src/agidl_img_search.o \
		extern/agidl/src/agidl_img_compression.o \
		extern/agidl/src/agidl_img_error.o \
		extern/agidl/src/agidl_file_utils.o \
		extern/agidl/src/agidl_mmu_utils.o \
		extern/agidl/src/agidl_gd_display.o \
		extern/agidl/src/agidl_vid_str.o \
		extern/agidl/src/agidl_img_export.o
		
OBJS =  src/agmv_utils.o \
		src/agmv_encode.o \
		src/agmv_decode.o \
		src/agmv_playback.o \
		src/main.o

TARGET = main
ARCHIVE = libs/libagmv.a

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	
archive:
	ar rcs $(ARCHIVE) $(OBJFILES)
	
	
clean:
	rm -f $(TARGET) $(OBJS) *~