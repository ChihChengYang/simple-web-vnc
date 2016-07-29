
package main
/*
#cgo LDFLAGS: -L/usr/lib/ -lapp
#cgo CFLAGS: -I./
#include <stdio.h>
#include <stdlib.h> 
#include "app.h"
*/
import "C"

import (
    "github.com/gorilla/websocket"
    "flag"
    "html/template"
    "log"
    "net/http"
    "fmt" 
    "time"
    "unsafe"
    "runtime"
   "encoding/json" 
    "sync"
)

var upgrader = websocket.Upgrader{} // use default options

func indexHandler(w http.ResponseWriter, r *http.Request) { 
    t, err := template.ParseFiles("../js/index.html")
    if err != nil {
        http.Error(w, err.Error(),
            http.StatusInternalServerError) 
        return
    }
    t.Execute(w,  "ws://"+r.Host+"/play")
}

func js(w http.ResponseWriter, r *http.Request) { 
    http.ServeFile(w, r, "../"+r.URL.Path[1:])
}
 
type sMouse struct {    
    t float64
    w float64
    h float64
    x float64
    y float64 
}
 
type connection struct {
  // The websocket connection.
  ws *websocket.Conn
  // Buffered channel of outbound messages.
  send chan []byte 
  appHandle *C.handle_App
  mouseTime int64
  mouseEvent chan sMouse
  sync.RWMutex
  streamingWindowInfo sStreamingWindow
}

const (
  maxMessageSize = 1024//512
)

type sStreamingWindow struct {    
    windowName string
    outputWidth int
    outputHeight int
    clip_top int
    clip_left int 
    clip_bottom int
    clip_right int
}
var streamingWindow sStreamingWindow
  
func (conn *connection) appMessage() {
 
    conn.ws.SetReadLimit(maxMessageSize)     
    for {
        _, message, err := conn.ws.ReadMessage()
        if err != nil {    
            break
        }
 
        conn.RLock()
 
        u := map[string]interface{}{}   
        json.Unmarshal(message, &u) 
       
        if u["t"].(float64) == 0 || u["t"].(float64) == 2 || u["t"].(float64) == 1 {
            
            now := time.Now()
            t:=now.UnixNano() / 1000000             
            
            var mouse sMouse
            mouse.t = u["t"].(float64)
            mouse.w = u["w"].(float64)
            mouse.h = u["h"].(float64)
            mouse.x = u["x"].(float64)
            mouse.y = u["y"].(float64)

            flag:=false
                        
            if  u["t"].(float64) != 1 || conn.mouseTime==0  {
                conn.mouseEvent <- mouse          
                conn.mouseTime = t
                flag = true
            }
            if t-conn.mouseTime > 10 && !flag{ // mouse move over X ms
                conn.mouseEvent <- mouse
                conn.mouseTime = t             
            }             
        }    
    
        u = nil 
        conn.RUnlock()           
    }
}

func (conn *connection) appStreaming() {
//------------------------------------------ 
    if conn.streamingWindowInfo.windowName != "" {    
//------------------------------------------    
        wn := C.CString(conn.streamingWindowInfo.windowName)
        conn.appHandle = C.app_create(1024, 0, 0, 
            C.int(conn.streamingWindowInfo.outputWidth),
            C.int(conn.streamingWindowInfo.outputHeight), 
            wn, 
            C.int(conn.streamingWindowInfo.clip_top), 
            C.int(conn.streamingWindowInfo.clip_left), 
            C.int(conn.streamingWindowInfo.clip_bottom), 
            C.int(conn.streamingWindowInfo.clip_right));
//------------------------------------------

    }else{
//------------------------------------------    
        conn.appHandle = C.app_create(1024, 0, 0, 
            C.int(conn.streamingWindowInfo.outputWidth),
            C.int(conn.streamingWindowInfo.outputHeight), 
            nil, 
            C.int(conn.streamingWindowInfo.clip_top), 
            C.int(conn.streamingWindowInfo.clip_left), 
            C.int(conn.streamingWindowInfo.clip_bottom), 
            C.int(conn.streamingWindowInfo.clip_right));
//------------------------------------------  
    }
 
    var outData [1024*1024]byte;
    var outSize uint32 
 
    tick := time.NewTicker(time.Millisecond * 30)
    flag := true    
    for {
        select {
            case <-tick.C:
                C.app_run(conn.appHandle,(*C.char)(unsafe.Pointer(&outData[0])),(*C.ulong)(unsafe.Pointer(&outSize)))
    
                smallArray := make([]byte,outSize)
                copy(smallArray[:],outData[0:outSize] ) 
                err := conn.ws.WriteMessage(2,  smallArray)               
                if err != nil {
                    fmt.Printf("conn.WriteMessage ERROR!!!\n")
                    flag = false
                    break
                }
                smallArray = nil
  
            case mouseEvent := <-conn.mouseEvent:               
                if mouseEvent.t == 0{
                    C.app_mouseButtonPress(conn.appHandle, C.int(mouseEvent.w), C.int(mouseEvent.h), C.float(mouseEvent.x),  C.float(mouseEvent.y) ) 
                    fmt.Printf("app_mouseButtonPress\n")
                }
                if mouseEvent.t == 2{
                    C.app_mouseButtonRelease();
                }
                if mouseEvent.t == 1{
                    C.app_mouseMove(conn.appHandle, C.int(mouseEvent.w), C.int(mouseEvent.h), C.float(mouseEvent.x),  C.float(mouseEvent.y) )
                }
                
        }
        if !flag {
            break
        }
        runtime.Gosched()
    }
    C.app_destroy(conn.appHandle);
}

func play(w http.ResponseWriter, r *http.Request) {
  
    ws, err := upgrader.Upgrade(w, r, nil)
    if err != nil {
        log.Print("upgrade:", err)
        return
    }
    defer ws.Close()

    c := &connection{send: make(chan []byte, 256), ws: ws} 
    c.mouseTime = 0
    c.mouseEvent = make(chan sMouse)   
    c.streamingWindowInfo = streamingWindow
    go c.appMessage() 
    c.appStreaming()  
    
    fmt.Fprintf(w, "ok")
}

func (ssw *sStreamingWindow) setStreamingWindow(windowName string, outputWidth int, outputHeight int, clip_top int, clip_left int, clip_bottom int, clip_right int) {
    ssw.windowName = windowName
    ssw.outputWidth = outputWidth
    ssw.outputHeight = outputHeight
    ssw.clip_top = clip_top
    ssw.clip_left = clip_left
    ssw.clip_bottom = clip_bottom
    ssw.clip_right = clip_right
}


//  windowName :=  "Android [Running] - Oracle VM VirtualBox" //"root@ubuntu: /home/jeff/vnc/src/bin"  
//"ARC Welder" //"Candy Crush Saga" //"Unsaved Image 1 - Pinta" //"root@ubuntu: /home/jeff/vnc/src/bin" 
func main() {

    runtime.GOMAXPROCS(runtime.NumCPU())
 
    streamingWindow.windowName = ""
    streamingWindow.outputWidth = 800
    streamingWindow.outputHeight = 600
    streamingWindow.clip_top = 0
    streamingWindow.clip_left = 0
    streamingWindow.clip_bottom = 0
    streamingWindow.clip_right = 0
    //streamingWindow.setStreamingWindow("Unsaved Image 1* - Pinta" ,800,600,0,0,0,0)
 
    flag.Parse()
    log.SetFlags(0)
  
    http.HandleFunc("/js/", js) 
    http.HandleFunc("/", indexHandler) 
    http.HandleFunc("/play", play)

    http.ListenAndServe("0.0.0.0:8888", nil)
}

 
