/*

Copyright (c) 2014, Helios
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "Exception.h"
#include "Threads.h"

class AudioPlayer;

class DeviceInitializationException : public CR_Exception{
public:
	DeviceInitializationException(const std::string &desc): CR_Exception(desc){}
	CR_Exception *clone() const{
		return new DeviceInitializationException(*this);
	}
};

class AudioDevice{
	friend class InitializeAudioDevice;
	AudioPlayer &player;
	RemoteThreadProcedureCallPerformer &rtpcp;
	bool audio_is_open;
	std::string error_string;

	void open_in_main();
public:
	AudioDevice(AudioPlayer &, RemoteThreadProcedureCallPerformer &);
	~AudioDevice();
	void open();
	void close();
	void close_in_main();
	void start_audio();
	void pause_audio();
	bool is_open() const{
		return this->audio_is_open;
	}
};

class InitializeAudioDevice : public RemoteThreadProcedureCall{
	AudioDevice *device;
	void entry_point_internal(){
		this->device->open_in_main();
	}
public:
	InitializeAudioDevice(AudioDevice *device, RemoteThreadProcedureCallPerformer &rtpcp): RemoteThreadProcedureCall(rtpcp), device(device){}
};

class CloseAudioDevice : public RemoteThreadProcedureCall{
	AudioDevice *device;
	void entry_point_internal(){
		this->device->close_in_main();
	}
public:
	CloseAudioDevice(AudioDevice *device, RemoteThreadProcedureCallPerformer &rtpcp): RemoteThreadProcedureCall(rtpcp), device(device){}
};

#define SDL_PauseAudio(_)
#endif
