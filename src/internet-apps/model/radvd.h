/*
 * Copyright (c) 2008 Telecom Bretagne
 * Copyright (c) 2009 Strasbourg University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 *         Mehdi Benamor <benamor.mehdi@ensi.rnu.tn>
 */

#ifndef RADVD_H
#define RADVD_H

#include "radvd-interface.h"

#include "ns3/application.h"

#include <map>

namespace ns3
{

class UniformRandomVariable;
class Socket;

/**
 * @ingroup internet-apps
 * @defgroup radvd Radvd
 */

/**
 * @ingroup radvd
 * @brief Router advertisement daemon.
 */
class Radvd : public Application
{
  public:
    /**
     * @brief Get the type ID.
     * @return type ID
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    Radvd();

    /**
     * @brief Destructor.
     */
    ~Radvd() override;

    /**
     * @brief Default value for maximum delay of RA (ms)
     */
    static const uint32_t MAX_RA_DELAY_TIME = 500;
    /**
     * @brief Default value for maximum initial RA advertisements
     */
    static const uint32_t MAX_INITIAL_RTR_ADVERTISEMENTS = 3;
    /**
     * @brief Default value for maximum initial RA advertisements interval (ms)
     */
    static const uint32_t MAX_INITIAL_RTR_ADVERT_INTERVAL = 16000;
    /**
     * @brief Default value for minimum delay between RA advertisements (ms)
     */
    static const uint32_t MIN_DELAY_BETWEEN_RAS = 3000;

    /**
     * @brief Add configuration for an interface;
     * @param routerInterface configuration
     */
    void AddConfiguration(Ptr<RadvdInterface> routerInterface);

    int64_t AssignStreams(int64_t stream) override;

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /// Container: Ptr to RadvdInterface
    typedef std::list<Ptr<RadvdInterface>> RadvdInterfaceList;
    /// Container Iterator: Ptr to RadvdInterface
    typedef std::list<Ptr<RadvdInterface>>::iterator RadvdInterfaceListI;
    /// Container Const Iterator: Ptr to RadvdInterface
    typedef std::list<Ptr<RadvdInterface>>::const_iterator RadvdInterfaceListCI;

    /// Container: interface number, EventId
    typedef std::map<uint32_t, EventId> EventIdMap;
    /// Container Iterator: interface number, EventId
    typedef std::map<uint32_t, EventId>::iterator EventIdMapI;
    /// Container Const Iterator: interface number, EventId
    typedef std::map<uint32_t, EventId>::const_iterator EventIdMapCI;

    /// Container: interface number, Socket
    typedef std::map<uint32_t, Ptr<Socket>> SocketMap;
    /// Container Iterator: interface number, Socket
    typedef std::map<uint32_t, Ptr<Socket>>::iterator SocketMapI;
    /// Container Const Iterator: interface number, Socket
    typedef std::map<uint32_t, Ptr<Socket>>::const_iterator SocketMapCI;

    /**
     * @brief Send a packet.
     * @param config interface configuration
     * @param dst destination address (default ff02::1)
     * @param reschedule if true another send will be reschedule (periodic)
     */
    void Send(Ptr<RadvdInterface> config,
              Ipv6Address dst = Ipv6Address::GetAllNodesMulticast(),
              bool reschedule = false);

    /**
     * @brief Handle received packet, especially router solicitation
     * @param socket socket to read data from
     */
    void HandleRead(Ptr<Socket> socket);

    /**
     * @brief Raw socket to receive RS.
     */
    Ptr<Socket> m_recvSocket;

    /**
     * @brief Raw socket to send RA.
     */
    SocketMap m_sendSockets;

    /**
     * @brief List of configuration for interface.
     */
    RadvdInterfaceList m_configurations;

    /**
     * @brief Event ID map for unsolicited RAs.
     */
    EventIdMap m_unsolicitedEventIds;

    /**
     * @brief Event ID map for solicited RAs.
     */
    EventIdMap m_solicitedEventIds;

    /**
     * @brief Variable to provide jitter in advertisement interval
     */
    Ptr<UniformRandomVariable> m_jitter;
};

} /* namespace ns3 */

#endif /* RADVD_H */
