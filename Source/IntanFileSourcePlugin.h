/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef FILESOURCEPLUGIN_H_DEFINED
#define FILESOURCEPLUGIN_H_DEFINED

#include <FileSourceHeaders.h>
#include "rhx/cnsrhx.h"

class IntanFileSourcePlugin : public FileSource
{
	IntanHeaderInfo m_intanHeaderInfo;

public:
	/** The class constructor, used to initialize any members. */
	IntanFileSourcePlugin();

	/** The class destructor, used to deallocate memory */
	~IntanFileSourcePlugin();

    // ------------------------------------------------------------
    //                  PURE VIRTUAL METHODS 
    //        (must be implemented by all File Sources)
    // ------------------------------------------------------------

    /** Attempt to open the file, and return true if successful */
    bool open(File file);

    /** Fill the infoArray and eventInfoArray with the relevant information for all recordings*/
    void fillRecordInfo();

    /** Update the recording to be read in */
    void updateActiveRecord(int index);

    /** Seek to a specific sample number within the active recording */
    void seekTo(int64 sample);

    /** Read in nSamples of int16 data into a temporary buffer */
    int readData(int16* buffer, int nSamples);

    /** Convert nSamples of data from int16 to float */
    void processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 nSamples);

    /** Add info about events occurring within a sample range */
    void processEventData(EventInfo& info, int64 startTimestamp, int64 stopTimestamp);

};

#endif
