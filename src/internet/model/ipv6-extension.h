/*
 * Copyright (c) 2007-2009 Strasbourg University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: David Gross <gdavid.devel@gmail.com>
 */

#ifndef IPV6_EXTENSION_H
#define IPV6_EXTENSION_H

#include "ipv6-extension-header.h"
#include "ipv6-header.h"
#include "ipv6-interface.h"
#include "ipv6-l3-protocol.h"

#include "ns3/buffer.h"
#include "ns3/ipv6-address.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

#include <list>
#include <map>
#include <tuple>

namespace ns3
{

/**
 * @ingroup ipv6
 * @defgroup ipv6HeaderExt IPV6 Header extension system.
 */

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension base
 * If you want to implement a new IPv6 extension, all you have to do is
 * implement a subclass of this class and add it to an Ipv6ExtensionDemux.
 */
class Ipv6Extension : public Object
{
  public:
    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6Extension();

    /**
     * @brief Destructor.
     */
    ~Ipv6Extension() override;

    /**
     * @brief Set the node.
     * @param node the node to set
     */
    void SetNode(Ptr<Node> node);

    /**
     * @brief Get the node.
     * @return the node
     */
    Ptr<Node> GetNode() const;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    virtual uint8_t GetExtensionNumber() const = 0;

    /**
     * @brief Process method
     * Called from Ipv6L3Protocol::Receive.
     *
     * @param packet the packet
     * @param offset the offset of the extension to process
     * @param ipv6Header the IPv6 header of packet received
     * @param dst destination address of the packet received (i.e. us)
     * @param nextHeader the next header
     * @param stopProcessing true if the packet must not be further processed
     * @param isDropped true if the packet must be dropped
     * @param dropReason dropping reason
     * @return the size processed
     */
    virtual uint8_t Process(Ptr<Packet>& packet,
                            uint8_t offset,
                            const Ipv6Header& ipv6Header,
                            Ipv6Address dst,
                            uint8_t* nextHeader,
                            bool& stopProcessing,
                            bool& isDropped,
                            Ipv6L3Protocol::DropReason& dropReason) = 0;

    /**
     * @brief Process options
     * Called by implementing classes to process the options
     *
     * @param packet the packet
     * @param offset the offset of the first option to process
     * @param length the total length of all options (as specified in the extension header)
     * @param ipv6Header the IPv6 header of packet received
     * @param dst destination address of the packet received (i.e. us)
     * @param nextHeader the next header
     * @param stopProcessing true if the packet must not be further processed
     * @param isDropped true if the packet must be dropped
     * @param dropReason dropping reason
     * @return the size processed
     */
    virtual uint8_t ProcessOptions(Ptr<Packet>& packet,
                                   uint8_t offset,
                                   uint8_t length,
                                   const Ipv6Header& ipv6Header,
                                   Ipv6Address dst,
                                   uint8_t* nextHeader,
                                   bool& stopProcessing,
                                   bool& isDropped,
                                   Ipv6L3Protocol::DropReason& dropReason);

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * @param stream first stream index to use
     * @return the number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream);

  protected:
    /**
     * @brief Provides uniform random variables.
     */
    Ptr<UniformRandomVariable> m_uvar;

  private:
    /**
     * @brief The node.
     */
    Ptr<Node> m_node;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension "Hop By Hop"
 */
class Ipv6ExtensionHopByHop : public Ipv6Extension
{
  public:
    /**
     * @brief Hop-by-hop extension number.
     */
    static const uint8_t EXT_NUMBER = 0;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionHopByHop();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionHopByHop() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension Destination
 */
class Ipv6ExtensionDestination : public Ipv6Extension
{
  public:
    /**
     * @brief Destination extension number.
     */
    static const uint8_t EXT_NUMBER = 60;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionDestination();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionDestination() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension Fragment
 */
class Ipv6ExtensionFragment : public Ipv6Extension
{
  public:
    /**
     * @brief Fragmentation extension number.
     */
    static const uint8_t EXT_NUMBER = 44;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionFragment();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionFragment() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;

    /**
     * @brief Pair of a packet and an Ipv6 header.
     */
    typedef std::pair<Ptr<Packet>, Ipv6Header> Ipv6PayloadHeaderPair;

    /**
     * @brief Fragment a packet.
     *
     * @param packet the packet.
     * @param ipv6Header the IPv6 header.
     * @param fragmentSize the maximal size of the fragment (unfragmentable part + fragmentation
     * header + fragmentable part).
     * @param listFragments the list of fragments.
     */
    void GetFragments(Ptr<Packet> packet,
                      Ipv6Header ipv6Header,
                      uint32_t fragmentSize,
                      std::list<Ipv6PayloadHeaderPair>& listFragments);

  protected:
    /**
     * @brief Dispose this object.
     */
    void DoDispose() override;

  private:
    /**
     * Key identifying a fragmented packet
     */
    typedef std::pair<Ipv6Address, uint32_t> FragmentKey_t;

    /**
     * Container for fragment timeouts.
     */
    typedef std::list<std::tuple<Time, FragmentKey_t, Ipv6Header>> FragmentsTimeoutsList_t;
    /**
     * Container Iterator for fragment timeouts.
     */
    typedef std::list<std::tuple<Time, FragmentKey_t, Ipv6Header>>::iterator
        FragmentsTimeoutsListI_t;

    /**
     * @ingroup ipv6HeaderExt
     *
     * @brief This class stores the fragments of a packet waiting to be rebuilt.
     */
    class Fragments : public SimpleRefCount<Fragments>
    {
      public:
        /**
         * @brief Constructor.
         */
        Fragments();

        /**
         * @brief Destructor.
         */
        ~Fragments();

        /**
         * @brief Add a fragment.
         * @param fragment the fragment
         * @param fragmentOffset the offset of the fragment
         * @param moreFragment the bit "More Fragment"
         */
        void AddFragment(Ptr<Packet> fragment, uint16_t fragmentOffset, bool moreFragment);

        /**
         * @brief Set the unfragmentable part of the packet.
         * @param unfragmentablePart the unfragmentable part
         */
        void SetUnfragmentablePart(Ptr<Packet> unfragmentablePart);

        /**
         * @brief If all fragments have been added.
         * @returns true if the packet is entire
         */
        bool IsEntire() const;

        /**
         * @brief Get the entire packet.
         * @return the entire packet
         */
        Ptr<Packet> GetPacket() const;

        /**
         * @brief Get the packet parts so far received.
         * @return the partial packet
         */
        Ptr<Packet> GetPartialPacket() const;

        /**
         * @brief Set the Timeout iterator.
         * @param iter The iterator.
         */
        void SetTimeoutIter(FragmentsTimeoutsListI_t iter);

        /**
         * @brief Get the Timeout iterator.
         * @returns The iterator.
         */
        FragmentsTimeoutsListI_t GetTimeoutIter();

      private:
        /**
         * @brief If other fragments will be sent.
         */
        bool m_moreFragment;

        /**
         * @brief The current fragments.
         */
        std::list<std::pair<Ptr<Packet>, uint16_t>> m_packetFragments;

        /**
         * @brief The unfragmentable part.
         */
        Ptr<Packet> m_unfragmentable;

        /**
         * @brief Timeout iterator to "event" handler
         */
        FragmentsTimeoutsListI_t m_timeoutIter;
    };

    /**
     * @brief Process the timeout for packet fragments
     * @param key representing the packet fragments
     * @param ipHeader the IP header of the original packet
     */
    void HandleFragmentsTimeout(FragmentKey_t key, Ipv6Header ipHeader);

    /**
     * @brief Get the packet parts so far received.
     * @return the partial packet
     */
    Ptr<Packet> GetPartialPacket() const;

    /**
     * @brief Set the Timeout EventId.
     * @param event The event.
     */
    void SetTimeoutEventId(EventId event);

    /**
     * @brief Cancel the timeout event
     */
    void CancelTimeout();

    /**
     * @brief Container for the packet fragments.
     */
    typedef std::map<FragmentKey_t, Ptr<Fragments>> MapFragments_t;

    /**
     * @brief The hash of fragmented packets.
     */
    MapFragments_t m_fragments;

    /**
     * @brief Set a new timeout "event" for a fragmented packet
     * @param key the fragment identification
     * @param ipHeader the IPv6 header of the fragmented packet
     * @return an iterator to the inserted "event"
     */
    FragmentsTimeoutsListI_t SetTimeout(FragmentKey_t key, Ipv6Header ipHeader);

    /**
     * @brief Handles a fragmented packet timeout
     */
    void HandleTimeout();

    FragmentsTimeoutsList_t m_timeoutEventList; //!< Timeout "events" container
    EventId m_timeoutEvent;                     //!< Event for the next scheduled timeout
    Time m_fragmentExpirationTimeout;           //!< Expiration timeout
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension Routing.
 *
 * If you want to implement a new IPv6 routing extension, all you have to do is
 * implement a subclass of this class and add it to an Ipv6ExtensionRoutingDemux.
 */
class Ipv6ExtensionRouting : public Ipv6Extension
{
  public:
    /**
     * @brief Routing extension number.
     */
    static const uint8_t EXT_NUMBER = 43;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionRouting();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionRouting() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    /**
     * @brief Get the type of routing.
     * @return type of routing
     */
    virtual uint8_t GetTypeRouting() const;

    /**
     * @brief Get a pointer to a new routing extension header.
     * The ownership is transferred to the caller.
     * @return a pointer to a new routing extension header.
     */
    virtual Ipv6ExtensionRoutingHeader* GetExtensionRoutingHeaderPtr();

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension Routing Demux.
 */
class Ipv6ExtensionRoutingDemux : public Object
{
  public:
    /**
     * @brief The interface ID.
     * @return type ID
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionRoutingDemux();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionRoutingDemux() override;

    /**
     * @brief Set the node.
     * @param node the node to set
     */
    void SetNode(Ptr<Node> node);

    /**
     * @brief Insert a new IPv6 Routing Extension.
     * @param extensionRouting the routing extension to insert
     */
    void Insert(Ptr<Ipv6ExtensionRouting> extensionRouting);

    /**
     * @brief Get the routing extension corresponding to typeRouting.
     * @param typeRouting the number of the routing extension to retrieve
     * @return a matching IPv6 routing extension
     */
    Ptr<Ipv6ExtensionRouting> GetExtensionRouting(uint8_t typeRouting);

    /**
     * @brief Get a pointer to a new routing extension header corresponding
     * to typeRouting. The ownership is transferred to the caller.
     * @param typeRouting the number of the routing extension to retrieve
     * @return a pointer to a new routing extension header matching IPv6 routing extension
     */
    Ipv6ExtensionRoutingHeader* GetExtensionRoutingHeaderPtr(uint8_t typeRouting);

    /**
     * @brief Remove a routing extension from this demux.
     * @param extensionRouting pointer on the extension to remove
     */
    void Remove(Ptr<Ipv6ExtensionRouting> extensionRouting);

  protected:
    /**
     * @brief Dispose this object.
     */
    void DoDispose() override;

  private:
    /**
     * @brief Container for the extension routing.
     */
    typedef std::list<Ptr<Ipv6ExtensionRouting>> Ipv6ExtensionRoutingList_t;

    /**
     * @brief List of IPv6 Routing Extensions supported.
     */
    Ipv6ExtensionRoutingList_t m_extensionsRouting;

    /**
     * @brief The node.
     */
    Ptr<Node> m_node;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension Loose Routing
 */
class Ipv6ExtensionLooseRouting : public Ipv6ExtensionRouting
{
  public:
    /**
     * @brief Routing type.
     */
    static const uint8_t TYPE_ROUTING = 0;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionLooseRouting();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionLooseRouting() override;

    /**
     * @brief Get the type of routing.
     * @return type of routing
     */
    uint8_t GetTypeRouting() const override;

    Ipv6ExtensionRoutingHeader* GetExtensionRoutingHeaderPtr() override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension ESP (Encapsulating Security Payload)
 */
class Ipv6ExtensionESP : public Ipv6Extension
{
  public:
    /**
     * @brief ESP extension number.
     */
    static const uint8_t EXT_NUMBER = 50;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionESP();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionESP() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

/**
 * @ingroup ipv6HeaderExt
 *
 * @brief IPv6 Extension AH (Authentication Header)
 */
class Ipv6ExtensionAH : public Ipv6Extension
{
  public:
    /**
     * @brief AH extension number.
     */
    static const uint8_t EXT_NUMBER = 51;

    /**
     * @brief Get the type identificator.
     * @return type identificator
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Ipv6ExtensionAH();

    /**
     * @brief Destructor.
     */
    ~Ipv6ExtensionAH() override;

    /**
     * @brief Get the extension number.
     * @return extension number
     */
    uint8_t GetExtensionNumber() const override;

    uint8_t Process(Ptr<Packet>& packet,
                    uint8_t offset,
                    const Ipv6Header& ipv6Header,
                    Ipv6Address dst,
                    uint8_t* nextHeader,
                    bool& stopProcessing,
                    bool& isDropped,
                    Ipv6L3Protocol::DropReason& dropReason) override;
};

} /* namespace ns3 */

#endif /* IPV6_EXTENSION_H */
