#ifndef __CTRL_H__
#define __CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif	

    int mouse_click(void *window, int x, int y);
    
    int mouse_button_release();
    
    int mouse_button_press();

    int mouse_move( void *window, int x, int y);

#ifdef __cplusplus
}
#endif

#endif 