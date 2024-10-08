/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "application-container.h"

#include "ns3/log.h"
#include "ns3/names.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ApplicationContainer");

ApplicationContainer::ApplicationContainer()
{
}

ApplicationContainer::ApplicationContainer(Ptr<Application> app)
{
    m_applications.push_back(app);
}

ApplicationContainer::ApplicationContainer(std::string name)
{
    Ptr<Application> app = Names::Find<Application>(name);
    m_applications.push_back(app);
}

ApplicationContainer::Iterator
ApplicationContainer::Begin() const
{
    return m_applications.begin();
}

ApplicationContainer::Iterator
ApplicationContainer::End() const
{
    return m_applications.end();
}

uint32_t
ApplicationContainer::GetN() const
{
    return m_applications.size();
}

Ptr<Application>
ApplicationContainer::Get(uint32_t i) const
{
    return m_applications[i];
}

void
ApplicationContainer::Add(ApplicationContainer other)
{
    for (auto i = other.Begin(); i != other.End(); i++)
    {
        m_applications.push_back(*i);
    }
}

void
ApplicationContainer::Add(Ptr<Application> application)
{
    m_applications.push_back(application);
}

void
ApplicationContainer::Add(std::string name)
{
    Ptr<Application> application = Names::Find<Application>(name);
    m_applications.push_back(application);
}

void
ApplicationContainer::Start(Time start) const
{
    for (auto i = Begin(); i != End(); ++i)
    {
        Ptr<Application> app = *i;
        app->SetStartTime(start);
    }
}

void
ApplicationContainer::StartWithJitter(Time start, Ptr<RandomVariableStream> rv) const
{
    for (auto i = Begin(); i != End(); ++i)
    {
        Ptr<Application> app = *i;
        double value = rv->GetValue();
        NS_LOG_DEBUG("Start application at time " << start.GetSeconds() + value << "s");
        app->SetStartTime(start + Seconds(value));
    }
}

void
ApplicationContainer::Stop(Time stop) const
{
    for (auto i = Begin(); i != End(); ++i)
    {
        Ptr<Application> app = *i;
        app->SetStopTime(stop);
    }
}

} // namespace ns3
