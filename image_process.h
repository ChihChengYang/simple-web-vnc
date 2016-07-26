#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif	
 
    int ip_image_clip( char *destData,  
    	char *srcData, int src_width, int src_height, int clip_top, int clip_left, int clip_bottom, int clip_right  );
 

#ifdef __cplusplus
}
#endif

#endif 	