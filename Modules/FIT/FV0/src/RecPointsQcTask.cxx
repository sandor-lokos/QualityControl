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
/// \author My Name
///

#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TROOT.h>

#include "QualityControl/QcInfoLogger.h"
#include "FV0/RecPointsQcTask.h"
#include <Framework/InputRecord.h>
#include <Framework/InputRecordWalker.h>
#include <Framework/DataRefUtils.h>

#include "DataFormatsFV0/RecPoints.h"
#include "DataFormatsFV0/ChannelData.h"

#include <Framework/InputRecord.h>
#include "FITCommon/HelperHist.h"
#include "FITCommon/HelperCommon.h"

namespace o2::quality_control_modules::fv0
{
	
using namespace o2::quality_control_modules::fit;

RecPointsQcTask::~RecPointsQcTask()
{
	// delete mHistogram
}

void RecPointsQcTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  // THUS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.

  // This is how logs are created. QcInfoLogger is used. In production, FairMQ logs will go to InfoLogger as well.
  ILOG(Debug, Devel) << "initialize RecPointsQcTask" << ENDM;
  ILOG(Debug, Support) << "A debug targeted for support" << ENDM;
  ILOG(Info, Ops) << "An Info log targeted for operators" << ENDM;

  // This creates and registers a histogram for publication at the end of each cycle, until the end of the task lifetime
  mHistogram = std::make_unique<TH1F>("Time_test", "FV0 time", 40, -20, 20);
  getObjectsManager()->startPublishing(mHistogram.get());

  // try {
    // getObjectsManager()->addMetadata(mHistogram->GetName(), "custom", "34");
  // } catch (...) {
    // some methods can throw exceptions, it is indicated in their doxygen.
    // In case it is recoverable, it is recommended to catch them and do something meaningful.
    // Here we don't care that the metadata was not added and just log the event.
    // ILOG(Warning, Support) << "Metadata could not be added to " << mHistogram->GetName() << ENDM;
  // }
}

void RecPointsQcTask::startOfActivity(const Activity& activity)
{
  // THIS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.
  // ILOG(Debug, Devel) << "startOfActivity " << activity.mId << ENDM;

  // We clean any histograms that could have been filled in previous runs.
  mHistogram->Reset();

  // This creates and registers a histogram for publication at the end of each cycle until and including the end of run.
  // Typically you may create and register a histogram here if you require Activity information for binning or decoration.
  // Since ROOT does not respond well to having two histograms with the same name in the memory,
  // we invoke the reset() to first delete the object that could remained from a previous run.

  // Example of retrieving a custom parameter based on the run type + beam type available in Activity
  // std::string parameter;
  // first we try for the current activity. It returns an optional.
  // if (auto param = mCustomParameters.atOptional("myOwnKey", activity)) {
    // parameter = param.value(); // we got a value
  // } else {
    // we did not get a value. We ask for the "default" runtype and beamtype and we specify a default return value.
    // parameter = mCustomParameters.atOrDefaultValue("myOwnKey", "some default");
  // }
}

void RecPointsQcTask::startOfCycle()
{
  // THUS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.
  ILOG(Debug, Devel) << "startOfCycle" << ENDM;
}

void RecPointsQcTask::monitorData(o2::framework::ProcessingContext& ctx)
{
  ILOG(Debug, Devel) << "Entering monitorData()  " << ENDM;	

	auto recpoints = ctx.inputs().get<gsl::span<o2::fv0::RecPoints>>("recpoints");
  auto channels = ctx.inputs().get<gsl::span<o2::fv0::ChannelDataFloat>>("channels");

  for (const auto& recpoint : recpoints) {
    mHistogram->Fill(recpoint.getCollisionGlobalMeanTime());
  }
	
  // THIS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.

  // In this function you can access data inputs specified in the JSON config file.
  // Typically, you should have asked for data inputs as a direct data source or as a data sampling policy.
  // In both cases you probably used a query such as:
  //   "query": "data:TST/RAWDATA/0"
  // which follows the format <binding>:<dataOrigin>/<dataDescription>[/<subSpecification]
  // Usually inputs should be accessed by their bindings:
  // auto randomData = ctx.inputs().get("data");
  // If you know what type the data is, you can specify it as such:
  // auto channels = ctx.inputs().get<gsl::span<o2::fdd::ChannelData>>("channels");
  // auto digits = ctx.inputs().get<gsl::span<o2::fdd::Digit>>("digits");

  // In our case we will access the header and the payload as a raw array of bytes
  // const auto* header = o2::framework::DataRefUtils::getHeader<header::DataHeader*>(randomData);
  // auto payloadSize = header->payloadSize;

  // as an example, let's fill the histogram A with payload sizes and histogram B with the value in the first byte

  // One can find some examples of using InputRecord at:
  // https://github.com/AliceO2Group/AliceO2/blob/dev/Framework/Core/README.md#using-inputs---the-inputrecord-api
  //
  // One can also iterate over all inputs in a loop:
  // for (const framework::DataRef& input : framework::InputRecordWalker(ctx.inputs())) {
    // do something with input
		// mHistogram->Fill(payloadSize);
  // }
  // To know how to access CCDB objects, please refer to:
  // https://github.com/AliceO2Group/QualityControl/blob/master/doc/Advanced.md#accessing-objects-in-ccdb
  // for GRP objects in particular:
  // https://github.com/AliceO2Group/QualityControl/blob/master/doc/Advanced.md#access-grp-objects-with-grp-geom-helper
}

void RecPointsQcTask::endOfCycle()
{
  // THIS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.
  ILOG(Debug, Devel) << "endOfCycle" << ENDM;
}

void RecPointsQcTask::endOfActivity(const Activity& /*activity*/)
{
  // THIS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.
  ILOG(Debug, Devel) << "endOfActivity" << ENDM;
}

void RecPointsQcTask::reset()
{
  // THIS FUNCTION BODY IS AN EXAMPLE. PLEASE REMOVE EVERYTHING YOU DO NOT NEED.

  // Clean all the monitor objects here.
  ILOG(Debug, Devel) << "Resetting the histograms" << ENDM;
  if (mHistogram) {
    mHistogram->Reset();
  }
}

} // namespace o2::quality_control_modules::fv0
