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

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-packet-info-tag.h"
#include "ns3/ipv6-header.h"
#include "ns3/icmpv6-header.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include "sixlowpan-nd-header.h"
#include "sixlowpan-radvd-dad-entry.h"
#include "sixlowpan-radvd-interface.h"
#include "sixlowpan-radvd.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("SixLowPanRadvd");

NS_OBJECT_ENSURE_REGISTERED (SixLowPanRadvd);

TypeId SixLowPanRadvd::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SixLowPanRadvd")
        .SetParent<Radvd> ()
        .SetGroupName("SixLowPanpan")
        .AddConstructor<SixLowPanRadvd> ();
  ;
  return tid;
}

SixLowPanRadvd::SixLowPanRadvd ()
{
  NS_LOG_FUNCTION (this);
}

SixLowPanRadvd::~SixLowPanRadvd ()
{
  NS_LOG_FUNCTION (this);
  for (SixLowPanRadvdInterfaceListI it = m_sixlowConfs.begin (); it != m_sixlowConfs.end (); ++it)
    {
      *it = 0;
    }
  m_sixlowConfs.clear ();
}

void SixLowPanRadvd::AddSixLowPanRadvdConfiguration (Ptr<SixLowPanRadvdInterface> routerInterface)
{
  NS_LOG_FUNCTION (this << routerInterface);
  m_sixlowConfs.push_back (routerInterface);
}

void SixLowPanRadvd::SendRA (Ptr<SixLowPanRadvdInterface> config, Ipv6Address dst)
{
  NS_LOG_FUNCTION (this << dst);

  Icmpv6RA raHdr;
  Icmpv6OptionLinkLayerAddress llaHdr;
  Icmpv6OptionPrefixInformation prefixHdr;
  Icmpv6OptionSixLowPanContext contextHdr;
  Icmpv6OptionAuthoritativeBorderRouter abroHdr;

  std::list<Ptr<RadvdPrefix> > prefixes = config->GetPrefixes ();
  std::list<Ptr<SixLowPanRadvdContext> > contexts = config->GetContexts ();
  Ptr<Packet> p = Create<Packet> ();
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();

  /* set RA header information */
  raHdr.SetFlagM (config->IsManagedFlag ());
  raHdr.SetFlagO (config->IsOtherConfigFlag ());
  raHdr.SetFlagH (config->IsHomeAgentFlag ());
  raHdr.SetCurHopLimit (config->GetCurHopLimit ());
  raHdr.SetLifeTime (config->GetDefaultLifeTime ());
  raHdr.SetReachableTime (config->GetReachableTime ());
  raHdr.SetRetransmissionTime (config->GetRetransTimer ());

  /* add SLLAO */
  Address addr = ipv6->GetNetDevice (config->GetInterface ())->GetAddress ();
  llaHdr = Icmpv6OptionLinkLayerAddress (true, addr);
  p->AddHeader (llaHdr);

  /* add list of PIO */
  for (std::list<Ptr<RadvdPrefix> >::const_iterator jt = prefixes.begin (); jt != prefixes.end (); jt++)
    {
      uint8_t flags = 0;
      prefixHdr = Icmpv6OptionPrefixInformation ();
      prefixHdr.SetPrefix ((*jt)->GetNetwork ());
      prefixHdr.SetPrefixLength ((*jt)->GetPrefixLength ());
      prefixHdr.SetValidTime ((*jt)->GetValidLifeTime ());
      prefixHdr.SetPreferredTime ((*jt)->GetPreferredLifeTime ());

      if ((*jt)->IsOnLinkFlag ())
        {
          flags += 1 << 7;
        }

      if ((*jt)->IsAutonomousFlag ())
        {
          flags += 1 << 6;
        }

      if ((*jt)->IsRouterAddrFlag ())
        {
          flags += 1 << 5;
        }

      prefixHdr.SetFlags (flags);

      p->AddHeader (prefixHdr);
    }

  /* add list of 6CO */
  for (std::list<Ptr<SixLowPanRadvdContext> >::const_iterator it = contexts.begin (); it != contexts.end (); it++)
    {
      contextHdr = Icmpv6OptionSixLowPanContext ();
      contextHdr.SetContextLen ((*it)->GetContextLen ());
      contextHdr.SetFlagC ((*it)->IsFlagC ());
      contextHdr.SetCid ((*it)->GetCid ());
      contextHdr.SetValidTime ((*it)->GetValidTime ());
      contextHdr.SetContextPrefix ((*it)->GetContextPrefix ());

      p->AddHeader (contextHdr);
    }

  Address sockAddr;
  std::map<uint32_t, Ptr<Socket> > sendSockets = GetSendSockets ();
  sendSockets[config->GetInterface ()]->GetSockName (sockAddr);
  Ipv6Address src = Inet6SocketAddress::ConvertFrom (sockAddr).GetIpv6 (); /* indirizzo global? */

  /* add ABRO */
  abroHdr = Icmpv6OptionAuthoritativeBorderRouter ();
  abroHdr.SetVersion (config->GetVersion ());
  abroHdr.SetValidTime (config->GetValidTime ());
  abroHdr.SetRouterAddress (src);
  p->AddHeader (abroHdr);

  /* as we know interface index that will be used to send RA and 
   * we always send RA with router's link-local address, we can 
   * calculate checksum here.
   */
  raHdr.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + raHdr.GetSerializedSize (), Icmpv6L4Protocol::PROT_NUMBER);
  p->AddHeader (raHdr);

  /* Router advertisements MUST always have a ttl of 255
   * The ttl value should be set as a socket option, but this is not yet implemented
   */
  SocketIpTtlTag ttl;
  ttl.SetTtl (255);
  p->AddPacketTag (ttl);

  /* send RA */
  NS_LOG_LOGIC ("Send RA to " << dst);
  sendSockets[config->GetInterface ()]->SendTo (p, 0, Inet6SocketAddress (dst, 0));
}

void SixLowPanRadvd::SendDAC (uint32_t interfaceIndex, Ipv6Address dst, uint8_t status, uint16_t time,
                           Mac64Address eui, Ipv6Address registered)
{
  NS_LOG_FUNCTION (this << interfaceIndex << dst << status << time << eui << registered);
  Ptr<Packet> p = Create<Packet> ();
  Icmpv6DuplicateAddress dac (status, time, eui, registered);

  Address sockAddr;
  std::map<uint32_t, Ptr<Socket> > sendSockets = GetSendSockets ();
  sendSockets[interfaceIndex]->GetSockName (sockAddr);
  Ipv6Address src = Inet6SocketAddress::ConvertFrom (sockAddr).GetIpv6 ();

  dac.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + dac.GetSerializedSize (), Icmpv6L4Protocol::PROT_NUMBER);
  p->AddHeader (dac);

  /* The ttl value should be set as a socket option, but this is not yet implemented */
  SocketIpTtlTag ttl;
  ttl.SetTtl (64); /* SixLowPanpanNdProtocol::MULTIHOP_HOPLIMIT */
  p->AddPacketTag (ttl);

  /* send DAC */
  NS_LOG_LOGIC ("Send DAC to " << dst);
  sendSockets[interfaceIndex]->SendTo (p, 0, Inet6SocketAddress (dst, 0));
}

void SixLowPanRadvd::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet = 0;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      if (Inet6SocketAddress::IsMatchingType (from))
        {
          Ipv6PacketInfoTag interfaceInfo;
          if (!packet->RemovePacketTag (interfaceInfo))
            {
              NS_ABORT_MSG ("No incoming interface on RADVD message, aborting.");
            }

          Ipv6Header hdr;

          packet->RemoveHeader (hdr);
          uint8_t type;
          packet->CopyData (&type, sizeof(type));

          switch (type)
          {
            case Icmpv6Header::ICMPV6_ND_ROUTER_SOLICITATION:
              HandleRS (packet, hdr.GetSourceAddress (), interfaceInfo);
              break;
            case Icmpv6Header::ICMPV6_ND_DUPLICATE_ADDRESS_REQUEST:
              HandleDAR (packet, hdr.GetSourceAddress (), interfaceInfo);
              break;
            default:
              break;
          }
        }
    }
}

void SixLowPanRadvd::HandleRS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6PacketInfoTag interfaceInfo)
{
  uint32_t incomingIf = interfaceInfo.GetRecvIf ();
  Ptr<NetDevice> dev = GetNode ()->GetDevice (incomingIf);
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice (dev);

  Icmpv6RS rsHdr;
  uint64_t delay = 0;
  Time t;
  packet->RemoveHeader (rsHdr);

  NS_LOG_INFO ("Received ICMPv6 Router Solicitation from " << src << " code = " << (uint32_t)rsHdr.GetCode ());

  for (SixLowPanRadvdInterfaceListCI it = m_sixlowConfs.begin (); it != m_sixlowConfs.end (); it++)
    {
      if (ipInterfaceIndex == (*it)->GetInterface ())
        {
          /* calculate minimum delay between RA */
          delay = static_cast<uint64_t> (GetJitter ()->GetValue (0, MAX_RA_DELAY_TIME * 1000));
          t = Simulator::Now () + MilliSeconds (delay); /* absolute time of solicited RA */

          if (Simulator::Now () < (*it)->GetLastRaTxTime () + Seconds (MIN_DELAY_BETWEEN_RAS))
            {
              t += MilliSeconds (MIN_DELAY_BETWEEN_RAS * 1000);
            }

          /* if our solicited RA is before the next periodic RA, we schedule it */
          bool scheduleSingle = true;

          std::map<uint32_t, EventId> solEventIds = GetSolEventIds ();
          if (solEventIds.find ((*it)->GetInterface ()) != solEventIds.end ())
            {
              if (solEventIds[(*it)->GetInterface ()].IsRunning ())
                {
                  scheduleSingle = false;
                }
            }

          std::map<uint32_t, EventId> unsolEventIds = GetUnsolEventIds ();
          if (unsolEventIds.find ((*it)->GetInterface ()) != unsolEventIds.end ())
            {
              if (t.GetTimeStep () > static_cast<int64_t> (unsolEventIds[(*it)->GetInterface ()].GetTs ()))
                {
                  scheduleSingle = false;
                }
            }

          if (scheduleSingle)
            {
              NS_LOG_INFO ("schedule new RA");
              SetSolEventId ((*it)->GetInterface (),
                             Simulator::Schedule (MilliSeconds (delay),
                                                  &SixLowPanRadvd::SendRA,
                                                  this, (*it),
                                                  src));
            }
        }
    }
}

void SixLowPanRadvd::HandleDAR (Ptr<Packet> packet, Ipv6Address const &src, Ipv6PacketInfoTag interfaceInfo)
{
  uint32_t incomingIf = interfaceInfo.GetRecvIf ();
  Ptr<NetDevice> dev = GetNode ()->GetDevice (incomingIf);
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice (dev);

  Icmpv6DuplicateAddress darHdr (1);
  packet->RemoveHeader (darHdr);

  NS_LOG_INFO ("Received ICMPv6 Duplicate Address Request from " << src << " code = " << (uint32_t)darHdr.GetCode ());

  Ipv6Address reg = darHdr.GetRegAddress ();

  if (!reg.IsMulticast () && src != Ipv6Address::GetAny () && !src.IsMulticast ())
    {
      Ptr<SixLowPanRadvdDadEntry> entry = 0;
      for (SixLowPanRadvdInterfaceListCI it = m_sixlowConfs.begin (); it != m_sixlowConfs.end (); it++)
        {
          if (ipInterfaceIndex == (*it)->GetInterface ())
            {
              for (SixLowPanRadvdInterface::DadTableI jt = (*it)->GetDadTable ().begin (); jt != (*it)->GetDadTable ().end (); jt++)
                {
                  if ((*jt)->GetRegAddress () == reg)
                    {
                      entry = (*jt);
                    }
                }
              if (entry)
                {
                  if (entry->GetEui64 () == darHdr.GetEui64 ())
                    {
                      NS_LOG_LOGIC ("No duplicate, same EUI-64. Entry updated.");

                      entry->SetRegTime (darHdr.GetRegTime ());

                      SendDAC (ipInterfaceIndex, src, 0, darHdr.GetRegTime (), darHdr.GetEui64 (), darHdr.GetRegAddress ());
                    }
                  else
                    {
                      NS_LOG_LOGIC ("Duplicate, different EUI-64.");
                      SendDAC (ipInterfaceIndex, src, 1, darHdr.GetRegTime (), darHdr.GetEui64 (), darHdr.GetRegAddress ());
                    }
                }
              else
                {
                  NS_LOG_LOGIC ("Entry did not exist. Entry created.");

                  entry = new SixLowPanRadvdDadEntry;
                  entry->SetRegTime (darHdr.GetRegTime ());
                  entry->SetEui64 (darHdr.GetEui64 ());
                  entry->SetRegAddress (darHdr.GetRegAddress ());

                  (*it)->AddDadEntry (entry);

                  SendDAC (ipInterfaceIndex, src, 0, darHdr.GetRegTime (), darHdr.GetEui64 (), darHdr.GetRegAddress ());
                }
            }
        }
    }
  else
    {
      NS_LOG_ERROR ("Validity checks for DAR not satisfied.");
    } 
}


void SixLowPanRadvd::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  TypeId tid = TypeId::LookupByName ("ns3::Ipv6RawSocketFactory");

  if (!GetRecvSocket ())
    {
      Ptr<Socket> recvSocket = Socket::CreateSocket (GetNode (), tid);

      NS_ASSERT (recvSocket);

      recvSocket->Bind (Inet6SocketAddress (Ipv6Address::GetAllRoutersMulticast (), 0));
      recvSocket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
      recvSocket->SetRecvCallback (MakeCallback (&SixLowPanRadvd::HandleRead, this));
      recvSocket->ShutdownSend ();
      recvSocket->SetRecvPktInfo (true);

      SetRecvSocket (recvSocket);
    }

  for (SixLowPanRadvdInterfaceListCI it = m_sixlowConfs.begin (); it != m_sixlowConfs.end (); it++)
    {
      if (GetSendSockets ().find ((*it)->GetInterface ()) == GetSendSockets ().end ())
        {
          Ptr<Ipv6L3Protocol> ipv6 = GetNode ()->GetObject<Ipv6L3Protocol> ();
          Ptr<Ipv6Interface> iFace = ipv6->GetInterface ((*it)->GetInterface ());

          Ptr<Socket> sendSocket = Socket::CreateSocket (GetNode (), tid);

          sendSocket->Bind (Inet6SocketAddress (iFace->GetLinkLocalAddress ().GetAddress (), 0));
          sendSocket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
          sendSocket->ShutdownRecv ();

          SetSendSocket ((*it)->GetInterface (), sendSocket);
        }
    }
}


} /* namespace ns3 */
