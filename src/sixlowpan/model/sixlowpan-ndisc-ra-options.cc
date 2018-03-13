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

#include <ns3/log.h>
#include "ns3/simulator.h"

#include "sixlowpan-ndisc-ra-options.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("SixLowPanNdiscRaOptions");

SixLowPanPrefix::SixLowPanPrefix ()
{
  NS_LOG_FUNCTION (this);
}

SixLowPanPrefix::SixLowPanPrefix (Ipv6Address prefix, uint8_t prefixLen, uint32_t prefTime, uint32_t validTime, uint8_t flags)
  : m_prefix (prefix),
    m_prefixLength (prefixLen),
    m_preferredLifeTime (prefTime),
    m_validLifeTime (validTime),
    m_flags (flags)
{
  NS_LOG_FUNCTION (this << prefix << prefixLen << prefTime << validTime << flags);

  m_setValidTime = Simulator::Now ();
  m_setPrefTime = Simulator::Now ();
}

SixLowPanPrefix::~SixLowPanPrefix ()
{
  NS_LOG_FUNCTION (this);
}

Ipv6Address SixLowPanPrefix::GetPrefix () const
{
  NS_LOG_FUNCTION (this);
  return m_prefix;
}

void SixLowPanPrefix::SetPrefix (Ipv6Address prefix)
{
  NS_LOG_FUNCTION (this << prefix);
  m_prefix = prefix;
}

uint8_t SixLowPanPrefix::GetPrefixLength () const
{
  NS_LOG_FUNCTION (this);
  return m_prefixLength;
}

void SixLowPanPrefix::SetPrefixLength (uint8_t prefixLen)
{
  NS_LOG_FUNCTION (this << prefixLen);
  m_prefixLength = prefixLen;
}

uint32_t SixLowPanPrefix::GetValidLifeTime () const
{
  NS_LOG_FUNCTION (this);
  double time = Simulator::Now ().GetSeconds () - m_setValidTime.GetSeconds ();

  return m_validLifeTime - static_cast<uint32_t> (time);
}

void SixLowPanPrefix::SetValidLifeTime (uint32_t validTime)
{
  NS_LOG_FUNCTION (this << validTime);
  m_validLifeTime = validTime;

  m_setValidTime = Simulator::Now ();
}

uint32_t SixLowPanPrefix::GetPreferredLifeTime () const
{
  NS_LOG_FUNCTION (this);
  double time = Simulator::Now ().GetSeconds () - m_setPrefTime.GetSeconds ();

  return m_preferredLifeTime - static_cast<uint32_t> (time);
}

void SixLowPanPrefix::SetPreferredLifeTime (uint32_t prefTime)
{
  NS_LOG_FUNCTION (this << prefTime);
  m_preferredLifeTime = prefTime;

  m_setPrefTime = Simulator::Now ();
}

uint8_t SixLowPanPrefix::GetFlags () const
{
  NS_LOG_FUNCTION (this);
  return m_flags;
}

void SixLowPanPrefix::SetFlags (uint8_t flags)
{
  NS_LOG_FUNCTION (this << flags);
  m_flags = flags;
}

void SixLowPanPrefix::PrintPrefix (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  *os << " Prefix Length: " << GetPrefixLength ();

  if (GetFlags () & (1 << 7))
    {
      *os << " On-link flag: true ";
    }
  else
    {
      *os << " On-link flag: false ";
    }
  if (GetFlags () & (1 << 6))
    {
      *os << " Autonomous flag: true ";
    }
      else
    {
      *os << " Autonomous flag: false ";
    }
  if (GetFlags () & (1 << 5))
    {
      *os << " Router address flag: true ";
    }
  else
    {
      *os << " Router address flag: false ";
    }

  *os << " Valid Lifetime: " << GetValidLifeTime ();
  *os << " Preferred Lifetime: " << GetPreferredLifeTime ();
  *os << " Prefix: " << GetPrefix ();
}

SixLowPanContext::SixLowPanContext ()
{
  NS_LOG_FUNCTION (this);
}

SixLowPanContext::SixLowPanContext (bool flagC, uint8_t cid, uint16_t time, Ipv6Prefix context)
  : m_c (flagC),
    m_cid (cid),
    m_validTime (time),
    m_context (context)
{
  NS_LOG_FUNCTION (this << flagC << static_cast<uint32_t> (cid) << time << context);

  m_setTime = Simulator::Now ();
}

SixLowPanContext::~SixLowPanContext ()
{
  NS_LOG_FUNCTION (this);
}

uint8_t SixLowPanContext::GetContextLen () const
{
  NS_LOG_FUNCTION (this);
  return m_length;
}

void SixLowPanContext::SetContextLen (uint8_t length)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (length));
  m_length = length;
}

bool SixLowPanContext::IsFlagC () const
{
  NS_LOG_FUNCTION (this);
  return m_c;
}

void SixLowPanContext::SetFlagC (bool c)
{
  NS_LOG_FUNCTION (this << c);
  m_c = c;
}

uint8_t SixLowPanContext::GetCid () const
{
  NS_LOG_FUNCTION (this);
  return m_cid;
}

void SixLowPanContext::SetCid (uint8_t cid)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (cid));
  NS_ASSERT (cid <= 15);
  m_cid = cid;
}

uint16_t SixLowPanContext::GetValidTime () const
{
  NS_LOG_FUNCTION (this);

  double time = Simulator::Now ().GetMinutes () - m_setTime.GetMinutes ();

  return m_validTime - static_cast<uint16_t> (time);
}

void SixLowPanContext::SetValidTime (uint16_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_validTime = time;

  m_setTime = Simulator::Now ();

  Simulator::Schedule (Time (Minutes (time)), &SixLowPanContext::ValidTimeout, this);
}

Ipv6Prefix SixLowPanContext::GetContextPrefix () const
{
  NS_LOG_FUNCTION (this);
  return m_context;
}

void SixLowPanContext::SetContextPrefix (Ipv6Prefix context)
{
  NS_LOG_FUNCTION (this << context);
  m_context = context;
}

void SixLowPanContext::PrintContext (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  *os << " Context Length: " << GetContextLen ();

  if (IsFlagC ())
    {
      *os << " Compression flag: true ";
    }
  else
    {
      *os << " Compression flag: false ";
    }

  *os << " Context Identifier: " << GetCid ();
  *os << " Valid Lifetime: " << GetValidTime ();
  *os << " Context Prefix: " << GetContextPrefix ();
}

void SixLowPanContext::ValidTimeout ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_c = false;

  Simulator::Schedule (Time (Seconds (2 * 1000 /*m_entry->GetRouterLifeTime ()*/)), &SixLowPanContext::RouterTimeout, this);
} /// \todo da finire!!!

void SixLowPanContext::RouterTimeout ()
{
  NS_LOG_FUNCTION_NOARGS ();

  //m_entry->RemoveContext (this);
  return;
} /// \todo da finire!!!

} /* namespace ns3 */
