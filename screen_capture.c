#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "screen_capture.h"
 
void enum_windows(Display* display, Window window, int depth, char *windowName, Window *childWindow, int *w, int *h) {

    int i;
 
    XTextProperty text;
    XGetWMName(display, window, &text);
    //  char* name;  
   // XFetchName(display, window, &name);
 
    //if(name != NULL )
    if(text.value != NULL ){    
   //     if (strcmp(name,windowName)==0)
        if (strcmp( text.value,windowName)==0){ 
            Window root_return;
            int x_return, y_return;
            unsigned int width_return, height_return;
            unsigned int border_width_return;
            unsigned int depth_return;

            XGetGeometry(display, window, &root_return, &x_return, &y_return, &width_return, 
                      &height_return, &border_width_return, &depth_return);
 
           *w =  width_return;
           *h =  height_return;    
           *childWindow = window;
        }
    }

    Window root, parent;
    Window* children;
    int n;
    XQueryTree(display, window, &root, &parent, &children, &n);
    if (children != NULL) {
        for (i = 0; i < n; i++) {
            enum_windows(display, children[i], depth + 1, windowName, childWindow, w, h);
        }
        XFree(children);
    }
}

int x11_screen_size(unsigned int displayIndex, char* windowName, int *x, int *y, int *w, int *h) {
 
    const char* display_str; 
    Display* display = NULL; 
    int screen = 0; 
    Window root_window = 0; 
    int cw=0,ch=0;
    char buf[16];
    snprintf(buf, sizeof(buf), ":0.%u", displayIndex);
    display_str = buf;

    display = XOpenDisplay(display_str);
    
    *x=0;  *y=0; *w=0; *h=0;

    if(!display){
        printf("Cannot open X11 display!\n"); 
        return -1;
    }

    screen = DefaultScreen(display);
    root_window =   RootWindow(display, screen);
 
    *w = DisplayWidth(display, screen);
    *h = DisplayHeight(display, screen);

 //----------------------
    if(windowName!=NULL){ 
        *w = 0;
        *h = 0;
        Window child_window=NULL;
        enum_windows(display, root_window, 0, windowName , &child_window, &cw, &ch);
 
        if(child_window){
            root_window = child_window;
            *w = cw;
            *h = ch;
        }
    }
 //----------------------- 
    XCloseDisplay(display);
    return 0;
}

int x11_screen_subwindow(unsigned int displayIndex, char* windowName, void** getWindow ) {
    
    if(windowName==NULL){
        return -1;
    } 
 
    const char* display_str; 
    Display* display = NULL; 
    int screen = 0; 
    Window root_window = 0;  
    char buf[16];
    int child_width=0,child_height=0;

    snprintf(buf, sizeof(buf), ":0.%u", displayIndex);
    display_str = buf;
    display = XOpenDisplay(display_str);

    screen = DefaultScreen(display);
    root_window =   RootWindow(display, screen);
 
    if(!display){
        printf("Cannot open X11 display!\n"); 
        return -1;
    } 
      
    if(*getWindow==NULL){
        *getWindow = (void*) malloc(sizeof(Window));   
    }
      
    Window child_window=NULL;
    enum_windows(display, root_window, 0, windowName , &child_window, &child_width, &child_height);
 
    if(child_window){
        (* (Window *) (*getWindow))  = child_window;              
    }      
  
    return 0;
}

/**
 * \brief Grab X11 screen.
 * \param x11display display string (i.e. :0.0), if NULL getenv("DISPLAY") is used
 * \param data array that will contain screen capture
 * \param displayIndex display index
 * \param x x position to start capture
 * \param y y position to start capture
 * \param w capture width
 * \param h capture height 
 * \return 0 if success, -1 otherwise
 */
int x11_grab_screen(char* data, unsigned int displayIndex, int x, int y, int w, int h, void *subwindow , int subwindow_width, int subwindow_height) {
 
  const char* display_str; /* display string */
  Display* display = NULL; /* X11 display */
  Visual* visual = NULL;
  int screen = 0; /* X11 screen */
  Window root_window = 0; /* X11 root window of a screen */
  int width = 0;
  int height = 0;
  int depth = 0;
  int shm_support = 0;
  XImage* img = NULL;
  XShmSegmentInfo shm_info;
  size_t off = 0;
  int i = 0;
  int j = 0;
  size_t size = 0;
  uint32_t test = 0;
  int little_endian = *((uint8_t*)&test);
  char buf[16];
  
  int child_width = 0;
  int child_height = 0;

  //XEvent ev;

  snprintf(buf, sizeof(buf), ":0.%u", displayIndex);
  display_str = buf;

  /* open current X11 display */
  display = XOpenDisplay(display_str);
//  display = XOpenDisplay(getenv("DISPLAY"));
 
  if(!display)
  {
    /* fprintf(stderr, "Cannot open X11 display!\n"); */
    return -1;
  }
 
  screen = DefaultScreen(display);
  root_window =   RootWindow(display, screen);
  visual = DefaultVisual(display, screen);
  width = DisplayWidth(display, screen);
  height = DisplayHeight(display, screen);
  depth = DefaultDepth(display, screen);
 
  //----------------------
  if(subwindow!=NULL && subwindow_width!=0 && subwindow_height!=0){
      root_window = (*(Window *) subwindow) ;
      width = subwindow_width;
      height = subwindow_height;     
         
  }  
  
  if((w + x) > width || (h + y) > height)
  {
    fprintf(stderr, "check that user-defined parameters are in image\n"); 
    XCloseDisplay(display);
    return -1;
  }
 
  size = w * h;
 
  /* test is XServer support SHM */
  shm_support = XShmQueryExtension(display);
 
  /* fprintf(stderr, "Display=%s width=%d height=%d depth=%d SHM=%s\n", display_str, width, height, depth, shm_support ? "true" : "false"); */ 
  if(shm_support)
  {
    /* fprintf(stderr, "Use XShmGetImage\n"); */
 
    /* create image for SHM use */
    img = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shm_info, w, h);

    if(!img)
    {
      printf( "Image cannot be crssseated!\n");  
      XCloseDisplay(display);
      return -1;
    }
 
    /* setup SHM stuff */
    shm_info.shmid = shmget(IPC_PRIVATE, img->bytes_per_line * img->height, IPC_CREAT | 0777);
    shm_info.shmaddr = (char*)shmat(shm_info.shmid, NULL, 0);
    img->data = shm_info.shmaddr;
    shmctl(shm_info.shmid, IPC_RMID, NULL);
    shm_info.readOnly = 0;
  
    /* attach segment and grab screen */
    if((shm_info.shmaddr == (void*)-1) || !XShmAttach(display, &shm_info))
    { 
      /* fprintf(stderr, "Cannot use shared memory!\n"); */
      if(shm_info.shmaddr != (void*)-1)
      {
        shmdt(shm_info.shmaddr);
      }
 
      img->data = NULL;
      XDestroyImage(img);
      img = NULL;
      shm_support = 0;
    }
    else if(!XShmGetImage(display, root_window, img, x, y, 0xffffffff))
    {      
      /* fprintf(stderr, "Cannot grab image!\n"); */
      XShmDetach(display, &shm_info);
      shmdt(shm_info.shmaddr);
      XDestroyImage(img);
      img = NULL;
      shm_support = 0;
    }
 
  }
 
  /* if XSHM is not available or has failed 
   * use XGetImage
   */
  if(!img)
  {
    /* fprintf(stderr, "Use XGetImage\n"); */
    img = XGetImage(display, root_window, x, y, w, h, 0xffffffff, ZPixmap);

    if(!img)
    {
      /* fprintf(stderr, "Cannot grab image!\n"); */
      XCloseDisplay(display);
      return -1;
    }
  }
 
  /* convert to bytes but keep ARGB */
  for(j = 0 ; j < h ; j++)
  {
    for(i = 0 ; i < w ; i++)
    {
      /* do not care about high 32-bit for Linux 64 bit 
       * machine (sizeof(unsigned long) = 8)
       */
      uint32_t pixel = (uint32_t)XGetPixel(img, i, j) | (0xff << 24);

      /* Java int is always big endian so output as ARGB */
      if(little_endian)
      {
        /* ARGB is BGRA in little-endian */
        uint8_t r = (pixel >> 16) & 0xff;
        uint8_t g = (pixel >> 8) & 0xff;
        uint8_t b = pixel & 0xff;
        pixel = b << 24 | g << 16 | r << 8 | 0xff;
 
      }else{
        uint8_t r = (pixel >> 16) & 0xff;
        uint8_t g = (pixel >> 8) & 0xff;
        uint8_t b = pixel & 0xff;
        pixel = 0xff << 24 | r << 16 | g << 8 | b ; //BGRA ARGB
    
      }
       
        memcpy(data + off, &pixel, 4);
        off += 4;
 
    }
  }

  /* free X11 resources and close display */
  XDestroyImage(img);
  
  if(shm_support)
  {
    XShmDetach(display, &shm_info);
    shmdt(shm_info.shmaddr);
  }

  XCloseDisplay(display);
   
  /* return array */
  return 0;
}
 
/*
the gnome window manager, ignores the function XRaiseWindow or - 
more precisely - only honours it under certain circumstances. 
    
    Metacity allows XRaiseWindow when the same application keeps 
    focus but defines an application by its window group. 
    Some [...] older applications also do not set the window group 
    and, consequently, metacity did not honor window-raising requests 
    from such applications. 

Apparently calling XRaiseWindow is the old way of doing it. 
The new way is creating an xevent and sending it to the root window 
of the window you actually want to raise.
*/
void raiseWindow(Display * display, Window win){

  XEvent xev;
  Window root;

  xev.type = ClientMessage;
  xev.xclient.display = display;
  xev.xclient.window = win;
  xev.xclient.message_type = XInternAtom(
  display,"_NET_ACTIVE_WINDOW",0);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 2L;
  xev.xclient.data.l[1] = CurrentTime;

  root = XDefaultRootWindow(display);

  XSendEvent( display,root,0,
  SubstructureNotifyMask | 
  SubstructureRedirectMask,
  &xev);
}

int x11_raise_subwindow(unsigned int displayIndex, void* setWindow ){
    
    const char* display_str; /* display string */
    char buf[16];
    Display* display = NULL; /* X11 display */
    Window subwindow = 0;  
  
    snprintf(buf, sizeof(buf), ":0.%u", displayIndex);
    display_str = buf;

    /* open current X11 display */
    display = XOpenDisplay(display_str);
 
    if(!display){
        printf("x11_raise_subwindow Cannot open X11 display!\n");
        return -1;
    }
  
    subwindow = (*(Window *) setWindow) ;
 
    // ret =  XRaiseWindow(display, subwindow);
    raiseWindow(display, subwindow);
 
    XCloseDisplay(display);

    return 0;
} 
 