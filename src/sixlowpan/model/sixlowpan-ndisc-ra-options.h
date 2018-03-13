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

#ifndef SIXLOW_NDISC_RA_OPTIONS_H
#define SIXLOW_NDISC_RA_OPTIONS_H

#include <stdint.h>

#include "ns3/nstime.h"
#include "ns3/ipv6-address.h"
#include "ns3/simple-ref-count.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3
{

/**
 * \ingroup sixlowpan
 * \class SixLowPanPrefix
 * \brief Router prefix container for 6LoWPAN ND.
 */
class SixLowPanPrefix : public SimpleRefCount<SixLowPanPrefix>
{
public:
  /**
   * \brief Constructor.
   */
  SixLowPanPrefix ();

  /**
   * \brief Constructor.
   * \param prefix network prefix advertised
   * \param prefixLen prefix length ( 0 < x <= 128)
   * \param prefTime preferred life time in seconds (default 7 days)
   * \param validTime valid life time in seconds (default 30 days)
   * \param flags the flags (L = 128, A = 64, R = 32)
   */
  SixLowPanPrefix (Ipv6Address prefix, uint8_t prefixLen, uint32_t prefTime, uint32_t validTime, uint8_t flags);

  /**
   * \brief Destructor.
   */
  ~SixLowPanPrefix ();

   /**
   * \brief Get network prefix.
   * \return network prefix
   */
  Ipv6Address GetPrefix () const;

  /**
   * \brief Set network prefix.
   * \param network network prefix
   */
  void SetPrefix (Ipv6Address prefix);

  /**
   * \brief Get prefix length.
   * \return prefix length
   */
  uint8_t GetPrefixLength () const;

  /**
   * \brief Set prefix length.
   * \param prefixLen prefix length
   */
  void SetPrefixLength (uint8_t prefixLen);

  /**
   * \brief Get preferred lifetime.
   * \return lifetime
   */
  uint32_t GetPreferredLifeTime () const;

  /**
   * \brief Set preferred lifetime.
   * \param prefTime lifetime
   */
  void SetPreferredLifeTime (uint32_t prefTime);

  /**
   * \brief Get valid lifetime.
   * \return lifetime
   */
  uint32_t GetValidLifeTime () const;

  /**
   * \brief Set valid lifetime.
   * \param validTime lifetime
   */
  void SetValidLifeTime (uint32_t validTime);

  /**
   * \brief Get the flags.
   * \return the flags (L = 128, A = 64, R = 32)
   */
  uint8_t GetFlags () const;

  /**
   * \brief Set the flags.
   * \param flags the flags to set (L = 128, A = 64, R = 32)
   */
  void SetFlags (uint8_t flags);

  /**
   * \brief Print the prefix
   * \param stream the ostream the prefix is printed to
   */
  void PrintPrefix (Ptr<OutputStreamWrapper> stream);

private:
  /**
   * \brief Network prefix.
   */
  Ipv6Address m_prefix;

  /**
   * \brief Prefix length.
   */
  uint8_t m_prefixLength;

  /**
   * \brief Preferred time.
   */
  uint32_t m_preferredLifeTime;

  /**
   * \brief Valid time.
   */
  uint32_t m_validLifeTime;

  /**
   * \brief Flags.
   */
  uint8_t m_flags;

  /**
   * \brief Prefix valid lifetime set time.
   */
  Time m_setValidTime;

  /**
   * \brief Prefix preferred lifetime set time.
   */
  Time m_setPrefTime;
};

/**
 * \ingroup sixlowpan
 * \class SixLowPanContext
 * \brief 6LoWPAN context container for 6LoWPAN ND.
 */
class SixLowPanContext : public SimpleRefCount<SixLowPanContext>
{
public:
  /**
   * \brief Constructor.
   */
  SixLowPanContext ();

  /**
   * \brief Constructor.
   * \param flagC compression flag
   * \param cid context identifier ( 0 <= x <= 15)
   * \param time valid lifetime of context (units of 60 seconds)
   * \param context 6LoWPAN context advertised
   */
  SixLowPanContext (bool flagC, uint8_t cid, uint16_t time, Ipv6Prefix context);

  /**
   * \brief Destructor.
   */
  ~SixLowPanContext ();

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

  /**
   * \brief Print the 6LoWPAN context.
   * \param stream the ostream the 6LoWPAN context is printed to
   */
  void PrintContext (Ptr<OutputStreamWrapper> stream);

  /**
   * \brief Function called when valid lifetime timeout.
   */
  void ValidTimeout ();

  /**
   * \brief Function called when router lifetime timeout.
   */
  void RouterTimeout ();

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

  /**
   * \brief Context set time.
   */
  Time m_setTime;
};

} /* namespace ns3 */

#endif /* SIXLOW_NDISC_RA_OPTIONS_H */
