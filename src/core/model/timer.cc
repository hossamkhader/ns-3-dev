/*
 * Copyright (c) 2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "timer.h"

#include "log.h"
#include "simulation-singleton.h"
#include "simulator.h"

/**
 * @file
 * @ingroup timer
 * ns3::Timer implementation.
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Timer");

Timer::Timer()
    : m_flags(CHECK_ON_DESTROY),
      m_delay(),
      m_event(),
      m_impl(nullptr)
{
    NS_LOG_FUNCTION(this);
}

Timer::Timer(DestroyPolicy destroyPolicy)
    : m_flags(destroyPolicy),
      m_delay(),
      m_event(),
      m_impl(nullptr)
{
    NS_LOG_FUNCTION(this << destroyPolicy);
}

Timer::~Timer()
{
    NS_LOG_FUNCTION(this);
    if (m_flags & CHECK_ON_DESTROY)
    {
        if (m_event.IsPending())
        {
            NS_FATAL_ERROR("Event is still running while destroying.");
        }
    }
    else if (m_flags & CANCEL_ON_DESTROY)
    {
        m_event.Cancel();
    }
    else if (m_flags & REMOVE_ON_DESTROY)
    {
        m_event.Remove();
    }
    delete m_impl;
}

void
Timer::SetDelay(const Time& time)
{
    NS_LOG_FUNCTION(this << time);
    m_delay = time;
}

Time
Timer::GetDelay() const
{
    NS_LOG_FUNCTION(this);
    return m_delay;
}

Time
Timer::GetDelayLeft() const
{
    NS_LOG_FUNCTION(this);
    switch (GetState())
    {
    case Timer::RUNNING:
        return Simulator::GetDelayLeft(m_event);
    case Timer::EXPIRED:
        return TimeStep(0);
    case Timer::SUSPENDED:
        return m_delayLeft;
    default:
        NS_ASSERT(false);
        return TimeStep(0);
    }
}

void
Timer::Cancel()
{
    NS_LOG_FUNCTION(this);
    m_event.Cancel();
}

void
Timer::Remove()
{
    NS_LOG_FUNCTION(this);
    m_event.Remove();
}

bool
Timer::IsExpired() const
{
    NS_LOG_FUNCTION(this);
    return !IsSuspended() && m_event.IsExpired();
}

bool
Timer::IsRunning() const
{
    NS_LOG_FUNCTION(this);
    return !IsSuspended() && m_event.IsPending();
}

bool
Timer::IsSuspended() const
{
    NS_LOG_FUNCTION(this);
    return (m_flags & TIMER_SUSPENDED) == TIMER_SUSPENDED;
}

Timer::State
Timer::GetState() const
{
    NS_LOG_FUNCTION(this);
    if (IsRunning())
    {
        return Timer::RUNNING;
    }
    else if (IsExpired())
    {
        return Timer::EXPIRED;
    }
    else
    {
        NS_ASSERT(IsSuspended());
        return Timer::SUSPENDED;
    }
}

void
Timer::Schedule()
{
    NS_LOG_FUNCTION(this);
    Schedule(m_delay);
}

void
Timer::Schedule(Time delay)
{
    NS_LOG_FUNCTION(this << delay);
    NS_ASSERT(m_impl != nullptr);
    if (m_event.IsPending())
    {
        NS_FATAL_ERROR("Event is still running while re-scheduling.");
    }
    m_event = m_impl->Schedule(delay);
}

void
Timer::Suspend()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(IsRunning());
    m_delayLeft = Simulator::GetDelayLeft(m_event);
    if (m_flags & CANCEL_ON_DESTROY)
    {
        m_event.Cancel();
    }
    else if (m_flags & REMOVE_ON_DESTROY)
    {
        m_event.Remove();
    }
    m_flags |= TIMER_SUSPENDED;
}

void
Timer::Resume()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_flags & TIMER_SUSPENDED);
    m_event = m_impl->Schedule(m_delayLeft);
    m_flags &= ~TIMER_SUSPENDED;
}

} // namespace ns3
