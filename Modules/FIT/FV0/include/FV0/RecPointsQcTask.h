// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   RecPointsQcTask.h
/// \brief Definition of the FV0 reconstructed-points QC task.
/// \author Sandor Lokos, sandor.lokos@cern.ch
///

#ifndef QC_MODULE_FV0_FV0RECPOINTSQCTASK_H
#define QC_MODULE_FV0_FV0RECPOINTSQCTASK_H

#include <cstddef>
#include <memory>
#include <array>

#include <TH1.h>
#include <TH2.h>

#include "FITCommon/DetectorFIT.h"
#include "FV0Base/Constants.h"
#include "QualityControl/TaskInterface.h"

namespace o2::quality_control_modules::fv0
{

/// \brief Monitors reconstructed FV0 channel times, amplitudes, and trigger information.
/// \author Sandor Lokos, sandor.lokos@cern.ch
class RecPointsQcTask final : public o2::quality_control::core::TaskInterface
{
  constexpr static std::size_t sNCHANNELS_FV0 = o2::fv0::Constants::nFv0Channels;
  constexpr static std::size_t sNCHANNELS_FV0_PLUSREF = o2::fv0::Constants::nFv0ChannelsPlusRef;

 public:
  /// \brief Constructor
  RecPointsQcTask() = default;
  /// Destructor
  ~RecPointsQcTask() override;

  /// Creates and publishes the monitoring histograms.
  /// \param ctx Framework initialization context.
  void initialize(o2::framework::InitContext& ctx) override;
  /// Resets the monitoring histograms for a new activity.
  /// \param activity Activity being started.
  void startOfActivity(const o2::quality_control::core::Activity& activity) override;
  /// Starts a monitoring cycle.
  void startOfCycle() override;
  /// Processes reconstructed FV0 points and their channel data.
  /// \param ctx Framework context providing the recpoints and channels inputs.
  void monitorData(o2::framework::ProcessingContext& ctx) override;
  /// Ends a monitoring cycle.
  void endOfCycle() override;
  /// Ends the current activity.
  /// \param activity Activity being ended.
  void endOfActivity(const o2::quality_control::core::Activity& activity) override;
  /// Resets the monitoring histograms.
  void reset() override;
  using Detector_t = o2::quality_control_modules::fit::detectorFIT::DetectorFV0;

 private:
  std::unique_ptr<TH2F> mHistTime2Ch;                                          ///< Time distribution versus channel.
  std::unique_ptr<TH1F> mHistFraction;                                         ///< Fraction of channel entries passing the CFD gate and status selection.
  std::unique_ptr<TH1I> mHistCfdGateCounts;                                    ///< Unpublished counts for TimeInWindowFraction.
  std::unique_ptr<TH1I> mHistChannel;                                          ///< Channel multiplicity per reconstructed point.
  std::unique_ptr<TH1I> mHistChStat;                                           ///< Number of entries per channel.
  std::unique_ptr<TH1I> mHistAvgTime;                                          ///< Average channel time per reconstructed point.
  std::unique_ptr<TH1I> mHistAvgAmpl;                                          ///< Average channel amplitude per reconstructed point.
  std::unique_ptr<TH1I> mHistSumAmp;                                           ///< Summed channel amplitude per reconstructed point.
  std::unique_ptr<TH2F> mHistAmpl2Ch;                                          ///< Amplitude distribution versus channel.
  std::unique_ptr<TH1F> mHistTrigger;                                          ///< Counts of event-trigger signals.
  std::unique_ptr<TH2F> mHistTrigBits2Ch;                                      ///< Complete channel-status word versus channel.
  std::unique_ptr<TH2F> mChanTrigBitsCorr;                                     ///< Reserved channel-trigger correlation histogram.
  std::unique_ptr<TH1F> mIntChanTrigBits;                                      ///< Published channel-trigger histogram; no filling is currently implemented.
  typename Detector_t::TrgMap_t mTechMapTrgBits = Detector_t::sMapTechTrgBits; ///< Labels for FV0 event-trigger bits.
  typename Detector_t::TrgMap_t mMapChannelTrigBits = Detector_t::sMapPMbits;  ///< Common FV0 channel-status bit labels.
  std::array<std::unique_ptr<TH2F>, sNCHANNELS_FV0_PLUSREF> mHistTimeVsAmpl;   /// Time versus amplitude for each channel, including the reference channel.
};

} // namespace o2::quality_control_modules::fv0

#endif // QC_MODULE_FV0_FV0RECPOINTSQCTASK_H
