#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"
 
#if 1 
#define J264 

handle_Encoder *encoder_create(int in_width, int in_height, int out_width, int out_height, int bitrate) {
	handle_Encoder *self = (handle_Encoder *)malloc(sizeof(handle_Encoder));
	memset(self, 0, sizeof(handle_Encoder));

	self->in_width = in_width;
	self->in_height = in_height;
	self->out_width = out_width;
	self->out_height = out_height;
	
	avcodec_register_all();
#ifdef J264
	self->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	self->context = avcodec_alloc_context3(self->codec);
	self->context->bit_rate = bitrate;
 
	self->context->width = out_width;
	self->context->height = out_height;
	self->context->time_base.num = 1;
	self->context->time_base.den = 30;
	//self->context->gop_size = 30;
	self->context->max_b_frames = 0;
	self->context->pix_fmt = AV_PIX_FMT_YUV420P;//PIX_FMT_YUV420P;

	self->context->coder_type = 0;
	self->context->flags |= CODEC_FLAG_LOOP_FILTER;
	self->context->profile = FF_PROFILE_H264_BASELINE; //Baseline
	self->context->scenechange_threshold = 40;
	self->context->gop_size = 90;
	self->context->max_b_frames = 0;
	self->context->max_qdiff = 4;
	self->context->me_method = 7;
	self->context->me_range = 16;
	self->context->me_cmp |= 1;
	self->context->me_subpel_quality = 6;
	self->context->qmin = 10;
	self->context->qmax = 51;
	self->context->qcompress =  0.6;
	self->context->keyint_min = 25;
	self->context->trellis = 0;
	self->context->level =   31;//13; //Level 1.3
	self->context->refs = 1;
	AVDictionary * codec_options(0);
 //	av_dict_set(&codec_options, "preset", "ultrafast", 0);
 	av_dict_set(&codec_options, "preset", "ultrafast", 0);
	av_dict_set(&codec_options, "tune", "zerolatency", 0);
	//self->context->weighted_p_pred = 0;
	//self->context->crf = 20.0f;
	//self->context->flags2 |= CODEC_FLAG2_BPYRAMID - CODEC_FLAG2_WPRED - CODEC_FLAG2_8X8DCT;
	avcodec_open2(self->context, self->codec, &codec_options);
#else
	self->codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	self->context = avcodec_alloc_context3(self->codec);
	self->context->dct_algo = FF_DCT_FASTINT;
	self->context->bit_rate = bitrate;
	self->context->width = out_width;
	self->context->height = out_height;
	self->context->time_base.num = 1;
	self->context->time_base.den = 30;
	self->context->gop_size = 30;
	self->context->max_b_frames = 0;
	self->context->pix_fmt = AV_PIX_FMT_YUV420P;
    
	avcodec_open2(self->context, self->codec, NULL);
#endif	
	 

	self->frame = av_frame_alloc();//avcodec_alloc_frame();
	self->frame->format = AV_PIX_FMT_YUV420P;
	self->frame->width  = out_width;
	self->frame->height = out_height;
	self->frame->pts = 0;
	
	int frame_size = avpicture_get_size(AV_PIX_FMT_YUV420P, out_width, out_height);
	self->frame_buffer = malloc(frame_size);
	avpicture_fill((AVPicture*)self->frame, (uint8_t*)self->frame_buffer, AV_PIX_FMT_YUV420P, out_width, out_height);
	
	self->sws = sws_getContext(
		in_width, in_height, AV_PIX_FMT_RGB32,
		out_width, out_height, AV_PIX_FMT_YUV420P,
		SWS_FAST_BILINEAR, 0, 0, 0
	);


	return self;
}

void encoder_destroy(handle_Encoder *self) {
	sws_freeContext(self->sws);
	avcodec_close(self->context);
	av_free(self->context);	
	av_free(self->frame);
	free(self->frame_buffer);
	free(self);
}


void encoder_encode(handle_Encoder *self, void *rgb_pixels, char *encoded_data, size_t *encoded_size) {
	uint8_t *in_data[1] = {(uint8_t *)rgb_pixels};
	int in_linesize[1] = {self->in_width * 4};
	sws_scale(self->sws, in_data, in_linesize, 0, self->in_height, self->frame->data, self->frame->linesize);
		
	int available_size = *encoded_size;
	*encoded_size = 0;
	self->frame->pts++;
	
	av_init_packet(&self->packet);
	int success = 0;
	avcodec_encode_video2(self->context, &self->packet, self->frame, &success);
	if( success ) {
		if( self->packet.size <= available_size ) {
			memcpy(encoded_data, self->packet.data, self->packet.size);
			*encoded_size = self->packet.size;
		}
		else {
			printf("Frame too large for buffer (size: %d needed: %d)\n", available_size, self->packet.size);
		}
	}
	av_free_packet(&self->packet);
}

#endif