/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Università di Firenze, Italy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alessio Bonadio <alessio.bonadio@gmail.com>
 */

#ifndef SIXLOWPAN_ND_HEADER_H
#define SIXLOWPAN_ND_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"
#include "ns3/icmpv6-header.h"

namespace ns3
{

/**
 * \ingroup sixlowpan
 * \class Icmpv6DuplicateAddress
 * \brief ICMPv6 Duplicate Address header.
 */
class Icmpv6DuplicateAddress : public Icmpv6Header
{
public:

  /**
   * \brief Constructor.
   */
  Icmpv6DuplicateAddress ();

  /**
     * \brief Constructor.
     * \param request duplicate address request or duplicate address confirm
     */
  Icmpv6DuplicateAddress (bool request);

  /**
   * \brief Constructor (DAR).
   * \param time the registration lifetime (units of 60 seconds)
   * \param eui the EUI-64
   * \param address the registrered address
   */
  Icmpv6DuplicateAddress (uint16_t time, Mac64Address eui, Ipv6Address address);

  /**
   * \brief Constructor (DAC).
   * \param status the status (DAC)
   * \param time the registration lifetime (units of 60 seconds)
   * \param eui the EUI-64
   * \param address the registrered address
   */
  Icmpv6DuplicateAddress (uint8_t status, uint16_t time, Mac64Address eui, Ipv6Address address);

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6DuplicateAddress ();

  /**
   * \brief Get the UID of this class.
   * \return UID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Get the status field.
   * \return status value
   */
  uint8_t GetStatus () const;

  /**
   * \brief Set the status field.
   * \param status the status value
   */
  void SetStatus (uint8_t status);

  /**
   * \brief Get the reserved field.
   * \return reserved value
   */
  uint8_t GetReserved () const;

  /**
   * \brief Set the reserved field.
   * \param reserved the reserved value
   */
  void SetReserved (uint8_t reserved);

  /**
   * \brief Get the registration lifetime field.
   * \return registration lifetime value (units of 60 seconds)
   */
  uint16_t GetRegTime () const;

  /**
   * \brief Set the registration lifetime field.
   * \param time the registration lifetime value (units of 60 seconds)
   */
  void SetRegTime (uint16_t time);

  /**
   * \brief Get the EUI-64 field.
   * \return EUI-64 value
   */
  Mac64Address GetEui64 () const;

  /**
   * \brief Set the EUI-64 field.
   * \param eui the EUI-64 value
   */
  void SetEui64 (Mac64Address eui);

  /**
   * \brief Get the registered address field.
   * \return registered address value
   */
  Ipv6Address GetRegAddress () const;

  /**
   * \brief Set the registered address field.
   * \param registered the registered address value
   */
  void SetRegAddress (Ipv6Address registered);

  /**
   * \brief Print informations.
   * \param os output stream
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size.
   * \return serialized size
   */
  virtual uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize the packet.
   * \param start start offset
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start start offset
   * \return length of packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:

  /**
   * \brief The status value.
   */
  uint8_t m_status;

  /**
   * \brief The reserved value.
   */
  uint8_t m_reserved;

  /**
   * \brief The registration lifetime value (units of 60 seconds).
   */
  uint16_t m_regTime;

  /**
   * \brief The EUI-64 value.
   */
  Mac64Address m_eui64;

  /**
   * \brief The registered address value.
   */
  Ipv6Address m_regAddress;

};

/**
 * \ingroup sixlowpan
 * \class Icmpv6OptionAddressRegistration
 * \brief ICMPv6 Address Registration Option header.
 */
class Icmpv6OptionAddressRegistration : public Icmpv6OptionHeader
{
public:

  /**
   * \brief Constructor.
   */
  Icmpv6OptionAddressRegistration ();

  /**
   * \brief Constructor.
   * \param time the registration lifetime (units of 60 seconds)
   * \param eui the EUI-64
   */
  Icmpv6OptionAddressRegistration (uint16_t time, Mac64Address eui);

  /**
   * \brief Constructor.
   * \param status the status value
   * \param time the registration lifetime (units of 60 seconds)
   * \param eui the EUI-64
   */
  Icmpv6OptionAddressRegistration (uint8_t status, uint16_t time, Mac64Address eui);

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6OptionAddressRegistration ();

  /**
   * \brief Get the UID of this class.
   * \return UID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Get the status field.
   * \return status value
   */
  uint8_t GetStatus () const;

  /**
   * \brief Set the status field.
   * \param status the status value
   */
  void SetStatus (uint8_t status);

  /**
   * \brief Get the first reserved field.
   * \return first reserved value
   */
  uint8_t GetReserved1 () const;

  /**
   * \brief Set the first reserved field.
   * \param reserved the first reserved value
   */
  void SetReserved1 (uint8_t reserved);

  /**
   * \brief Get the second reserved field.
   * \return second reserved value
   */
  uint16_t GetReserved2 () const;

  /**
   * \brief Set the second reserved field.
   * \param reserved the second reserved value
   */
  void SetReserved2 (uint16_t reserved);

  /**
   * \brief Get the registration lifetime field.
   * \return registration lifetime value (units of 60 seconds)
   */
  uint16_t GetRegTime () const;

  /**
   * \brief Set the registration lifetime field.
   * \param time the registration lifetime value (units of 60 seconds)
   */
  void SetRegTime (uint16_t time);

  /**
   * \brief Get the EUI-64 field.
   * \return EUI-64 value
   */
  Mac64Address GetEui64 () const;

  /**
   * \brief Set the EUI-64 field.
   * \param eui the EUI-64 value
   */
  void SetEui64 (Mac64Address eui);

  /**
   * \brief Print informations.
   * \param os output stream
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size.
   * \return serialized size
   */
  virtual uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize the packet.
   * \param start start offset
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start start offset
   * \return length of packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:

  /**
   * \brief The status value.
   */
  uint8_t m_status;

  /**
   * \brief The first reserved value.
   */
  uint8_t m_reserved1;

  /**
   * \brief The second reserved value.
   */
  uint16_t m_reserved2;

  /**
   * \brief The registration lifetime value (units of 60 seconds).
   */
  uint16_t m_regTime;

  /**
   * \brief The EUI-64 value.
   */
  Mac64Address m_eui64;

};

/**
 * \ingroup sixlowpan
 * \class Icmpv6OptionSixLowPanContext
 * \brief ICMPv6 SixLowPan Context Option header.
 */
class Icmpv6OptionSixLowPanContext : public Icmpv6OptionHeader
{
public:

  /**
   * \brief Constructor.
   */
  Icmpv6OptionSixLowPanContext ();

  /**
   * \brief Constructor.
   * \param c the c flag
   * \param cid the context identifier
   * \param time the valid lifetime (units of 60 seconds)
   * \param prefix the context prefix
   */
  Icmpv6OptionSixLowPanContext (bool c, uint8_t cid, uint16_t time, Ipv6Prefix prefix);

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6OptionSixLowPanContext ();

  /**
   * \brief Get the UID of this class.
   * \return UID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Get the context length field.
   * \return context length value
   */
  uint8_t GetContextLen () const;

  /**
   * \brief Set the context length field.
   * \param cLen the context length value
   */
  void SetContextLen (uint8_t cLen);

  /**
   * \brief Get the first reserved field.
   * \return first reserved value
   */
  uint8_t GetReserved1 () const;

  /**
   * \brief Set the first reserved field.
   * \param reserved the first reserved value
   */
  void SetReserved1 (uint8_t reserved1);

  /**
   * \brief Is compression flag ?
   * \return true if context is valid for use in compression, false otherwise
   */
  bool IsFlagC () const;

  /**
   * \brief Set the C flag.
   * \param c the C flag
   */
  void SetFlagC (bool c);

  /**
   * \brief Get the context identifier field.
   * \return context identifier value
   */
  uint8_t GetCid () const;

  /**
   * \brief Set the context identifier field.
   * \param cid the context identifier value
   */
  void SetCid (uint8_t cid);

  /**
   * \brief Get the second reserved field.
   * \return second reserved value
   */
  uint16_t GetReserved2 () const;

  /**
   * \brief Set the second reserved field.
   * \param reserved the second reserved value
   */
  void SetReserved2 (uint16_t reserved);

  /**
   * \brief Get the valid lifetime field.
   * \return valid lifetime value (units of 60 seconds)
   */
  uint16_t GetValidTime () const;

  /**
   * \brief Set the valid lifetime field.
   * \param time the valid lifetime value (units of 60 seconds)
   */
  void SetValidTime (uint16_t time);

  /**
   * \brief Get the context prefix field.
   * \return context prefix value
   */
  Ipv6Prefix GetContextPrefix () const;

  /**
   * \brief Set the context prefix field.
   * \param prefix the context prefix value
   */
  void SetContextPrefix (Ipv6Prefix prefix);

  /**
   * \brief Print informations.
   * \param os output stream
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size.
   * \return serialized size
   */
  virtual uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize the packet.
   * \param start start offset
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start start offset
   * \return length of packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:

  /**
   * \brief The context length value.
   */
  uint8_t m_contextLen;

  /**
   * \brief The first reserved value.
   */
  uint8_t m_reserved1;

  /**
   * \brief The compression flag, indicates that this context is valid for use in compression.
   */
  bool m_c;

  /**
   * \brief The context identifier value.
   */
  uint8_t m_cid;

  /**
   * \brief The second reserved value.
   */
  uint16_t m_reserved2;

  /**
   * \brief The valid lifetime value (units of 60 seconds).
   */
  uint16_t m_validTime;

  /**
   * \brief The context prefix value.
   */
  Ipv6Prefix m_prefix;

};

/**
 * \ingroup sixlowpan
 * \class Icmpv6OptionAuthoritativeBorderRouter
 * \brief ICMPv6 Authoritative Border Router Option header.
 */
class Icmpv6OptionAuthoritativeBorderRouter : public Icmpv6OptionHeader
{
public:

  /**
   * \brief Constructor.
   */
  Icmpv6OptionAuthoritativeBorderRouter ();

  /**
   * \brief Constructor.
   * \param version the version value
   * \param time the valid lifetime (units of 60 seconds)
   * \param address the 6LBR address
   */
  Icmpv6OptionAuthoritativeBorderRouter (uint32_t version, uint16_t time, Ipv6Address address);

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6OptionAuthoritativeBorderRouter ();

  /**
   * \brief Get the UID of this class.
   * \return UID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Get the version field.
   * \return version value
   */
  uint32_t GetVersion () const;

  /**
   * \brief Set the version field.
   * \param version the version value
   */
  void SetVersion (uint32_t version);

  /**
   * \brief Get the valid lifetime field.
   * \return valid lifetime value (units of 60 seconds)
   */
  uint16_t GetValidTime () const;

  /**
   * \brief Set the valid lifetime field.
   * \param time the valid lifetime value (units of 60 seconds)
   */
  void SetValidTime (uint16_t time);

  /**
   * \brief Get the 6LB router address field.
   * \return 6LB router address value
   */
  Ipv6Address GetRouterAddress () const;

  /**
   * \brief Set the 6LB router address field.
   * \param router the 6LB router address value
   */
  void SetRouterAddress (Ipv6Address router);

  /**
   * \brief Print informations.
   * \param os output stream
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size.
   * \return serialized size
   */
  virtual uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize the packet.
   * \param start start offset
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start start offset
   * \return length of packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  /**
   * \brief The version value.
   */
  uint32_t m_version;

  /**
   * \brief The valid lifetime value (units of 60 seconds).
   */
  uint16_t m_validTime;

  /**
   * \brief The 6LB router address value.
   */
  Ipv6Address m_routerAddress;

};

} /* namespace ns3 */

#endif /* SIXLOWPAN_ND_HEADER_H */
