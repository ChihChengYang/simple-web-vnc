#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "image_process.h"


int ip_image_clip( char *destData, char *srcData, int src_width, int src_height,
     int clip_top, int clip_left, int clip_bottom, int clip_right  ){

    // memcpy(data + off, &pixel, 4);
    int dest_height = clip_bottom - clip_top;
    int dest_width = clip_right - clip_left;
    int dW = dest_width * 4;
    int sW = src_width * 4;
    int sT = clip_top*sW;
    int sL = clip_left*4;
    int j,i,d=0,s=0;
    
    for(j = 0 ; j < dest_height ; j++){
        for(i = 0 ; i < dW ; i+=4){
            d = i+(j*dW);
            s = (i+sL)+((j*sW)+sT);
            destData[d] = srcData[s];
            destData[d+1] = srcData[s+1];
            destData[d+2] = srcData[s+2];
            destData[d+3] = srcData[s+3];
        }
    }
    return 0;
}
 
