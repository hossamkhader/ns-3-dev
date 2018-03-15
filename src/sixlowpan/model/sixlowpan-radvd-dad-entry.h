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

#ifndef SIXLOWPAN_RADVD_DAD_ENTRY_H
#define SIXLOWPAN_RADVD_DAD_ENTRY_H 

#include <stdint.h>

#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"

namespace ns3
{

/**
 * \ingroup sixlowpanradvd
 * \class SixLowPanRadvdDadEntry
 * \brief 6LoWPAN DAD entry for SixLowPanRadvd application.
 */
class SixLowPanRadvdDadEntry : public SimpleRefCount<SixLowPanRadvdDadEntry>
{
public:
  SixLowPanRadvdDadEntry ();

  /**
   * \brief Constructor.
   * \param regTime the registration lifetime
   * \param eui64 the EUI-64
   * \param regAddress the registered Address
   */
  SixLowPanRadvdDadEntry (uint16_t regTime, Mac64Address eui64, Ipv6Address regAddress);

  ~SixLowPanRadvdDadEntry ();

  /**
   * \brief Get the registration lifetime.
   * \return registration lifetime value (units of 60 seconds)
   */
  uint16_t GetRegTime () const;

  /**
   * \brief Set the registration lifetime.
   * \param time the registration lifetime value (units of 60 seconds)
   */
  void SetRegTime (uint16_t time);

  /**
   * \brief Get the EUI-64.
   * \return EUI-64 value
   */
  Mac64Address GetEui64 () const;

  /**
   * \brief Set the EUI-64.
   * \param eui the EUI-64 value
   */
  void SetEui64 (Mac64Address eui);

  /**
   * \brief Get the registered address.
   * \return registered address value
   */
  Ipv6Address GetRegAddress () const;

  /**
   * \brief Set the registered address.
   * \param registered the registered address value
   */
  void SetRegAddress (Ipv6Address registered);

private:
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

} /* namespace ns3 */

#endif /* SIXLOWPAN_RADVD_DAD_ENTRY_H */
