 
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
}

const (
  maxMessageSize = 512
)
 
func (conn *connection) appMessage() {

    conn.ws.SetReadLimit(maxMessageSize)     
    for {
        _, message, err := conn.ws.ReadMessage()
        if err != nil {
           break
        }
        
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
    }
}


func (conn *connection) appStreaming() {
 
    /// windowName :=  "Android [Running] - Oracle VM VirtualBox" //"root@ubuntu: /home/jeff/vnc/src/bin"  //"ARC Welder" //"Candy Crush Saga" //"Unsaved Image 1 - Pinta" //"root@ubuntu: /home/jeff/vnc/src/bin" 
    // wn := C.CString(windowName)
     conn.appHandle = C.app_create(1024, 0, 0, 800,600, nil);
   // conn.appHandle = C.app_create(1024, 0,0, 640,400, wn);
 
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
                    break
                }
  
            case mouseEvent := <-conn.mouseEvent:               
                if mouseEvent.t == 0{
                    C.app_mouseButtonPress(conn.appHandle, C.int(mouseEvent.w), C.int(mouseEvent.h), C.float(mouseEvent.x),  C.float(mouseEvent.y) )
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
    go c.appMessage() 
    c.appStreaming()  
    
    fmt.Fprintf(w, "ok")
}

func main() {

	runtime.GOMAXPROCS(runtime.NumCPU())
  
  flag.Parse()
	log.SetFlags(0)
  
  http.HandleFunc("/js/", js) 
	http.HandleFunc("/", indexHandler) 
  http.HandleFunc("/play", play)

  http.ListenAndServe("0.0.0.0:8888", nil)
}

 
