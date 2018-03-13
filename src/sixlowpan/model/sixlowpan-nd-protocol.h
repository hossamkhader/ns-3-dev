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

#ifndef SIXLOWPAN_ND_PROTOCOL_H
#define SIXLOWPAN_ND_PROTOCOL_H

#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"

#include "sixlowpan-ndisc-cache.h"

namespace ns3 {

/**
 * \ingroup sixlowpan
 * \class SixLowPanNdProtocol
 * \brief An optimization of the ND protocol for 6LoWPANs.
 */
class SixLowPanNdProtocol : public Icmpv6L4Protocol
{
public:
  /**
   * \brief 6LBR constants : min context change delay.
   */
  static const uint16_t MIN_CONTEXT_CHANGE_DELAY;

  /**
   * \brief 6LR constants : max RA transmission.
   */
  static const uint8_t MAX_RTR_ADVERTISEMENTS;

  /**
   * \brief 6LR constants : min delay between RA.
   */
  static const uint8_t MIN_DELAY_BETWEEN_RAS;

  /**
   * \brief 6LR constants : max delay between RA.
   */
  static const uint8_t MAX_RA_DELAY_TIME;

  /**
   * \brief 6LR constants : tentative neighbor cache entry lifetime.
   */
  static const uint8_t TENTATIVE_NCE_LIFETIME;

  /**
   * \brief router constants : multihop hoplimit.
   */
  static const uint8_t MULTIHOP_HOPLIMIT;

  /**
   * \brief host constants : RS interval.
   */
  static const uint8_t RTR_SOLICITATION_INTERVAL;

  /**
   * \brief host constants : max RS transmission.
   */
  static const uint8_t MAX_RTR_SOLICITATIONS;

  /**
   * \brief host constants : max RS interval.
   */
  static const uint8_t MAX_RTR_SOLICITATION_INTERVAL;

  /**
   * \brief Constructor.
   */
  SixLowPanNdProtocol ();

  /**
   * \brief Destructor.
   */
  virtual ~SixLowPanNdProtocol ();

  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Forge a Neighbor Advertisement.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param target target IPv6 address
   * \param hardwareAddress our mac address
   * \param flags flags (bitfield => R (4), S (2), O (1))
   * \return NA packet (with IPv6 header)
   */
  Ptr<Packet> ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress, uint8_t flags);

  /**
   * \brief Send a NS for 6LoWPAN ND (+ SLLAO).
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param target target IPv6 address
   * \param linkAddr link-layer address (SLLAO)
   */
  virtual void SendNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address linkAddr);

  /**
   * \brief Send a NS for 6LoWPAN ND (+ ARO, SLLAO).
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param time registration lifetime (ARO)
   * \param eui EUI-64 (ARO)
   * \param linkAddr link-layer address (SLLAO)
   */
  void SendSixLowPanARO (Ipv6Address src, Ipv6Address dst, uint16_t time, Mac64Address eui,
                         Address linkAddr);

  /**
   * \brief Send a NA for 6LoWPAN ND (+ ARO).
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param status status (ARO)
   * \param time registration lifetime (ARO)
   * \param eui EUI-64 (ARO)
   */
  void SendSixLowPanARO (Ipv6Address src, Ipv6Address dst, uint8_t status, uint16_t time,
                         Mac64Address eui);

  /**
   * \brief Send a RA for 6LoWPAN ND (+ PIO, 6CO, ABRO, SLLAO).
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param interface the interface from which the packet will be sent
   */
  void SendSixLowPanRA (Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Send a DAR for 6LoWPAN ND.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param time registration lifetime
   * \param eui EUI-64
   * \param registered registered IPv6 address
   */
  void SendSixLowPanDAR (Ipv6Address src, Ipv6Address dst, uint16_t time, Mac64Address eui,
                      Ipv6Address registered);

  /**
   * \brief Receive method.
   * \param p the packet
   * \param header the IPv6 header
   * \param interface the interface from which the packet is coming
   * \returns the receive status
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                               Ipv6Header const &header,
                                               Ptr<Ipv6Interface> interface);

  /**
   * \brief Get the cache corresponding to the device.
   * \param device the device
   * \returns the SixLowPanNdiscCache associated with the device
   */
  Ptr<SixLowPanNdiscCache> FindSixLowPanCache (Ptr<NetDevice> device);

  /**
   * \brief Function called to send NS + ARO + SLLAO.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param time registration lifetime (ARO)
   * \param eui EUI-64 (ARO)
   * \param linkAddr link-layer address (SLLAO)
   */
  void RetransmitARO (Ipv6Address src, Ipv6Address dst, uint16_t time, Mac64Address eui,
                      Address linkAddr);

  /**
   * \brief Function called to send RS + SLLAO.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param linkAddress link-layer address (SLLAO)
   */
  void RetransmitRS (Ipv6Address src, Ipv6Address dst, Address linkAddr);

protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

private:
  typedef std::list<Ptr<SixLowPanNdiscCache> > SixLowPanCacheList; //!< container of SixLowPanNdiscCaches

  /**
   * \brief A list of cache by device.
   */
  SixLowPanCacheList m_sixlowCList;

  /**
   * \brief Use multihop DAD mechanism
   */
  bool m_multihopDad;

  /**
   * \brief Number of RS retransmission.
   */
  uint8_t m_rsRetransmit;

  /**
   * \brief Is an RA received.
   */
  bool m_receivedRA;

  /**
   * \brief Number of NS + ARO + SLLAO retransmission.
   */
  uint8_t m_aroRetransmit;

  /**
   * \brief The amount of time (units of 60 seconds) that the router should retain the NCE for the node.
   */
  uint16_t m_regTime;

  /**
   * \brief The advance to perform maintaining of RA's information and registration.
   */
  uint16_t m_advance;

  /**
   * \brief Is a 6LoWPAN Border router.
   */
  bool m_border;

  /**
   * \brief Receive NS for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleSixLowPanNS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                       Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive NA for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleSixLowPanNA (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                       Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive RS for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleSixLowPanRS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                       Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive RA for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleSixLowPanRA (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                       Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive DAC for 6LoWPAN ND method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleSixLowPanDAC (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                        Ptr<Ipv6Interface> interface);

  /**
   * \brief Set the m_receivedRA flag.
   * \param received value
   */
  void SetReceivedRA (bool received);
};

} /* namespace ns3 */

#endif
