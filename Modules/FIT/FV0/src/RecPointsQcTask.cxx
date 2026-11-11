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
/// \file   RecPointsQcTask.cxx
/// \brief Implementation of the FV0 reconstructed-points QC task.
/// \author Sandor Lokos, sandor.lokos@cern.ch
///

#include "FV0/RecPointsQcTask.h"

#include <gsl/span>
#include <cstdint>
#include <string>

#include "DataFormatsFIT/Triggers.h"
#include "DataFormatsFV0/ChannelData.h"
#include "DataFormatsFV0/RecPoints.h"
#include "FITCommon/HelperHist.h"
#include "Framework/InputRecord.h"
#include "QualityControl/QcInfoLogger.h"

namespace o2::quality_control_modules::fv0
{

using o2::quality_control::core::Activity;
using o2::quality_control::core::PublicationPolicy;
using namespace o2::quality_control_modules::fit;

RecPointsQcTask::~RecPointsQcTask()
{
}

void RecPointsQcTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  ILOG(Debug, Devel) << "initialize RecPointsQcTask" << ENDM;

  mHistTime2Ch = helper::registerHist<TH2F>(getObjectsManager(), PublicationPolicy::Forever, "", "TimePerChannel", "FV0 Time vs Channel;Channel;Time (ns)", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 240, -30, 30);
  mHistFraction = helper::registerHist<TH1F>(getObjectsManager(), PublicationPolicy::Forever, "", "TimeInWindowFraction", "FV0 Fraction of events with CFD in time gate(-192,192) vs ChannelID ;Channel", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
  mHistChannel = helper::registerHist<TH1I>(getObjectsManager(), PublicationPolicy::Forever, "", "ChannelOccupancy", "FV0 Number of channels;N_{ch};", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
  mHistChStat = helper::registerHist<TH1I>(getObjectsManager(), PublicationPolicy::Forever, "", "ChannelStat", "FV0 ChannelID stat;N_{ch};", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
  mHistAvgAmpl = helper::registerHist<TH1I>(getObjectsManager(), PublicationPolicy::Forever, "", "AvgAmpl", "FV0 Average amplitude ;", 1000, 0, 1000);
  mHistSumAmp = helper::registerHist<TH1I>(getObjectsManager(), PublicationPolicy::Forever, "", "SumAmp", "FV0 sum of amplitude ;", 10000, 0, 10000);
  mHistAmpl2Ch = helper::registerHist<TH2F>(getObjectsManager(), PublicationPolicy::Forever, "", "AmplPerChannel", "FV0 Ampl vs Channel;Channel;Amplitude", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 2000, 0, 2000);
  mHistTrigger = helper::registerHist<TH1F>(getObjectsManager(), PublicationPolicy::Forever, "", "TriggerSignals", "Triggers signals ;", 8, 0.5, 8.5);
  mHistTrigBits2Ch = helper::registerHist<TH2F>(getObjectsManager(), PublicationPolicy::Forever, "", "ChTrigPerChannel", "Channel trigger bits frequency in channels;", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 256, 0.5, 256.5);
  mIntChanTrigBits = helper::registerHist<TH1F>(getObjectsManager(), PublicationPolicy::Forever, "", "IntChanTrigBits", "Channel trigger bits relative frequency", 256, 0.5, 256.5);
  mHistTime2Ch->SetOption("colz");
  mHistAmpl2Ch->SetOption("colz");
  mHistTrigBits2Ch->SetOption("colz");

  // Use the FIT factory to create the task's private numerator.
  mHistCfdGateCounts = helper::makeHist<TH1I>("CfdGateCounts", "", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
  // Detach from ROOT's directory so the task controls the histogram's lifetime.
  mHistCfdGateCounts->SetDirectory(nullptr);

  mHistAvgTime = helper::registerHist<TH1I>(getObjectsManager(), PublicationPolicy::Forever, "", "AvgTime", "FV0 Average time ;", 240, -30, 30);

  for (const auto& entry : mTechMapTrgBits) {
    mHistTrigger->GetXaxis()->SetBinLabel(entry.first + 1, entry.second.c_str());
  }

  for (std::size_t channel = 0; channel < mHistTimeVsAmpl.size(); ++channel) {
    const auto channelSuffix = std::to_string(channel);
    mHistTimeVsAmpl[channel] = helper::registerHist<TH2F>(getObjectsManager(), PublicationPolicy::Forever, "", "TimeVsAmplitudeChannel" + channelSuffix, "FV0 time vs amplitude, channel " + channelSuffix + ";Amplitude;Time (ns)", 2000, 0, 2000, 240, -30, 30);
    mHistTimeVsAmpl[channel]->SetOption("colz");
  }

  // getObjectsManager()->startPublishing(mChanTrigBitsCorr.get());
}

void RecPointsQcTask::startOfActivity(const Activity& /*activity*/)
{
  mHistTime2Ch->Reset();
  mHistFraction->Reset();
  mHistChannel->Reset();
  // Start the activity with an empty numerator and denominator.
  mHistChStat->Reset();
  mHistCfdGateCounts->Reset();
  mHistAvgTime->Reset();
  mHistAvgAmpl->Reset();
  mHistSumAmp->Reset();
  mHistAmpl2Ch->Reset();
  mHistTrigger->Reset();
  mHistTrigBits2Ch->Reset();
  mIntChanTrigBits->Reset();
  // Clear all per-channel time-amplitude correlations.
  for (auto& hist : mHistTimeVsAmpl) {
    if (hist) {
      hist->Reset();
    }
  }
}

void RecPointsQcTask::startOfCycle()
{
}

void RecPointsQcTask::monitorData(o2::framework::ProcessingContext& ctx)
{

  auto recpoints = ctx.inputs().get<gsl::span<o2::fv0::RecPoints>>("recpoints");
  auto channels = ctx.inputs().get<gsl::span<o2::fv0::ChannelDataFloat>>("channels");
  for (const auto& recpoint : recpoints) {
    o2::fit::Triggers triggersignals = recpoint.getTrigger();

    const auto triggerWord = triggersignals.getTriggersignals();
    for (const auto& entry : mTechMapTrgBits) {
      if ((triggerWord & (1u << entry.first)) != 0) {
        // Preserve the original histogram coordinates 1 through 8.
        mHistTrigger->Fill(entry.first + 1);
      }
    }

    const auto& vecChData = recpoint.getBunchChannelData(channels);
    int numCh = vecChData.size();
    mHistChannel->Fill(numCh);
    double sumTime = 0.;
    double sumAmpl = 0.;

    for (const auto& chData : vecChData) {
      int chainQTC = (int)chData.adcId;
      double time = static_cast<Double_t>(chData.time);
      double ampl = static_cast<Double_t>(chData.charge);
      int channel = chData.channel;

      mHistTrigBits2Ch->Fill(channel, chainQTC);
      sumTime += time;
      sumAmpl += ampl;

      // Accumulate raw counts across all input batches.
      mHistChStat->Fill(channel);

      // adcId contains the full channel-status word.
      // Copy it into ChannelData to use its boolean flag accessor.
      o2::fv0::ChannelData channelFlags;
      channelFlags.setFlag(static_cast<std::uint8_t>(chData.adcId));

      // Count entries with the CFD-in-ADC-gate flag set.
      const bool cfdcheck = channelFlags.getFlag(o2::fv0::ChannelData::kIsCFDinADCgate);
      if (cfdcheck) {
        mHistCfdGateCounts->Fill(channel);
      }

      mHistTime2Ch->Fill(channel, time);
      mHistAmpl2Ch->Fill(channel, ampl);
      // Fill the correlation belonging to this channel.
      if (channel >= 0 && static_cast<std::size_t>(channel) < mHistTimeVsAmpl.size()) {
        mHistTimeVsAmpl[channel]->Fill(ampl, time);
      }
    }

    mHistSumAmp->Fill(sumAmpl);

    // A reconstructed point with no channels has no defined average.
    if (numCh > 0) {
      mHistAvgTime->Fill(sumTime / numCh);
      mHistAvgAmpl->Fill(sumAmpl / numCh);
    }
  }
}

void RecPointsQcTask::endOfCycle()
{
  ILOG(Debug, Devel) << "endOfCycle" << ENDM;
  // Compute the fraction just before publication. The two-input Divide
  // writes the ratio into mHistFraction and preserves both raw counts.
  mHistFraction->Divide(mHistCfdGateCounts.get(), mHistChStat.get());
}

void RecPointsQcTask::endOfActivity(const Activity& /*activity*/)
{
  ILOG(Debug, Devel) << "endOfActivity" << ENDM;
}

void RecPointsQcTask::reset()
{
  // Clear the published histograms and the private numerator
  ILOG(Debug, Devel) << "Resetting the histograms" << ENDM;

  if (mHistTime2Ch) {
    mHistTime2Ch->Reset();
  }
  if (mHistFraction) {
    mHistFraction->Reset();
  }
  if (mHistChannel) {
    mHistChannel->Reset();
  }
  // Reset both counters together so they cover the same accumulation interval.
  if (mHistChStat) {
    mHistChStat->Reset();
  }
  if (mHistCfdGateCounts) {
    mHistCfdGateCounts->Reset();
  }
  if (mHistAvgTime) {
    mHistAvgTime->Reset();
  }
  if (mHistAvgAmpl) {
    mHistAvgAmpl->Reset();
  }
  if (mHistSumAmp) {
    mHistSumAmp->Reset();
  }
  if (mHistAmpl2Ch) {
    mHistAmpl2Ch->Reset();
  }
  if (mHistTrigger) {
    mHistTrigger->Reset();
  }
  if (mHistTrigBits2Ch) {
    mHistTrigBits2Ch->Reset();
  }
  if (mIntChanTrigBits) {
    mIntChanTrigBits->Reset();
  }
  // Clear all per-channel time-amplitude correlations.
  for (auto& hist : mHistTimeVsAmpl) {
    if (hist) {
      hist->Reset();
    }
  }
}

} // namespace o2::quality_control_modules::fv0
