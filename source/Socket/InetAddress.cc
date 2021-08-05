#include "InetAddress.h"

#include <strings.h>  
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
#include <stdint.h>
#include <assert.h>


static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
	"InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
	static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
	static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
	if (ipv6) {
		memset(&addr6_, 0, sizeof addr6_);
		addr6_.sin6_family = AF_INET6;
		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		addr6_.sin6_addr = ip;
		addr6_.sin6_port = htobe16(port);
	}
	else {
		memset(&addr_, 0, sizeof addr_);
		addr_.sin_family = AF_INET;
		in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
		addr_.sin_addr.s_addr = htobe32(ip);
		addr_.sin_port = htobe16(port);
	}
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
	if (ipv6) {
		memset(&addr6_, 0, sizeof addr6_);
		addr6_.sin6_family = AF_INET6;
		addr6_.sin6_port = htobe16(port);
		inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr);
	}
	else {
		memset(&addr_, 0, sizeof addr_);
		addr_.sin_family = AF_INET;
		addr_.sin_port = htobe16(port);
		inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
	}
}

std::string InetAddress::toIpPort() const
{
	char buf[64] = { 0 };
	if (family() == AF_INET6) {
		inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof buf));
	}
	else {
		inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof buf));
	}
	size_t size = sizeof buf;
	size_t end = strlen(buf);
	uint16_t port = be16toh(addr_.sin_port);
	snprintf(buf + end, size - end, ":%u", port);
	return buf;
}

std::string InetAddress::toIp() const {
	char buf[64] = { 0 };
	if (family() == AF_INET6) {
		inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof buf));
	}
	else {
		inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof buf));
	}
	return buf;
}

uint16_t InetAddress::toPort() const {
	return be16toh(portNetEndian());
}

uint32_t InetAddress::ipNetEndian() const {
	assert(family() == AF_INET);
	return addr_.sin_addr.s_addr;
}

void InetAddress::setScopeId(uint32_t scope_id) {
	if (family() == AF_INET6) {
		addr6_.sin6_scope_id = scope_id;
	}
}
