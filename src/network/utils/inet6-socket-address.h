/*
 * Copyright (c) 2007-2008 Louis Pasteur University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#ifndef INET6_SOCKET_ADDRESS_H
#define INET6_SOCKET_ADDRESS_H

#include "ipv6-address.h"

#include "ns3/address.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup address
 * @class Inet6SocketAddress
 * @brief An Inet6 address class.
 */
class Inet6SocketAddress
{
  public:
    /**
     * @brief Constructor.
     * @param ipv6 the IPv6 address
     * @param port the port
     */
    Inet6SocketAddress(Ipv6Address ipv6, uint16_t port);

    /**
     * @brief Constructor (the port is set to zero).
     * @param ipv6 the IPv6 address
     */
    Inet6SocketAddress(Ipv6Address ipv6);

    /**
     * @brief Constructor (the address is set to "any").
     * @param port the port
     */
    Inet6SocketAddress(uint16_t port);

    /**
     * @brief Constructor.
     * @param ipv6 string which represents an IPv6 address
     * @param port the port
     */
    Inet6SocketAddress(const char* ipv6, uint16_t port);

    /**
     * @brief Constructor.
     * @param ipv6 string which represents an IPv6 address
     */
    Inet6SocketAddress(const char* ipv6);

    /**
     * @brief Get the port.
     * @return the port
     */
    uint16_t GetPort() const;

    /**
     * @brief Set the port
     * @param port the port
     */
    void SetPort(uint16_t port);

    /**
     * @brief Get the IPv6 address.
     * @return the IPv6 address
     */
    Ipv6Address GetIpv6() const;

    /**
     * @brief Set the IPv6 address.
     * @param ipv6 the address
     */
    void SetIpv6(Ipv6Address ipv6);

    /**
     * @brief If the address match.
     * @param addr the address to test
     * @return true if the address match, false otherwise
     */
    static bool IsMatchingType(const Address& addr);

    /**
     * @brief Get an Address instance which represents this
     * Inet6SocketAddress instance.
     */
    operator Address() const;

    /**
     * @brief Convert the address to a InetSocketAddress.
     * @param addr the address to convert
     * @return an Inet6SocketAddress instance corresponding to address
     */
    static Inet6SocketAddress ConvertFrom(const Address& addr);

    /**
     * @brief Convert to Address.
     * @return Address instance
     */
    Address ConvertTo() const;

  private:
    /**
     * @brief Get the type.
     * @return the type of Inet6SocketAddress
     */
    static uint8_t GetType();

    /**
     * @brief The IPv6 address.
     */
    Ipv6Address m_ipv6;

    /**
     * @brief The port.
     */
    uint16_t m_port;
};

} /* namespace ns3 */

#endif /* INET6_SOCKET_ADDRESS_H */
