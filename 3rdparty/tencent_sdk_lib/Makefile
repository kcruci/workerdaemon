vpath %.cpp ./src
RM = /bin/rm -f
CP = /bin/cp

CC = g++
CPPFLAGS = -g  -rdynamic  -Wall -D_REENTRANT

LIBDIR = $(LIB_JSON) $(LIB_OAUTH) $(LIB_CURL) $(LIB_OPENSSL) -ldl -lrt
INCL = $(INC_JSON)  $(INC_OPENSSL) $(INC_OAUTH) $(INC_CURL)  -I./include

SOURCES := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp,obj/%.o, $(SOURCES))

TARGET = lib/libopenapi.a

all: $(TARGET)

$(TARGET): $(OBJS)
	ar -rc  $(TARGET) $(OBJS)

obj/%.o: src/%.cpp
	$(CC) -c -o $@ $(CPPFLAGS) $(INCL) $<

clean:
	$(RM) $(OBJS) $(TARGET)
