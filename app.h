#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C" {
#endif	

    typedef struct {
	    void *hEncoder; 
	    char *imageData;
	    char *scWindowName;
        int in_width, in_height;
        int out_width, out_height;
        void *scWindow; 
    } handle_App;
    
    handle_App *app_create(int bit_rate, int in_width, int in_height, int out_width, int out_height, char *windowName);

    void app_destroy(handle_App *self);

    void app_run(handle_App *self, char *outData, unsigned long *outSize);    
 
    void app_mouseButtonPress(handle_App *self, int w, int h, float x, float y);

    void app_mouseMove(handle_App *self, int w, int h, float x, float y);

    void app_mouseButtonRelease(); 

#ifdef __cplusplus
}
#endif

#endif 