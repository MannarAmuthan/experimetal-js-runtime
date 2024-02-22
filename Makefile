CC = g++
LD = g++
CCOPT = -g -O0
CCFLAGS = -pthread -std=c++17 -fno-rtti

V8_LIBS = -lv8_monolith -lv8_libbase -lv8_libplatform -ldl
V8_FLAGS = -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

main: build/context.o build/file.o
	$(CC)  -I. -Iincludes main.cc $^ -o build/$@ $(CCFLAGS) -I$(V8_INCLUDE_PATH) $(V8_FLAGS) -L$(V8_LIB_PATH) $(V8_LIBS)

build/context.o: context/context.cc
	$(CC)  -I. -Iincludes -c $^ -o $@ $(CCFLAGS) -I$(V8_INCLUDE_PATH) $(V8_FLAGS) 

build/file.o: context/file.cc
	$(CC)  -I. -Iincludes -c $^ -o $@ $(CCFLAGS) -I$(V8_INCLUDE_PATH) $(V8_FLAGS) 

clean:
	rm -rf build/*.o build/main

