MAINSOURCE := main.cc
SOURCE  := $(wildcard *.cc ./WebServer/*.cc ./EventLoop/*.cc Timer/*.cc ./HttpConn/*.cc ./Socket/*.cc ./Thread/*.cc ./Log/*.cc ./Buffer/*.cc)
override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
OBJS    := $(patsubst %.cc,%.o,$(SOURCE))

TARGET  := webserver
CC      := g++
LIBS    := -lpthread
LDFLAGS:= 
CXXFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS -I.


.PHONY : objs clean veryclean rebuild all debug
all : $(TARGET) 
objs : $(OBJS)
rebuild: veryclean all

tests : $(SUBTARGET1) $(SUBTARGET2)
clean :
	find . -name '*.o' | xargs rm -f
veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f

debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) main.o
	$(CC) $(CXXFLAGS) -o ../bin/$@ $^ $(LDFLAGS) $(LIBS)



