// jsmpeg by Dominic Szablewski - phoboslab.org, github.com/phoboslab

(function(window){ "use strict";
 
var SOCKET_MAGIC_BYTES = 'jsmp'; 
var sequenceStarted2 = false;
//======================================
    var canvas = document.getElementById('videoCanvas');
    var pictureCount = 0;
    var lastPictureCount = 0;
    // Create the decoder and canvas
    var decoder = new Worker('js/h264bsd_worker.js');
    // var decoder = new Worker('h264bsd_worker.min.js');
    var display = new H264bsdCanvas(canvas);    
    var buf = null;

    var ws;

    var mouseFlag = false;
//======================================
    
var js264 = window.js264 = function( url, opts ) {
    opts = opts || {}; 
    this.canvas = opts.canvas || document.createElement('canvas'); 
    if( url instanceof WebSocket ) {
        ws = this.client = url;
        this.client.onopen = this.initSocketClient.bind(this);      
    } 
    else {
        this.load(url);
    }   
    
    this.canvas.addEventListener("mousedown", this.mousedownPosition, false);
    this.canvas.addEventListener("mousemove", this.mousemovePosition, false);
    this.canvas.addEventListener("mouseup", this.mouseupPosition, false);
}; 

js264.prototype.mouseupPosition = function (event){
   
    mouseFlag = false;

    event = event || window.event;
    // firefox , event.pageX = undefined
    var scrollLeft = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
    var scrollTop = document.documentElement.scrollTop || document.body.scrollTop || 0;
    
    var x = event.pageX != undefined ? event.pageX : event.clientX;
    var y = event.pageY != undefined ? event.pageY : event.clientY;

    // var x = event.x;
    // var y = event.y;
    var borderW,borderH;
    var rect = canvas.getBoundingClientRect();
    
    borderW = ((rect.bottom - rect.top ) - canvas.height ) / 2;
    borderH = ((rect.right - rect.left ) - canvas.width ) / 2;
   
    x = x-rect.left-borderW  - scrollLeft;
    y = y-rect.top-borderH - scrollTop;

    if(x<0){
        x=0;
    }
    if(y<0){
        y=0;
    }
    if(x>=canvas.width){
        x=canvas.width-1;
    }
    if(y>=canvas.height){
        y=canvas.height-1;
    }
        
    ws.send(JSON.stringify({
        t: 2, // mouseup
        w: canvas.width,
        h: canvas.height,
        x: x,
        y: y
    }));

    //console.log('mouseupPosition ' );
}

js264.prototype.mousemovePosition = function (event){
    event.target.style.cursor = 'pointer';
    if(mouseFlag){
        event = event || window.event;
        // firefox , event.pageX = undefined
        var scrollLeft = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
        var scrollTop = document.documentElement.scrollTop || document.body.scrollTop || 0;
    
        var x = event.pageX != undefined ? event.pageX : event.clientX;
        var y = event.pageY != undefined ? event.pageY : event.clientY;
        // var x = event.x;
        // var y = event.y;
        var borderW,borderH;
        var rect = canvas.getBoundingClientRect();
    
        borderW = ((rect.bottom - rect.top ) - canvas.height ) / 2;
        borderH = ((rect.right - rect.left ) - canvas.width ) / 2;
   
        x = x-rect.left-borderW  - scrollLeft;
        y = y-rect.top-borderH  - scrollTop;

        if(x<0){
            x=0;
        }
        if(y<0){
            y=0;
        }
        if(x>=canvas.width){
            x=canvas.width-1;
        }
        if(y>=canvas.height){
            y=canvas.height-1;
        }
        
        ws.send(JSON.stringify({
            t: 1, // mousemove
            w: canvas.width,
            h: canvas.height,
            x: x,
            y: y
        }));

        //console.log('mousemovePosition ' , x,y);
    }

}

js264.prototype.mousedownPosition = function (event){

    event = event || window.event;
    // firefox , event.pageX = undefined
    var scrollLeft = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
    var scrollTop = document.documentElement.scrollTop || document.body.scrollTop || 0;

    var x = event.pageX != undefined ? event.pageX : event.clientX;
    var y = event.pageY != undefined ? event.pageY : event.clientY;

    // var x = event.x;
    // var y = event.y;
    var borderW,borderH;
    var rect = canvas.getBoundingClientRect();
    
    borderW = ((rect.bottom - rect.top ) - canvas.height ) / 2;
    borderH = ((rect.right - rect.left ) - canvas.width ) / 2;
   
    x = x-rect.left-borderW - scrollLeft;
    y = y-rect.top-borderH - scrollTop;

    if(x<0){
        x=0;
    }
    if(y<0){
        y=0;
    }
    if(x>=canvas.width){
        x=canvas.width-1;
    }
    if(y>=canvas.height){
        y=canvas.height-1;
    }
 
    ws.send(JSON.stringify({
        t: 0, // mousedown
        w: canvas.width,
        h: canvas.height,
        x: x,
        y: y
    }));
 
    mouseFlag = true;

//console.log('mousedownPosition => ' , document.body.scrollLeft , document.body.scrollTop,document.documentElement.scrollLeft , document.documentElement.scrollTop, scrollLeft, scrollTop);
//  console.log('mousedownPosition ' , x,y,borderW,borderH,rect.left,rect.top, event.x, event.y,event.pageX,event.pageY);
 
} 

// ----------------------------------------------------------------------------
// Streaming over WebSockets
js264.prototype.waitForIntraFrame = true;
js264.prototype.socketBufferSize = 5 * 512 * 1024; // 512kb each

js264.prototype.initSocketClient = function( client ) {
    this.client.binaryType = 'arraybuffer';
    this.client.onmessage = this.receiveSocketMessage.bind(this);
};

js264.prototype.decodeSocketHeader = function( data ) {
 
        this.width = 640;
        this.height = 480;
    if( this.sequenceStarted ) { return; }
        this.sequenceStarted = true;
//======================================
    decoder.addEventListener('error', function(e) {
        console.log('Decoder error', e);
    })

    decoder.addEventListener('message', function(e) {
        var message = e.data;
        if (!message.hasOwnProperty('type')) return;

        switch(message.type) {
        case 'pictureParams':
        console.log('pictureParams ready'); 
            var croppingParams = message.croppingParams;   
            if(croppingParams === null) {
                canvas.width = message.width;
                canvas.height = message.height; 
            } else {
                canvas.width = croppingParams.width;
                canvas.height = croppingParams.height;   
            }
            break;
        case 'noInput':
            var copy = new Uint8Array(buf);
            decoder.postMessage({
                'type' : 'queueInput',
                'data' : copy.buffer
            }, [copy.buffer]); 
            break;
        case 'pictureReady':
            display.drawNextOutputPicture(
                message.width, 
                message.height, 
                message.croppingParams, 
                new Uint8Array(message.data));
                ++pictureCount; 
            break;
        case 'decoderReady':
            console.log('Decoder ready');
            break;
        }
    });
//======================================    
}; 

js264.prototype.receiveSocketMessage = function( event ) {
    
    buf = new Uint8Array(event.data);
    if( !this.sequenceStarted ) {
        this.decodeSocketHeader(buf);
        return;
    } 
 
//======================================    
    if( !this.sequenceStarted2 && buf[0] === 0x00 &&
        buf[1] === 0x00 && buf[2] === 0x00 &&   buf[3] === 0x01 && buf[4] == 0x67 )  {   
        this.sequenceStarted2 = true; 
        console.log('JxJ ',buf.length , buf[0],buf[1],buf[2],buf[3],buf[4] ); 
    }
   
    if( this.sequenceStarted2 ) {
        var copy = new Uint8Array(buf)                
        decoder.postMessage(
           {'type' : 'queueInput', 'data' : copy.buffer}, 
           [copy.buffer]);
        if(pictureCount==0){ //may fial, try again
            this.sequenceStarted2  = false;
        }
 
    }   
//======================================     
 
};  
    
})(window);
