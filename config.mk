CC := gcc
CXX := g++
PREFIX := $(HOME)/.local
CFLAGS := -Os -Wall -I$(PREFIX)/include/quickjs
CXXFLAGS := -Os -std=c++17 -Wall -I$(PREFIX)/include/quickjs
LDFLAGS := -Wl,-s -L$(PREFIX)/lib/quickjs
