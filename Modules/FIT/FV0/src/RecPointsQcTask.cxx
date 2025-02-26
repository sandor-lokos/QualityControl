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
	// delete mHistColGloMeanTime
}

void RecPointsQcTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  ILOG(Debug, Devel) << "initialize RecPointsQcTask" << ENDM;
  ILOG(Debug, Support) << "A debug targeted for support" << ENDM;
  ILOG(Info, Ops) << "An Info log targeted for operators" << ENDM;

	// To be implemented:
	//	Done --> * Amplitude vs channel TH2D log-yz
	//	Done --> * Sum of amplitude (TCM) TH1D log-log
	//	Done --> * Time vs channel TH2D log-z
	//	Done --> * Average time (TCM) TH1D
	//	Done --> * Number of channels (TCM)
	//	Done --> * ChannelID stat TH1I
	// * Triggers from TCM
	
  mHistColGloMeanTime = std::make_unique<TH1F>("CollisionGlobalMeanTime", "FV0 global collision time", 400, -2000, 2000);
	mHistTime2Ch = std::make_unique<TH2F>("TimePerChannel", "FV0 Time vs Channel;Channel;Time (ps)", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 4100, -2050, 2050);
	mHistChannel = std::make_unique<TH1I>("ChannelOccupancy", "FV0 Number of channels;N_{ch};", 48, 0, 48);
	mHistChStat = std::make_unique<TH1I>("ChannelStat", "FV0 ChannelID stat;N_{ch};", 48, 0, 48);
	mHistAvgTime = std::make_unique<TH1I>("AvgTime", "FV0 Average time ;", 4000, -2000, 2000);
	mHistAvgAmpl = std::make_unique<TH1I>("AvgAmpl", "FV0 Average amplitude ;", 1000, 0, 1000);
	mHistSumAmp = std::make_unique<TH1I>("SumAmp", "FV0 sum of amplitude ;", 10000, 0, 10000);
	mHistAmpl2Ch = std::make_unique<TH2F>("AmplPerChannel", "FV0 Ampl vs Channel;Channel;Amplitude", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 2000, 0, 2000);
	mHistTrigger = std::make_unique<TH1F>("TriggerSignals", "Triggers from TCM ;", 8, 0.5, 8.5);
  mHistTime2Ch->SetOption("colz");
  mHistAmpl2Ch->SetOption("colz");
	
	for (const auto& entry : mTechMapTrgBits) {
    mHistTrigger->GetXaxis()->SetBinLabel(entry.first + 1, entry.second.c_str());
	}
	
  getObjectsManager()->startPublishing(mHistColGloMeanTime.get());
	getObjectsManager()->startPublishing(mHistTime2Ch.get());
	getObjectsManager()->startPublishing(mHistChannel.get());
	getObjectsManager()->startPublishing(mHistChStat.get());
	getObjectsManager()->startPublishing(mHistAvgTime.get());
	getObjectsManager()->startPublishing(mHistAvgAmpl.get());
	getObjectsManager()->startPublishing(mHistSumAmp.get());
	getObjectsManager()->startPublishing(mHistAmpl2Ch.get());
	getObjectsManager()->startPublishing(mHistTrigger.get());
}

void RecPointsQcTask::startOfActivity(const Activity& activity)
{
  mHistColGloMeanTime->Reset();
  mHistTime2Ch->Reset();
  mHistChannel->Reset();
  mHistChStat->Reset();
  mHistAvgTime->Reset();
  mHistAvgAmpl->Reset();
  mHistSumAmp->Reset();
  mHistAmpl2Ch->Reset();
  mHistTrigger->Reset();
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

  for (const auto& recpoint : recpoints) 
	{
    mHistColGloMeanTime->Fill(recpoint.getCollisionGlobalMeanTime());
		o2::fit::Triggers triggersignals = recpoint.getTrigger() ;
		// std::cerr << "----------> MB: " << triggersignals.getOrA() << 
													 // "\tOR: " << triggersignals.getOrAOut() << 
													 // "\tCN: " << triggersignals.getTrgNChan() << 
													 // "\tCH: " << triggersignals.getTrgCharge() << 
													 // "\tIR: " << triggersignals.getOrAIn() << 
													 // "\tLa: " << triggersignals.getLaser() <<
													 // "\tOB: " << triggersignals.getOutputsAreBlocked() <<
													 // "\tDV: " << triggersignals.getDataIsValid() << std::endl;
		
		if(triggersignals.getOrA()) {
			mHistTrigger->Fill(1);
		}
		if(triggersignals.getOrAOut()) {
			mHistTrigger->Fill(2);
		}
		if(triggersignals.getTrgNChan()) {
			mHistTrigger->Fill(3);
		}
		if(triggersignals.getTrgCharge()) {
			mHistTrigger->Fill(4);
		}
		if(triggersignals.getOrAIn()) {
			mHistTrigger->Fill(5);
		}
		if(triggersignals.getLaser()) {
			mHistTrigger->Fill(6);
		}
		if(triggersignals.getOutputsAreBlocked()) {
			mHistTrigger->Fill(7);
		}
		if(triggersignals.getDataIsValid()) {
			mHistTrigger->Fill(8);
		}
		
		const auto& vecChData = recpoint.getBunchChannelData(channels);
		int numCh = vecChData.size();
		mHistChannel->Fill(numCh);
		double sumTime = 0.;
		double sumAmpl = 0.;
    for (const auto& chData : vecChData) 
		{
			double time = static_cast<Double_t>(chData.time) ;
			double ampl = static_cast<Double_t>(chData.charge) ;
			double channel = static_cast<Double_t>(chData.channel) ;
			sumTime += time;
			sumAmpl += ampl;
			mHistChStat->Fill( channel );
			mHistTime2Ch->Fill( channel , time );
			mHistAmpl2Ch->Fill( channel , ampl );
    }
		
		mHistSumAmp->Fill( sumAmpl );
		mHistAvgTime->Fill( sumTime / numCh );
		mHistAvgAmpl->Fill( sumAmpl / numCh );
  }
	
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
  if (mHistColGloMeanTime) {
    mHistColGloMeanTime->Reset();
  }
	if (mHistTime2Ch) {
    mHistTime2Ch->Reset();
  }
	if (mHistChannel) {
    mHistChannel->Reset();
  }
	if (mHistChStat) {
    mHistChStat->Reset();
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
	
}

} // namespace o2::quality_control_modules::fv0
