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

#include "sixlowpan-radvd-dad-entry.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("SixLowPanRadvdDadEntry");

SixLowPanRadvdDadEntry::SixLowPanRadvdDadEntry ()
  : m_regTime (0),
    m_eui64 (Mac64Address ("00:00:00:00:00:00")),
    m_regAddress (Ipv6Address ("::"))
{
  NS_LOG_FUNCTION (this);
}

SixLowPanRadvdDadEntry::SixLowPanRadvdDadEntry (uint16_t regTime, Mac64Address eui64, Ipv6Address regAddress)
  : m_regTime (regTime),
    m_eui64 (eui64),
    m_regAddress (regAddress)
{
  NS_LOG_FUNCTION (this << regTime << eui64 << regAddress);
}

SixLowPanRadvdDadEntry::~SixLowPanRadvdDadEntry ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t SixLowPanRadvdDadEntry::GetRegTime () const
{
  NS_LOG_FUNCTION (this);
  return m_regTime;
}

void SixLowPanRadvdDadEntry::SetRegTime (uint16_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_regTime = time;
}

Mac64Address SixLowPanRadvdDadEntry::GetEui64 () const
{
  NS_LOG_FUNCTION (this);
  return m_eui64;
}

void SixLowPanRadvdDadEntry::SetEui64 (Mac64Address eui)
{
  NS_LOG_FUNCTION (this << eui);
  m_eui64 = eui;
}

Ipv6Address SixLowPanRadvdDadEntry::GetRegAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_regAddress;
}

void SixLowPanRadvdDadEntry::SetRegAddress (Ipv6Address registered)
{
  NS_LOG_FUNCTION (this << registered);
  m_regAddress = registered;
}

} /* namespace ns3 */
