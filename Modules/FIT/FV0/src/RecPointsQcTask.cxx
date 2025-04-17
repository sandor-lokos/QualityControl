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
/// \author Sandor Lokos, sandor.lokos@cern.ch
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

RecPointsQcTask::~RecPointsQcTask() {
	
}

void RecPointsQcTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  ILOG(Debug, Devel) << "initialize RecPointsQcTask" << ENDM;
  ILOG(Debug, Support) << "A debug targeted for support" << ENDM;
  ILOG(Info, Ops) << "An Info log targeted for operators" << ENDM;

  mHistTime2Ch = std::make_unique<TH2F>("TimePerChannel", "FV0 Time vs Channel;Channel;Time (ns)", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 240, -30, 30);
	mHistFraction = std::make_unique<TH1F>("TimeInWindowFraction", "FV0 Fraction of events with CFD in time gate(-192,192) vs ChannelID ;Channel", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
	mHistChannel = std::make_unique<TH1I>("ChannelOccupancy", "FV0 Number of channels;N_{ch};", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
	mHistChStat = std::make_unique<TH1I>("ChannelStat", "FV0 ChannelID stat;N_{ch};", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF);
	mHistAvgTime = std::make_unique<TH1I>("AvgTime", "FV0 Average time ;", 240, -30, 30);
	mHistAvgAmpl = std::make_unique<TH1I>("AvgAmpl", "FV0 Average amplitude ;", 1000, 0, 1000);
	mHistSumAmp = std::make_unique<TH1I>("SumAmp", "FV0 sum of amplitude ;", 10000, 0, 10000);
	mHistAmpl2Ch = std::make_unique<TH2F>("AmplPerChannel", "FV0 Ampl vs Channel;Channel;Amplitude", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 2000, 0, 2000);
	mHistTrigger = std::make_unique<TH1F>("TriggerSignals", "Triggers signals ;", 8, 0.5, 8.5);
	mHistTrigBits2Ch = std::make_unique<TH2F>("ChTrigPerChannel", "Channel trigger bits frequency in channels;", sNCHANNELS_FV0_PLUSREF, 0, sNCHANNELS_FV0_PLUSREF, 256, 0.5, 256.5);
	mIntChanTrigBits = std::make_unique<TH1F>("IntChanTrigBits", "Channel trigger bits relative frequency", 256, 0.5, 256.5);
  mHistTime2Ch->SetOption("colz");
  mHistAmpl2Ch->SetOption("colz");
  mHistTrigBits2Ch->SetOption("colz");
  
	for (const auto& entry : mTechMapTrgBits) {
		mHistTrigger->GetXaxis()->SetBinLabel(entry.first + 1, entry.second.c_str());
	}
	
  getObjectsManager()->startPublishing(mHistTime2Ch.get());
  getObjectsManager()->startPublishing(mHistFraction.get());
	getObjectsManager()->startPublishing(mHistChannel.get());
	getObjectsManager()->startPublishing(mHistChStat.get());
	getObjectsManager()->startPublishing(mHistAvgTime.get());
	getObjectsManager()->startPublishing(mHistAvgAmpl.get());
	getObjectsManager()->startPublishing(mHistSumAmp.get());
	getObjectsManager()->startPublishing(mHistAmpl2Ch.get());
	getObjectsManager()->startPublishing(mHistTrigger.get());
	getObjectsManager()->startPublishing(mHistTrigBits2Ch.get());
	getObjectsManager()->startPublishing(mIntChanTrigBits.get());
	// getObjectsManager()->startPublishing(mChanTrigBitsCorr.get());
}

void RecPointsQcTask::startOfActivity(const Activity& activity)
{
  mHistTime2Ch->Reset();
  mHistFraction->Reset();
  mHistChannel->Reset();
  mHistChStat->Reset();
  mHistAvgTime->Reset();
  mHistAvgAmpl->Reset();
  mHistSumAmp->Reset();
  mHistAmpl2Ch->Reset();
  mHistTrigger->Reset();
  mHistTrigBits2Ch->Reset();
  mIntChanTrigBits->Reset();
}

void RecPointsQcTask::startOfCycle() {
  ILOG(Debug, Devel) << "startOfCycle" << ENDM;
}

void RecPointsQcTask::monitorData(o2::framework::ProcessingContext& ctx) {
	
  ILOG(Debug, Devel) << "Entering monitorData()  " << ENDM;	

	auto recpoints = ctx.inputs().get<gsl::span<o2::fv0::RecPoints>>("recpoints");
  auto channels = ctx.inputs().get<gsl::span<o2::fv0::ChannelDataFloat>>("channels");
	// int i = 0;
	// int j = 0;
  for (const auto& recpoint : recpoints) {
    o2::fit::Triggers triggersignals = recpoint.getTrigger() ;
		
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
		
    for (const auto& chData : vecChData) {
			int chainQTC = (int)chData.adcId ;
			int triggerInt = 0;
			double time = static_cast<Double_t>(chData.time) ;
			double ampl = static_cast<Double_t>(chData.charge) ;
			double channel = static_cast<Double_t>(chData.channel) ;

			mHistTrigBits2Ch->Fill( channel , chainQTC );
			sumTime += time;
			sumAmpl += ampl;
			mHistChStat->Fill( channel );
			// i++;
			if( chainQTC == 8 || chainQTC == 9 )
			{
				// j++;
				mHistFraction->Fill( channel );
			}
			mHistTime2Ch->Fill( channel , time );
			mHistAmpl2Ch->Fill( channel , ampl );
			
    }
		
		mHistSumAmp->Fill( sumAmpl );
		mHistAvgTime->Fill( sumTime / numCh );
		mHistAvgAmpl->Fill( sumAmpl / numCh );
  }
	mHistFraction->Divide(mHistChStat.get());
	// std::cerr << j << " / " << i << " = " << (float)j/i << std::endl;
}

void RecPointsQcTask::endOfCycle() {
  ILOG(Debug, Devel) << "endOfCycle" << ENDM;
}

void RecPointsQcTask::endOfActivity(const Activity& /*activity*/) {
  ILOG(Debug, Devel) << "endOfActivity" << ENDM;
}

void RecPointsQcTask::reset() {
  // Clean all the monitor objects here.
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
	if (mHistTrigBits2Ch) {
    mHistTrigBits2Ch->Reset();
  }
	if (mIntChanTrigBits) {
    mIntChanTrigBits->Reset();
  }	
}

} // namespace o2::quality_control_modules::fv0
