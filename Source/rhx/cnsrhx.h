/*
 * cnsrhx.h
 *
 *  Created on: Apr 5, 2023
 *      Author: dan
 */

#ifndef RHX_CNSRHX_H_
#define RHX_CNSRHX_H_

#include "rhxglobals.h"
#include <string>
#include <vector>
#include <fstream>

enum HeaderFileType {
    RHDHeaderFile,
    RHSHeaderFile
};

enum DataFileFormat {
    TraditionalIntanFormat,
    FilePerSignalTypeFormat,
    FilePerChannelFormat
};

struct HeaderFileChannel
{
    std::string nativeChannelName;
    std::string customChannelName;
    int nativeOrder;
    int customOrder;
    SignalType signalType;
    bool enabled;
    int chipChannel;
    int commandStream;
    int boardStream;
    int spikeScopeTriggerMode;      // 0 = trigger on digital input; 1 = trigger on voltage threshold
    int spikeScopeVoltageThreshold;
    int spikeScopeTriggerChannel;
    int spikeScopeTriggerPolarity;  // 0 = trigger on falling edge; 1 = trigger on rising edge
    double impedanceMagnitude;
    double impedancePhase;
    //DJS int channelNumber() const { return nativeChannelName.section('-', -1).toInt(); }
    //DJS int endingNumber(int numChars = 1) const { return nativeChannelName.right(numChars).toInt(); }
};

struct HeaderFileGroup
{
	std::string name;
	std::string prefix;
    bool enabled;
    int numAmplifierChannels;
    std::vector<HeaderFileChannel> channels;
    int numChannels() const { return (int) channels.size(); }
};

struct IntanHeaderInfo
{
public:
    HeaderFileType fileType;
    ControllerType controllerType;
    int dataFileMainVersionNumber;
    int dataFileSecondaryVersionNumber;
    double dataFileVersionNumber() const { return dataFileMainVersionNumber + (double)dataFileSecondaryVersionNumber / 10.0; }

    int samplesPerDataBlock;

    AmplifierSampleRate sampleRate;
    bool dspEnabled;
    double actualDspCutoffFreq;
    double actualLowerBandwidth;
    double actualLowerSettleBandwidth;
    double actualUpperBandwidth;
    double desiredDspCutoffFreq;
    double desiredLowerBandwidth;
    double desiredLowerSettleBandwidth;
    double desiredUpperBandwidth;

    bool notchFilterEnabled;
    double notchFilterFreq;

    double desiredImpedanceTestFreq;
    double actualImpedanceTestFreq;

    bool ampSettleMode;         // false = switch lower bandwidth; true = traditional fast settle
    bool chargeRecoveryMode;    // false = current-limited charge recovery circuit; true = charge recovery switch

    bool stimDataPresent;
    StimStepSize stimStepSize;
    double chargeRecoveryCurrentLimit;
    double chargeRecoveryTargetVoltage;

    std::string note1;
    std::string note2;
    std::string note3;

    bool dcAmplifierDataSaved;

    int numTempSensors;
    int boardMode;

    std::string refChannelName;

    std::vector<HeaderFileGroup> groups;
    int numGroups() const { return (int) groups.size(); }
    //DJS int groupIndex(const QString& prefix) const;
    int numChannels() const;
    int numAmplifierChannels() const;
    //DJS bool removeChannel(const QString& nativeChannelName);
    void removeAllChannels(SignalType signalType);
//    QString getChannelName(SignalType signalType, int stream, int channel);

    int numDataStreams;
    int numEnabledAmplifierChannels;
    int numEnabledAuxInputChannels;
    int numEnabledSupplyVoltageChannels;
    int numEnabledBoardAdcChannels;
    int numEnabledBoardDacChannels;
    int numEnabledDigitalInChannels;
    int numEnabledDigitalOutChannels;
    int adjustNumChannels(SignalType signalType, int delta);

    int numSPIPorts;
    bool expanderConnected;

    bool headerOnly;
    int headerSizeInBytes;
    int bytesPerDataBlock;
    int64_t dataSizeInBytes;
    int64_t numDataBlocksInFile;
    int64_t numSamplesInFile;
    double timeInFile;

    int32_t firstTimeStamp;
    int32_t lastTimeStamp;

//    void printInfo() const;
};



void readIntanHeader(const char *filename, IntanHeaderInfo& info);
void printHeader(const IntanHeaderInfo& info);





#endif /* RHX_CNSRHX_H_ */
