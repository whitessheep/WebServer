all:
	mkdir -p bin
	cd source && make

clean: 
	cd source && make clean


.PHONY : all clean

