#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup

.PHONY: all clean

all:
	@echo "loader checking ..."
	@$(MAKE) -C loader
	@echo "launcher checking..."
	@$(MAKE) -C launcher
	@echo "test checking..."
	@$(MAKE) -C test

submit:
	cd bin && ./launch ../test/fib

clean:
	@$(MAKE) -C loader clean
	@$(MAKE) -C launcher clean
	@$(MAKE) -C test clean
	@rm -rf bin


