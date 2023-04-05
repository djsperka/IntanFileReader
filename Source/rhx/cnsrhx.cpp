/*
 * cnsrhx.cpp
 *
 *  Created on: Apr 5, 2023
 *      Author: dan
 */

#include <iostream>
#include <sstream>
#include <exception>
#include <string>
#include <vector>
#include "abstractrhxcontroller.h"
#include "channel.h"
#include "utf8.h"
using namespace std;

#include "cnsrhx.h"


// read a value from a binary file. Will throw() on fail
template <typename T>
void readFromBin(ifstream& in, T& value, string description)
{
    if (not in.read((char *)&value, sizeof(value)))
    {
    	string s("Cannot read ");
    	s.append(description);
    	throw std::runtime_error(s);
    }
}

// read a QString from a binary file, convert from UTF-16 to UTF-8, return a std::string.
void readQStringFromBin(ifstream& in, string& value, string description)
{
	uint32_t uiSize=0;
	value.clear();
	readFromBin(in, uiSize, description);
	if (uiSize == 0xffffffff)
		value = "";	// empty string
	else if (uiSize > 0)
	{
		char16_t *a;
		a = new char16_t[uiSize/2];
		if (not in.read((char *)&a[0], uiSize))
		{
			string s("Cannot read ");
			s.append(description);
			throw std::runtime_error(s);
		}
		// this does the conversion from utf16 to utf8.
		utf8::utf16to8(a, a + uiSize/2, back_inserter(value));
		delete a;
	}
	else
		value = "";

	return;

}


// read the intan file header.
// Will only read header-only, not "all-in-one" type data files.
// At this writing, only look for info.rhd, timestamp, amplifier, digitalin, spike files.
void readIntanHeader(const char *filename, IntanHeaderInfo& info)
{
    int16_t int16Buffer;
    uint32_t uint32Buffer;
    float floatBuffer;
    int numberOfSignalGroups = 0;
    int numberOfChannelsInSignalGroup = 0;


    // Open file, then read (and check) the magic number.
    ifstream in(filename, ios::in | ios::binary);
	if (!in)
		throw std::runtime_error("Cannot open");

	readFromBin(in, uint32Buffer, "magic number");
    if (uint32Buffer == DataFileMagicNumberRHD) {
        info.fileType = RHDHeaderFile;
    } else if (uint32Buffer == DataFileMagicNumberRHS) {
        info.fileType = RHSHeaderFile;
    } else {
    	ostringstream oss;
    	oss << "Header Error: Invalid Intan header identifier: " << hex << uint32Buffer;
        throw std::runtime_error(oss.str());
    }

    // Get version number. As of this writing, the version
    readFromBin(in, int16Buffer, "main version number");
    info.dataFileMainVersionNumber = int16Buffer;
    readFromBin(in, int16Buffer, "secondary version number");
    info.dataFileSecondaryVersionNumber = int16Buffer;


    if (info.fileType == RHDHeaderFile && info.dataFileMainVersionNumber == 1) {
        info.samplesPerDataBlock = 60;
    } else {
        info.samplesPerDataBlock = 128;
    }


    // global sampling rate and amplifier frequency parameters
    readFromBin(in, floatBuffer, "sample rate");
    info.sampleRate = AbstractRHXController::nearestSampleRate(floatBuffer);
    if ((int)info.sampleRate == -1)
    {
    	std::ostringstream oss;
    	oss << "Header Error: Invalid sample rate: " << floatBuffer;
    	throw std::runtime_error(oss.str());
    }

    // dsp enabled
    readFromBin(in, int16Buffer, "dsp enabled");
    info.dspEnabled = (int16Buffer != 0);

    readFromBin(in, floatBuffer, "dsp cutoff freq");
    info.actualDspCutoffFreq = floatBuffer;
    readFromBin(in, floatBuffer, "dsp lower bandwidth");
    info.actualLowerBandwidth = floatBuffer;
    if (info.fileType == RHSHeaderFile) {
        readFromBin(in, floatBuffer, "actualLowerSettleBandwidth");
        info.actualLowerSettleBandwidth = floatBuffer;
    } else {
        info.actualLowerSettleBandwidth = 0.0;
    }
    readFromBin(in, floatBuffer, "actualUpperBandwidth");
    info.actualUpperBandwidth = floatBuffer;

    readFromBin(in, floatBuffer, "desiredDspCutoffFreq");
    info.desiredDspCutoffFreq = floatBuffer;
    readFromBin(in, floatBuffer, "desiredLowerBandwidth");
    info.desiredLowerBandwidth = floatBuffer;
    if (info.fileType == RHSHeaderFile) {
        readFromBin(in, floatBuffer, "desiredLowerSettleBandwidth");
        info.desiredLowerSettleBandwidth = floatBuffer;
    } else {
        info.desiredLowerSettleBandwidth = 0.0;
    }
    readFromBin(in, floatBuffer, "desiredUpperBandwidth");
    info.desiredUpperBandwidth = floatBuffer;

    readFromBin(in, int16Buffer, "notchFilterMode");
    if (int16Buffer == 0) {
        info.notchFilterEnabled = false;
        info.notchFilterFreq = 0.0;
    } else if (int16Buffer == 1) {
        info.notchFilterEnabled = true;
        info.notchFilterFreq = 50.0;
    } else if (int16Buffer == 2) {
        info.notchFilterEnabled = true;
        info.notchFilterFreq = 60.0;
    } else {
    	std::ostringstream oss;
        oss << "Header Error: Invalid notch filter mode: " << int16Buffer;
    	throw std::runtime_error(oss.str());
    }

    readFromBin(in, floatBuffer, "actualImpedanceTestFreq");
    info.actualImpedanceTestFreq = floatBuffer;
    readFromBin(in, floatBuffer, "desiredImpedanceTestFreq");
    info.desiredImpedanceTestFreq = floatBuffer;

    if (info.fileType == RHSHeaderFile) {
        info.stimDataPresent = true;
        readFromBin(in, int16Buffer, "ampSettleMode");
        info.ampSettleMode = (int16Buffer != 0);
        readFromBin(in, int16Buffer, "chargeRecoveryMode");
        info.chargeRecoveryMode = (int16Buffer != 0);
        readFromBin(in, int16Buffer, "stimStepSize");
        info.stimStepSize = AbstractRHXController::nearestStimStepSize(floatBuffer);
        if ((int)info.stimStepSize == -1) {
        	std::ostringstream oss;
            oss << "Header Error: Invalid stim step size: " << floatBuffer;
        	throw std::runtime_error(oss.str());
        }
        readFromBin(in, floatBuffer, "chargeRecoveryCurrentLimit");
        info.chargeRecoveryCurrentLimit = floatBuffer;
        readFromBin(in, floatBuffer, "chargeRecoveryTargetVoltage");
        info.chargeRecoveryTargetVoltage = floatBuffer;
    } else {
        info.stimDataPresent = false;
        info.ampSettleMode = false;
        info.chargeRecoveryMode = false;
        info.stimStepSize = (StimStepSize) -1;
        info.chargeRecoveryCurrentLimit = 0.0;
        info.chargeRecoveryTargetVoltage = 0.0;
    }
    readQStringFromBin(in, info.note1, "note1");
    readQStringFromBin(in, info.note2, "note2");
    readQStringFromBin(in, info.note3, "note3");

    if (info.fileType == RHSHeaderFile) {
    	readFromBin(in, int16Buffer, "dcAmplifierDataSaved");
        info.dcAmplifierDataSaved = (int16Buffer != 0); // Warning: The old RHS software saves this value as 0 for non-Intan format!
    } else {
        info.dcAmplifierDataSaved = false;
    }

    if (info.fileType == RHDHeaderFile && info.dataFileVersionNumber() > 1.09) {
    	readFromBin(in, int16Buffer, "numTempSensors");
        info.numTempSensors = int16Buffer;
    } else {
        info.numTempSensors = 0;
    }

    if ((info.fileType == RHDHeaderFile && info.dataFileVersionNumber() > 1.29) ||
            info.fileType == RHSHeaderFile) {
    	readFromBin(in, int16Buffer, "boardMode");
        info.boardMode = int16Buffer;
    } else {
        info.boardMode = RHDUSBInterfaceBoardMode;
    }

    if (info.fileType == RHSHeaderFile) {   // Note: Synth mode recordings from old software saves boardMode = 0.
        info.controllerType = ControllerStimRecord;
    } else if (info.dataFileMainVersionNumber == 2) {
        info.controllerType = ControllerRecordUSB3;
    } else if (info.dataFileMainVersionNumber == 1) {
        info.controllerType = ControllerRecordUSB2;
    } else if (info.boardMode == RHDUSBInterfaceBoardMode) {
        info.controllerType = ControllerRecordUSB2;
    } else if (info.boardMode == RHDControllerBoardMode) {
        info.controllerType = ControllerRecordUSB3;
    } else if (info.boardMode == RHSControllerBoardMode) {
        info.controllerType = ControllerStimRecord;
    } else {
    	ostringstream oss;
    	oss << "Header Error: Invalid board mode: " << info.boardMode;
        throw std::runtime_error(oss.str());
    }

    if ((info.fileType == RHDHeaderFile && info.dataFileVersionNumber() > 1.99) ||
            info.fileType == RHSHeaderFile) {
    	readQStringFromBin(in, info.refChannelName, "ref channel name");
    } else {
        info.refChannelName.clear();
    }

    readFromBin(in, int16Buffer, "number of signal groups");
    if ((int16Buffer < 0) || (int16Buffer > 12))
    {
    	ostringstream oss;
    	oss << "Header Error: Invalid number of signal groups: " << int16Buffer;
        throw std::runtime_error(oss.str());
    }
    numberOfSignalGroups = (int)int16Buffer;
    cout << "Found " << numberOfSignalGroups << " signal groups" << endl;

    info.numDataStreams = 0;
    info.numEnabledAmplifierChannels = 0;
    info.numEnabledAuxInputChannels = 0;
    info.numEnabledSupplyVoltageChannels = 0;
    info.numEnabledBoardAdcChannels = 0;
    info.numEnabledBoardDacChannels = 0;
    info.numEnabledDigitalInChannels = 0;
    info.numEnabledDigitalOutChannels = 0;

    bool moreThanFourSPIPorts = false;
    info.expanderConnected = false;

    // load headers&channel descriptions for each signal group.
    // If signal group is enabled and has >0 channels, then channel descriptions
    // follow the header.
    for (int i = 0; i < numberOfSignalGroups; i++) {
    	HeaderFileGroup group;
        readQStringFromBin(in, group.name, "group name");
        readQStringFromBin(in, group.prefix, "group prefix");
        // DJS avoid std lack of toUpper, without adding boost.
        if (	group.prefix == "E" || group.prefix == "e" ||
        		group.prefix == "F" || group.prefix == "f" ||
                group.prefix == "G" || group.prefix == "g" ||
				group.prefix == "H" || group.prefix == "h") {
            moreThanFourSPIPorts = true;
        }
        readFromBin(in, int16Buffer, "signal group enabled");
        group.enabled = (int16Buffer != 0);
        readFromBin(in, int16Buffer, "number of channels in signal group");
        if ((int16Buffer < 0) || (int16Buffer > 2 * (64 + 3 + 1))) {
        	ostringstream oss;
        	oss << "Header Error: Invalid number of channels in signal group " << i << ": " << int16Buffer;
        	throw std::runtime_error(oss.str());
        }
        cout << "Signal group " << group.name << "has " << int16Buffer << " channels." << endl;
        numberOfChannelsInSignalGroup = (int)int16Buffer;
        readFromBin(in, int16Buffer, "number of amplifier channels in signal group");
        if ((int16Buffer < 0) || (int16Buffer > 2 * 64)) {
        	ostringstream oss;
        	oss << "Header Error: Invalid number of amplifier channels in signal group " << i << ": " << int16Buffer;
            throw std::runtime_error(oss.str());
        }
        group.numAmplifierChannels = int16Buffer;

        cout << "Signal group " << group.name << " prefix " << group.prefix << " enabled? " << group.enabled << " #channels " << group.channels.size() << endl;
        for (int j = 0; j < numberOfChannelsInSignalGroup; ++j) {
        	string sBuffer;
            //djs HeaderFileChannel& channel = group.channels[j];
            HeaderFileChannel channel;
            readQStringFromBin(in, sBuffer, "Native channel name");
            channel.nativeChannelName = sBuffer;
            readQStringFromBin(in, channel.customChannelName, "Custom channel name");
            readFromBin(in, int16Buffer, "native order");
            channel.nativeOrder = int16Buffer;
            readFromBin(in, int16Buffer, "custom order");
            channel.customOrder = int16Buffer;
            readFromBin(in, int16Buffer, "Signal type");
            if (info.fileType == RHDHeaderFile) {
                if ((int16Buffer < 0) || (int16Buffer > 5)) {
                	ostringstream oss;
                	oss << "Header Error: Invalid signal type in group " << i << ", channel " << j << ": " << int16Buffer;
                    throw std::runtime_error(oss.str());
                }
                channel.signalType = Channel::convertRHDIntToSignalType(int16Buffer);
            } else if (info.fileType == RHSHeaderFile) {
                if ((int16Buffer < 0) || (int16Buffer > 6)) {
                	ostringstream oss;
                	oss << "Header Error: Invalid signal type in group " << i << ", channel " << j <<  ": " << int16Buffer;
                    throw std::runtime_error(oss.str());
                }
                channel.signalType = Channel::convertRHSIntToSignalType(int16Buffer);
            }

            readFromBin(in, int16Buffer, "channel enabled");
            channel.enabled = (int16Buffer != 0);

            if (channel.enabled) info.adjustNumChannels(channel.signalType, 1);

            if (channel.signalType == BoardAdcSignal || channel.signalType == BoardDacSignal ||
                    channel.signalType == BoardDigitalInSignal || channel.signalType == BoardDigitalOutSignal) {
                if (channel.nativeOrder > 1) info.expanderConnected = true;
            }

            readFromBin(in, int16Buffer, "chip channel");
            channel.chipChannel = int16Buffer;
            if (info.fileType == RHSHeaderFile) {
            	readFromBin(in, int16Buffer, "command stream");
                channel.commandStream = int16Buffer;
            }
            readFromBin(in, int16Buffer, "board stream");
            channel.boardStream = int16Buffer;
            if (channel.boardStream + 1 > info.numDataStreams) {
                info.numDataStreams = channel.boardStream + 1;
            }

            if (info.fileType == RHDHeaderFile) {
                channel.commandStream = channel.boardStream;  // reasonable guess; probably not used in file playback
            }
            readFromBin(in, int16Buffer, "spikeScopeTriggerMode");
            channel.spikeScopeTriggerMode = int16Buffer;
            readFromBin(in, int16Buffer, "spikeScopeVoltageThreshold");
            channel.spikeScopeVoltageThreshold = int16Buffer;
            readFromBin(in, int16Buffer, "spikeScopeTriggerChannel");
            channel.spikeScopeTriggerChannel = int16Buffer;
            readFromBin(in, int16Buffer, "spikeScopeTriggerPolarity");
            channel.spikeScopeTriggerPolarity = int16Buffer;
            readFromBin(in, floatBuffer, "impedanceMagnitude");
            channel.impedanceMagnitude = floatBuffer;
            readFromBin(in, floatBuffer, "impedancePhase");
            channel.impedancePhase = floatBuffer;


            cout << "Channel " << channel.nativeChannelName << endl;
            group.channels.push_back(channel);
        }
        info.groups.push_back(group);
    }

    if (info.controllerType == ControllerRecordUSB2) {
        info.numSPIPorts = 4;
    } else {
        info.numSPIPorts = moreThanFourSPIPorts ? 8 : 4;
    }

    //djs info.headerOnly = file.atEnd();
    info.headerSizeInBytes = in.tellg();
    //djs info.dataSizeInBytes = file.size() - (int64_t)info.headerSizeInBytes;
    info.dataSizeInBytes = 0;

    in.seekg(0, ios::end);
    info.headerOnly = (info.headerSizeInBytes == in.tellg());
    cout << "headers size " << info.headerSizeInBytes << " eof pos " << in.tellg() << " header only? " << info.headerOnly << endl;

    if (!info.headerOnly)
    {
    	throw std::runtime_error("Cannot parse all-in-one file. Not implemented.");
    }

	in.close();
	return;
}



void printHeader(const IntanHeaderInfo& info)
{
    cout << "File header contents:\n";
    cout << "Header file type: " << ((info.fileType == RHDHeaderFile) ? "RHD file\n" : "RHS file\n");
    string controllerString;
    if (info.controllerType == ControllerRecordUSB2) controllerString = "RHD USB interface board";
    else if (info.controllerType == ControllerRecordUSB3) controllerString = "RHD recording controller";
    else if (info.controllerType == ControllerStimRecord) controllerString = "RHS stim/recording controller";
    cout << "Controller type: " << controllerString << '\n';
    cout << "Data file version: " << info.dataFileMainVersionNumber << "." << info.dataFileSecondaryVersionNumber << '\n';
    cout << "Number of samples per data block: " << info.samplesPerDataBlock << '\n';
    cout << "sample rate: " << AbstractRHXController::getSampleRateString(info.sampleRate) << '\n';
    cout << "DSP enabled: " << (info.dspEnabled ? "true\n" : "false\n");

    // ...

    if (info.fileType == RHSHeaderFile) {
        cout << "stim step size: " << AbstractRHXController::getStimStepSizeString(info.stimStepSize) << '\n';
    }

    // ...

    cout << "board mode: " << info.boardMode << '\n';

    // ...

    cout << "total number of data streams: " << info.numDataStreams << '\n';
    cout << "total number of amplifier channels: " << info.numEnabledAmplifierChannels << '\n';
    cout << "total number of auxiliary input channels: " << info.numEnabledAuxInputChannels << '\n';
    cout << "total number of supply voltage channels: " << info.numEnabledSupplyVoltageChannels << '\n';
    cout << "total number of analog input channels: " << info.numEnabledBoardAdcChannels << '\n';
    cout << "total number of analog output channels: " << info.numEnabledBoardDacChannels << '\n';
    cout << "total number of digital input channels: " << info.numEnabledDigitalInChannels << '\n';
    cout << "total number of digital output channels: " << info.numEnabledDigitalOutChannels << '\n';

    cout << "number of SPI ports: " << info.numSPIPorts << '\n';
    cout << "expander connected: " << (info.expanderConnected ? "true" : "false") << '\n';
    cout << "header only: " << (info.headerOnly ? "true" : "false") << '\n';
    cout << "header size in bytes: " << info.headerSizeInBytes << '\n';
//    cout << "bytes per data block: " << info.bytesPerDataBlock << '\n';
//    cout << "data size in bytes: " << info.dataSizeInBytes << '\n';
//    cout << "number of data blocks in file: " << info.numDataBlocksInFile << '\n';
//    cout << "number of samples in file: " << info.numSamplesInFile << '\n';
//    cout << "time in file: " << info.timeInFile << " s" << '\n';

}

