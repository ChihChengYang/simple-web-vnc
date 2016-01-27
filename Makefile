
# COMPILER FLAGS
CC = gcc
CCPLUS = g++  
 
CFLAGS = -Wall -pedantic -lpthread -fpermissive -w
 
OUTPUT_PATH = ./bin/ 
LIBS_PATH = ./libs/

FF_LIBS_PATH = ./ffmpeg_build


LIBS = -lXtst -lXext -lX11 -ldl -L$(FF_LIBS_PATH)/lib/ -lx264 -lavcodec -lswscale -lavutil -lswresample -lavformat  
 

INC_PATH = -I$(FF_LIBS_PATH)/include/ 
  
DEBUG = 0
ifeq ($(DEBUG),1)
	OPTIMIZEFLAG = -g
else
	OPTIMIZEFLAG = -O3  
endif

# OBJECTS
VNCENCODER_OBJECTS =  encoder.c 
VNCENCODER_OUTPUT =  vncencoder.o
 
SC_OBJECTS =  screen_capture.c 
SC_OUTPUT =  screen_capture.o

CTRL_OBJECTS =  ctrl.c 
CTRL_OUTPUT =  ctrl.o
 
APP_OBJECTS =  app.c
APP_OUTPUT = libapp.so
APP_LIB = -L$(LIBS_PATH) -lapp
 
GO_BUILD = ./build.sh
GO_OUTPUT = server
 
all:	vnc_encoder screen_capture ctl applib go 
 
vnc_encoder: $(VNCENCODER_OBJECTS)
	$(CCPLUS)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(VNCENCODER_OUTPUT) $(VNCENCODER_OBJECTS) $(LIBS)  
   
screen_capture: $(SC_OBJECTS)
	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(SC_OUTPUT) $(SC_OBJECTS) $(LIBS) 

ctl: $(CTRL_OBJECTS)
	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(CTRL_OUTPUT) $(CTRL_OBJECTS) $(LIBS)  

applib: $(APP_OBJECTS)
	$(CCPLUS) $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) $(VNCENCODER_OUTPUT) $(SC_OUTPUT) $(CTRL_OUTPUT)  -fPIC -shared -o $(APP_OUTPUT) $(APP_OBJECTS)  $(LIBS)
	cp ./$(APP_OUTPUT) /usr/lib/
	mv ./$(APP_OUTPUT) $(LIBS_PATH)
	ldconfig
 
go: 	
	$(GO_BUILD)
 
clean:
	rm -rf $(OUTPUT_PATH)$(APP_OUTPUT) 
 