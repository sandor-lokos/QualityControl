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
/// \author My Name
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

class TH1F;

using namespace o2::quality_control::core;

namespace o2::quality_control_modules::fv0
{

/// \brief Example Quality Control Task
/// \author My Name
class RecPointsQcTask final : public TaskInterface
{
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

	private:
		std::unique_ptr<TH1F> mHistogram = nullptr;
};

} // namespace o2::quality_control_modules::fv0

#endif // QC_MODULE_FV0_FV0RECPOINTSQCTASK_H