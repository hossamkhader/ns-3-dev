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

#include "sixlowpan-radvd-interface.h"

namespace ns3 
{

NS_LOG_COMPONENT_DEFINE ("SixLowPanRadvdInterface");

SixLowPanRadvdInterface::SixLowPanRadvdInterface (uint32_t interface)
  : RadvdInterface (interface)
{
  NS_LOG_FUNCTION (this << interface);
  
  SetSendAdvert (false);
  
  m_version = 0;
  m_validTime = 10000;
}

SixLowPanRadvdInterface::SixLowPanRadvdInterface (uint32_t interface, uint32_t maxRtrAdvInterval, uint32_t minRtrAdvInterval)
  : RadvdInterface (interface, maxRtrAdvInterval, minRtrAdvInterval)
{
  NS_LOG_FUNCTION (this << interface << maxRtrAdvInterval << minRtrAdvInterval);
  
  SetSendAdvert (false);
  
  m_version = 0;
  m_validTime = 10000;
}

SixLowPanRadvdInterface::~SixLowPanRadvdInterface ()
{
  NS_LOG_FUNCTION (this);
  /* clear prefixes */
  for (SixLowPanRadvdContextListI it = m_contexts.begin (); it != m_contexts.end (); ++it)
    {
      (*it) = 0;
    }
  m_contexts.clear ();
}

void SixLowPanRadvdInterface::AddContext (Ptr<SixLowPanRadvdContext> context)
{
  NS_LOG_FUNCTION (this << context);
  m_contexts.push_back (context);
}

std::list<Ptr<SixLowPanRadvdContext> > SixLowPanRadvdInterface::GetContexts () const
{
  NS_LOG_FUNCTION (this);
  return m_contexts;
}

void SixLowPanRadvdInterface::AddDadEntry (Ptr<SixLowPanRadvdDadEntry> entry)
{
  NS_LOG_FUNCTION (this << entry);
  m_dadTable.push_back (entry);
}

std::list<Ptr<SixLowPanRadvdDadEntry> > SixLowPanRadvdInterface::GetDadTable () const
{
  NS_LOG_FUNCTION (this);
  return m_dadTable;
}

uint32_t SixLowPanRadvdInterface::GetVersion () const
{
  NS_LOG_FUNCTION (this);
  return m_version;
}

void SixLowPanRadvdInterface::SetVersion (uint32_t version)
{
  NS_LOG_FUNCTION (this << version);
  m_version = version;
}

uint16_t SixLowPanRadvdInterface::GetValidTime () const
{
  NS_LOG_FUNCTION (this);
  return m_validTime;
}

void SixLowPanRadvdInterface::SetValidTime (uint16_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_validTime = time;
}

} /* namespace ns3 */
