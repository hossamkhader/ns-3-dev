/*
 * Copyright (c) 2013 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mitch Watrous (watrous@u.washington.edu)
 */

#include "time-series-adaptor.h"

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

#include <cfloat>
#include <cmath>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TimeSeriesAdaptor");

NS_OBJECT_ENSURE_REGISTERED(TimeSeriesAdaptor);

TypeId
TimeSeriesAdaptor::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TimeSeriesAdaptor")
                            .SetParent<DataCollectionObject>()
                            .SetGroupName("Stats")
                            .AddConstructor<TimeSeriesAdaptor>()
                            .AddTraceSource("Output",
                                            "The current simulation time versus "
                                            "the current value converted to a double",
                                            MakeTraceSourceAccessor(&TimeSeriesAdaptor::m_output),
                                            "ns3::TimeSeriesAdaptor::OutputTracedCallback");
    return tid;
}

TimeSeriesAdaptor::TimeSeriesAdaptor()
{
    NS_LOG_FUNCTION(this);
}

TimeSeriesAdaptor::~TimeSeriesAdaptor()
{
    NS_LOG_FUNCTION(this);
}

void
TimeSeriesAdaptor::TraceSinkDouble(double oldData, double newData)
{
    NS_LOG_FUNCTION(this << oldData << newData);

    // Don't do anything if the time series adaptor is not enabled.
    if (!IsEnabled())
    {
        NS_LOG_DEBUG("Time series adaptor not enabled");
        return;
    }

    // Time stamp the value with the current time in seconds.
    m_output(Simulator::Now().GetSeconds(), newData);
}

void
TimeSeriesAdaptor::TraceSinkBoolean(bool oldData, bool newData)
{
    NS_LOG_FUNCTION(this << oldData << newData);

    // Call the trace sink that actually does something.
    TraceSinkDouble(oldData, newData);
}

void
TimeSeriesAdaptor::TraceSinkUinteger8(uint8_t oldData, uint8_t newData)
{
    NS_LOG_FUNCTION(this << oldData << newData);

    // Call the trace sink that actually does something.
    TraceSinkDouble(oldData, newData);
}

void
TimeSeriesAdaptor::TraceSinkUinteger16(uint16_t oldData, uint16_t newData)
{
    NS_LOG_FUNCTION(this << oldData << newData);

    // Call the trace sink that actually does something.
    TraceSinkDouble(oldData, newData);
}

void
TimeSeriesAdaptor::TraceSinkUinteger32(uint32_t oldData, uint32_t newData)
{
    NS_LOG_FUNCTION(this << oldData << newData);

    // Call the trace sink that actually does something.
    TraceSinkDouble(oldData, newData);
}

} // namespace ns3
