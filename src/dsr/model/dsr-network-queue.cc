/*
 * Copyright (c) 2011 Yufei Cheng
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "dsr-network-queue.h"

#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/test.h"

#include <algorithm>
#include <functional>
#include <map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DsrNetworkQueue");

namespace dsr
{

NS_OBJECT_ENSURE_REGISTERED(DsrNetworkQueue);

TypeId
DsrNetworkQueue::GetTypeId()
{
    static TypeId tid = TypeId("ns3::dsr::DsrNetworkQueue")
                            .SetParent<Object>()
                            .SetGroupName("Dsr")
                            .AddConstructor<DsrNetworkQueue>();
    return tid;
}

DsrNetworkQueue::DsrNetworkQueue(uint32_t maxLen, Time maxDelay)
    : m_size(0),
      m_maxSize(maxLen),
      m_maxDelay(maxDelay)
{
    NS_LOG_FUNCTION(this);
}

DsrNetworkQueue::DsrNetworkQueue()
    : m_size(0)
{
    NS_LOG_FUNCTION(this);
}

DsrNetworkQueue::~DsrNetworkQueue()
{
    NS_LOG_FUNCTION(this);
    Flush();
}

void
DsrNetworkQueue::SetMaxNetworkSize(uint32_t maxSize)
{
    m_maxSize = maxSize;
}

void
DsrNetworkQueue::SetMaxNetworkDelay(Time delay)
{
    m_maxDelay = delay;
}

uint32_t
DsrNetworkQueue::GetMaxNetworkSize() const
{
    return m_maxSize;
}

Time
DsrNetworkQueue::GetMaxNetworkDelay() const
{
    return m_maxDelay;
}

bool
DsrNetworkQueue::FindPacketWithNexthop(Ipv4Address nextHop, DsrNetworkQueueEntry& entry)
{
    Cleanup();
    for (auto i = m_dsrNetworkQueue.begin(); i != m_dsrNetworkQueue.end(); ++i)
    {
        if (i->GetNextHopAddress() == nextHop)
        {
            entry = *i;
            i = m_dsrNetworkQueue.erase(i);
            return true;
        }
    }
    return false;
}

bool
DsrNetworkQueue::Find(Ipv4Address nextHop)
{
    Cleanup();
    for (auto i = m_dsrNetworkQueue.begin(); i != m_dsrNetworkQueue.end(); ++i)
    {
        if (i->GetNextHopAddress() == nextHop)
        {
            return true;
        }
    }
    return false;
}

bool
DsrNetworkQueue::Enqueue(DsrNetworkQueueEntry& entry)
{
    NS_LOG_FUNCTION(this << m_size << m_maxSize);
    if (m_size >= m_maxSize)
    {
        return false;
    }
    Time now = Simulator::Now();
    entry.SetInsertedTimeStamp(now);
    m_dsrNetworkQueue.push_back(entry);
    m_size++;
    NS_LOG_LOGIC("The network queue size is " << m_size);
    return true;
}

bool
DsrNetworkQueue::Dequeue(DsrNetworkQueueEntry& entry)
{
    NS_LOG_FUNCTION(this);
    Cleanup();
    auto i = m_dsrNetworkQueue.begin();
    if (i == m_dsrNetworkQueue.end())
    {
        // no elements in array
        NS_LOG_LOGIC("No queued packet in the network queue");
        return false;
    }
    entry = *i;
    m_dsrNetworkQueue.erase(i);
    m_size--;
    return true;
}

void
DsrNetworkQueue::Cleanup()
{
    NS_LOG_FUNCTION(this);
    if (m_dsrNetworkQueue.empty())
    {
        return;
    }

    Time now = Simulator::Now();
    uint32_t n = 0;
    for (auto i = m_dsrNetworkQueue.begin(); i != m_dsrNetworkQueue.end();)
    {
        if (i->GetInsertedTimeStamp() + m_maxDelay > now)
        {
            i++;
        }
        else
        {
            NS_LOG_LOGIC("Outdated packet");
            i = m_dsrNetworkQueue.erase(i);
            n++;
        }
    }
    m_size -= n;
}

uint32_t
DsrNetworkQueue::GetSize()
{
    NS_LOG_FUNCTION(this);
    return m_size;
}

void
DsrNetworkQueue::Flush()
{
    NS_LOG_FUNCTION(this);
    m_dsrNetworkQueue.erase(m_dsrNetworkQueue.begin(), m_dsrNetworkQueue.end());
    m_size = 0;
}

} // namespace dsr
} // namespace ns3
