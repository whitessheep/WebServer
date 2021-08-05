//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include "base/StringPiece.h"

#include <string>
#include <netinet/in.h>

class InetAddress
{
public:

	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

	InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

	explicit InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{ }

	explicit InetAddress(const struct sockaddr_in6& addr)
		: addr6_(addr)
	{ }

	sa_family_t family() const { return addr_.sin_family; }
	std::string toIpPort() const;
	std::string toIp() const;
	uint16_t toPort() const;

	const struct sockaddr* getSockAddr() const { return (sockaddr*)(&addr6_); }
	void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const { return addr_.sin_port; }

	void setScopeId(uint32_t scope_id);

private:
	union {
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};


#endif //WEBSERVER_INETADDRESS_H
