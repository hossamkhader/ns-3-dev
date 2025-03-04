/*
 * Copyright (c) 2007,2008, 2009 INRIA, UDcast
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

#ifndef COST231_PROPAGATION_LOSS_MODEL_H
#define COST231_PROPAGATION_LOSS_MODEL_H

#include "propagation-loss-model.h"

#include "ns3/nstime.h"

namespace ns3
{

/**
 * @ingroup propagation
 *
 *  @brief The COST-Hata-Model is the most often cited of the COST 231 models.
 *
 *  Also called the Hata Model PCS Extension, it is a radio propagation model
 *  that extends the Hata Model (which in turn is based on the Okumura Model)
 *  to cover a more elaborated range of frequencies. COST (COperation
 *  europ�enne dans le domaine de la recherche Scientifique et Technique)
 *  is a European Union Forum for cooperative scientific research which has
 *  developed this model accordingly to various experiments and researches.
 *  This model is applicable to urban areas. To further evaluate Path Loss
 *  in Suburban or Rural Quasi-open/Open Areas.
 *  Frequency: 1500 MHz to 2000 MHz
 *  Mobile Station Antenna Height: 1 up to 10m
 *  Base station Antenna Height: 30m to 200m
 *  Link Distance:up to 20 km
 *
 */

class Cost231PropagationLossModel : public PropagationLossModel
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    Cost231PropagationLossModel();

    // Delete copy constructor and assignment operator to avoid misuse
    Cost231PropagationLossModel(const Cost231PropagationLossModel&) = delete;
    Cost231PropagationLossModel& operator=(const Cost231PropagationLossModel&) = delete;

    /**
     * Get the propagation loss
     * @param a the mobility model of the source
     * @param b the mobility model of the destination
     * @returns the propagation loss (in dBm)
     */
    double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

    /**
     * Set the BS antenna height
     * @param height BS antenna height [m]
     */
    void SetBSAntennaHeight(double height);
    /**
     * Set the SS antenna height
     * @param height SS antenna height [m]
     */
    void SetSSAntennaHeight(double height);

    /**
     * Set the wavelength
     * @param lambda the wavelength
     */
    void SetLambda(double lambda);
    /**
     * Set the wavelength
     * @param frequency the signal frequency [Hz]
     * @param speed the signal speed [m/s]
     */
    void SetLambda(double frequency, double speed);
    /**
     * Set the minimum model distance
     * @param minDistance the minimum model distance
     */
    void SetMinDistance(double minDistance);
    /**
     * Get the BS antenna height
     * @returns BS antenna height [m]
     */
    double GetBSAntennaHeight() const;
    /**
     * Get the SS antenna height
     * @returns SS antenna height [m]
     */
    double GetSSAntennaHeight() const;
    /**
     * Get the minimum model distance
     * @returns the minimum model distance
     */
    double GetMinDistance() const;
    /**
     * Get the wavelength
     * @returns the wavelength
     */
    double GetLambda() const;
    /**
     * Get the shadowing value
     * @returns the shadowing value
     */
    double GetShadowing() const;
    /**
     * Set the shadowing value
     * @param shadowing the shadowing value
     */
    void SetShadowing(double shadowing);

  private:
    double DoCalcRxPower(double txPowerDbm,
                         Ptr<MobilityModel> a,
                         Ptr<MobilityModel> b) const override;
    int64_t DoAssignStreams(int64_t stream) override;

    double m_BSAntennaHeight; //!< BS Antenna Height [m]
    double m_SSAntennaHeight; //!< SS Antenna Height [m]
    double m_lambda;          //!< The wavelength
    double m_minDistance;     //!< minimum distance [m]
    double m_frequency;       //!< frequency [Hz]
    double m_shadowing;       //!< Shadowing loss [dB]
};

} // namespace ns3

#endif /* COST231PROPAGATIONMODEL_H */
