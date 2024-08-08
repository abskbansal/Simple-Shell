#Compile the launch.c by linking it with the lib_simpleloader.so
FINAL_TARGET = launch
CFLAGS = -m32
LDFLAGS = ../bin/lib_simpleloader.so
all:

	gcc $(CFLAGS) -o $(FINAL_TARGET) launch.c $(LDFLAGS)
	@mv launch ../bin


#Provide the command for cleanup
clean:
	@rm -f $(FINAL_TARGET)

.PHONY: clean
