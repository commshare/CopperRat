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

#include "stdafx.h"
#include "OggDecoder.h"
#include "AudioStream.h"
#include "CommonFunctions.h"

OggDecoder::OggDecoder(AudioStream &parent, const std::wstring &path): Decoder(parent, path), metadata(path){
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
	auto utf8 = string_to_utf8(path);
	this->file = 
#ifdef WIN32
		_wfopen(path.c_str(), L"rb");
#else
		fopen(string_to_utf8(path).c_str(), "rb");
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	if (!this->file)
		throw DecoderInitializationException("Open file for Vorbis decoder failed.");
	this->bitstream = 0;
	ov_callbacks cb;
	cb.read_func = OggDecoder::read;
	cb.seek_func = OggDecoder::seek;
	cb.tell_func = OggDecoder::tell;
	cb.close_func = 0;
	int error = ov_open_callbacks(this, &this->ogg_file, 0, 0, cb);
	if (error < 0)
		throw DecoderInitializationException("ov_open_callbacks() failed???");
	vorbis_info *i = ov_info(&this->ogg_file, this->bitstream);
	if (!i)
		throw DecoderInitializationException("Failed to read Ogg Vorbis stream info.");
	this->frequency = i->rate;
	this->channels = i->channels;
	vorbis_comment *comment = ov_comment(&this->ogg_file, this->bitstream);
	if (!!comment){
		for (auto i = comment->comments; i--;)
			this->metadata.add_vorbis_comment(comment->user_comments[i], comment->comment_lengths[i]);
		this->parent.metadata_update(this->metadata.clone());
	}
}

OggDecoder::~OggDecoder(){
	ov_clear(&this->ogg_file);
	if (this->file)
		fclose(this->file);
}

const char *ogg_code_to_string(int e){
	switch (e){
		case 0:
			return "no error";
		case OV_EREAD:
			return "a read from media returned an error";
		case OV_ENOTVORBIS:
			return "bitstream does not contain any Vorbis data";
		case OV_EVERSION:
			return "vorbis version mismatch";
		case OV_EBADHEADER:
			return "invalid Vorbis bitstream header";
		case OV_EFAULT:
			return "internal logic fault; indicates a bug or heap/stack corruption";
		default:
			return "unknown error";
	}
}

audio_buffer_t OggDecoder::read_more_internal(){
	const size_t samples_to_read = 1024;
	const size_t bytes_per_sample = this->channels*2;
	const size_t bytes_to_read = samples_to_read*bytes_per_sample;
	audio_buffer_t ret(sizeof(Sint16), this->channels, samples_to_read);
	size_t size = 0;
	while (size < bytes_to_read){
		int r = ov_read(&this->ogg_file, ((char *)ret.get_sample_use_channels<Sint16>(0)) + size, int(bytes_to_read - size), 0, 2, 1, &this->bitstream);
		if (r < 0)
			abort();
		if (!r){
			if (!size)
				ret.unref();
			break;
		}
		size += r;
	}
	ret.set_sample_count(unsigned(size / bytes_per_sample));
	return ret;
}

sample_count_t OggDecoder::get_pcm_length_internal(){
	return ov_pcm_total(&this->ogg_file, this->bitstream);
}

double OggDecoder::get_seconds_length_internal(){
	return ov_time_total(&this->ogg_file, this->bitstream);
}

bool OggDecoder::seek(audio_position_t pos){
	return !ov_pcm_seek(&this->ogg_file, pos);
}

bool OggDecoder::fast_seek(audio_position_t pos, audio_position_t &new_position){
	bool ret = !ov_pcm_seek_page(&this->ogg_file, pos);
	if (ret)
		new_position = ov_pcm_tell(&this->ogg_file);
	return ret;
}

size_t OggDecoder::read(void *buffer, size_t size, size_t nmemb, void *s){
	OggDecoder *_this = (OggDecoder *)s;
	size_t ret = fread(buffer, size, nmemb, _this->file);
	return ret;
}

int OggDecoder::seek(void *s, ogg_int64_t offset, int whence){
	OggDecoder *_this = (OggDecoder *)s;
	int ret = fseek(_this->file, (long)offset, whence);
	return ret;
}

long OggDecoder::tell(void *s){
	OggDecoder *_this = (OggDecoder *)s;
	long ret = ftell(_this->file);
	return ret;
}