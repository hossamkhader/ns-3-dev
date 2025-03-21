/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#ifndef ENERGY_SOURCE_CONTAINER_H
#define ENERGY_SOURCE_CONTAINER_H

#include "ns3/energy-source.h"
#include "ns3/object.h"

#include <stdint.h>
#include <vector>

namespace ns3
{
namespace energy
{

/**
 * @ingroup energy
 * @brief Holds a vector of ns3::EnergySource pointers.
 *
 * EnergySourceHelper returns a list of EnergySource pointers installed onto a
 * node. Users can use this list to access EnergySource objects to obtain total
 * energy consumption on a node easily.
 *
 * @see NetDeviceContainer
 *
 */
class EnergySourceContainer : public Object
{
  public:
    /// Const iterator for EnergySource container
    typedef std::vector<Ptr<EnergySource>>::const_iterator Iterator;

  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Creates an empty EnergySourceContainer.
     */
    EnergySourceContainer();
    ~EnergySourceContainer() override;

    /**
     * @param source Pointer to an EnergySource.
     *
     * Creates an EnergySourceContainer with exactly one EnergySource previously
     * instantiated.
     */
    EnergySourceContainer(Ptr<EnergySource> source);

    /**
     * @param sourceName Name of EnergySource.
     *
     * Creates an EnergySourceContainer with exactly one EnergySource previously
     * instantiated and assigned a name using the Object name service. This
     * EnergySource is specified by its assigned name.
     */
    EnergySourceContainer(std::string sourceName);

    /**
     * @param a A EnergySourceContainer.
     * @param b Another EnergySourceContainer.
     *
     * Creates an EnergySourceContainer by concatenating EnergySourceContainer b
     * to EnergySourceContainer a.
     *
     * @note Can be used to concatenate 2 Ptr<EnergySource> directly. C++ will be
     * calling EnergySourceContainer constructor with Ptr<EnergySource> first.
     */
    EnergySourceContainer(const EnergySourceContainer& a, const EnergySourceContainer& b);

    /**
     * @brief Get an iterator which refers to the first EnergySource pointer in
     * the container.
     *
     * @returns An iterator which refers to the first EnergySource in container.
     *
     * EnergySources can be retrieved from the container in two ways. First,
     * directly by an index into the container, and second, using an iterator.
     * This method is used in the iterator method and is typically used in a
     * for-loop to run through the EnergySources.
     *
     * @code
     *   EnergySourceContainer::Iterator i;
     *   for (i = container.Begin (); i != container.End (); ++i)
     *     {
     *       (*i)->method ();  // some EnergySource method
     *     }
     * @endcode
     */
    Iterator Begin() const;

    /**
     * @brief Get an iterator which refers to the last EnergySource pointer in
     * the container.
     *
     * @returns An iterator which refers to the last EnergySource in container.
     *
     * EnergySources can be retrieved from the container in two ways. First,
     * directly by an index into the container, and second, using an iterator.
     * This method is used in the iterator method and is typically used in a
     * for-loop to run through the EnergySources.
     *
     * @code
     *   EnergySourceContainer::Iterator i;
     *   for (i = container.Begin (); i != container.End (); ++i)
     *     {
     *       (*i)->method ();  // some EnergySource method
     *     }
     * @endcode
     */
    Iterator End() const;

    /**
     * @brief Get the number of Ptr<EnergySource> stored in this container.
     *
     * @returns The number of Ptr<EnergySource> stored in this container.
     */
    uint32_t GetN() const;

    /**
     * @brief Get the i-th Ptr<EnergySource> stored in this container.
     *
     * @param i Index of the requested Ptr<EnergySource>.
     * @returns The requested Ptr<EnergySource>.
     */
    Ptr<EnergySource> Get(uint32_t i) const;

    /**
     * @param container Another EnergySourceContainer to append.
     *
     * Appends the contents of another EnergySourceContainer to the end of this
     * EnergySourceContainer.
     */
    void Add(EnergySourceContainer container);

    /**
     * @brief Append a single Ptr<EnergySource> to the end of this container.
     *
     * @param source Pointer to an EnergySource.
     */
    void Add(Ptr<EnergySource> source);

    /**
     * @brief Append a single Ptr<EnergySource> referred to by its object name to
     * the end of this container.
     *
     * @param sourceName Name of EnergySource object.
     */
    void Add(std::string sourceName);

  private:
    void DoDispose() override;

    /**
     * @brief Calls Object::Start () for all EnergySource objects.
     */
    void DoInitialize() override;

  private:
    std::vector<Ptr<EnergySource>> m_sources; //!< Energy source container
};

} // namespace energy
} // namespace ns3

#endif /* ENERGY_SOURCE_CONTAINER_H */
