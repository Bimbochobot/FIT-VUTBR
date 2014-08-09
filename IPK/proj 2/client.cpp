#include "client.h"
#include "util.h"

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

bool Client::init(std::string host, int port)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		std::cerr << "Failed to create socket." << std::endl;
		return false;
	}

	// Set timeout on socket read
	struct timeval timeout;

	timeout.tv_sec = 50;
	timeout.tv_usec = 0;

	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

	// Resolves hostname into IP address
	hostent *he = gethostbyname(host.c_str());

	if (he == NULL || he->h_length == 0)
	{
		std::cerr << "Failed to resolve host name." << std::endl;
		return false;
	}

	in_addr** addr_list = (in_addr **) he->h_addr_list;

	// Create address for socket
	sockaddr_in address;
	// Initialize address
	memset(&address, 0, sizeof(address));
	// Set address to IPv4 family
	address.sin_family = AF_INET;
	// Set destination IP address
	address.sin_addr = *(addr_list[0]);
	// Set destination port
	address.sin_port = htons(port);

	int res = connect(_socket, (sockaddr*)&address, sizeof(address));
	if (res < 0)
	{
		std::cerr << "Failed to connect to socket." << std::endl;
		return false;
	}

	// Handshaking on connection
	if (!readMsg(HELLO) ||
		!sendMsg(HELLO))
	{
		std::cerr << "Handshake with server failed." << std::endl;
		return false;
	}
	return true;
}

bool Client::release()
{
	// Close connection with server
	sendMsg(BYE);
	return close(_socket) == 0;
}


bool Client::requestFile(std::string path)
{
	// Cut out filename from path
	size_t begin = path.rfind('/');
	if (begin == std::string::npos)
		begin = 0;

	std::string filename = path.substr(begin + 1);

	// Append new line to path as end of string
	path.append("\n");

	// Request file, include null character in length
	if (!sendMsg(REQUEST, path.c_str(), path.length()) ||
		!readMsg(READY))
	{
		if (readMsg(NOT_FOUND))
		{
			std::cerr << "File not found at server side." << std::endl;

			// Send message confirming end of file transfer.
			sendMsg(THANKS);
		}
		return false;
	}

	std::cout << "Chunk size is: " << _chunkSize << std::endl;

	// Indicate that client is ready for transfer
	if (!sendMsg(READY))
	{
		return false;
	}

	// Open file to save into
	std::ofstream fout(filename, std::ios::out | std::ios::binary);

	// Start file transfer
	bool transfer = true;
	while(transfer)
	{
		// Read file content
		if (!readMsg(SENDING))
		{
			// Check if case of interruption was EOF
			if (!readMsg(FINISHING))
			{
				std::cerr << "Error occurred during file transfer, file not complete." << std::endl;
				fout.close();
				return false;
			}
			else
				transfer = false;
		}

		// Save received content to file
		if (!_rcvMsg.empty())
			fout << _rcvMsg;
	}

	fout.close();

	// Send message confirming end of file transfer.
	if (!sendMsg(THANKS))
		return false;

	return true;
}

bool Client::readMsg(Codes code)
{
	std::cout << "Reading " << code << std::endl;
	switch (code)
	{
	case READY: {
		// Ready message from server contains chunk size
		uint32_t num;
		if(receiveIntMsg(code, num))
		{
			_chunkSize = num;
			return true;
		}
		break;
	}

	case SENDING:
		return receiveMsg(code, _chunkSize);

	case FINISHING:{
		// Finishing message from server contains number of bytes left to send
		uint32_t spareBytes;
		if (receiveIntMsg(code, spareBytes))
		{
			if (receiveMsg(SENDING, spareBytes))
				return true;
		}
		break;
	}

	default:
		// Just check if codes agree
		return receiveMsg(code, 0);
	}

	return false;
}

int processParams(int argc, char* argv[], std::string &url, int &port, std::string &path)
{
	if (argc != 2)
		return 1;

	std::string param(argv[1]);
	size_t urlBegin = 0, urlEnd = std::string::npos;
	size_t portBegin = std::string::npos, portEnd = std::string::npos;
	size_t pathBegin = std::string::npos;

	urlEnd = param.find(":", urlBegin);
	if (urlEnd != std::string::npos)
	{
		url = param.substr(urlBegin, urlEnd - urlBegin);
	}
	else
	{
		// Missing url
		return 1;
	}

	portBegin = urlEnd + 1;
	portEnd = param.find("/", portBegin);
	if (portEnd != std::string::npos)
	{
		if(!stringToNum(param.substr(portBegin, portEnd - portBegin), port) || port < 0 || port >= 65536)
		{
			// Incorrect port value
			return 1;
		}
	}
	else
	{
		// Missing port
		return 1;
	}

	pathBegin = portEnd + 1;
	path = param.substr(pathBegin, param.length() - pathBegin);

	return 0;
}

int main(int argc, char *argv[])
{
	std::string url, path;
	int port;
	int res = processParams(argc, argv, url, port, path);
	if (res > 0)
	{
		std::cerr << "Program run with incorrect arguments." << std::endl;
		return res;
	}

	installSigPipeHandle();

	Client client;
	if (!client.init(url, port))
		return 10;

	if (!client.requestFile(path))
	{
		client.release();
		std::cerr << "File transfer was unsuccessful." << std::endl;
		return 50;
	}

	client.release();
	return 0;
}
