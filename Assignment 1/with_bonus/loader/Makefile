#Create lib_simpleloader.so from loader.c
CFLAGS = -m32


OBJ_FILE = loader.o
Header_File = loader.h
FINAL = lib_simpleloader.so
C_FILE = loader.c

all:

	gcc -shared -m32 -o $(FINAL) -fPIC $(C_FILE)
	mkdir -p ../bin
	@mv lib_simpleloader.so ../bin/


	
clean:
	@rm -f *.o



