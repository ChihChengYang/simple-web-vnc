
# COMPILER FLAGS
CC = gcc
CCPLUS = g++  
 
CFLAGS = -Wall -pedantic -lpthread -fpermissive -w
 
OUTPUT_PATH = ./bin/ 
LIBS_PATH = ./libs/

FF_LIBS_PATH = ../../ffmpeg_build


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
#VNCENCODER_LIB = -L$(LIBS_PATH) -lvncencoder

SC_OBJECTS =  screen_capture.c 
SC_OUTPUT =  screen_capture.o

CTRL_OBJECTS =  ctrl.c 
CTRL_OUTPUT =  ctrl.o

IP_OBJECTS =  image_process.c 
IP_OUTPUT =  image_process.o

APP_OBJECTS =  app.c
APP_OUTPUT = libapp.so
APP_LIB = -L$(LIBS_PATH) -lapp

#APPMAIN = app_test.c   
#APPMAIN_OUTPUT = app_test

GO_BUILD = ./build.sh
GO_OUTPUT = server
 
all:	image_process vnc_encoder screen_capture ctl applib go 
#go
#appmain

image_process: $(IP_OBJECTS)
	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(IP_OUTPUT) $(IP_OBJECTS) $(LIBS)  
 
vnc_encoder: $(VNCENCODER_OBJECTS)
	$(CCPLUS)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(VNCENCODER_OUTPUT) $(VNCENCODER_OBJECTS) $(LIBS)  
   
screen_capture: $(SC_OBJECTS)
	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(SC_OUTPUT) $(SC_OBJECTS) $(LIBS) 

ctl: $(CTRL_OBJECTS)
	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -c -fPIC -o $(CTRL_OUTPUT) $(CTRL_OBJECTS) $(LIBS)  

applib: $(APP_OBJECTS)
	$(CCPLUS) $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) $(IP_OUTPUT) $(VNCENCODER_OUTPUT) $(SC_OUTPUT) $(CTRL_OUTPUT)  -fPIC -shared -o $(APP_OUTPUT) $(APP_OBJECTS)  $(LIBS)
	cp ./$(APP_OUTPUT) /usr/lib/
	mv ./$(APP_OUTPUT) $(LIBS_PATH)
	ldconfig

#appmain: $(APPMAIN)    
#	$(CC)  $(INC_PATH) $(CFLAGS) $(OPTIMIZEFLAG) -o $(APPMAIN_OUTPUT) $(APPMAIN) $(APP_LIB) $(LIBS)
#	mv ./$(APPMAIN_OUTPUT) $(OUTPUT_PATH)

go: 	
	$(GO_BUILD)

	
clean:
	rm -rf $(OUTPUT_PATH)$(APP_OUTPUT) 
 