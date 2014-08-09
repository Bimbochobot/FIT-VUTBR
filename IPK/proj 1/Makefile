# Variables
PROJECT = ftpclient

CC = g++
RM = rm -rf
TAR = tar -cf
GZIP = gzip

CFLAGS = -Wall -Wextra -c -O3 -s -std=c++11

SRC_FILES = main.cpp client.cpp util.cpp
HEADER_FILES = client.h util.h
OBJ_FILES = $(SRC_FILES:.cpp=.o)
TAR_FILE = xsevci50.tar
GZIP_FILE = xsevci50.tar.gz
PACKED_FILES = $(SRC_FILES) $(HEADER_FILES) Makefile

# Targets
build: $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(PROJECT)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(PROJECT) $(OBJ_FILES) $(GZIP_FILE)

pack:
	$(TAR) $(TAR_FILE) $(PACKED_FILES)
	$(GZIP) $(TAR_FILE)

.PHONY: build release debug clean pack