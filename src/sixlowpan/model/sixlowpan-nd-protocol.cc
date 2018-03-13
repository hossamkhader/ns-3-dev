/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007-2009 Strasbourg University
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
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 *         David Gross <gdavid.devel@gmail.com>
 *         Mehdi Benamor <benamor.mehdi@ensi.rnu.tn>
 *         Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/mac64-address.h"
#include "ns3/mac48-address.h"
#include "ns3/mac16-address.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/sgi-hashmap.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"

#include "sixlowpan-nd-protocol.h"
#include "sixlowpan-ndisc-cache.h"
#include "sixlowpan-ndisc-ra-options.h"
#include "sixlowpan-nd-header.h"
#include "sixlowpan-net-device.h"
#include "src/network/model/packet.h"
#include "src/core/model/integer.h"
#include "src/core/model/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SixLowPanNdProtocol");

NS_OBJECT_ENSURE_REGISTERED (SixLowPanNdProtocol);

const uint16_t SixLowPanNdProtocol::MIN_CONTEXT_CHANGE_DELAY = 300;

const uint8_t SixLowPanNdProtocol::MAX_RTR_ADVERTISEMENTS = 3;
const uint8_t SixLowPanNdProtocol::MIN_DELAY_BETWEEN_RAS = 10;
const uint8_t SixLowPanNdProtocol::MAX_RA_DELAY_TIME = 2;
const uint8_t SixLowPanNdProtocol::TENTATIVE_NCE_LIFETIME = 20;

const uint8_t SixLowPanNdProtocol::MULTIHOP_HOPLIMIT = 64;

const uint8_t SixLowPanNdProtocol::RTR_SOLICITATION_INTERVAL = 10;
const uint8_t SixLowPanNdProtocol::MAX_RTR_SOLICITATIONS = 3;
const uint8_t SixLowPanNdProtocol::MAX_RTR_SOLICITATION_INTERVAL = 60;

SixLowPanNdProtocol::SixLowPanNdProtocol ()
: m_rsRetransmit (0),
  m_aroRetransmit (0)
{
  NS_LOG_FUNCTION (this);
}

SixLowPanNdProtocol::~SixLowPanNdProtocol ()
{
  NS_LOG_FUNCTION (this);
}

TypeId SixLowPanNdProtocol::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SixLowPanNdProtocol")
            .SetParent<Icmpv6L4Protocol> ()
            .SetGroupName ("Internet")
            .AddConstructor<SixLowPanNdProtocol> ()
            .AddAttribute ("Border", "Is the node a 6LoWPAN Border Router.",
                           BooleanValue (false),
                           MakeBooleanAccessor (&SixLowPanNdProtocol::m_border),
                           MakeBooleanChecker ())
            .AddAttribute ("RegistrationLifeTime", "The amount of time (units of 60 seconds) that the router should retain the NCE for the node.",
                           UintegerValue (60),
                           MakeUintegerAccessor (&SixLowPanNdProtocol::m_regTime),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("AdvanceTime", "The advance to perform maintaining of RA's information and registration.",
                           UintegerValue (5),
                           MakeUintegerAccessor (&SixLowPanNdProtocol::m_advance),
                           MakeUintegerChecker<uint16_t> ())
            ;
  return tid;
}

Ptr<Packet> SixLowPanNdProtocol::ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress, uint8_t flags)
{
  NS_LOG_FUNCTION (this << src << dst << hardwareAddress << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  Ipv6Header ipHeader;
  Icmpv6NA na;
  Icmpv6OptionLinkLayerAddress llOption (0, hardwareAddress);  /* we give our mac address in response */

  NS_LOG_LOGIC ("Send NA ( from " << src << " to " << dst << " target " << target << ")");

  /* forge the entire NA packet from IPv6 header to ICMPv6 link-layer option, so that the packet does not pass by Icmpv6L4Protocol::Lookup again */

  p->AddHeader (llOption);
  na.SetIpv6Target (target);

  if ((flags & 1))
    {
      na.SetFlagO (true);
    }
  if ((flags & 2) && src != Ipv6Address::GetAny ())
    {
      na.SetFlagS (true);
    }
  if ((flags & 4))
    {
      na.SetFlagR (true);
    }

  na.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + na.GetSerializedSize (), PROT_NUMBER);
  p->AddHeader (na);

  ipHeader.SetSourceAddress (src);
  ipHeader.SetDestinationAddress (dst);
  ipHeader.SetNextHeader (PROT_NUMBER);
  ipHeader.SetPayloadLength (p->GetSize ());
  ipHeader.SetHopLimit (255);

  p->AddHeader (ipHeader);

  return p;
}

void SixLowPanNdProtocol::SendNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target,
                                  Address linkAddr)
{
  NS_LOG_FUNCTION (this << src << dst << target << linkAddr);

  Ptr<Ipv6L3Protocol> ipv6 = GetNode ()->GetObject<Ipv6L3Protocol> ();
  uint32_t interfaceId = ipv6->GetInterfaceForAddress (src);
  Ptr<Ipv6Interface> interface = ipv6->GetInterface (interfaceId);
  Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());

  if (src != Ipv6Address::GetAny ())
    {
      NS_LOG_WARN ("An unspecified source address MUST NOT be used in NS messages.");
      return;
    }

  // only send NS if the target is a router.
  NdiscCache::Entry *entry = cache->Lookup (dst);
  if (entry)
    {
      if (entry->IsRouter ())
        {
          Icmpv6L4Protocol::SendNS (interface->GetLinkLocalAddress ().GetAddress (), dst,
                                    target, linkAddr);
        }
    }
}

void SixLowPanNdProtocol::SendSixLowPanARO (Ipv6Address src, Ipv6Address dst, uint16_t time,
                                            Mac64Address eui, Address linkAddr)
{
  NS_LOG_FUNCTION (this << src << dst << time << eui << linkAddr);

  /* an unspecified src MUST NOT be used in NS */

  NS_ASSERT_MSG (src != Ipv6Address::GetAny (), "An unspecified source address MUST NOT be used in ARO messages.");
  NS_ASSERT_MSG (!dst.IsMulticast (), "Destination address must not be a multicast address in ARO messages.");

  Ptr<Packet> p = Create<Packet> ();
  Icmpv6NS ns (Ipv6Address::GetZero ());

  /* ARO (request) + SLLAO */
  Icmpv6OptionAddressRegistration arOption (time, eui);
  Icmpv6OptionLinkLayerAddress llOption (1, linkAddr);

  NS_LOG_LOGIC ("Send NS ( from " << src << " to " << dst << ")");

  p->AddHeader (arOption);
  p->AddHeader (llOption);
  ns.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + ns.GetSerializedSize (), PROT_NUMBER);
  p->AddHeader (ns);

  SendMessage (p, src, dst, 255);
}

void SixLowPanNdProtocol::SendSixLowPanARO (Ipv6Address src, Ipv6Address dst, uint8_t status,
                                            uint16_t time, Mac64Address eui)
{
  NS_LOG_FUNCTION (this << src << dst << static_cast<uint32_t> (status) << time << eui);
  Ptr<Packet> p = Create<Packet> ();
  Icmpv6NA na;

  /* ARO (response) */
  Icmpv6OptionAddressRegistration arOption (status, time, eui);

  NS_LOG_LOGIC ("Send NA ( from " << src << " to " << dst << ")");

  na.SetIpv6Target (Ipv6Address::GetZero ());
  na.SetFlagO (false);
  na.SetFlagS (true);
  na.SetFlagR (true);

  p->AddHeader (arOption);
  na.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + na.GetSerializedSize (), PROT_NUMBER);
  p->AddHeader (na);

  SendMessage (p, src, dst, 255);
}

void SixLowPanNdProtocol::SendSixLowPanRA (Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << src << dst << interface);

  Ptr<SixLowPanNdiscCache> sixCache = FindSixLowPanCache (interface->GetDevice ());
  std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *> raCache = sixCache->GetRaCache ();

  for (std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *>::iterator it = raCache.begin ();
      it != raCache.end (); it++)
    {
      Icmpv6RA raHdr;
      Ipv6Header ipHeader;

      Ptr<Packet> p = Create<Packet> ();

      /* set RA header information */
      raHdr.SetFlagM (it->second->IsManagedFlag ());
      raHdr.SetFlagO (it->second->IsOtherConfigFlag ());
      raHdr.SetFlagH (it->second->IsHomeAgentFlag ());
      raHdr.SetCurHopLimit (it->second->GetCurHopLimit ());
      raHdr.SetLifeTime (it->second->GetRouterLifeTime ());
      raHdr.SetReachableTime (it->second->GetReachableTime ());
      raHdr.SetRetransmissionTime (it->second->GetRetransTimer ());

      /* Add ABRO */
      Icmpv6OptionAuthoritativeBorderRouter abroHdr;
      abroHdr.SetVersion (it->second->GetVersion ());
      abroHdr.SetValidTime (it->second->GetValidTime ());
      abroHdr.SetRouterAddress (it->second->GetBorderAddress ());
      p->AddHeader (abroHdr);

      /* Add SLLAO */
      Address addr = interface->GetDevice ()->GetAddress ();
      Icmpv6OptionLinkLayerAddress llaHdr (1, addr);
      p->AddHeader (llaHdr);

      /* Add PIOs */
      std::map<Ipv6Address, Ptr<SixLowPanPrefix> > prefixes = it->second->GetPrefixes ();
      for (std::map<Ipv6Address, Ptr<SixLowPanPrefix> >::iterator jt = prefixes.begin (); jt != prefixes.end (); jt++)
        {
          Icmpv6OptionPrefixInformation prefixHdr;
          prefixHdr.SetPrefixLength (jt->second->GetPrefixLength ());
          prefixHdr.SetFlags (jt->second->GetFlags ());
          prefixHdr.SetValidTime (jt->second->GetValidLifeTime ());
          prefixHdr.SetPreferredTime (jt->second->GetPreferredLifeTime ());
          prefixHdr.SetPrefix (jt->second->GetPrefix ());
          p->AddHeader (prefixHdr);
        }

      /* Add 6COs */
      std::map<uint8_t, Ptr<SixLowPanContext> > contexts = it->second->GetContexts ();
      for (std::map<uint8_t, Ptr<SixLowPanContext> >::iterator i = contexts.begin (); i != contexts.end (); i++)
        {
          Icmpv6OptionSixLowPanContext sixHdr;
          sixHdr.SetContextLen (i->second->GetContextLen ());
          sixHdr.SetFlagC (i->second->IsFlagC ());
          sixHdr.SetCid (i->second->GetCid ());
          sixHdr.SetValidTime (i->second->GetValidTime ());
          sixHdr.SetContextPrefix (i->second->GetContextPrefix ());
          p->AddHeader (sixHdr);
        }

      raHdr.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + raHdr.GetSerializedSize (), PROT_NUMBER);
      p->AddHeader (raHdr);

      ipHeader.SetSourceAddress (src);
      ipHeader.SetDestinationAddress (dst);
      ipHeader.SetNextHeader (PROT_NUMBER);
      ipHeader.SetPayloadLength (p->GetSize ());
      ipHeader.SetHopLimit (255);

      /* send RA */
      NS_LOG_LOGIC ("Send RA to " << dst);
      interface->Send (p, ipHeader, dst);
    }
}

void SixLowPanNdProtocol::SendSixLowPanDAR (Ipv6Address src, Ipv6Address dst, uint16_t time, Mac64Address eui,
                                            Ipv6Address registered)
{
  NS_LOG_FUNCTION (this << src << dst << time << eui << registered);
  Ptr<Packet> p = Create<Packet> ();
  Icmpv6DuplicateAddress dar (time, eui, registered);

  NS_LOG_LOGIC ("Send DAR ( from " << src << " to " << dst << ")");

  dar.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + dar.GetSerializedSize (), PROT_NUMBER);
  p->AddHeader (dar);

  SendMessage (p, src, dst, MULTIHOP_HOPLIMIT);
}

enum IpL4Protocol::RxStatus SixLowPanNdProtocol::Receive (Ptr<Packet> packet, Ipv6Header const &header,
                                                          Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << header.GetSourceAddress () << header.GetDestinationAddress () <<
                   interface);
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice());
  NS_ASSERT_MSG (sixDevice != NULL, "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  uint8_t type;
  packet->CopyData (&type, sizeof(type));

  switch (type)
  {
    case Icmpv6Header::ICMPV6_ND_ROUTER_SOLICITATION:
      if (ipv6->IsForwarding (ipv6->GetInterfaceForDevice (sixDevice)))
        {
          HandleSixLowPanRS (packet, header.GetSourceAddress (), header.GetDestinationAddress (), interface);
        }
      break;
    case Icmpv6Header::ICMPV6_ND_ROUTER_ADVERTISEMENT:
      HandleSixLowPanRA (packet, header.GetSourceAddress (), header.GetDestinationAddress (), interface);
      break;
    case Icmpv6Header::ICMPV6_ND_NEIGHBOR_SOLICITATION:
      if (ipv6->IsForwarding (ipv6->GetInterfaceForDevice (sixDevice)))
        {
          HandleSixLowPanNS (packet, header.GetSourceAddress (), header.GetDestinationAddress (), interface);
        }
      break;
    case Icmpv6Header::ICMPV6_ND_NEIGHBOR_ADVERTISEMENT:
      HandleSixLowPanNA (packet, header.GetSourceAddress (), header.GetDestinationAddress (), interface);
      break;
    case Icmpv6Header::ICMPV6_ND_DUPLICATE_ADDRESS_CONFIRM:
      if (ipv6->IsForwarding (ipv6->GetInterfaceForDevice (sixDevice)))
        {
          HandleSixLowPanDAC (packet, header.GetSourceAddress (), header.GetDestinationAddress (), interface);
        }
      break;
    default:
      return Icmpv6L4Protocol::Receive (packet, header, interface);
      break;
  }

  return IpL4Protocol::RX_OK;
}

void SixLowPanNdProtocol::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (SixLowPanCacheList::const_iterator it = m_sixlowCList.begin (); it != m_sixlowCList.end (); it++)
    {
      Ptr<SixLowPanNdiscCache> cache = *it;
      cache->Dispose ();
      cache = 0;
    }
  m_sixlowCList.clear ();

  Icmpv6L4Protocol::DoDispose ();
}

void SixLowPanNdProtocol::HandleSixLowPanNS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                                             Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Icmpv6NS nsHdr;
  packet->RemoveHeader (nsHdr);
  Ipv6Address target = nsHdr.GetIpv6Target ();

  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice());
  NS_ASSERT_MSG (sixDevice != NULL, "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  Icmpv6OptionLinkLayerAddress llaHdr (1); /* SLLAO */
  Icmpv6OptionAddressRegistration aroHdr;

  bool next = true;
  bool hasLla = false;
  bool hasAro = false;

  if (src != Ipv6Address::GetAny ())
    {
      /* search all options following the NS header */
      while (next == true)
        {
          uint8_t type;
          packet->CopyData (&type, sizeof(type));

          switch (type)
          {
            case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE:
              if (!hasLla)
                {
                  packet->RemoveHeader (llaHdr);
                  hasLla = true;
                }
              break;
            case Icmpv6Header::ICMPV6_OPT_ADDRESS_REGISTRATION:
              if (!hasAro)
                {
                  packet->RemoveHeader (aroHdr);
                  hasAro = true;
                }
              break;
            default:
              /* unknow option, quit */
              next = false;
          }
        }

      if (!hasLla) /* error! */
        {
          NS_LOG_ERROR ("NS message MUST have source link layer option.");
          return;
        }
      else /* NS + SLLAO */
        {
          /* Update "ARPv6" table with information of src */
          Ptr<NdiscCache> cache = FindCache (sixDevice);

          NdiscCache::Entry* entry = 0;
          entry = cache->Lookup (src);

          if (!entry && src.IsLinkLocal())
            {
              entry = cache->Add (src);
              uint8_t buf[16];
              src.GetBytes (buf);
              Mac16Address address;
              address.CopyFrom (buf+14);
              entry->SetRouter (false);
              entry->SetMacAddress (llaHdr.GetAddress ());
              entry->MarkReachable ();
            }
          else if (!entry)
            {
              entry = cache->Add (src);
              entry->SetRouter (false);
              entry->MarkStale (llaHdr.GetAddress ());
            }
          else if (entry->GetMacAddress () != llaHdr.GetAddress ())
            {
              entry->MarkStale (llaHdr.GetAddress ());
            }
        }

      if (!hasAro) /* NS + SLLAO */
        {
          /* Send NA to src with information requested about target (in "ARPv6" table) */
          Ptr<NdiscCache> cache = FindCache (sixDevice);

          NdiscCache::Entry* entry = 0;
          entry = cache->Lookup (target);
          uint8_t flags;

          if (!entry)
            {
              NS_LOG_LOGIC ("No entry with address " << target);
              return;
            }
          else
            {
              if (entry->IsRouter ())
                {
                  flags = 7; /* R + S + O flag */
                }
              else
                {
                  flags = 3; /* S + O flags */
                }

              /* NA + TLLAO */
              Ptr<Packet> p = ForgeNA (interface->GetLinkLocalAddress ().GetAddress (), src, target,
                                       entry->GetMacAddress (), flags);
              Ipv6Header header;
              p->RemoveHeader (header);
              interface->Send (p, header, src);
            }
        }
      else /* NS + SLLAO + ARO */
        {
          /* Update NCE with information of src and ARO */
          Ptr<SixLowPanNdiscCache> nCache = FindSixLowPanCache (sixDevice);

          SixLowPanNdiscCache::SixLowPanEntry* nEntry = 0;
          nEntry = nCache->SixLowPanLookup (src);

          std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *> raCache = nCache->GetRaCache ();

          if (!nEntry) /* no entry, creating */
            {
              if (aroHdr.GetRegTime () != 0)
                {
                  nEntry = nCache->SixLowPanAdd (src);
                  nEntry->SetRouter (false);
                  nEntry->SetMacAddress (llaHdr.GetAddress ());
                  nEntry->SetEui64 (aroHdr.GetEui64 ());
                  nEntry->MarkReachable ();

                  if (m_multihopDad) /* multihop DAD used */
                    {
                      nEntry->MarkTentative ();
                      for (std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *>::iterator it = raCache.begin ();
                          it != raCache.end (); it++)
                        {
                          Ipv6Address destination = it->second->GetBorderAddress ();
                          SendSixLowPanDAR (interface->GetAddressMatchingDestination (destination).GetAddress (), destination,
                                            aroHdr.GetRegTime (), aroHdr.GetEui64 (), src);
                        }
                      return;
                    }
                  else  /* multihop DAD NOT used */
                    {
                      nEntry->MarkRegistered (aroHdr.GetRegTime ());
                      SendSixLowPanARO (interface->GetLinkLocalAddress ().GetAddress (), src, 0, aroHdr.GetRegTime (),
                                        aroHdr.GetEui64 ());
                      return;
                    }
                }
            }
          else if (nEntry->GetEui64 () == aroHdr.GetEui64 ()) /* entry found, same EUI-64, updating */
            {
              if (aroHdr.GetRegTime () != 0)
                {
                  nEntry->SetRouter (false);
                  nEntry->SetMacAddress (llaHdr.GetAddress ());
                  nEntry->MarkReachable ();
                  nEntry->MarkRegistered (aroHdr.GetRegTime ());
                  SendSixLowPanARO (interface->GetLinkLocalAddress ().GetAddress (), src, 0, aroHdr.GetRegTime (),
                                    aroHdr.GetEui64 ());

                  if (m_multihopDad)
                    {
                      for (std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *>::iterator it = raCache.begin ();
                          it != raCache.end (); it++)
                        {
                          Ipv6Address destination = it->second->GetBorderAddress ();

                          /* Send request to update entry from DAD table */
                          SendSixLowPanDAR (interface->GetAddressMatchingDestination (destination).GetAddress (), destination,
                                            aroHdr.GetRegTime (), aroHdr.GetEui64 (), src);
                        }
                    }

                  return;
                }
              else /* ARO's Registration Lifetime = 0 */
                {
                  if (m_multihopDad)
                    {
                      for (std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *>::iterator it = raCache.begin ();
                          it != raCache.end (); it++)
                        {
                          Ipv6Address destination = it->second->GetBorderAddress ();

                          /* Send request to remove entry from DAD table */
                          SendSixLowPanDAR (interface->GetAddressMatchingDestination (destination).GetAddress (), destination,
                                            aroHdr.GetRegTime (), aroHdr.GetEui64 (), src);
                        }
                    }
                  /* Remove the entry from Neighbor Cache */
                  nCache->SixLowPanRemove (nEntry);

                  /* Remove the entry from "ARPv6" table */
                  Ptr<NdiscCache> cache = FindCache (sixDevice);

                  NdiscCache::Entry* entry = 0;
                  entry = cache->Lookup (src);
                  cache->Remove (entry);
                  return;
                }
            }
          else /* entry found, different EUI-64 */
            {
              if (m_multihopDad && nEntry->IsRegistered ())
                {
                  SendSixLowPanARO (interface->GetLinkLocalAddress ().GetAddress (),
                                    sixDevice->MakeLinkLocalAddressFromMac (aroHdr.GetEui64 ()), 1,
                                    aroHdr.GetRegTime (), aroHdr.GetEui64 ());
                }
              else if (!m_multihopDad)
                {
                  SendSixLowPanARO (interface->GetLinkLocalAddress ().GetAddress (),
                                    sixDevice->MakeLinkLocalAddressFromMac (aroHdr.GetEui64 ()), 1,
                                    aroHdr.GetRegTime (), aroHdr.GetEui64 ());
                }
              return;
            }
        }
    }
  else
    {
      NS_LOG_ERROR ("An unspecified source address MUST NOT be used in NS messages.");
      return;
    }
}

void SixLowPanNdProtocol::HandleSixLowPanNA (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                                             Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Ptr<Packet> p = packet->Copy ();
  Icmpv6NS naHdr;
  packet->RemoveHeader (naHdr);
  Ipv6Address target = naHdr.GetIpv6Target ();

  Icmpv6OptionLinkLayerAddress llaHdr (0); /* TLLAO */
  Icmpv6OptionAddressRegistration aroHdr;

  uint8_t type;
  packet->CopyData (&type, sizeof(type));

  switch (type)
  {
    case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET: /* NA + TLLAO */

      HandleNA (p, target, dst, interface); /* Handle response of Address Resolution */

      break;
    case Icmpv6Header::ICMPV6_OPT_ADDRESS_REGISTRATION: /* NA + ARO */

      packet->RemoveHeader (aroHdr);

      m_aroRetransmit = 0;

      if (aroHdr.GetStatus () == 0) /* status=0, success! */
        {
          /* schedule a new ARO to maintain NCE in routers */

          Simulator::Schedule (Time (Minutes (aroHdr.GetRegTime () - m_advance)), &SixLowPanNdProtocol::RetransmitARO, this,
                               dst, src, aroHdr.GetRegTime (), aroHdr.GetEui64 (), interface->GetDevice ()->GetAddress ());
        }
      else /* status NOT 0, fail! */
        {
          NS_LOG_LOGIC ("ARO status is NOT 0, registration failed!");
          /// \todo implement method to remove address that generated error.
          return;
        }
      break;
    default:
      NS_LOG_ERROR ("NA message MUST have option.");
      return;
  }
}

void SixLowPanNdProtocol::HandleSixLowPanRS (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                                             Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice());
  NS_ASSERT_MSG (sixDevice != NULL, "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  Icmpv6RS rsHeader;
  Icmpv6OptionLinkLayerAddress lla (1);

  packet->RemoveHeader (rsHeader);

  if (src != Ipv6Address::GetAny ())
    {
      uint8_t type;
      packet->CopyData (&type, sizeof(type));

      if (type != Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE)
        {
          NS_LOG_ERROR ("RS message MUST have source link layer option.");
          return;
        }

      packet->RemoveHeader (lla);

      /* Update Neighbor Cache */
      Ptr<SixLowPanNdiscCache> sixCache = FindSixLowPanCache (sixDevice);
      SixLowPanNdiscCache::SixLowPanEntry* sixEntry = 0;
      sixEntry = sixCache->SixLowPanLookup (src);
      if (!sixEntry)
        {
          sixEntry = sixCache->SixLowPanAdd (src);
          sixEntry->SetRouter (false);
          sixEntry->MarkStale (lla.GetAddress ());
          sixEntry->MarkTentative ();
          NS_LOG_LOGIC ("Tentative entry created from RS");
        }
      else if (sixEntry->GetMacAddress () != lla.GetAddress ())
        {
          sixEntry->MarkStale (lla.GetAddress ());
        }

      /* Update "ARPv6" table */
      Ptr<NdiscCache> cache = FindCache (sixDevice);
      NdiscCache::Entry* entry = 0;
      entry = cache->Lookup (src);
      if (!entry)
        {
          entry = cache->Add (src);
          entry->SetRouter (false);
          entry->MarkStale (lla.GetAddress ());
        }
      else if (entry->GetMacAddress () != lla.GetAddress ())
        {
          entry->MarkStale (lla.GetAddress ());
        }
    }

  if (!m_border)
    {
      Address addr = lla.GetAddress ();
      Ipv6Address destination = sixDevice->MakeLinkLocalAddressFromMac(addr);

      SendSixLowPanRA (interface->GetLinkLocalAddress ().GetAddress (), destination, interface);
    }
}

void SixLowPanNdProtocol::HandleSixLowPanRA (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                                             Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  m_rsRetransmit = 0;
  m_receivedRA = true;

  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice());
  NS_ASSERT_MSG (sixDevice != NULL, "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  Address addr = sixDevice->GetAddress ();

  Icmpv6RA raHeader;
  Ptr<Ipv6L3Protocol> ipv6 = GetNode ()->GetObject<Ipv6L3Protocol> ();

  Icmpv6OptionAuthoritativeBorderRouter abrHdr; /* ABRO */
  Icmpv6OptionLinkLayerAddress llaHdr (1); /* SLLAO */

  bool next = true;
  Ptr<SixLowPanNdiscCache> sixCache = FindSixLowPanCache (sixDevice);
  SixLowPanNdiscCache::SixLowPanRaEntry* ra = 0;

  Ptr<NdiscCache> cache = FindCache (sixDevice);
  // sgi::hash_map<Ipv6Address, NdiscCache::Entry *, Ipv6AddressHash> ndiscCache = cache->GetNdiscCache ();

  uint32_t version;
  Ipv6Address border;

  packet->RemoveHeader (raHeader);

  Ipv6Address defaultRouter = Ipv6Address::GetZero ();

  if (raHeader.GetLifeTime())
    {
      defaultRouter = src;
    }

  std::list<Icmpv6OptionPrefixInformation> prefixList;
  std::list<Icmpv6OptionSixLowPanContext> contextList;
  Icmpv6OptionPrefixInformation prefixHdr;
  Icmpv6OptionSixLowPanContext contextHdr;

  // \todo check the stop condiiton
  while (next == true)
    {
      uint8_t type = 0;
      packet->CopyData (&type, sizeof(type));

      switch (type)
      {
        case Icmpv6Header::ICMPV6_OPT_PREFIX:
          packet->RemoveHeader (prefixHdr);
          prefixList.push_back (prefixHdr);
          break;
        case Icmpv6Header::ICMPV6_OPT_SIXLOWPAN_CONTEXT:
          packet->RemoveHeader (contextHdr);
          contextList.push_back (contextHdr);
          break;
        case Icmpv6Header::ICMPV6_OPT_AUTHORITATIVE_BORDER_ROUTER:
          packet->RemoveHeader (abrHdr);
          version = abrHdr.GetVersion ();
          border = abrHdr.GetRouterAddress ();
          break;
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE:
          packet->RemoveHeader (llaHdr);
          ReceiveLLA (llaHdr, src, dst, interface); // generates an entry in NDISC table with m_router = true
          break;
        default:
          /* Unknown option, quit */
          next = false;
          break;
      }
    }

  ra = sixCache->RaEntryLookup (border);
  if (!ra) // Create a new entry
    {
      ra = sixCache->AddRaEntry (border);
      ra->SetManagedFlag (raHeader.GetFlagM ());
      ra->SetOtherConfigFlag (raHeader.GetFlagO ());
      ra->SetHomeAgentFlag (raHeader.GetFlagH ());
      ra->SetReachableTime (raHeader.GetReachableTime ());
      ra->SetRouterLifeTime (raHeader.GetLifeTime ());
      ra->SetRetransTimer (raHeader.GetRetransmissionTime ());
      ra->SetCurHopLimit (raHeader.GetCurHopLimit ());
      ra->SetVersion (version);
      ra->SetValidTime (abrHdr.GetValidTime ());

//      for (std::list<Icmpv6OptionPrefixInformation>::iterator it = prefixList.begin (); it != prefixList.end (); it++)
//        {
//          Ptr<SixLowPanPrefix> prefix = new SixLowPanPrefix;
//          prefix->SetPrefixLength ((*it).GetPrefixLength ());
//          prefix->SetFlags ((*it).GetFlags ());
//          prefix->SetValidLifeTime ((*it).GetValidTime ());
//          prefix->SetPreferredLifeTime ((*it).GetPreferredTime ());
//          prefix->SetPrefix ((*it).GetPrefix ());
//
//          ra->AddPrefix (prefix);
//
//          ipv6->AddAutoconfiguredAddress (ipv6->GetInterfaceForDevice (sixDevice),
//                                          (*it).GetPrefix (),(*it).GetPrefixLength (),
//                                          (*it).GetFlags (), (*it).GetValidTime (),
//                                          (*it).GetPreferredTime (), defaultRouter);
//
//          for (sgi::hash_map<Ipv6Address, NdiscCache::Entry *, Ipv6AddressHash>::iterator k = ndiscCache.begin ();
//              k != ndiscCache.end (); k++)
//            {
//              if (k->second->IsRouter ())
//                {
//                  SendSixLowPanARO (sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                    k->second->GetIpv6Address (), m_regTime, Mac64Address::ConvertFrom (addr),
//                                    addr);
//
//                  Simulator::Schedule (Time (Minutes (m_regTime - m_advance)), &SixLowPanNdProtocol::RetransmitARO, this,
//                                       sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                       k->second->GetIpv6Address (), m_regTime, Mac64Address::ConvertFrom (addr),
//                                       addr);
//                }
//            }
//        }
      // \todo quiet compiler
      NS_ASSERT (m_regTime);

      for (std::list<Icmpv6OptionSixLowPanContext>::iterator jt = contextList.begin (); jt != contextList.end (); jt++)
        {
          Ptr<SixLowPanContext> context = new SixLowPanContext;
          context->SetCid ((*jt).GetCid ());
          context->SetFlagC ((*jt).IsFlagC ());
          context->SetValidTime ((*jt).GetValidTime ());
          context->SetContextPrefix ((*jt).GetContextPrefix ());

          ra->AddContext (context);
        }
    }
  else if (ra && version > (ra->GetVersion ())) // Update existing entry from 6LBR with new information
    {
      ra->SetManagedFlag (raHeader.GetFlagM ());
      ra->SetOtherConfigFlag (raHeader.GetFlagO ());
      ra->SetHomeAgentFlag (raHeader.GetFlagH ());
      ra->SetReachableTime (raHeader.GetReachableTime ());
      ra->SetRouterLifeTime (raHeader.GetLifeTime ());
      ra->SetRetransTimer (raHeader.GetRetransmissionTime ());
      ra->SetCurHopLimit (raHeader.GetCurHopLimit ());
      ra->SetVersion (version);
      ra->SetValidTime (abrHdr.GetValidTime ());

      for (std::list<Icmpv6OptionPrefixInformation>::iterator it = prefixList.begin (); it != prefixList.end (); it++)
        {
          if (ra->GetPrefixes ().find ((*it).GetPrefix ()) == ra->GetPrefixes ().end ()) /* prefix NOT found */
            {
              Ptr<SixLowPanPrefix> prefix = new SixLowPanPrefix;
              prefix->SetPrefixLength ((*it).GetPrefixLength ());
              prefix->SetFlags ((*it).GetFlags ());
              prefix->SetValidLifeTime ((*it).GetValidTime ());
              prefix->SetPreferredLifeTime ((*it).GetPreferredTime ());
              prefix->SetPrefix ((*it).GetPrefix ());

              ra->AddPrefix (prefix);

              ipv6->AddAutoconfiguredAddress (ipv6->GetInterfaceForDevice (sixDevice),
                                              (*it).GetPrefix (), (*it).GetPrefixLength (),
                                              (*it).GetFlags (), (*it).GetValidTime (),
                                              (*it).GetPreferredTime (), defaultRouter);
//              for (sgi::hash_map<Ipv6Address, NdiscCache::Entry *, Ipv6AddressHash>::iterator kt = ndiscCache.begin ();
//                  kt != ndiscCache.end (); kt++)
//                {
//                  if (kt->second->IsRouter ())
//                    {
//                      SendSixLowPanARO (sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                        kt->second->GetIpv6Address (), m_regTime,
//                                        Mac64Address::ConvertFrom (addr), addr);
//
//                      Simulator::Schedule (Time (Minutes (m_regTime - m_advance)), &SixLowPanNdProtocol::RetransmitARO, this,
//                                           sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                           kt->second->GetIpv6Address (), m_regTime, Mac64Address::ConvertFrom (addr),
//                                           addr);
//                    }
//                }
            }
          else /* prefix found, updating! */
            {
              Ptr<SixLowPanPrefix> prefix = (ra->GetPrefixes().find((*it).GetPrefix ()))->second;

              prefix->SetPrefixLength ((*it).GetPrefixLength ());
              prefix->SetFlags ((*it).GetFlags ());
              prefix->SetValidLifeTime ((*it).GetValidTime ());
              prefix->SetPreferredLifeTime ((*it).GetPreferredTime ());

              ipv6->AddAutoconfiguredAddress (ipv6->GetInterfaceForDevice (sixDevice),
                                              (*it).GetPrefix (), (*it).GetPrefixLength (),
                                              (*it).GetFlags (), (*it).GetValidTime (),
                                              (*it).GetPreferredTime (), defaultRouter);
//              for (sgi::hash_map<Ipv6Address, NdiscCache::Entry *, Ipv6AddressHash>::iterator lt = ndiscCache.begin ();
//                  lt != ndiscCache.end (); lt++)
//                {
//                  if (lt->second->IsRouter ())
//                    {
//                      SendSixLowPanARO (sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                        lt->second->GetIpv6Address (), m_regTime, Mac64Address::ConvertFrom (addr),
//                                        addr);
//
//                      Simulator::Schedule (Time (Minutes (m_regTime - m_advance)), &SixLowPanNdProtocol::RetransmitARO, this,
//                                           sixDevice->MakeGlobalAddressFromMac(addr, (*it).GetPrefix ()),
//                                           lt->second->GetIpv6Address (), m_regTime, Mac64Address::ConvertFrom (addr),
//                                           addr);
//                    }
//                }
            }
        }

      for (std::list<Icmpv6OptionSixLowPanContext>::iterator jt = contextList.begin (); jt != contextList.end (); jt++)
        {
          if (ra->GetContexts ().find ((*jt).GetCid ()) == ra->GetContexts ().end ()) /* context NOT found */
            {
              Ptr<SixLowPanContext> context = new SixLowPanContext;
              context->SetCid ((*jt).GetCid ());
              context->SetFlagC ((*jt).IsFlagC ());
              context->SetValidTime ((*jt).GetValidTime ());
              context->SetContextPrefix ((*jt).GetContextPrefix ());

              ra->AddContext (context);
            }
          else
            {
              Ptr<SixLowPanContext> context = (ra->GetContexts ().find ((*jt).GetCid ()))->second;

              context->SetFlagC ((*jt).IsFlagC ());
              context->SetValidTime ((*jt).GetValidTime ());
              context->SetContextPrefix ((*jt).GetContextPrefix ());
            }
        }
    }
  else  // Old information, not updating
    {
      return;
    }

  uint32_t t = raHeader.GetLifeTime ();

  for (std::list<Icmpv6OptionPrefixInformation>::iterator it = prefixList.begin (); it != prefixList.end (); it++)
    {
      t = t < ((*it).GetValidTime ()) ? t : ((*it).GetValidTime ());
    }
  for (std::list<Icmpv6OptionSixLowPanContext>::iterator jt = contextList.begin (); jt != contextList.end (); jt++)
    {
      t = (60 * ((*jt).GetValidTime ())) < t ? (60 * ((*jt).GetValidTime ())) : t;
    }

  t -= (60 * m_advance);

  Simulator::Schedule (Time (Seconds (t - 1)), &SixLowPanNdProtocol::SetReceivedRA, this, false);
  Simulator::Schedule (Time (Seconds (t)), &SixLowPanNdProtocol::RetransmitRS, this,
                       interface->GetLinkLocalAddress ().GetAddress (), src, addr);
}
/// \todo da finire!! (controlla codice, controlla regTime)

void SixLowPanNdProtocol::HandleSixLowPanDAC (Ptr<Packet> packet, Ipv6Address const &src, Ipv6Address const &dst,
                                              Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice());
  NS_ASSERT_MSG (sixDevice != NULL, "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  Icmpv6DuplicateAddress dacHdr (0);
  packet->RemoveHeader (dacHdr);

  if (m_multihopDad)
    {
      Ipv6Address reg = dacHdr.GetRegAddress ();

      if (!reg.IsMulticast () && src != Ipv6Address::GetAny () && !src.IsMulticast ())
        {
          Ptr<SixLowPanNdiscCache> cache = FindSixLowPanCache (sixDevice);
          SixLowPanNdiscCache::SixLowPanEntry* entry = 0;
          entry = cache->SixLowPanLookup (reg);

          if (dacHdr.GetStatus () == 0) /* mark the entry as registered, send ARO with status=0 */
            {
              entry->MarkRegistered (dacHdr.GetRegTime ());

              SendSixLowPanARO (dst, dacHdr.GetRegAddress (), dacHdr.GetStatus (), dacHdr.GetRegTime (),
                                dacHdr.GetEui64 ());
            }
          else /* remove the tentative entry, send ARO with error code */
            {
              cache->SixLowPanRemove (entry);

              Ipv6Address address = sixDevice->MakeLinkLocalAddressFromMac (dacHdr.GetEui64 ());

              SendSixLowPanARO (dst, address, dacHdr.GetStatus (), dacHdr.GetRegTime (), dacHdr.GetEui64 ());
            }
        }
      else
        {
          NS_LOG_ERROR ("Validity checks for DAR not satisfied.");
          return;
        }
    }
}

Ptr<SixLowPanNdiscCache> SixLowPanNdProtocol::FindSixLowPanCache (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);

  for (SixLowPanCacheList::const_iterator it = m_sixlowCList.begin (); it != m_sixlowCList.end (); it++)
    {
      if ((*it)->GetDevice () == device)
        {
          return *it;
        }
    }

  /* quiet compiler */
  return 0;
}

void SixLowPanNdProtocol::RetransmitARO (Ipv6Address src, Ipv6Address dst, uint16_t time,
                                         Mac64Address eui, Address linkAddr)
{
  NS_LOG_FUNCTION (this << src << dst << time << eui << linkAddr);

  IntegerValue maxUnicastSolicit;
  GetAttribute ("MaxUnicastSolicit", maxUnicastSolicit);
  if (m_aroRetransmit < maxUnicastSolicit.Get ())
    {
      m_aroRetransmit++;

      SendSixLowPanARO (src, dst, time, eui, linkAddr);

      TimeValue retransmissionTime;
      GetAttribute ("RetransmissionTime", retransmissionTime);

      Simulator::Schedule (retransmissionTime.Get (), &SixLowPanNdProtocol::RetransmitARO, this,
                           src, dst, time, eui, linkAddr);
      return;
    }
  else
    {
      return;
    }
}

void SixLowPanNdProtocol::RetransmitRS (Ipv6Address src, Ipv6Address dst, Address linkAddr)
{
  NS_LOG_FUNCTION (this << src << dst << linkAddr);

  /* if the source is NOT unspec, send a RS message + SLLA option */
  if (src != Ipv6Address::GetAny ())
    {
      if (!m_receivedRA && m_rsRetransmit < MAX_RTR_SOLICITATIONS)
        {
          m_rsRetransmit++;

          SendRS (src, dst, linkAddr);

          Simulator::Schedule (Time (Seconds (RTR_SOLICITATION_INTERVAL)), &SixLowPanNdProtocol::RetransmitRS, this,
                               src, dst, linkAddr);
          return;
        }
      else if (!m_receivedRA)
        {
          m_rsRetransmit++;

          Ipv6Address destination = Ipv6Address::GetAllRoutersMulticast ();

          SendRS (src, destination, linkAddr);

          Simulator::Schedule (Time (Seconds (MAX_RTR_SOLICITATION_INTERVAL)), &SixLowPanNdProtocol::RetransmitRS, this,
                               src, destination, linkAddr);
          /* inserire truncated binary exponential backoff */
          return;
        }
      else
        {
          return;
        }
    }
  else
    {
      NS_LOG_ERROR ("An unspecified source address MUST NOT be used in RS messages.");
      return;
    }
}
/// \todo da finire!! (truncated binary exponential backoff)

void SixLowPanNdProtocol::SetReceivedRA (bool received)
{
  NS_LOG_FUNCTION (this << received);
  m_receivedRA = received;
}

} /* namespace ns3 */
