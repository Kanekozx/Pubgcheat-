CXX      := clang++
CXXFLAGS := -std=c++20 -O2 -fPIE -fvisibility=hidden -Wall -Wextra \
            -Wno-unused-parameter -D__ANDROID__
LDFLAGS  := -pie -pthread -L/system/lib64 -llog -ldl

SRC := src/main.cpp src/module.cpp src/esp.cpp \
       src/aimbot.cpp src/offsets.cpp src/overlay_null.cpp \
       src/menu.cpp

pubg_cheat: $(SRC) $(wildcard src/*.hpp)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)
	llvm-strip $@

clean:
	rm -f pubg_cheat

.PHONY: clean