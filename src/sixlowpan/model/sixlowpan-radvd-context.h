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

#ifndef SIXLOWPAN_RADVD_CONTEXT_H
#define SIXLOWPAN_RADVD_CONTEXT_H

#include <stdint.h>

#include "ns3/ipv6-address.h"
#include "ns3/simple-ref-count.h"

namespace ns3
{

/**
 * \ingroup sixlowpanradvd
 * \class SixLowPanRadvdContext
 * \brief 6LoWPAN context for SixLowPanRadvd application.
 */
class SixLowPanRadvdContext : public SimpleRefCount<SixLowPanRadvdContext>
{
public:
  /**
   * \brief Constructor.
   * \param flagC compression flag
   * \param cid context identifier ( 0 <= x <= 15)
   * \param time valid lifetime of context (units of 60 seconds)
   * \param context 6LoWPAN context advertised
   */
  SixLowPanRadvdContext (bool flagC, uint8_t cid, uint16_t time, Ipv6Prefix context);

  /**
   * \brief Destructor.
   */
  ~SixLowPanRadvdContext ();
  
  /**
   * \brief Get the context length.
   * \return context length value
   */
  uint8_t GetContextLen () const;

  /**
   * \brief Set the context length.
   * \param length the context length value
   */
  void SetContextLen (uint8_t length);

  /**
   * \brief Is compression flag ?
   * \return true if context is valid for use in compression, false otherwise
   */
  bool IsFlagC () const;

  /**
   * \brief Set the compression flag.
   * \param c the compression flag
   */
  void SetFlagC (bool c);

  /**
   * \brief Get the context identifier.
   * \return context identifier value
   */
  uint8_t GetCid () const;

  /**
   * \brief Set the context identifier.
   * \param cid the context identifier value
   */
  void SetCid (uint8_t cid);

  /**
   * \brief Get the valid lifetime.
   * \return valid lifetime value (units of 60 seconds)
   */
  uint16_t GetValidTime () const;

  /**
   * \brief Set the valid lifetime.
   * \param time the valid lifetime value (units of 60 seconds)
   */ 
  void SetValidTime (uint16_t time);

  /**
   * \brief Get the 6LoWPAN context prefix.
   * \return context prefix value
   */
  Ipv6Prefix GetContextPrefix () const;

  /**
   * \brief Set the 6LoWPAN context prefix.
   * \param prefix the context prefix value
   */
  void SetContextPrefix (Ipv6Prefix context);

private:
  /**
   * \brief The context length value.
   */
  uint8_t m_length;

  /**
   * \brief The compression flag, indicates that this context is valid for use in compression.
   */
  bool m_c;

  /**
   * \brief The context identifier value.
   */
  uint8_t m_cid;

  /**
   * \brief The valid lifetime value (units of 60 seconds).
   */
  uint16_t m_validTime;

  /**
   * \brief The context prefix value.
   */
  Ipv6Prefix m_context;
};

} /* namespace ns3 */

#endif /* SIXLOW_RADVD_CONTEXT_H */
