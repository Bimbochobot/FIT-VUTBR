#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client
{
public:
	~Client();
	bool init(std::string host, int port);
	bool init(std::string host, int port, std::string user, std::string pass);
	bool auth(std::string user, std::string pass);
	bool list(std::string path, std::string &list);
private:
	enum Cmds { User, Pass, Pasv, List, Quit };
	bool sendCmd(Cmds cmd, std::string param = "");
	bool sendMsg(std::string msg);
	bool checkResponse(int expectedCode);
	bool parsePasive(std::string &host, int &port);
	bool clearIncoming();
	bool readIncoming();
	bool receiveMsg();

	int _socket;

	std::string _rcvBuffer;

	int _rcvCode;
	std::string _rcvMsg;
};

#endif /* CLIENT_H */
