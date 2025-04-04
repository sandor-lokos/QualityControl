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
/// \file   ITSFeeTask.h
/// \author Jian Liu
/// \author Liang Zhang
/// \author Pietro Fecchio
/// \author Antonio Palasciano
///

#ifndef QC_MODULE_ITS_ITSFEETASK_H
#define QC_MODULE_ITS_ITSFEETASK_H

#include "QualityControl/TaskInterface.h"
#include "Headers/RAWDataHeader.h"
#include "Headers/RDHAny.h"
#include "DetectorsRaw/RDHUtils.h"

#include <TH2Poly.h>
#include <TMath.h>
#include <TLine.h>
#include <TLatex.h>

class TH1;
class TH2;

using namespace o2::quality_control::core;

namespace o2::quality_control_modules::its
{

/// \brief ITS FEE task aiming at 100% online data integrity checking
class ITSFeeTask final : public TaskInterface
{
  struct GBTDiagnosticWord { // DDW GBT word
    union {
      uint64_t word0 = 0x0;
      struct {
        uint64_t laneStatus : 56;
        uint16_t zero0 : 8;
      } laneBits;
    } laneWord;
    union {
      uint64_t word1 = 0x0;
      struct {
        uint8_t flag1 : 4;
        uint8_t index : 4;
        uint8_t id : 8;
        uint64_t padding : 48;
      } indexBits;
    } indexWord;
  };

  // TODO: use proper casting and extend to full word
  struct CalibrationWordUserField { // CDW GBT word user field
    union {
      uint16_t word0 = 0x0;
      struct {
        uint16_t rowid : 9;  // Mask Stage / row
        uint8_t runtype : 7; // Run type
      } content;
    } userField0;
    union {
      uint16_t word1 = 0x0;
      struct {
        uint16_t loopvalue : 16; // Loop value
      } content;
    } userField1;
    union {
      uint16_t word2 = 0x0;
      struct {
        uint16_t confdb : 13; // config version
        uint8_t cdwver : 3;   // CDW version
      } content;
    } userField2;
  };

  struct GBTITSHeaderWord { // IHW GBT word
    union {
      uint64_t word0 = 0x0;
      struct {
        uint32_t activeLanes : 28; // bit mask
        uint64_t reserved : 36;    // nothing
      } laneBits;
    } IHWcontent;
    union {
      uint64_t word1 = 0x0;
      struct {
        uint8_t reserved : 8; // nothing
        uint8_t id : 8;
        uint64_t padding : 48;
      } indexBits;
    } indexWord;
  };

 public:
  /// \brief Constructor
  ITSFeeTask();
  /// Destructor
  ~ITSFeeTask() override;

  void initialize(o2::framework::InitContext& ctx) override;
  void startOfActivity(const Activity& activity) override;
  void startOfCycle() override;
  void monitorData(o2::framework::ProcessingContext& ctx) override;
  void endOfCycle() override;
  void endOfActivity(const Activity& activity) override;
  void reset() override;

 private:
  std::vector<std::pair<int, TString>> mRDHDetField{ // <bit in DetField, description>
                                                     std::make_pair(0, "Missing data"),
                                                     std::make_pair(1, "Warning"),
                                                     std::make_pair(2, "Error"),
                                                     std::make_pair(3, "Fault"),
                                                     std::make_pair(4, "TriggerRamp"),
                                                     std::make_pair(5, "Recovery"),
                                                     std::make_pair(24, "TimebaseUnsyncEvt"),
                                                     std::make_pair(25, "TimebaseEvt"),
                                                     std::make_pair(26, "ClockEvt")
  };

  std::vector<std::pair<int, TString>> mTriggerType{ // <bit in RDH, description>
                                                     std::make_pair(0, "ORBIT"),
                                                     std::make_pair(1, "HB"),
                                                     std::make_pair(2, "HBr"),
                                                     std::make_pair(3, "HC"),
                                                     std::make_pair(4, "PHYSICS"),
                                                     std::make_pair(5, "PP"),
                                                     std::make_pair(6, "CAL"),
                                                     std::make_pair(7, "SOT"),
                                                     std::make_pair(8, "EOT"),
                                                     std::make_pair(9, "SOC"),
                                                     std::make_pair(10, "EOC"),
                                                     std::make_pair(11, "TF"),
                                                     std::make_pair(12, "INT")
  };

  void getParameters(); // get Task parameters from json file
  void setAxisTitle(TH1* object, const char* xTitle, const char* yTitle);
  void createFeePlots();
  void getStavePoint(int layer, int stave, double* px, double* py); // prepare for fill TH2Poly, get all point for add TH2Poly bin
  void setPlotsFormat();
  void drawLayerName(TH2* histo2D);
  void resetGeneralPlots();
  void resetLanePlotsAndCounters(bool isFullReset = false);
  static constexpr int NLayer = 7;
  const int NStaves[NLayer] = { 12, 16, 20, 24, 30, 42, 48 };
  static constexpr int NLayerIB = 3;
  static constexpr int NLanesMax = 28;
  static constexpr int NFees = 48 * 3 + 144 * 2;
  static constexpr int NFlags = 3;
  const int StaveBoundary[NLayer + 1] = { 0, 12, 28, 48, 72, 102, 144, 192 };
  const int NLanePerStaveLayer[NLayer] = { 9, 9, 9, 16, 16, 28, 28 };
  const int LayerBoundaryFEE[NLayer - 1] = { 35, 83, 143, 191, 251, 335 };
  const float StartAngle[7] = { 16.997 / 360 * (TMath::Pi() * 2.), 17.504 / 360 * (TMath::Pi() * 2.), 17.337 / 360 * (TMath::Pi() * 2.), 8.75 / 360 * (TMath::Pi() * 2.), 7 / 360 * (TMath::Pi() * 2.), 5.27 / 360 * (TMath::Pi() * 2.), 4.61 / 360 * (TMath::Pi() * 2.) }; // start angle of first stave in each layer
  const float MidPointRad[7] = { 23.49, 31.586, 39.341, 197.598, 246.944, 345.348, 394.883 };
  const int laneMax[NLayer] = { 108, 144, 180, 384, 480, 1176, 1344 };
  const int NLanesIB = 432, NLanesML = 864, NLanesOL = 2520, NLanesTotal = 3816;
  const int lanesPerFeeId[NLayer] = { 3, 3, 3, 8, 8, 14, 14 };
  const int feePerLayer[NLayer] = { 36, 48, 60, 48, 60, 84, 96 };
  const int StavePerLayer[NLayer] = { 12, 16, 20, 24, 30, 42, 48 };
  const int feePerStave[NLayer] = { 3, 3, 3, 2, 2, 2, 2 };
  const int feeBoundary[NLayer] = { 0, 35, 83, 143, 191, 251, 335 };
  const int indexFeeLow[NLayer] = { 0, 3, 6, 3, 17, 0, 14 };
  const int indexFeeUp[NLayer] = { 3, 6, 9, 11, 25, 14, 28 };
  int mTimeFrameId = 0;
  std::string mLaneStatusFlag[NFlags] = { "WARNING", "ERROR", "FAULT" }; // b00 OK, b01 WARNING, b10 ERROR, b11 FAULT

  int mStatusFlagNumber[7][48][28][3] = { { { 0 } } }; //[iLayer][iStave][iLane][iLaneStatusFlag]
  int mStatusSummaryLayerNumber[7][3] = { { 0 } };     //[iLayer][iflag]
  int mStatusSummaryNumber[4][3] = { { 0 } };          //[summary][iflag] ---> Global, IB, ML, OL

  // parameters taken from the .json
  int mNPayloadSizeBins = 4096;
  bool mResetLaneStatus = false;
  bool mResetPayload = false;
  bool precisePayload = false;
  int mPayloadParseEvery_n_HBF_per_TF = 32; // -1 to disable, 1 to process all the HBFs
  int mPayloadParseEvery_n_TF = 1;          // Use >= 1 values
  bool mEnableIHWReading = 0;
  bool mDecodeCDW = 0;
  int nResetCycle = 1;
  int nCycleID = 0;
  TH1I* mTFInfo; // count vs TF ID
  TH2I* mTriggerVsFeeId;
  TH2I* mTriggerVsFeeId_reset;
  TH1I* mTrigger;
  TH2I* mFlag1Check;    // include transmission_timeout, packet_overflow, lane_starts_violation
  TH2I* mDecodingCheck; // summary of errors during custom decoding of specific bytes (see plot description)
  TH2I* mRDHSummary;
  TH2I* mRDHSummaryCumulative; // RDH plot which does NOT reset at every QC cycle
  TH2I* mLaneStatus[NFlags];   // 4 flags for each lane. 3/8/14 lane for each link. 3/2/2 link for each RU.
  // Misconfiguration plot
  TH2I* mTrailerCount;
  TH2I* mTrailerCount_reset;
  // Calibration plots
  TH1I* mCalibrationWordCount;
  TH2I* mCalibStage;
  TH2I* mCalibLoop;
  TH2I* mActiveLanes;
  TH2I* mLaneStatusCumulative[NFlags];
  TH2Poly* mLaneStatusOverview[2] = { 0x0 }; // Warning and Error/Fatal
  TH1I* mLaneStatusSummary[NLayer];
  TH1D* mLaneStatusSummaryIB;
  TH1D* mLaneStatusSummaryML;
  TH1D* mLaneStatusSummaryOL;
  TH1D* mLaneStatusSummaryGlobal;
  TH1I* mProcessingTime;
  TH1D* mProcessingTime2;
  TH2F* mPayloadSize; // average payload size vs linkID
  // TH1D* mInfoCanvas;//TODO: default, not implemented yet
};

} // namespace o2::quality_control_modules::its

#endif // QC_MODULE_ITS_ITSFEETASK_H
