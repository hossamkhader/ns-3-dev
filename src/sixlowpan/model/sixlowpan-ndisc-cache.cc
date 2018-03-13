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
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"

#include "sixlowpan-ndisc-cache.h"
#include "sixlowpan-nd-protocol.h"
#include "sixlowpan-ndisc-ra-options.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("SixLowPanNdiscCache");

NS_OBJECT_ENSURE_REGISTERED (SixLowPanNdiscCache);

TypeId SixLowPanNdiscCache::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SixLowPanNdiscCache")
    .SetParent<NdiscCache> ()
    .SetGroupName ("SixLowPan")
  ;
  return tid;
}

SixLowPanNdiscCache::SixLowPanNdiscCache ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

SixLowPanNdiscCache::~SixLowPanNdiscCache ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
}

void SixLowPanNdiscCache::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();

  Flush ();
  FlushRaCache ();
  NdiscCache::DoDispose();
}

SixLowPanNdiscCache::SixLowPanEntry* SixLowPanNdiscCache::SixLowPanLookup (Ipv6Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  SixLowPanNdiscCache::SixLowPanEntry* entry;

  if (m_sixlowNdCache.find (dst) != m_sixlowNdCache.end ())
    {
      entry = m_sixlowNdCache[dst];
    }
  else
    {
      entry = 0;
    }
  return entry;
}

SixLowPanNdiscCache::SixLowPanEntry* SixLowPanNdiscCache::SixLowPanAdd (Ipv6Address to)
{
  NS_LOG_FUNCTION (this << to);
  NS_ASSERT (m_sixlowNdCache.find (to) == m_sixlowNdCache.end ());

  SixLowPanNdiscCache::SixLowPanEntry* entry = new SixLowPanNdiscCache::SixLowPanEntry (this);
  entry->SetIpv6Address (to);
  m_sixlowNdCache[to] = entry;
  return entry;
}

void SixLowPanNdiscCache::SixLowPanRemove (SixLowPanNdiscCache::SixLowPanEntry* entry)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (SixLowPanCacheI i = m_sixlowNdCache.begin (); i != m_sixlowNdCache.end (); i++)
    {
      if ((*i).second == entry)
        {
          m_sixlowNdCache.erase (i);
          entry->ClearWaitingPacket ();
          delete entry;
          return;
        }
    }
}

void SixLowPanNdiscCache::Flush ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (SixLowPanCacheI i = m_sixlowNdCache.begin (); i != m_sixlowNdCache.end (); i++)
    {
      delete (*i).second;
    }

  m_sixlowNdCache.erase (m_sixlowNdCache.begin (), m_sixlowNdCache.end ());
}

void SixLowPanNdiscCache::PrintNdiscCache (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  for (SixLowPanCacheI i = m_sixlowNdCache.begin (); i != m_sixlowNdCache.end (); i++)
    {
      *os << i->first << " dev ";
      std::string found = Names::FindName (GetDevice ());
      if (Names::FindName (GetDevice ()) != "")
        {
          *os << found;
        }
      else
        {
          *os << static_cast<int> (GetDevice ()->GetIfIndex ());
        }

      *os << " lladdr " << i->second->GetMacAddress ();

      if (i->second->IsReachable ())
        {
          *os << " REACHABLE ";
        }
      else if (i->second->IsDelay ())
        {
          *os << " DELAY ";
        }
      else if (i->second->IsIncomplete ())
        {
          *os << " INCOMPLETE ";
        }
      else if (i->second->IsProbe ())
        {
          *os << " PROBE ";
        }
      else
        {
          *os << " STALE ";
        }

      if (i->second->IsRegistered ())
        {
          *os << " REGISTERED\n";
        }
      else if (i->second->IsTentative ())
        {
          *os << " TENTATIVE\n";
        }
      else
        {
          *os << " GARBAGE\n";
        }
    }
}

SixLowPanNdiscCache::SixLowPanEntry::SixLowPanEntry (NdiscCache* nd)
  : NdiscCache::Entry::Entry (nd),
    m_registeredTimer (Timer::CANCEL_ON_DESTROY),
	  m_tentativeTimer (Timer::CANCEL_ON_DESTROY)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void SixLowPanNdiscCache::SixLowPanEntry::MarkRegistered (uint16_t time)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_type = REGISTERED;

  if (m_registeredTimer.IsRunning ())
    {
      m_registeredTimer.Cancel ();
    }
  m_registeredTimer.SetFunction (&SixLowPanNdiscCache::SixLowPanEntry::FunctionTimeout, this);
  m_registeredTimer.SetDelay (Minutes (time));
  m_registeredTimer.Schedule ();
}

void SixLowPanNdiscCache::SixLowPanEntry::MarkTentative ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_type = TENTATIVE;

  if (m_tentativeTimer.IsRunning ())
    {
      m_tentativeTimer.Cancel ();
    }
  m_tentativeTimer.SetFunction (&SixLowPanNdiscCache::SixLowPanEntry::FunctionTimeout, this);
  m_tentativeTimer.SetDelay (Seconds (SixLowPanNdProtocol::TENTATIVE_NCE_LIFETIME));
  m_tentativeTimer.Schedule ();
}

void SixLowPanNdiscCache::SixLowPanEntry::MarkGarbage ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_type = GARBAGE;
}

bool SixLowPanNdiscCache::SixLowPanEntry::IsRegistered () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return (m_type == REGISTERED);
}

bool SixLowPanNdiscCache::SixLowPanEntry::IsTentative () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return (m_type == TENTATIVE);
}

bool SixLowPanNdiscCache::SixLowPanEntry::IsGarbage () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return (m_type == GARBAGE);
}

void SixLowPanNdiscCache::SixLowPanEntry::FunctionTimeout ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_ndCache->Remove (this);
  return;
}

Mac64Address SixLowPanNdiscCache::SixLowPanEntry::GetEui64 () const
{
  NS_LOG_FUNCTION (this);
  return m_eui64;
}

void SixLowPanNdiscCache::SixLowPanEntry::SetEui64 (Mac64Address eui)
{
  NS_LOG_FUNCTION (this << eui);
  m_eui64 = eui;
}

SixLowPanNdiscCache::SixLowPanRaEntry* SixLowPanNdiscCache::RaEntryLookup (Ipv6Address border)
{
  NS_LOG_FUNCTION (this << border);
  SixLowPanNdiscCache::SixLowPanRaEntry* entry = 0;

  if (m_raCache.find (border) != m_raCache.end ())
    {
      entry = m_raCache[border];
    }
  return entry;
}

SixLowPanNdiscCache::SixLowPanRaEntry* SixLowPanNdiscCache::AddRaEntry (Ipv6Address border)
{
  NS_LOG_FUNCTION (this << border);
  NS_ASSERT (m_raCache.find (border) == m_raCache.end ());

  SixLowPanNdiscCache::SixLowPanRaEntry* entry = new SixLowPanNdiscCache::SixLowPanRaEntry (this);
  entry->SetBorderAddress (border);
  m_raCache[border] = entry;
  return entry;
}

void SixLowPanNdiscCache::RemoveRaEntry (SixLowPanNdiscCache::SixLowPanRaEntry* entry)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (SixLowPanRaCacheI it = m_raCache.begin (); it != m_raCache.end (); it++)
    {
      if ((*it).second == entry)
        {
          m_raCache.erase (it);
          delete entry;
          return;
        }
    }
}

void SixLowPanNdiscCache::FlushRaCache ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (SixLowPanRaCacheI it = m_raCache.begin (); it != m_raCache.end (); it++)
    {
      delete (*it).second;
    }

  m_raCache.erase (m_raCache.begin (), m_raCache.end ());
}

void SixLowPanNdiscCache::PrintRaCache (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  for (SixLowPanRaCacheI it = m_raCache.begin (); it != m_raCache.end (); it++)
    {
      *os << it->first << " RA ";

      *os << " Cur Hop Limit: " << it->second->GetCurHopLimit ();

      if (it->second->IsManagedFlag ())
        {
          *os << " M flag: true ";
        }
      else
        {
          *os << " M flag: false ";
        }
      if (it->second->IsOtherConfigFlag ())
        {
          *os << " O flag: true ";
        }
      else
        {
          *os << " O flag: false ";
        }

      *os << " Router Lifetime: " << it->second->GetRouterLifeTime ();
      *os << " Reachable Time: " << it->second->GetReachableTime ();
      *os << " Retrans Timer: " << it->second->GetRetransTimer ();
      *os << " (ABRO) Version: " << it->second->GetVersion ();
      *os << " (ABRO) Valid Lifetime: " << it->second->GetValidTime ();
      *os << " (ABRO) Border Router address: " << it->second->GetBorderAddress ();

      std::map<Ipv6Address, Ptr<SixLowPanPrefix> > prefixes = it->second->GetPrefixes ();
      for (std::map<Ipv6Address, Ptr<SixLowPanPrefix> >::iterator jt = prefixes.begin (); jt != prefixes.end (); jt++)
        {
          jt->second->PrintPrefix (stream);
        }
      std::map<uint8_t, Ptr<SixLowPanContext> > contexts = it->second->GetContexts ();
      for (std::map<uint8_t, Ptr<SixLowPanContext> >::iterator i = contexts.begin (); i != contexts.end (); i++)
        {
          i->second->PrintContext (stream);
        }
    }
}

std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *> SixLowPanNdiscCache::GetRaCache () const
{
  NS_LOG_FUNCTION (this);
  return m_raCache;
}

SixLowPanNdiscCache::SixLowPanRaEntry::SixLowPanRaEntry (SixLowPanNdiscCache* nd)
  //: m_cache (nd)
{
  NS_LOG_FUNCTION (this);
}

SixLowPanNdiscCache::SixLowPanRaEntry::~SixLowPanRaEntry ()
{
  NS_LOG_FUNCTION (this);
}

void SixLowPanNdiscCache::SixLowPanRaEntry::AddPrefix (Ptr<SixLowPanPrefix> prefix)
{
  NS_LOG_FUNCTION (this << prefix);
  m_prefixes.insert (std::pair<Ipv6Address, Ptr<SixLowPanPrefix> > (prefix->GetPrefix (), prefix));
}

void SixLowPanNdiscCache::SixLowPanRaEntry::RemovePrefix (Ptr<SixLowPanPrefix> prefix)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (std::map<Ipv6Address, Ptr<SixLowPanPrefix> >::iterator it = m_prefixes.begin (); it != m_prefixes.end (); it++)
    {
      if (it->second == prefix)
        {
          m_prefixes.erase (it);
          return;
        }
    }
}

std::map<Ipv6Address, Ptr<SixLowPanPrefix> > SixLowPanNdiscCache::SixLowPanRaEntry::GetPrefixes () const
{
  NS_LOG_FUNCTION (this);
  return m_prefixes;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::AddContext (Ptr<SixLowPanContext> context)
{
  NS_LOG_FUNCTION (this << context);
  m_contexts.insert (std::pair<uint8_t, Ptr<SixLowPanContext> > (context->GetCid (), context));
}

void SixLowPanNdiscCache::SixLowPanRaEntry::RemoveContext (Ptr<SixLowPanContext> context)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (std::map<uint8_t, Ptr<SixLowPanContext> >::iterator it = m_contexts.begin (); it != m_contexts.end (); it++)
    {
      if (it->second == context)
        {
          m_contexts.erase (it);
          return;
        }
    }
}

std::map<uint8_t, Ptr<SixLowPanContext> > SixLowPanNdiscCache::SixLowPanRaEntry::GetContexts () const
{
  NS_LOG_FUNCTION (this);
  return m_contexts;
}

bool SixLowPanNdiscCache::SixLowPanRaEntry::IsManagedFlag () const
{
  NS_LOG_FUNCTION (this);
  return m_managedFlag;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetManagedFlag (bool managedFlag)
{
  NS_LOG_FUNCTION (this << managedFlag);
  m_managedFlag = managedFlag;
}

bool SixLowPanNdiscCache::SixLowPanRaEntry::IsOtherConfigFlag () const
{
  NS_LOG_FUNCTION (this);
  return m_otherConfigFlag;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetOtherConfigFlag (bool otherConfigFlag)
{
  NS_LOG_FUNCTION (this << otherConfigFlag);
  m_otherConfigFlag = otherConfigFlag;
}

bool SixLowPanNdiscCache::SixLowPanRaEntry::IsHomeAgentFlag () const
{
  NS_LOG_FUNCTION (this);
  return m_homeAgentFlag;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetHomeAgentFlag (bool homeAgentFlag)
{
  NS_LOG_FUNCTION (this << homeAgentFlag);
  m_homeAgentFlag = homeAgentFlag;
}

uint32_t SixLowPanNdiscCache::SixLowPanRaEntry::GetReachableTime () const
{
  NS_LOG_FUNCTION (this);
  return m_reachableTime;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetReachableTime (uint32_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_reachableTime = time;
}

uint32_t SixLowPanNdiscCache::SixLowPanRaEntry::GetRouterLifeTime () const
{
  NS_LOG_FUNCTION (this);
  return m_routerLifeTime;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetRouterLifeTime (uint32_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_routerLifeTime = time;
}

uint32_t SixLowPanNdiscCache::SixLowPanRaEntry::GetRetransTimer () const
{
  NS_LOG_FUNCTION (this);
  return m_retransTimer;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetRetransTimer (uint32_t timer)
{
  NS_LOG_FUNCTION (this << timer);
  m_retransTimer = timer;
}

uint8_t SixLowPanNdiscCache::SixLowPanRaEntry::GetCurHopLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_curHopLimit;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetCurHopLimit (uint8_t curHopLimit)
{
  NS_LOG_FUNCTION (this << curHopLimit);
  m_curHopLimit = curHopLimit;
}

uint32_t SixLowPanNdiscCache::SixLowPanRaEntry::GetVersion () const
{
  NS_LOG_FUNCTION (this);
  return m_version;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetVersion (uint32_t version)
{
  NS_LOG_FUNCTION (this << version);
  m_version = version;
}

uint16_t SixLowPanNdiscCache::SixLowPanRaEntry::GetValidTime () const
{
  NS_LOG_FUNCTION (this);
  return m_validTime;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetValidTime (uint16_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_validTime = time;
}

Ipv6Address SixLowPanNdiscCache::SixLowPanRaEntry::GetBorderAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_border;
}

void SixLowPanNdiscCache::SixLowPanRaEntry::SetBorderAddress (Ipv6Address border)
{
  NS_LOG_FUNCTION (this << border);
  m_border = border;
}

} /* namespace ns3 */
