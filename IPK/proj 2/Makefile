# Variables
PROJECT = IPK2

CXX = g++48
RM = rm -rf
TAR = tar -cf
GZIP = gzip

CXXFLAGS = -static-libstdc++ -Wall -Wextra -std=c++11 -O3

SRC_FILES = server.cpp client.cpp util.cpp
HEADER_FILES = server.h client.h util.h
SERVER_OBJ_FILES = server.o util.o
CLIENT_OBJ_FILES = client.o util.o
TAR_FILE = xsevci50.tar
GZIP_FILE = xsevci50.tar.gz
PACKED_FILES = $(SRC_FILES) $(HEADER_FILES) Makefile

# Targets
build: server client

server: $(SERVER_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(SERVER_OBJ_FILES) -lpthread -o server

client: $(CLIENT_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJ_FILES) -lpthread -o client

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) server client $(SERVER_OBJ_FILES) $(CLIENT_OBJ_FILES) $(GZIP_FILE)

pack:
	$(TAR) $(TAR_FILE) $(PACKED_FILES)
	$(GZIP) $(TAR_FILE)

.PHONY: build release debug clean pack