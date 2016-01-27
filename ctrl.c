 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/XTest.h>

struct position{
    int x;
    int y;
};

int current_pos(Window w, struct position *pos){
    Display *dpy = XOpenDisplay(NULL);
    XEvent event;

    if(!pos)
        return -1;
    if(!dpy)
        return -1;

    XQueryPointer(dpy, w,
        &event.xbutton.root, &event.xbutton.window,
        &event.xbutton.x_root, &event.xbutton.y_root,
        &event.xbutton.x, &event.xbutton.y,
        &event.xbutton.state);
	
    pos->x = event.xbutton.x_root;
    pos->y = event.xbutton.y_root;
	
    XCloseDisplay(dpy);
    return 0;
}

int mouse_click(void *window, int x, int y){

    Display *dpy = XOpenDisplay(NULL);
    if(dpy == NULL){
        return -1;
    } 	
 	if(window == NULL){
        return -1;
    }
 
    XEvent event;
 
  /* get info about current pointer position */
    XQueryPointer(dpy,  (*(Window *) window), //RootWindow(dpy, DefaultScreen(dpy)),
        &event.xbutton.root, &event.xbutton.window,
        &event.xbutton.x_root, &event.xbutton.y_root,
        &event.xbutton.x, &event.xbutton.y,
        &event.xbutton.state);
 
    XTestFakeMotionEvent(dpy, -1, x, y, 0);
    XTestFakeButtonEvent(dpy, 1, 1, 0);
    usleep(10);
    XTestFakeButtonEvent(dpy, 1, 0, 0);
    /* place the mouse where it was */
    XTestFakeMotionEvent(dpy, -1, event.xbutton.x, event.xbutton.y, 0);
    XCloseDisplay(dpy);

    fprintf( stdout, "clickAt %d,%d\n",x,y);
    return 0;
}
 
int mouse_button_release(){
    Display *dpy = XOpenDisplay(NULL);
	if(!dpy)
		return -1;
    XTestFakeButtonEvent(dpy, 1, 0, 0);
    XFlush(dpy);
	XCloseDisplay(dpy);

    return 0;
}

int mouse_button_press(){

    Display *dpy = XOpenDisplay(NULL);
	if(!dpy)
		return -1;
  
    Window root_window = RootWindow(dpy, 0);

    struct position position;
 	int ret;
 	ret = current_pos( root_window ,&position);
 	if(ret < 0)
 		return ret; 

	XTestFakeMotionEvent(dpy, -1,  position.x,  position.y, 0); 
    XFlush(dpy);
    XTestFakeButtonEvent(dpy, 1, 1, 0);
    XFlush(dpy);

   	XCloseDisplay(dpy);
    return 0;
}

int mouse_move( void *window, int x, int y){
	Display *dpy = XOpenDisplay(NULL);
	if(!dpy)
		return -1;

    Window root_window = RootWindow(dpy, 0);
    int cx=0,cy=0;
  
    if(window!=NULL){
        Window sub_window = (*(Window *) window);
        if(root_window!=sub_window){
            XMapRaised(dpy, sub_window);
            XFlush(dpy);
 	
            Window child=NULL;
            XWindowAttributes xwa;
            XTranslateCoordinates( dpy, sub_window, root_window, 0, 0, &cx, &cy, &child );
        }
    } 
 	XTestFakeMotionEvent(dpy, -1, cx+x , cy+y, 0); 	
    XFlush(dpy);    
	XCloseDisplay(dpy);
 
    return 0;
}