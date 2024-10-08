/*
 * Copyright (c) 2007,2008,2009 INRIA, UDcast
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Jahanzeb Farooq <jahanzeb.farooq@sophia.inria.fr>
 *          Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                               <amine.ismail@UDcast.com>
 */

#include "connection-manager.h"

#include "bs-net-device.h"
#include "cid-factory.h"
#include "mac-messages.h"
#include "service-flow.h"
#include "ss-net-device.h"
#include "ss-record.h"

#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/pointer.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ConnectionManager");

NS_OBJECT_ENSURE_REGISTERED(ConnectionManager);

TypeId
ConnectionManager::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ConnectionManager").SetParent<Object>().SetGroupName("Wimax");
    return tid;
}

ConnectionManager::ConnectionManager()
    : m_cidFactory(nullptr)
{
}

void
ConnectionManager::DoDispose()
{
}

ConnectionManager::~ConnectionManager()
{
}

void
ConnectionManager::SetCidFactory(CidFactory* cidFactory)
{
    m_cidFactory = cidFactory;
}

void
ConnectionManager::AllocateManagementConnections(SSRecord* ssRecord, RngRsp* rngrsp)
{
    Ptr<WimaxConnection> basicConnection = CreateConnection(Cid::BASIC);
    ssRecord->SetBasicCid(basicConnection->GetCid());

    Ptr<WimaxConnection> primaryConnection = CreateConnection(Cid::PRIMARY);
    ssRecord->SetPrimaryCid(primaryConnection->GetCid());

    rngrsp->SetBasicCid(basicConnection->GetCid());
    rngrsp->SetPrimaryCid(primaryConnection->GetCid());
}

Ptr<WimaxConnection>
ConnectionManager::CreateConnection(Cid::Type type)
{
    Cid cid;
    switch (type)
    {
    case Cid::BASIC:
    case Cid::MULTICAST:
    case Cid::PRIMARY:
        cid = m_cidFactory->Allocate(type);
        break;
    case Cid::TRANSPORT:
        cid = m_cidFactory->AllocateTransportOrSecondary();
        break;
    default:
        NS_FATAL_ERROR("Invalid connection type");
        break;
    }
    Ptr<WimaxConnection> connection = CreateObject<WimaxConnection>(cid, type);
    AddConnection(connection, type);
    return connection;
}

void
ConnectionManager::AddConnection(Ptr<WimaxConnection> connection, Cid::Type type)
{
    switch (type)
    {
    case Cid::BASIC:
        m_basicConnections.push_back(connection);
        break;
    case Cid::PRIMARY:
        m_primaryConnections.push_back(connection);
        break;
    case Cid::TRANSPORT:
        m_transportConnections.push_back(connection);
        break;
    case Cid::MULTICAST:
        m_multicastConnections.push_back(connection);
        break;
    default:
        NS_FATAL_ERROR("Invalid connection type");
        break;
    }
}

Ptr<WimaxConnection>
ConnectionManager::GetConnection(Cid cid)
{
    for (auto iter = m_basicConnections.begin(); iter != m_basicConnections.end(); ++iter)
    {
        if ((*iter)->GetCid() == cid)
        {
            return *iter;
        }
    }

    for (auto iter = m_primaryConnections.begin(); iter != m_primaryConnections.end(); ++iter)
    {
        if ((*iter)->GetCid() == cid)
        {
            return *iter;
        }
    }

    for (auto iter = m_transportConnections.begin(); iter != m_transportConnections.end(); ++iter)
    {
        if ((*iter)->GetCid() == cid)
        {
            return *iter;
        }
    }

    return nullptr;
}

std::vector<Ptr<WimaxConnection>>
ConnectionManager::GetConnections(Cid::Type type) const
{
    std::vector<Ptr<WimaxConnection>> connections;

    switch (type)
    {
    case Cid::BASIC:
        connections = m_basicConnections;
        break;
    case Cid::PRIMARY:
        connections = m_primaryConnections;
        break;
    case Cid::TRANSPORT:
        connections = m_transportConnections;
        break;
    default:
        NS_FATAL_ERROR("Invalid connection type");
        break;
    }

    return connections;
}

uint32_t
ConnectionManager::GetNPackets(Cid::Type type, ServiceFlow::SchedulingType schedulingType) const
{
    uint32_t nrPackets = 0;

    switch (type)
    {
    case Cid::BASIC: {
        for (auto iter = m_basicConnections.begin(); iter != m_basicConnections.end(); ++iter)
        {
            nrPackets += (*iter)->GetQueue()->GetSize();
        }
        break;
    }
    case Cid::PRIMARY: {
        for (auto iter = m_primaryConnections.begin(); iter != m_primaryConnections.end(); ++iter)
        {
            nrPackets += (*iter)->GetQueue()->GetSize();
        }
        break;
    }
    case Cid::TRANSPORT: {
        for (auto iter = m_transportConnections.begin(); iter != m_transportConnections.end();
             ++iter)
        {
            if (schedulingType == ServiceFlow::SF_TYPE_ALL ||
                (*iter)->GetSchedulingType() == schedulingType)
            {
                nrPackets += (*iter)->GetQueue()->GetSize();
            }
        }
        break;
    }
    default:
        NS_FATAL_ERROR("Invalid connection type");
        break;
    }

    return nrPackets;
}

bool
ConnectionManager::HasPackets() const
{
    for (auto iter = m_basicConnections.begin(); iter != m_basicConnections.end(); ++iter)
    {
        if ((*iter)->HasPackets())
        {
            return true;
        }
    }

    for (auto iter = m_primaryConnections.begin(); iter != m_primaryConnections.end(); ++iter)
    {
        if ((*iter)->HasPackets())
        {
            return true;
        }
    }

    for (auto iter = m_transportConnections.begin(); iter != m_transportConnections.end(); ++iter)
    {
        if ((*iter)->HasPackets())
        {
            return true;
        }
    }

    return false;
}
} // namespace ns3
