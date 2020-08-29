VERSION 	 =#V1.0.0
COMPILE_TIME = $(shell date +"%Y-%M-%d-%H-%M-%S")
CC			 =gcc
DEBUG 		 =#-DUSE_DEBUG
CFLAGS		 =--std=c99 -Wall
SOURCES	 	 =$(wildcard *.c ../../GM_CLI/*.c)
INCLUDES  	 =-I../../GM_CLI
LIB_NAMES 	 =#-lfun_a -lfun_so
LIB_PATH 	 =#-L./lib
OBJ			 =$(patsubst %.c, %.o, $(SOURCES))
TARGET		 =gm_cli

#links
$(TARGET):$(OBJ)
	$(CC) $(OBJ) $(LIB_PATH) $(LIB_NAMES) -o ./$(TARGET)$(VERSION)_$(COMPILE_TIME).exe
	@rm -rf $(OBJ)
	
#compile
%.o: %.c
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET) 