#include "server.h"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>

Connection::Connection(int socket, int limit, int id) :
	Comm(),
	_id(id),
	_interval(1000),
	_running(false),
	_stop(false)
{
	_socket = socket;
	// Set timeout on socket read
	struct timeval timeout;

	timeout.tv_sec = 180;
	timeout.tv_usec = 0;

	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

	_chunkSize = (int)(limit * 1000 * (_interval / 1000.0));

	printOut("Chunk size is: ");// << _chunkSize << std::endl;
}

Connection::Connection(const Connection& copy) :
	Comm(),
	_id(copy._id),
	_interval(copy._interval),
	_chunkSize(copy._chunkSize),
	_running(false),
	_stop(false)
{
	_socket = copy._socket;
	// Set timeout on socket read
	struct timeval timeout;

	timeout.tv_sec = 180;
	timeout.tv_usec = 0;

	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

	printOut("Chunk size is: ");// << _chunkSize << std::endl;
}

void Connection::start()
{
	std::thread loopThread(&Connection::loop, this);
	loopThread.detach();
}

void Connection::stop()
{
	_stop = true;
}

bool Connection::loop()
{
	_running = true;

	// Variable indicating state of connection
	bool good = true;

	// Handshaking on connection
	if (!sendMsg(HELLO) ||
		!readMsg(HELLO))
	{
		printErr("Handshake with client failed.");
		good = false;
	}

	// Wait for request
	while (good && !_stop)
	{
		if (readMsg(REQUEST))
		{ // Request received
			printOut("Request received: ");// << _fileRequest << std::endl;
			// Initiate file transfer, send message to client on error
			if (!sendFile())
			{
				sendMsg(ERROR);
				std::cerr << "Failed to send file." << std::endl;
				good = false;
			}
		}
		else if (readMsg(BYE))
		{ // Client closed the connection correctly
			break;
		}
		else
		{
			printErr("Communication with client failed or timeout.");
			sendMsg(ERROR);
			good = false;
		}
	}

	printOut("Closing connection.");
	close(_socket);
	_running = false;
	return good;
}

bool Connection::readMsg(Codes code)
{
	printOut("Reading");
	switch (code)
	{
	case REQUEST:
		// Request message from client contains file name for transfer
		if (receiveStrMsg(code))
		{
			_fileRequest = std::string(_rcvMsg);
			return true;
		}
		break;

	default:
		// Just check if codes agree
		return receiveMsg(code, 0);
	}

	return false;
}

bool Connection::sendFile()
{
	printOut("Sending file");
	std::ifstream fin(_fileRequest, std::ios::in | std::ios::binary);
	if (fin.good())
	{
		// Indicate that server is read for transfer, wait for client.
		if (!sendMsg(READY, _chunkSize) ||
			!readMsg(READY))
			return false;

		int chunkSize = _chunkSize;
		char *buffer = new char[_chunkSize];

		bool error = false;
		while (fin.good() && !error && !_stop)
		{
			auto t0 = std::chrono::high_resolution_clock::now();
			fin.read(buffer, _chunkSize);

			if (fin.eof())
			{
				chunkSize = fin.gcount();
				// Indicate that end of file will follow with different chunk size
				if ((error = !sendMsg(FINISHING, chunkSize)))
					break;
			}

			// Send chunk of data to client
			if ((error = !sendMsg(SENDING, buffer, chunkSize)))
				break;

			auto t1 = std::chrono::high_resolution_clock::now();

			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

			auto sleepTime = std::chrono::milliseconds(_interval - dur);

			if (sleepTime.count() > 0)
			{
				// Sleep to limit bandwidth
				printOut("Sleeping");
				//std::cout << "Sleeping " << sleepTime.count() << " ms" << std::endl;
				std::this_thread::sleep_for(sleepTime);
			}
		}

		// Release resources
		fin.close();
		delete buffer; buffer = nullptr;

		if (error)
		{
			printErr("Error while sending file.");
			return false;
		}
	}
	else
		// Indicate to client that file was not found on server
		if (!sendMsg(NOT_FOUND))
			return false;

	// Wait for client to confirm file transfer
	if (!readMsg(THANKS))
		return false;

	printOut("Done file transfer");

	return true;
}

bool Server::init(int limit, int port)
{
	_limit = limit;
	_port = port;

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		printErr("Failed to create socket.");
		return false;
	}

	// Create address for socket
	sockaddr_in address;
	// Initialize address
	memset(&address, 0, sizeof(address));
	// Set address to IPv4 family
	address.sin_family = AF_INET;
	// Set destination IP address
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	// Set destination port
	address.sin_port = htons(_port);

	int res = bind(_socket, (sockaddr*)&address, sizeof(address));
	if (res < 0)
	{
		close(_socket);
		printErr("Failed to bind socket.");
		return false;
	}

	// Allow queue for 10 clients
	res = listen(_socket, 10);

	return true;
}

bool Server::run()
{
	int idCounter = 0;
	while (true)
	{
		// Wait for connection
		int socket = accept(_socket, nullptr, nullptr);

		// Add new connection to list
		connections.push_back(Connection(socket, _limit, idCounter++));
		connections.back().start();
	}

	return true;
}

int processParams(int argc, char *argv[], int &limit, int &port)
{
	if (argc != 5)
		return 1;

	bool limitSet = false;
	bool portSet = false;
	for (int i = 1; i < 5; i += 2)
	{
		if (strcmp(argv[i], "-d") == 0 && !limitSet)
		{
			if (stringToNum(argv[i + 1], limit))
				limitSet = true;
		}
		else if (strcmp(argv[i], "-p") == 0 && !portSet)
		{
			if (stringToNum(argv[i + 1], port))
				portSet = true;
		}
		else
			return 1;
	}

	if (!limitSet || !portSet)
		return 1;

	return 0;
}


int main(int argc, char *argv[])
{
	int limit, port;
	int res = processParams(argc, argv, limit, port);
	if (res > 0)
	{
		printErr("Program run with incorrect arguments.");
		return res;
	}

	installSigPipeHandle();

	Server server;
	if (!server.init(limit, port))
		return 10;

	if (!server.run())
		return 50;

	return 0;
}
