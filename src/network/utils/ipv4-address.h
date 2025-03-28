/*
 * Copyright (c) 2005 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef IPV4_ADDRESS_H
#define IPV4_ADDRESS_H

#include "ns3/address.h"
#include "ns3/attribute-helper.h"

#include <ostream>
#include <stdint.h>

namespace ns3
{

class Ipv4Mask;

/**
 * @ingroup address
 *
 * @brief Ipv4 addresses are stored in host order in this class.
 *
 * @see attribute_Ipv4Address
 */
class Ipv4Address
{
  public:
    Ipv4Address();
    /**
     * input address is in host order.
     * @param address The host order 32-bit address
     */
    explicit Ipv4Address(uint32_t address);
    /**
     * @brief Constructs an Ipv4Address by parsing a the input C-string
     *
     * Input address is in format:
     * \c hhh.xxx.xxx.lll
     * where \c h is the high byte and \c l the
     * low byte
     * @param address C-string containing the address as described above
     */
    Ipv4Address(const char* address);
    /**
     * Get the host-order 32-bit IP address
     * @return the host-order 32-bit IP address
     */
    uint32_t Get() const;
    /**
     * input address is in host order.
     * @param address The host order 32-bit address
     */
    void Set(uint32_t address);
    /**
     * @brief Sets an Ipv4Address by parsing a the input C-string
     *
     * Input address is in format:
     * \c hhh.xxx.xxx.lll
     * where \c h is the high byte and \c l the
     * low byte
     * @param address C-string containing the address as described above
     */
    void Set(const char* address);
    /**
     * Serialize this address to a 4-byte buffer
     *
     * @param buf output buffer to which this address gets overwritten with this
     * Ipv4Address
     */
    void Serialize(uint8_t buf[4]) const;
    /**
     * @param buf buffer to read address from
     * @return an Ipv4Address
     *
     * The input address is expected to be in network byte order format.
     */
    static Ipv4Address Deserialize(const uint8_t buf[4]);
    /**
     * @brief Print this address to the given output stream
     *
     * The print format is in the typical "192.168.1.1"
     * @param os The output stream to which this Ipv4Address is printed
     */
    void Print(std::ostream& os) const;

    /**
     * @return true if address is initialized (i.e., set to something), false otherwise
     */
    bool IsInitialized() const;
    /**
     * @return true if address is 0.0.0.0; false otherwise
     */
    bool IsAny() const;
    /**
     * @return true if address is 127.0.0.1; false otherwise
     */
    bool IsLocalhost() const;
    /**
     * @return true if address is 255.255.255.255; false otherwise
     */
    bool IsBroadcast() const;
    /**
     * @return true only if address is in the range 224.0.0.0 - 239.255.255.255
     */
    bool IsMulticast() const;
    /**
     * @return true only if address is in local multicast address scope, 224.0.0.0/24
     */
    bool IsLocalMulticast() const;
    /**
     * @brief If the IPv4 address is an APIPA address (169.254/16).
     *
     * The Automatic Private IP Address is described in \RFC{3927}
     *
     * @return true if the address is link-local, false otherwise
     */
    bool IsLinkLocal() const;
    /**
     * @brief Combine this address with a network mask
     *
     * This method returns an IPv4 address that is this address combined
     * (bitwise and) with a network mask, yielding an IPv4 network
     * address.
     *
     * @param mask a network mask
     * @returns the address combined with the mask
     */
    Ipv4Address CombineMask(const Ipv4Mask& mask) const;
    /**
     * @brief Generate subnet-directed broadcast address corresponding to mask
     *
     * The subnet-directed broadcast address has the host bits set to all
     * ones.  If this method is called with a mask of 255.255.255.255,
     * (i.e., the address is a /32 address), the program will assert, since
     * there is no subnet associated with a /32 address.
     *
     * @param mask a network mask
     * @returns a broadcast address for the subnet.
     */
    Ipv4Address GetSubnetDirectedBroadcast(const Ipv4Mask& mask) const;
    /**
     * @brief Generate subnet-directed broadcast address corresponding to mask
     *
     * The subnet-directed broadcast address has the host bits set to all
     * ones.  If this method is called with a mask of 255.255.255.255,
     * (i.e., the address is a /32 address), the program will assert, since
     * there is no subnet associated with a /32 address.
     *
     * @param mask a network mask
     * @return true if the address, when combined with the input mask, has all
     * of its host bits set to one
     */
    bool IsSubnetDirectedBroadcast(const Ipv4Mask& mask) const;
    /**
     * @param address an address to compare type with
     *
     * @return true if the type of the address stored internally
     * is compatible with the type of the input address, false otherwise.
     */
    static bool IsMatchingType(const Address& address);
    /**
     * Convert an instance of this class to a polymorphic Address instance.
     *
     * @return a new Address instance
     */
    operator Address() const;
    /**
     * @param address a polymorphic address
     * @return a new Ipv4Address from the polymorphic address
     *
     * This function performs a type check and asserts if the
     * type of the input address is not compatible with an
     * Ipv4Address.
     */
    static Ipv4Address ConvertFrom(const Address& address);
    /**
     * @brief Convert to an Address type
     * @return the Address corresponding to this object.
     */
    Address ConvertTo() const;

    /**
     * @return the 0.0.0.0 address
     */
    static Ipv4Address GetZero();
    /**
     * @return the 0.0.0.0 address
     */
    static Ipv4Address GetAny();
    /**
     * @return the 255.255.255.255 address
     */
    static Ipv4Address GetBroadcast();
    /**
     * @return the 127.0.0.1 address
     */
    static Ipv4Address GetLoopback();

  private:
    /**
     * @brief Get the underlying address type (automatically assigned).
     *
     * @returns the address type
     */
    static uint8_t GetType();
    uint32_t m_address; //!< IPv4 address
    bool m_initialized; //!< IPv4 address has been explicitly initialized to a valid value.

    /**
     * @brief Equal to operator.
     *
     * @param a the first operand.
     * @param b the first operand.
     * @returns true if the operands are equal.
     */
    friend bool operator==(const Ipv4Address& a, const Ipv4Address& b);

    /**
     * @brief Not equal to operator.
     *
     * @param a the first operand.
     * @param b the first operand.
     * @returns true if the operands are not equal.
     */
    friend bool operator!=(const Ipv4Address& a, const Ipv4Address& b);

    /**
     * @brief Less than to operator.
     *
     * @param a the first operand.
     * @param b the first operand.
     * @returns true if the first operand is less than the second.
     */
    friend bool operator<(const Ipv4Address& a, const Ipv4Address& b);
};

/**
 * @ingroup address
 *
 * @brief a class to represent an Ipv4 address mask
 *
 * The constructor takes arguments according to a few formats.
 * Ipv4Mask ("255.255.255.255"), Ipv4Mask ("/32"), and Ipv4Mask (0xffffffff)
 * are all equivalent.
 *
 * @see attribute_Ipv4Mask
 */
class Ipv4Mask
{
  public:
    /**
     * Will initialize to a garbage value (0x66666666)
     */
    Ipv4Mask();
    /**
     * @param mask bitwise integer representation of the mask
     *
     * For example, the integer input 0xffffff00 yields a 24-bit mask
     */
    Ipv4Mask(uint32_t mask);
    /**
     * @param mask String constant either in "255.255.255.0" or "/24" format
     */
    Ipv4Mask(const char* mask);
    /**
     * @param a first address to compare
     * @param b second address to compare
     * @return true if both addresses are equal in their masked bits,
     * corresponding to this mask
     */
    bool IsMatch(Ipv4Address a, Ipv4Address b) const;
    /**
     * Get the host-order 32-bit IP mask
     * @return the host-order 32-bit IP mask
     */
    uint32_t Get() const;
    /**
     * input mask is in host order.
     * @param mask The host order 32-bit mask
     */
    void Set(uint32_t mask);
    /**
     * @brief Return the inverse mask in host order.
     * @return The inverse mask
     */
    uint32_t GetInverse() const;
    /**
     * @brief Print this mask to the given output stream
     *
     * The print format is in the typical "255.255.255.0"
     * @param os The output stream to which this Ipv4Address is printed
     */
    void Print(std::ostream& os) const;
    /**
     * @return the prefix length of mask (the yy in x.x.x.x/yy notation)
     */
    uint16_t GetPrefixLength() const;
    /**
     * @return the 255.0.0.0 mask corresponding to a typical loopback address
     */
    static Ipv4Mask GetLoopback();
    /**
     * @return the 0.0.0.0 mask
     */
    static Ipv4Mask GetZero();
    /**
     * @return the 255.255.255.255 mask
     */
    static Ipv4Mask GetOnes();

    /**
     * @brief Equal to operator.
     *
     * @param a the first operand.
     * @param b the first operand.
     * @returns true if the operands are equal.
     */
    friend bool operator==(const Ipv4Mask& a, const Ipv4Mask& b);

    /**
     * @brief Not equal to operator.
     *
     * @param a the first operand.
     * @param b the first operand.
     * @returns true if the operands are not equal.
     */
    friend bool operator!=(const Ipv4Mask& a, const Ipv4Mask& b);

  private:
    uint32_t m_mask; //!< IP mask
};

ATTRIBUTE_HELPER_HEADER(Ipv4Address);
ATTRIBUTE_HELPER_HEADER(Ipv4Mask);

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param address the address
 * @returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const Ipv4Address& address);
/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param mask the mask
 * @returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const Ipv4Mask& mask);
/**
 * @brief Stream extraction operator.
 *
 * @param is the stream
 * @param address the address
 * @returns a reference to the stream
 */
std::istream& operator>>(std::istream& is, Ipv4Address& address);
/**
 * @brief Stream extraction operator.
 *
 * @param is the stream
 * @param mask the mask
 * @returns a reference to the stream
 */
std::istream& operator>>(std::istream& is, Ipv4Mask& mask);

inline bool
operator==(const Ipv4Address& a, const Ipv4Address& b)
{
    return a.m_address == b.m_address;
}

inline bool
operator!=(const Ipv4Address& a, const Ipv4Address& b)
{
    return a.m_address != b.m_address;
}

inline bool
operator<(const Ipv4Address& a, const Ipv4Address& b)
{
    return a.m_address < b.m_address;
}

/**
 * @ingroup address
 *
 * @brief Class providing an hash for IPv4 addresses
 */
class Ipv4AddressHash
{
  public:
    /**
     * @brief Returns the hash of an IPv4 address.
     * @param x the address
     * @return the hash
     *
     * This method uses std::hash rather than class Hash
     * as speed is more important than cryptographic robustness.
     */
    size_t operator()(const Ipv4Address& x) const;
};

inline bool
operator==(const Ipv4Mask& a, const Ipv4Mask& b)
{
    return a.m_mask == b.m_mask;
}

inline bool
operator!=(const Ipv4Mask& a, const Ipv4Mask& b)
{
    return a.m_mask != b.m_mask;
}

} // namespace ns3

#endif /* IPV4_ADDRESS_H */
