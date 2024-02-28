CC = g++
LD = g++
CCOPT = -g -O0


V8_LIBS = -lv8_monolith -lv8_libbase -lv8_libplatform -ldl
V8_FLAGS = -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

CCFLAGS = -pthread -std=c++17 -fno-rtti $(V8_FLAGS) 
DEPS_INCLUDES = -I$(V8_INCLUDE_PATH) -I$(LIBUV_INCLUDE_PATH)
DEPS_LIB_PATHS = -L$(V8_LIB_PATH)
LIBS = $(V8_LIBS) $(LIBUV_STATIC_LIB) -lcurl

main: build/context.o build/file.o build/request.o
	$(CC)  -I. -Iincludes main.cc $^  -o build/$@ $(CCFLAGS) $(DEPS_INCLUDES) $(DEPS_LIB_PATHS) $(LIBS)

build/context.o: context/context.cc
	$(CC)  -I. -Iincludes -c $^ -o $@ $(CCFLAGS) $(DEPS_INCLUDES) 

build/file.o: context/file.cc
	$(CC)  -I. -Iincludes -c $^ -o $@ $(CCFLAGS) $(DEPS_INCLUDES)

build/request.o: context/request.cc
	$(CC)  -I. -Iincludes -c $^ -o $@ $(CCFLAGS) $(DEPS_INCLUDES)

clean:
	rm -rf build/*.o build/main

