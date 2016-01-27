#ifndef __ENCODER_H__
#define __ENCODER_H__
 
#ifdef __cplusplus
extern "C" {
#endif	

	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
 	#include "libavutil/avutil.h"
 	#include "libavutil/pixfmt.h"
  
 	typedef struct {
		AVCodec *codec;
		AVCodecContext *context;
		AVFrame *frame;
		void *frame_buffer;

		int in_width, in_height;
		int out_width, out_height;
	
		AVPacket packet;
		SwsContext *sws;
    } handle_Encoder; 
 
    handle_Encoder *encoder_create(int in_width, int in_height, int out_width, int out_height, int bitrate);
 
    void encoder_destroy(handle_Encoder *self);
    
    void encoder_encode(handle_Encoder *self, void *rgb_pixels, char *encoded_data, size_t *encoded_size);

 #ifdef __cplusplus
}
#endif

#endif


