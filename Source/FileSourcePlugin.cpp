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

#include "FileSourcePlugin.h"

FileSourcePlugin::FileSourcePlugin()
{

}

FileSourcePlugin::~FileSourcePlugin()
{
	
}

bool FileSourcePlugin::open(File file)
{
	return true;
}

void FileSourcePlugin::fillRecordInfo()
{

}

void FileSourcePlugin::updateActiveRecord(int index)
{

}

void FileSourcePlugin::seekTo(int64 sample)
{

}

int FileSourcePlugin::readData(int16* buffer, int nSamples)
{
	return nSamples;
}

void FileSourcePlugin::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 nSamples)
{

}

void FileSourcePlugin::processEventData(EventInfo& info, int64 startTimestamp, int64 stopTimestamp)
{

}