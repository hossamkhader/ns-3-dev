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

#ifndef SIXLOWPAN_RADVD_INTERFACE_H
#define SIXLOWPAN_RADVD_INTERFACE_H

#include <list>
#include "ns3/radvd-interface.h"

#include "sixlowpan-radvd-context.h"
#include "sixlowpan-radvd-dad-entry.h"


namespace ns3
{

/**
 * \ingroup sixlowpanradvd
 * \class SixLowPanRadvdInterface
 * \brief Radvd interface configuration for 6LoWPAN ND.
 */
class SixLowPanRadvdInterface : public RadvdInterface
{
public:
  /// Container: Ptr to SixLowPanRadvdContext
  typedef std::list<Ptr<SixLowPanRadvdContext> > SixLowPanRadvdContextList;
  /// Container Iterator: Ptr to SixLowPanRadvdContext
  typedef std::list<Ptr<SixLowPanRadvdContext> >::iterator SixLowPanRadvdContextListI;
  /// Container Const Iterator: Ptr to SixLowPanRadvdContext
  typedef std::list<Ptr<SixLowPanRadvdContext> >::const_iterator SixLowPanRadvdContextListCI;

  ///Container: Ptr to SixLowPanRadvdDadEntry
  typedef std::list<Ptr<SixLowPanRadvdDadEntry> > DadTable;
  /// Container Iterator: Ptr to SixLowPanRadvdDadEntry
  typedef std::list<Ptr<SixLowPanRadvdDadEntry> >::iterator DadTableI;
  /// Container Const Iterator: Ptr to SixLowPanRadvdDadEntry
  typedef std::list<Ptr<SixLowPanRadvdDadEntry> >::const_iterator DadTableCI;

  /**
   * \brief Constructor.
   * \param interface interface index
   */
  SixLowPanRadvdInterface (uint32_t interface);

  /**
   * \brief Constructor.
   * \param interface interface index
   * \param maxRtrAdvInterval maximum RA interval (ms)
   * \param minRtrAdvInterval minimum RA interval (ms)
   */
  SixLowPanRadvdInterface (uint32_t interface, uint32_t maxRtrAdvInterval, uint32_t minRtrAdvInterval);

  /**
   * \brief Destructor.
   */
  ~SixLowPanRadvdInterface ();

  /**
   * \brief Get list of 6LoWPAN contexts advertised for this interface.
   * \return list of 6LoWPAN contexts
   */
  SixLowPanRadvdContextList GetContexts () const;

  /**
   * \brief Add a 6LoWPAN context to advertise on interface.
   * \param routerContext 6LoWPAN context to advertise
   */
  void AddContext (Ptr<SixLowPanRadvdContext> routerContext);
  
  /**
   * \brief Get the DAD Table for this interface.
   * \return DAD Table
   */
  DadTable GetDadTable () const;

  /**
   * \brief Add an entry to DAD Table on interface.
   * \param entry entry to add
   */
  void AddDadEntry (Ptr<SixLowPanRadvdDadEntry> entry);

  /**
   * \brief Get version value (ABRO).
   * \return the version value
   */
  uint32_t GetVersion () const;

  /**
   * \brief Set version value (ABRO).
   * \param version the version value
   */
  void SetVersion (uint32_t version);

  /**
   * \brief Get valid lifetime value (ABRO).
   * \return the valid lifetime (units of 60 seconds)
   */
  uint16_t GetValidTime () const;

  /**
   * \brief Set valid lifetime value (ABRO).
   * \param time the valid lifetime (units of 60 seconds)
   */
  void SetValidTime (uint16_t time);

private:

  /**
   * \brief List of 6LoWPAN contexts to advertise.
   */
  SixLowPanRadvdContextList m_contexts;

  /**
   * \brief A list of DAD entry (IPv6 Address, EUI-64, Lifetime).
   */
  DadTable m_dadTable;

  /**
   * \brief Version value for ABRO.
   */
  uint32_t m_version;

  /**
   * \brief Valid lifetime value for ABRO (units of 60 seconds).
   */
  uint16_t m_validTime;
};

} /* namespace ns3 */

#endif /* SIXLOW_RADVD_INTERFACE_H */
