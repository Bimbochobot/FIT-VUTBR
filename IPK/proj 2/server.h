#ifndef SERVER_H
#define SERVER_H

#include <list>
#include <atomic>

#include "util.h"

class Connection : public Comm
{
public:
	Connection(int socket, int limit, int id);
	Connection(const Connection& copy);

	void start();
	void stop();

private:
	bool loop();

	bool readMsg(Codes code);
	bool sendFile();

	std::string _fileRequest;
	int _id;
	const int _interval;
	int _chunkSize;

	std::atomic_bool _running;
	std::atomic_bool _stop;
};

class Server
{
public:
	bool init(int limit, int port);
	bool run();

private:
	std::list<Connection> connections;

	int _limit;
	int _port;
	int _socket;
};

#endif
