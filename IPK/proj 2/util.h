#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <signal.h>

#define UINT32_SIZE 4
enum Codes{NO_CODE = 0, HELLO, BYE, REQUEST, READY, NOT_FOUND, SENDING, FINISHING, THANKS, ERROR};

bool stringToNum(const std::string &str, int &num);
void handleSigPipe(int signum, siginfo_t *siginfo, void *context);
void installSigPipeHandle();

void printOut(std::string str);
void printErr(std::string str);

class Comm
{
public:
	Comm();

protected:
	bool sendMsg(Codes code);
	bool sendMsg(Codes code, const char *data, int length);
	bool sendMsg(Codes code, uint32_t num);

	bool clearIncoming();
	bool readIncoming();
	bool receiveStrMsg(Codes code);
	bool receiveIntMsg(Codes code, uint32_t &num);
	bool receiveMsg(Codes code, uint32_t len);

	int _socket;

	unsigned char _rcvCode;
	std::string _rcvMsg;

private:
	std::string _rcvBuffer;
};

#endif /* UTIL_H */
