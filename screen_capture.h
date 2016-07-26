#ifndef __SCREEN_CAPTURE_H__
#define __SCREEN_CAPTURE_H__
 
#ifdef __cplusplus
extern "C" {
#endif	

    int x11_grab_screen(char* data, unsigned int displayIndex, int x, int y, int w, int h, void *subwindow , int subwindow_width, int subwindow_height);

    int x11_screen_size(unsigned int displayIndex, char* windowName, int *x, int *y, int *w, int *h);

    int x11_screen_subwindow( unsigned int displayIndex, char* windowName, void** getWindow );

    int x11_raise_subwindow(unsigned int displayIndex, void* setWindow );
 
#ifdef __cplusplus
}
#endif

#endif 