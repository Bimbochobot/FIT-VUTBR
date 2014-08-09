#include "util.h"

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <cstring>
#include <sstream>
#include <mutex>

bool stringToNum(const std::string &str, int &num)
{
	char *endptr;
	// Work with decadic format
	num = strtol(str.c_str(), &endptr, 10);
	if (endptr != str.c_str() + str.length() * sizeof(char))
		return false;
	return true;
}

void handleSigPipe(int signum, siginfo_t *siginfo, void *context)
{
}

static std::mutex outMutex;
void printOut(std::string str)
{
	outMutex.lock();
	//std::cout << "Connection " << id  << ": " << str << std::endl;
	std::cout << str << std::endl;
	outMutex.unlock();
}

static std::mutex errMutex;
void printErr(std::string str)
{
	errMutex.lock();
	//std::cout << "ERR: Connection " << id << ": " << str << std::endl;
	std::cout << str << std::endl;
	errMutex.unlock();
}

void installSigPipeHandle()
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	// Specify handler
	action.sa_sigaction = &handleSigPipe;
	// Use the sa_sigaction field, not sa_handler
	action.sa_flags = SA_SIGINFO;
	// Install handler
	if (sigaction(SIGPIPE, &action, NULL) < 0)
		printErr("Failed to install handle for SIGPIPE");
}

Comm::Comm() :
		_socket(-1),
		_rcvCode(0)
{
}

bool Comm::sendMsg(Codes code)
{
	printOut("Sending code.");

	while (true)
	{
		ssize_t wr = write(_socket, ((char*) &code), 1);
		if (wr < 0)
			break;
		else if (wr == 1)
			return true;
	}

	printErr("Error writing to socket.");
	return false;
}

bool Comm::sendMsg(Codes code, const char *data, int length)
{
	if (!sendMsg(code))
		return false;

	if (length <= 0)
		return true;

	printOut("Sending data.");

	while (true)
	{
		ssize_t wr = write(_socket, data, length);
		if (wr < 0)
			break;
		else if (wr == (ssize_t)length)
			return true;
		else
		{
			data = data + wr;
			length -= wr;
		}
	}

	printErr("Error writing to socket.");
	return false;
}

bool Comm::sendMsg(Codes code, uint32_t num)
{
	uint32_t x = htonl(num);
	const char *data = (char*)&x;

	return sendMsg(code, data, UINT32_SIZE);
}

bool Comm::clearIncoming()
{
	const ssize_t bs = 100;
	char buffer[bs];
	int pending;

	ioctl(_socket, FIONREAD, &pending);
	while (pending)
	{
		ssize_t rd = read(_socket, buffer, bs);
		if (rd < 0)
		{
			printErr("Error reading from socket.");
			return false;
		}
		else if (rd < bs)
			break;

		ioctl(_socket, FIONREAD, &pending);
	}

	_rcvBuffer.clear();

	return true;
}

bool Comm::readIncoming()
{
	printOut("Reading incoming");
	const ssize_t bs = 100;
	char buffer[bs];
	int pending;

	while (true)
	{
		ssize_t rd = read(_socket, buffer, bs);
		if (rd < 0)
		{
			printErr("Error reading from socket.");
			return false;
		}

		_rcvBuffer.append(buffer, rd);

		// Test if anything's left in read buffer
		if (rd == bs)
		{
			ioctl(_socket, FIONREAD, &pending);
			if (pending == 0)
				break;
		}
		else
			break;
	}
	printOut("Done");

	return true;
}

// Receives message as string
bool Comm::receiveStrMsg(Codes code)
{
	size_t end;

	if (_rcvBuffer.empty())
		if (!readIncoming())
			return false;
	_rcvCode = _rcvBuffer[0];

	if (_rcvCode != code)
		return false;

	while (true)
	{ // Poll for message until one complete arrives
		end = _rcvBuffer.find("\n", 1);
		if (end != std::string::npos)
			break;

		if (!readIncoming())
			return false;
	}

	_rcvMsg = _rcvBuffer.substr(1, end - 1);

	// Erase message from buffer
	_rcvBuffer.erase(0, end + 1);

	return true;
}

bool Comm::receiveIntMsg(Codes code, uint32_t &num)
{
	if (receiveMsg(code, UINT32_SIZE))
	{
		uint32_t *numPtr = (uint32_t*) _rcvMsg.c_str();
		num = ntohl(*numPtr);
		return true;
	}

	return false;
}

// len = length of message
bool Comm::receiveMsg(Codes code, uint32_t len)
{
	// Account for code byte
	len += 1;

	// Fill receive buffer
	if (_rcvBuffer.empty())
		if (!readIncoming())
			return false;

	_rcvCode = _rcvBuffer[0];

	if (_rcvCode != code)
		return false;

	while (_rcvBuffer.size() < len)
	{ // Poll for message until one complete arrives
		if (!readIncoming())
			return false;
	}

	_rcvMsg = _rcvBuffer.substr(1, len - 1);

	// Erase message from buffer
	_rcvBuffer.erase(0, len);

	return true;
}
