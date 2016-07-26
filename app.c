
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include "image_process.h"
#include "encoder.h"
#include "screen_capture.h"
#include "ctrl.h"

#include "app.h"

#define APP_FRAME_BUFFER_SIZE 1024*1024
#define APP_WINDOW_NAME_SIZE 255
 
handle_App *app_create(int bit_rate, int in_width, int in_height, int out_width, int out_height, char *windowName,
    int clip_top, int clip_left, int clip_bottom, int clip_right ) {

    int x, y, w, h;
	handle_App *self = (handle_App *)malloc(sizeof(handle_App));
	memset(self, 0, sizeof(handle_App));
  
    if(windowName==NULL){
        self->scWindowName = NULL;
    }else{
    	self->scWindowName = malloc(APP_WINDOW_NAME_SIZE); 
        snprintf( self->scWindowName, APP_WINDOW_NAME_SIZE, "%s", windowName );	
    }
 
    w=in_width;
    h=in_height;
    if(in_width==0 || in_height==0){
    	x11_screen_size(0, self->scWindowName, &x , &y , &w, &h);    	
    } 
     
    self->in_width = w;
    self->in_height = h;
    
    self->imageData = malloc(self->in_width * self->in_height * 4);
 
    self->out_width = out_width; 
    self->out_height = out_height;
    self->scWindow = NULL;

    x11_screen_subwindow( 0, self->scWindowName, &self->scWindow);

    //=====================================
    int clip_W,clip_H;
    clip_W = clip_right-clip_left;
    clip_H = clip_bottom-clip_top;
    if(clip_bottom>0 && clip_right>0 && (clip_W>0 && clip_W<=w) && (clip_H>0 && clip_H<=h)){
        self->clip_top = clip_top; 
        self->clip_left = clip_left;
        self->clip_bottom = clip_bottom; 
        self->clip_right = clip_right;
 
        self->shrink_in_width = clip_W; 
        self->shrink_in_height = clip_H;
        self->shrinkImageData = malloc(self->shrink_in_width * self->shrink_in_height * 4); 
        memset(self->shrinkImageData, 0, self->shrink_in_width*self->shrink_in_height*4);

        self->hEncoder = (void *) encoder_create(            
            self->shrink_in_width,self->shrink_in_height, // in size 
            out_width, out_height, // out size
            bit_rate * 1024
        );
 
    }else{
        self->shrinkImageData = NULL;
      
        self->hEncoder = (void *) encoder_create(
        w, h, // in size      
        out_width, out_height, // out size
        bit_rate * 1024
    );
    }
    printf(" app_create ========> (%d,%d)\n",w,h); //(1100,750)
    
    //===================================== 


    
    return self;
}

void app_destroy(handle_App *self) {
	encoder_destroy((handle_Encoder *)self->hEncoder);
	if( self->imageData ){
	    free( self->imageData );
	    self->imageData =0;
	} 
	if( self->scWindowName ){
	    free( self->scWindowName );
	    self->scWindowName =0;
	}
	if(self->scWindow){
		free(self->scWindow);
		self->scWindow = 0;
	}
//=====================================
    if( self->shrinkImageData ){
        free( self->shrinkImageData );
        self->shrinkImageData =0;
    } 
//=====================================     
}
   
void app_run(handle_App *self, char *outData, unsigned long *outSize) {
     
    int ret = x11_grab_screen( self->imageData, 0, 0, 0, self->in_width, self->in_height, self->scWindow, self->in_width, self->in_height );    
    
    *outSize = 0; 
    if(ret==0){
        *outSize = APP_FRAME_BUFFER_SIZE;
        //
        //=====================================
        if(self->shrinkImageData != NULL){
            ip_image_clip(self->shrinkImageData, self->imageData, self->in_width, self->in_height, 
                self->clip_top, self->clip_left, self->clip_bottom, self->clip_right );
            encoder_encode((handle_Encoder *)self->hEncoder, self->shrinkImageData, outData, outSize); 
        }else{
            encoder_encode((handle_Encoder *)self->hEncoder, self->imageData, outData, outSize); 
        }
        //=====================================
    } 
} 

// static int uuu=10;
void app_mouseButtonPress(handle_App *self, int w, int h, float x, float y) { 
   printf("%s (%d,%d) %d,%d,%f,%f (%d,%d)\n","mouseEvent.w server...",self->in_width,self->in_height, w,h,x,y , (int) (self->in_width * (x/(float)w)), (int) (self->in_height * (y/(float)h)));
 
    if(self->scWindowName != NULL){
        x11_raise_subwindow(0, self->scWindow );
    }

    mouse_move( self->scWindow,  (int) (self->in_width * (x/(float)w)) , (int) (self->in_height * (y/(float)h)));
 
    //mouse_move( self->scWindow,  0 , uuu);
    //uuu+=10;
   	mouse_button_press();  
  
}

void app_mouseButtonRelease() { 
  	mouse_button_release();  
}

void app_mouseMove(handle_App *self, int w, int h, float x, float y) {
 
    if(w==0||h==0){
    	return;
    }  
    mouse_move( self->scWindow,  (int) (self->in_width * (x/(float)w)) , (int) (self->in_height * (y/(float)h)));  
}
 