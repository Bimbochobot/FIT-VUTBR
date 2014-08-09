#include "client.h"
#include "util.h"

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <cstring>
#include <sstream>

Client::~Client()
{
	sendCmd(Quit);
	close(_socket);
}

bool Client::init(std::string host, int port)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);

	// Resolves hostname into IP address
	hostent *he = gethostbyname(host.c_str());

	if (he == NULL || he->h_length == 0)
		return false;

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
		return false;

	// Set timeout on socket read
	struct timeval timeout;

	timeout.tv_sec = 50;
	timeout.tv_usec = 0;

	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

	return true;
}

bool Client::init(std::string host, int port, std::string user, std::string pass)
{
	if (!init(host, port))
		return false;

	// Receive greeting
	if (!receiveMsg() ||
		!checkResponse(220))
		return false;

	// Authenticate
	if (!auth(user, pass))
	{
		std::cerr << "Failed to log in to server." << std::endl;
		return false;
	}

	return true;
}

bool Client::auth(std::string user, std::string pass)
{

	if (!sendCmd(User, user))
		return false;

	if (!receiveMsg() ||
		!checkResponse(331))
	{
		// Might have passed authentication
		if (checkResponse(230))
			return true;
		else
			return false;
	}

	if(!sendCmd(Pass, pass))
		return false;

	if (!receiveMsg() ||
		!checkResponse(230))
		return false;

	if (!clearIncoming())
		return false;

	return true;
}

bool Client::list(std::string path, std::string &list)
{
	// Get into passive mode
	if (!sendCmd(Pasv, ""))
		return false;

	// Receive response
	if (!receiveMsg() ||
		!checkResponse(227))
		return false;

	// Parse response for new IP address and Port
	std::string host;
	int port;
	if (!parsePasive(host, port))
		return false;

	Client pasiveClient;
	if (!pasiveClient.init(host, port))
		return false;

	if (!pasiveClient.clearIncoming())
		return false;

	sendCmd(List, path);

	// Receive response about transmission
	if (!receiveMsg() ||
		!checkResponse(150))
		return false;

	// Receive response about result of transmission
	if (!receiveMsg() ||
		!checkResponse(226))
		return false;

	// Read received data
	if(!pasiveClient.readIncoming())
		return false;

	list.assign(pasiveClient._rcvBuffer);

	return true;
}

bool Client::sendCmd(Cmds cmd, std::string param)
{
	std::string msg;
	switch (cmd)
	{
	case User:
		msg.assign("USER ");
		msg.append(param);
		break;
	case Pass:
		msg.assign("PASS ");
		msg.append(param);
		break;
	case Pasv:
		msg.assign("PASV");
		break;
	case List:
		msg.assign("LIST ");
		msg.append(param);
		break;
	case Quit:
		msg.assign("QUIT");
		break;
	}
	msg.append("\r\n");
	return sendMsg(msg);
}

bool Client::sendMsg(std::string msg)
{
	while (true)
	{
		ssize_t wr = write(_socket, msg.c_str(), msg.length());
		if (wr < 0)
		{
			std::cerr << "Error writing to socket." << std::endl;
			return false;
		}
		else if (wr == (ssize_t)msg.length())
			break;
		else
			msg = msg.substr(wr, std::string::npos);
	}

	return true;
}

bool Client::checkResponse(int expectedCode)
{
	if (_rcvCode != expectedCode)
	{
		std::cerr << "Expected " << expectedCode << ", server returned: " << _rcvCode << " " << _rcvMsg << std::endl;
		return false;
	}
	return true;
}

bool Client::parsePasive(std::string &host, int &port)
{
	host.clear();
	size_t b, e;
	// Build IP address
	// First
	b = _rcvMsg.find("(") + 1;
	e = _rcvMsg.find(",", b);
	if (b == std::string::npos || e == std::string::npos)
		return false;
	host.append(_rcvMsg.substr(b, e - b));
	host.append(".");
	// Second
	b = e + 1;
	e = _rcvMsg.find(",", b);
	if (e == std::string::npos)
		return false;
	host.append(_rcvMsg.substr(b, e - b));
	host.append(".");
	// Third
	b = e + 1;
	e = _rcvMsg.find(",", b);
	if (e == std::string::npos)
		return false;
	host.append(_rcvMsg.substr(b, e - b));
	host.append(".");
	// Fourth
	b = e + 1;
	e = _rcvMsg.find(",", b);
	if (e == std::string::npos)
		return false;
	host.append(_rcvMsg.substr(b, e - b));

	// Build port
	int tmp;
	// Higher part
	b = e + 1;
	e = _rcvMsg.find(",", b);
	if (e == std::string::npos)
		return false;
	if(!stringToNum(_rcvMsg.substr(b, e - b), tmp))
		return false;
	port = tmp * 256;
	// Lower part
	b = e + 1;
	e = _rcvMsg.find(")", b);
	if (e == std::string::npos)
		return false;
	if(!stringToNum(_rcvMsg.substr(b, e - b), tmp))
		return false;
	port += tmp;

	return true;
}

bool Client::clearIncoming()
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
			std::cerr << "Error reading from socket." << std::endl;
			return false;
		}
		else if (rd < bs)
			break;

		ioctl(_socket, FIONREAD, &pending);
	}

	_rcvBuffer.clear();

	return true;
}

bool Client::readIncoming()
{
	const ssize_t bs = 100;
	char buffer[bs];
	int pending;

	while (true)
	{
		ssize_t rd = read(_socket, buffer, bs);
		if (rd < 0)
		{
			std::cerr << "Error reading from socket." << std::endl;
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

	return true;
}

bool Client::receiveMsg()
{
	size_t msgEnd;
	while (true)
	{ // Poll for messages until it's in correct format
		while (true)
		{ // Poll for messages until one complete arrives
			msgEnd = _rcvBuffer.find("\r\n");
			if (msgEnd == std::string::npos)
			{
				if (!readIncoming())
					return false;
			}
			else
				break;
		}

		if (msgEnd > 3)
		{
			std::string codePart = _rcvBuffer.substr(0,3);
			if (stringToNum(codePart, _rcvCode))
			{
				char delim = _rcvBuffer[3];

				if (delim == ' ')
				{
					_rcvMsg.clear();
					if (msgEnd > 4)
						_rcvMsg = _rcvBuffer.substr(4, msgEnd);

					break;
				}
			}
		}

		// Erase incorrect message from buffer
		_rcvBuffer.erase(0, msgEnd + 2);
	}

	// Erase correct message from buffer
	_rcvBuffer.erase(0, msgEnd + 2);

	return true;
}
