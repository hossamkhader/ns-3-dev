/*
 * Copyright (c) 2009 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Leonard Tracy <lentracy@gmail.com>
 */

#include "mac8-address.h"

#include "ns3/address.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Mac8Address");

uint8_t Mac8Address::m_allocationIndex = 0;

Mac8Address::Mac8Address(uint8_t addr)
    : m_address(addr)
{
}

Mac8Address::~Mac8Address()
{
}

uint8_t
Mac8Address::GetType()
{
    static uint8_t type = Address::Register();
    return type;
}

Address
Mac8Address::ConvertTo() const
{
    return Address(GetType(), &m_address, 1);
}

Mac8Address
Mac8Address::ConvertFrom(const Address& address)
{
    NS_ASSERT(IsMatchingType(address));
    Mac8Address uAddr;
    address.CopyTo(&uAddr.m_address);
    return uAddr;
}

bool
Mac8Address::IsMatchingType(const Address& address)
{
    return address.CheckCompatible(GetType(), 1);
}

Mac8Address::operator Address() const
{
    return ConvertTo();
}

void
Mac8Address::CopyFrom(const uint8_t* pBuffer)
{
    m_address = *pBuffer;
}

void
Mac8Address::CopyTo(uint8_t* pBuffer) const
{
    *pBuffer = m_address;
}

Mac8Address
Mac8Address::GetBroadcast()
{
    return Mac8Address(255);
}

Mac8Address
Mac8Address::Allocate()
{
    NS_LOG_FUNCTION_NOARGS();

    if (m_allocationIndex == 0)
    {
        Simulator::ScheduleDestroy(Mac8Address::ResetAllocationIndex);
    }

    uint8_t address = m_allocationIndex++;
    if (m_allocationIndex == 255)
    {
        m_allocationIndex = 0;
    }

    return Mac8Address(address);
}

void
Mac8Address::ResetAllocationIndex()
{
    NS_LOG_FUNCTION_NOARGS();

    m_allocationIndex = 0;
}

bool
operator<(const Mac8Address& a, const Mac8Address& b)
{
    return a.m_address < b.m_address;
}

bool
operator==(const Mac8Address& a, const Mac8Address& b)
{
    return a.m_address == b.m_address;
}

bool
operator!=(const Mac8Address& a, const Mac8Address& b)
{
    return !(a == b);
}

std::ostream&
operator<<(std::ostream& os, const Mac8Address& address)
{
    os << (int)address.m_address;
    return os;
}

std::istream&
operator>>(std::istream& is, Mac8Address& address)
{
    uint8_t x;
    is >> x;
    NS_ASSERT(0 <= x);
    NS_ASSERT(x <= 255);
    address.m_address = x;
    return is;
}

} // namespace ns3
