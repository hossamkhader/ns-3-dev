/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Kirill Andreev <andreev@iitp.ru>
 */

#include "ns3/ipv4-interface-container.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/pcap-file.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup mesh-test
 * @defgroup flame-test flame sub-module tests
 */

/**
 * @ingroup flame-test
 *
 * @brief FLAME protocol regression test of three stations:
 * @verbatim
 * <-----------|----------->   Broadcast frame
 *             |----------->|  Unicast frame
 *           Source                   Destination
 * (node ID)   2            1            0
 * <-----------|----------->|            |             ARP request
 *             |<-----------|----------->|             ARP request
 *             |            |<-----------|             ARP reply
 *             |<-----------|            |             ARP reply
 *             |----------->|            |             Data
 *             |            |----------->|             Data
 *             |            |<-----------|-----------> PATH_UPDATE (no broadcast was sent)
 *             |<-----------|----------->|             PATH_UPDATE
 * <-----------|----------->|            |             PATH_UPDATE
 *             |            |<-----------|-----------> ARP request
 *             |<-----------|----------->|             ARP request
 * <-----------|----------->|            |             ARP request
 *             |----------->|            |             ARP reply
 *             |            |----------->|             ARP reply
 *             |            |<-----------|             Data
 *             |<-----------|            |             Data
 *             |............|............|
 *             After five seconds data is transmitted again as
 *             broadcast, and PATH_UPDATE is sent
 * @endverbatim
 */
class FlameRegressionTest : public TestCase
{
  public:
    FlameRegressionTest();
    ~FlameRegressionTest() override;

    void DoRun() override;
    /// Check results function
    void CheckResults();

  private:
    /// @internal It is important to have pointers here
    NodeContainer* m_nodes;
    /// Simulation time
    Time m_time;
    /// Needed to install applications
    Ipv4InterfaceContainer m_interfaces;

    /// Create nodes function
    void CreateNodes();
    /// Create devices function
    void CreateDevices();
    /// Install application function
    void InstallApplications();

    /// Server-side socket
    Ptr<Socket> m_serverSocket;
    /// Client-side socket
    Ptr<Socket> m_clientSocket;

    /// sent packets counter
    uint32_t m_sentPktsCounter;

    /**
     * Send data
     * @param socket the sending socket
     */
    void SendData(Ptr<Socket> socket);

    /**
     * @brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * @param socket the socket the packet was received to.
     */
    void HandleReadServer(Ptr<Socket> socket);

    /**
     * @brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * @param socket the socket the packet was received to.
     */
    void HandleReadClient(Ptr<Socket> socket);
};
