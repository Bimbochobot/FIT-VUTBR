#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "util.h"

class Client : public Comm
{
public:
	bool init(std::string host, int port);
	bool release();
	bool requestFile(std::string filename);
private:
	bool readMsg(Codes code);

	int _chunkSize;
};

#endif /* CLIENT_H */
