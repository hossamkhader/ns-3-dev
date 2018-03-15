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

#ifndef SIXLOWPAN_RADVD_H
#define SIXLOWPAN_RADVD_H

#include "ns3/radvd.h"
#include "ns3/socket.h"
#include "ns3/ipv6-packet-info-tag.h"

#include "sixlowpan-radvd-interface.h"


namespace ns3
{

/**
 * \ingroup sixlowpan
 * \defgroup sixlowpanradvd SixLowPanRadvd
 */

/**
 * \ingroup sixlowpanradvd
 * \class SixLowPanRadvd
 * \brief Router advertisement daemon for 6LoWPAN Border Router.
 */
class SixLowPanRadvd : public Radvd
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  SixLowPanRadvd ();

  virtual ~SixLowPanRadvd ();

  /**
   * \brief 6LBR constants : min context change delay (s).
   */
  static const uint16_t MIN_CONTEXT_CHANGE_DELAY = 300;

  /**
   * \brief 6LR constants : max RA transmission.
   */
  static const uint8_t MAX_RTR_ADVERTISEMENTS = 3;

  /**
   * \brief 6LR constants : min delay between RA (s).
   */
  static const uint8_t MIN_DELAY_BETWEEN_RAS = 10;

  /**
   * \brief 6LR constants : max delay between RA (s).
   */
  static const uint8_t MAX_RA_DELAY_TIME = 2;

  /**
   * \brief Add configuration for an interface;
   * \param routerInterface configuration
   */
  void AddSixLowPanRadvdConfiguration (Ptr<SixLowPanRadvdInterface> routerInterface);

private:
  /// Container: Ptr to SixLowPanRadvdInterface
  typedef std::list<Ptr<SixLowPanRadvdInterface> > SixLowPanRadvdInterfaceList;
  /// Container Iterator: Ptr to SixLowPanRadvdInterface
  typedef std::list<Ptr<SixLowPanRadvdInterface> >::iterator SixLowPanRadvdInterfaceListI;
  /// Container Const Iterator: Ptr to SixLowPanRadvdInterface
  typedef std::list<Ptr<SixLowPanRadvdInterface> >::const_iterator SixLowPanRadvdInterfaceListCI;
  
  /**
   * \brief Start the application.
   */
  virtual void StartApplication ();

  /**
   * \brief Send a RA for 6LoWPAN ND (+ PIO, 6CO, ABRO, SLLAO).
   * \param config interface configuration
   * \param dst destination address
   */
  void SendRA (Ptr<SixLowPanRadvdInterface> config, Ipv6Address dst);
  
  /**
   * \brief Send a DAC for 6LoWPAN ND.
   * \param interfaceIndex interface index
   * \param dst destination IPv6 address
   * \param status status field
   * \param time registration lifetime
   * \param eui EUI-64
   * \param registered registered IPv6 address
   */
  void SendDAC (uint32_t interfaceIndex, Ipv6Address dst, uint8_t status, uint16_t time,
                Mac64Address eui, Ipv6Address registered);

  /**
   * \brief Handle received packet, especially router solicitation
   * \param socket socket to read data from
   */
  void HandleRead (Ptr<Socket> socket);
  
  /**
   * \brief Receive RS for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param interfaceInfo tag that carries socket ancillary data
   */
  void HandleRS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6PacketInfoTag interfaceInfo);

  /**
   * \brief Receive DAR for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param interfaceInfo tag that carries socket ancillary data
   */
  void HandleDAR (Ptr<Packet> packet, Ipv6Address const &src, Ipv6PacketInfoTag interfaceInfo);

  /**
   * \brief List of configuration for interface.
   */
  SixLowPanRadvdInterfaceList m_sixlowConfs;

};

} /* namespace ns3 */

#endif /* SIXLOWPAN_RADVD_H */
