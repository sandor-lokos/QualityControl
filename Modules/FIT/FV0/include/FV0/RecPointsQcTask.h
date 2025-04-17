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
/// \author Sandor Lokos, sandor.lokos@cern.ch
///

#ifndef QC_MODULE_FV0_FV0RECPOINTSQCTASK_H
#define QC_MODULE_FV0_FV0RECPOINTSQCTASK_H

#include "QualityControl/TaskInterface.h"
#include <memory>
#include <Framework/InputRecord.h>

#include "QualityControl/QcInfoLogger.h"
#include <DataFormatsFV0/RecPoints.h>
#include <DataFormatsFV0/ChannelData.h>
#include "QualityControl/TaskInterface.h"
#include "FITCommon/DetectorFIT.h"
#include <memory>
#include <regex>
#include <set>
#include <map>
#include <string>
#include <array>
#include <type_traits>
#include <boost/algorithm/string.hpp>
#include <TH1.h>
#include <TH2.h>
#include <TList.h>
#include <Rtypes.h>
#include "FV0Base/Constants.h"

class TH1F;

using namespace o2::quality_control::core;

namespace o2::quality_control_modules::fv0
{

/// \brief Example Quality Control Task
/// \author My Name
class RecPointsQcTask final : public TaskInterface
{
	constexpr static std::size_t sNCHANNELS_FV0 = o2::fv0::Constants::nFv0Channels;
	constexpr static std::size_t sNCHANNELS_FV0_PLUSREF = o2::fv0::Constants::nFv0ChannelsPlusRef;
	
	public:
		/// \brief Constructor
		RecPointsQcTask() = default;
		/// Destructor
		~RecPointsQcTask() override;

		// Definition of the methods for the template method pattern
		void initialize(o2::framework::InitContext& ctx) override;
		void startOfActivity(const Activity& activity) override;
		void startOfCycle() override;
		void monitorData(o2::framework::ProcessingContext& ctx) override;
		void endOfCycle() override;
		void endOfActivity(const Activity& activity) override;
		void reset() override;
		using Detector_t = o2::quality_control_modules::fit::detectorFIT::DetectorFV0;

	private:
		std::unique_ptr<TH2F> mHistTime2Ch = nullptr;
		std::unique_ptr<TH1F> mHistFraction = nullptr;
		std::unique_ptr<TH1I> mHistChannel = nullptr;
		std::unique_ptr<TH1I> mHistChStat = nullptr;
		std::unique_ptr<TH1I> mHistAvgTime = nullptr;
		std::unique_ptr<TH1I> mHistAvgAmpl = nullptr;
		std::unique_ptr<TH1I> mHistSumAmp = nullptr;
		std::unique_ptr<TH2F> mHistAmpl2Ch = nullptr;
		std::unique_ptr<TH1F> mHistTrigger = nullptr;
		std::unique_ptr<TH2F> mHistTrigBits2Ch = nullptr;
		std::unique_ptr<TH2F> mChanTrigBitsCorr = nullptr;
		std::unique_ptr<TH1F> mIntChanTrigBits = nullptr;
		typename Detector_t::TrgMap_t mTechMapTrgBits = Detector_t::sMapTechTrgBits;
		const char * channelTrigBits[8] = { "kNumberADC",
																			  "kIsDoubleEvent",
																			  "kIsTimeInfoNOTvalid",
																			  "kIsCFDinADCgate",
																			  "kIsTimeInfoLate",
																			  "kIsAmpHigh",
																			  "kIsEventInTVDC",
																			  "kIsTimeInfoLost"
																			};
																			
};

} // namespace o2::quality_control_modules::fv0

#endif // QC_MODULE_FV0_FV0RECPOINTSQCTASK_H