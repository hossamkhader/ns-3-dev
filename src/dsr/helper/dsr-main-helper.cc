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

#include "dsr-main-helper.h"

#include "dsr-helper.h"

#include "ns3/dsr-rcache.h"
#include "ns3/dsr-routing.h"
#include "ns3/dsr-rreq-table.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/ptr.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DsrMainHelper");

DsrMainHelper::DsrMainHelper()
    : m_dsrHelper(nullptr)
{
    NS_LOG_FUNCTION(this);
}

DsrMainHelper::DsrMainHelper(const DsrMainHelper& o)
{
    NS_LOG_FUNCTION(this);
    m_dsrHelper = o.m_dsrHelper->Copy();
}

DsrMainHelper::~DsrMainHelper()
{
    NS_LOG_FUNCTION(this);
    delete m_dsrHelper;
}

DsrMainHelper&
DsrMainHelper::operator=(const DsrMainHelper& o)
{
    if (this == &o)
    {
        return *this;
    }
    m_dsrHelper = o.m_dsrHelper->Copy();
    return *this;
}

void
DsrMainHelper::Install(DsrHelper& dsrHelper, NodeContainer nodes)
{
    NS_LOG_DEBUG("Passed node container");
    delete m_dsrHelper;
    m_dsrHelper = dsrHelper.Copy();
    for (auto i = nodes.Begin(); i != nodes.End(); ++i)
    {
        Install(*i);
    }
}

void
DsrMainHelper::Install(Ptr<Node> node)
{
    NS_LOG_FUNCTION(node);
    Ptr<ns3::dsr::DsrRouting> dsr = m_dsrHelper->Create(node);
    //  Ptr<ns3::dsr::RouteCache> routeCache = CreateObject<ns3::dsr::RouteCache> ();
    //  Ptr<ns3::dsr::RreqTable> rreqTable = CreateObject<ns3::dsr::RreqTable> ();
    //  dsr->SetRouteCache (routeCache);
    //  dsr->SetRequestTable (rreqTable);
    dsr->SetNode(node);
    //  node->AggregateObject (routeCache);
    //  node->AggregateObject (rreqTable);
}

void
DsrMainHelper::SetDsrHelper(DsrHelper& dsrHelper)
{
    NS_LOG_FUNCTION(this);
    delete m_dsrHelper;
    m_dsrHelper = dsrHelper.Copy();
}

} // namespace ns3
