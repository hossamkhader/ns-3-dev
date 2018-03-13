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

#ifndef SIXLOW_NDISC_CACHE_H
#define SIXLOW_NDISC_CACHE_H

#include "ns3/ipv6-address.h"
#include "ns3/mac64-address.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/sgi-hashmap.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ndisc-cache.h"

// #include "sixlow-ndisc-ra-options.h"

namespace ns3
{

class SixLowPanPrefix;
class SixLowPanContext;

/**
 * \infroup sixlowpan
 * \class SixLowPanNdiscCache
 * \brief Neighbor Discovery cache for 6LoWPAN ND. Keeps also RAs, prefixes and contexts.
 */
class SixLowPanNdiscCache : public NdiscCache
{
public:
  class SixLowPanEntry;

  class SixLowPanRaEntry;

  /**
   * \brief Get the type ID
   * \return type ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Constructor.
   */
  SixLowPanNdiscCache ();

  /**
   * \brief Destructor.
   */
  ~SixLowPanNdiscCache ();

  /**
   * \brief Lookup in the cache.
   * \param dst destination address
   * \return the entry if found, 0 otherwise
   */
  SixLowPanNdiscCache::SixLowPanEntry* SixLowPanLookup (Ipv6Address dst);

  /**
   * \brief Add an entry.
   * \param to address to add
   * \return an new Entry
   */
  SixLowPanNdiscCache::SixLowPanEntry* SixLowPanAdd (Ipv6Address to);

  /**
   * \brief Delete an entry.
   * \param entry pointer to delete from the list.
   */
  void SixLowPanRemove (SixLowPanNdiscCache::SixLowPanEntry* entry);

  /**
   * \brief Flush the cache.
   */
  virtual void Flush ();

  /**
   * \brief Print the SixLowPanNdisc cache entries
   *
   * \param stream the ostream the SixLowPanNdisc cache entries is printed to
   */
  virtual void PrintNdiscCache (Ptr<OutputStreamWrapper> stream);

  /**
   * \class SixLowPanEntry
   * \brief A record that holds information about an SixLowPanNdiscCache entry.
   */
  class SixLowPanEntry : public NdiscCache::Entry
  {
  public:
    /**
     * \brief Constructor.
     * \param nd The NdiscCache this entry belongs to.
     */
    SixLowPanEntry (NdiscCache* nd);

    /**
     * \brief Changes the state to this entry to REGISTERED.
     * It starts the registered timer.
     * \param time the lifetime in units of 60 seconds (from ARO)
     */
    void MarkRegistered (uint16_t time);

    /**
     * \brief Changes the state to this entry to TENTATIVE.
     * It starts the tentative timer (20 seconds).
     */
    void MarkTentative ();

    /**
     * \brief Change the state to this entry to GARBAGE.
     */
    void MarkGarbage ();

    /**
     * \brief Is the entry REGISTERED.
     * \return true if the type of entry is REGISTERED, false otherwise
     */
    bool IsRegistered () const;

    /**
     * \brief Is the entry TENTATIVE.
     * \return true if the type of entry is TENTATIVE, false otherwise
     */
    bool IsTentative () const;

    /**
     * \brief Is the entry GARBAGE-COLLECTIBLE.
     * \return true if the type of entry GARBAGE-COLLECTIBLE, false otherwise
     */
    bool IsGarbage () const;

    /**
     * \brief Function called when timer timeout.
     */
    void FunctionTimeout ();

    /**
     * \brief Get the EUI-64 of this entry.
     * \return EUI-64 value
     */
    Mac64Address GetEui64 () const;

    /**
     * \brief Set the EUI-64.
     * \param eui the EUI-64 value
     */
    void SetEui64 (Mac64Address eui);

  private:
    /**
     * \brief The SixLowPanEntry type enumeration.
     */
    enum SixLowPanNdiscCacheEntryType_e
    {
      REGISTERED, /**< Have an explicit registered lifetime */
      TENTATIVE, /**< Have a short lifetime, typically get converted to REGISTERED */
      GARBAGE /**< Allow for garbage collection when low on memory */
    };

    /**
     * \brief The EUI-64 value.
     */
    Mac64Address m_eui64;

    /**
     * \brief The state of the entry.
     */
    SixLowPanNdiscCacheEntryType_e m_type;

    /**
     * \brief Timer (used for REGISTERED entries).
     */
    Timer m_registeredTimer;

    /**
     * \brief Timer (used for TENTATIVE entries).
     */
    Timer m_tentativeTimer;
  };

  /**
   * \brief Lookup in the RA cache.
   * \param border the Border Router address
   * \return the entry if found, 0 otherwise
   */
  SixLowPanNdiscCache::SixLowPanRaEntry* RaEntryLookup (Ipv6Address border);

  /**
   * \brief Add an entry to the RA cache.
   * \param border the Border Router address
   * \return a new SixLowPanContext
   */
  SixLowPanNdiscCache::SixLowPanRaEntry* AddRaEntry (Ipv6Address border);

  /**
   * \brief Delete an entry from the RA cache.
   * \param entry pointer to delete from the list.
   */
  void RemoveRaEntry (SixLowPanNdiscCache::SixLowPanRaEntry* entry);

  /**
   * \brief Flush the RA cache.
   */
  void FlushRaCache ();

  /**
   * \brief Print the RA cache entries
   *
   * \param stream the ostream the RA cache entries is printed to
   */
  void PrintRaCache (Ptr<OutputStreamWrapper> stream);

  /**
   * \brief Get list of RA received.
   * \return list of RA
   */
  std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *> GetRaCache () const;

  /**
   * \class SixLowPanRaEntry
   * \brief RA advertised from routers for 6LoWPAN ND.
   */
  class SixLowPanRaEntry
  {
  public:
    /**
     * \brief Constructor.
     * \param nd The NdiscCache this ra entry belongs to.
     */
    SixLowPanRaEntry (SixLowPanNdiscCache* nd);

    /**
     * \brief Destructor.
     */
    ~SixLowPanRaEntry ();

    /**
     * \brief Get list of prefixes advertised for this interface.
     * \return list of IPv6 prefixes
     */
    std::map<Ipv6Address, Ptr<SixLowPanPrefix> > GetPrefixes () const;

    /**
     * \brief Add a prefix to advertise on interface.
     * \param prefix prefix to advertise
     */
    void AddPrefix (Ptr<SixLowPanPrefix> prefix);

    /**
     * \brief Remove a prefix.
     * \param prefix prefix to remove
     */
    void RemovePrefix (Ptr<SixLowPanPrefix> prefix);

    /**
     * \brief Get list of 6LoWPAN contexts advertised for this interface.
     * \return list of 6LoWPAN contexts
     */
    std::map<uint8_t, Ptr<SixLowPanContext> > GetContexts () const;

    /**
     * \brief Add a 6LoWPAN context to advertise on interface.
     * \param context 6LoWPAN context to advertise
     */
    void AddContext (Ptr<SixLowPanContext> context);

    /**
     * \brief Remove a 6LoWPAN context.
     * \param context 6LoWPAN context to remove
     */
    void RemoveContext (Ptr<SixLowPanContext> context);

    /**
     * \brief Is managed flag enabled ?
     * \return managed flag
     */
    bool IsManagedFlag () const;

    /**
     * \brief Set managed flag
     * \param managedFlag value
     */
    void SetManagedFlag (bool managedFlag);

    /**
     * \brief Is "other config" flag enabled ?
     * \return other config flag
     */
    bool IsOtherConfigFlag () const;

    /**
     * \brief Is "home agent" flag enabled ?
     * \return "home agent" flag
     */
    bool IsHomeAgentFlag () const;

    /**
     * \brief Set "home agent" flag.
     * \param homeAgentFlag value
     */
    void SetHomeAgentFlag (bool homeAgentFlag);

    /**
     * \brief Set "other config" flag
     * \param otherConfigFlag value
     */
    void SetOtherConfigFlag (bool otherConfigFlag);

    /**
     * \brief Get reachable time.
     * \return reachable time
     */
    uint32_t GetReachableTime () const;

    /**
     * \brief Set reachable time.
     * \param time reachable time
     */
    void SetReachableTime (uint32_t itme);

    /**
     * \brief Get router lifetime.
     * \return router lifetime
     */
    uint32_t GetRouterLifeTime () const;

    /**
     * \brief Set router lifetime.
     * \param time router lifetime
     */
    void SetRouterLifeTime (uint32_t time);

    /**
     * \brief Get retransmission timer.
     * \return retransmission timer
     */
    uint32_t GetRetransTimer () const;

    /**
     * \brief Set retransmission timer.
     * \param timer retransmission timer
     */
    void SetRetransTimer (uint32_t timer);

    /**
     * \brief Get current hop limit.
     * \return current hop limit for the link
     */
    uint8_t GetCurHopLimit () const;

    /**
     * \brief Set current hop limit.
     * \param curHopLimit current hop limit for the link
     */
    void SetCurHopLimit (uint8_t curHopLimit);

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

    /**
     * \brief Get Border Router address (ABRO).
     * \return the Border Router address
     */
    Ipv6Address GetBorderAddress () const;

    /**
     * \brief Set Border Router address (ABRO).
     * \param border the Border Router address
     */
    void SetBorderAddress (Ipv6Address border);

  private:
    /**
     * \brief List of prefixes advertised.
     */
    std::map<Ipv6Address, Ptr<SixLowPanPrefix> > m_prefixes;

    /**
     * \brief List of 6LoWPAN contexts advertised.
     */
    std::map<uint8_t, Ptr<SixLowPanContext> > m_contexts;

    /**
     * \brief Managed flag. If true host use the stateful protocol for address autoconfiguration.
     */
    bool m_managedFlag;

    /**
     * \brief Other configuration flag. If true host use stateful protocol for other (non-address) information.
     */
    bool m_otherConfigFlag;

    /**
     * \brief Flag to add HA (home agent) flag in RA.
     */
    bool m_homeAgentFlag;

    /**
     * \brief Reachable time in milliseconds.
     */
    uint32_t m_reachableTime;

    /**
     * \brief Retransmission timer in milliseconds.
     */
    uint32_t m_retransTimer;

    /**
     * \brief Current hop limit (TTL).
     */
    uint32_t m_curHopLimit;

    /**
     * \brief Router life time in seconds.
     */
    uint32_t m_routerLifeTime;

    /**
     * \brief Version value for ABRO.
     */
    uint32_t m_version;

    /**
     * \brief Valid lifetime value for ABRO (units of 60 seconds).
     */
    uint16_t m_validTime;

    /**
     * \brief Border Router address for ABRO.
     */
    Ipv6Address m_border;
  };

  protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

  private:
  /**
   * \brief RA entry container
   */
  typedef std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *> SixLowPanRaCache;

  /**
   * \brief RA entry container iterator
   */
  typedef std::map<Ipv6Address, SixLowPanNdiscCache::SixLowPanRaEntry *>::iterator SixLowPanRaCacheI;

  /**
   * \brief A list of RA entry.
   */
  SixLowPanRaCache m_raCache;

  /**
   * \brief 6LoWPAN Neighbor Discovery Cache container
   */
  typedef sgi::hash_map<Ipv6Address, SixLowPanNdiscCache::SixLowPanEntry *, Ipv6AddressHash> SixLowPanCache;

  /**
   * \brief 6LoWPAN Neighbor Discovery Cache container iterator
   */
  typedef sgi::hash_map<Ipv6Address, SixLowPanNdiscCache::SixLowPanEntry *, Ipv6AddressHash>::iterator SixLowPanCacheI;

  /**
   * \brief A list of Entry.
   */
  SixLowPanCache m_sixlowNdCache;
};

} /* namespace ns3 */

#endif /* SIXLOW_NDISC_CACHE_H */
