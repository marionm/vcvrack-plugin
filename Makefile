RACK_DIR ?= ../Rack-SDK

FLAGS +=
CFLAGS +=
CXXFLAGS += -isystem extern
CXXFLAGS += -D_WIN32_WINNT=0x0A00 # Target Windows 10/11 for httplib

LDFLAGS +=
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32 -lcrypt32
endif

SOURCES += $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk
