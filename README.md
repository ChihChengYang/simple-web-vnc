================ 
simple-web-vnc
================
 
Try to create a low latency web vnc (no concern network conditions).
 
It captures the screen or a application window based on Linux (X11) , encodes that into H.264 (FFmpeg libs), 
stream it over [WebSocket](https://github.com/gorilla/websocket "WebSocket") to the browser where it is finally decoded in a [H.264 JavaScript](https://github.com/oneam/h264bsd "H.264 JavaScript") software-based library.

Browser sends control commands (mouse commands) to server over WebSocket as well.

1. make
2. run /bin/server

Server --- H.264 ---> Browser (http://serverIP:8888)
Server <--- Commands --- Browser

Encoder: H.264 (FFmpeg libs)
Protocol: Websocket (Go lib)
Decoder: H.264 (JavaScript H.264)  

Build on Ubuntu 14.04 64bit
 
[Info & Demo](http://chihchengyang.github.io/2016/01/27/Simple-web-vnc/ "Info & Demo")

 
	
	

	
	


 