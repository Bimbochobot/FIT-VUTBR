#include "client.h"
#include "util.h"

#include <iostream>
#include <string>
#include <sstream>

bool checkParams(int argc, char* argv[], std::string &user, std::string &pwd, std::string &url, int &port, std::string &path)
{
	if (argc != 2)
		return false;

	// Initialize optional fields
	user = "anonymous";
	pwd = "";
	port = 21;
	path = "/";

	std::string param(argv[1]);
	size_t b = 0, e = 0;
	size_t ftpPart = std::string("ftp://").length();
	size_t urlBegin = 0, urlEnd = std::string::npos;
	size_t portBegin = std::string::npos, portEnd = std::string::npos;
	size_t pathBegin = std::string::npos, pathEnd = std::string::npos;

	if (param.substr(b, ftpPart - b).compare("ftp://") == 0)
	{
		b = ftpPart;
		e = param.find("@", b);

		if (e != std::string::npos)
		{
			size_t sep = param.find(":", b);
			if (sep != std::string::npos && sep < e)
			{
				user = param.substr(b, sep - b);
				pwd = param.substr(sep + 1, e - (sep + 1));
			}
			else
			{
				// Missing password
				return false;
			}
			b = e + 1;
		}
	}

	urlBegin = b;

	e = param.find(":", b);
	if (e != std::string::npos)
	{
		// Port part exists
		urlEnd = e;
		portBegin = e + 1;
	}

	e = param.find("/", b);
	if (e != std::string::npos)
	{
		// Path part exists
		if (urlEnd == std::string::npos)
			urlEnd = e;
		else
			portEnd = e;
		pathBegin = e;
		pathEnd = param.length();
	}

	if (urlEnd == std::string::npos)
		urlEnd = param.length();

	if (portBegin != std::string::npos && portEnd == std::string::npos)
		portEnd = param.length();

	if (urlEnd - urlBegin > 0)
		url = param.substr(urlBegin, urlEnd - urlBegin);
	else
	{
		// Missing host part
		return false;
	}

	if (portEnd - portBegin > 0)
	{
		int tmp;

		std::string portStr = param.substr(portBegin, portEnd - portBegin);
		if(stringToNum(portStr, tmp))
		{
			if (tmp >= 0 && tmp < 65536)
				port = tmp;
		}
	}

	if (pathEnd - pathBegin > 0)
	{
		path = param.substr(pathBegin, pathEnd - pathBegin);
	}

	return true;
}

int main(int argc, char* argv[])
{
	std::string user, pwd, url, path;
	int port;
	if (!checkParams(argc, argv, user, pwd, url, port, path))
	{
		std::cerr << "Program run with incorrect arguments!" << std::endl;
		return 1;
	}

	Client client;
	if (!client.init(url, port, user, pwd))
	{
		std::cerr << "Failed to initialize the client." << std::endl;
		return 1;
	}
	std::string list;
	if (!client.list(path, list))
	{
		std::cerr << "Failed to receive list of files." << std::endl;
		return 1;
	}
	std::cout << list << std::endl;

	return 0;
}




