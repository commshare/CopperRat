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

#ifndef METADATA_H
#define METADATA_H

#ifndef HAVE_PRECOMPILED_HEADERS
#include <string>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#endif

class GenericMetadata{
	std::wstring path;
public:
	GenericMetadata(const std::wstring &path): path(path){}
	virtual ~GenericMetadata(){}
	virtual std::wstring album() = 0;
	virtual std::wstring track_title() = 0;
	virtual int track_number_int();
	virtual std::wstring track_number() = 0;
	virtual std::wstring track_artist() = 0;
	virtual std::wstring date() = 0;
	virtual bool track_gain(double &) = 0;
	virtual bool track_peak(double &) = 0;
	virtual bool album_gain(double &) = 0;
	virtual bool album_peak(double &) = 0;
	std::wstring track_id();
	virtual bool picture(unsigned char *&buffer, size_t &length) = 0;
	const std::wstring &get_path() const{
		return this->path;
	}
};

class OggMetadata : public GenericMetadata{
	std::map<std::wstring, std::wstring> map;
	std::vector<unsigned char> ogg_picture;
	static std::wstring ALBUM,
		TITLE,
		ARTIST,
		TRACKNUMBER,
		DATE,
		OPUS,
		PART,
		METADATA_BLOCK_PICTURE;
	void add(const std::wstring &key, std::wstring &value){
		this->map[key] = value;
	}
public:
	OggMetadata(const std::wstring &path): GenericMetadata(path){}
	void add_vorbis_comment(const void *, size_t);
	template <typename F>
	void iterate(F &f){
		for (auto &p : this->map){
			f(p.first, p.second);
		}
	}
	boost::shared_ptr<OggMetadata> clone(){
		return boost::shared_ptr<OggMetadata>(new OggMetadata(*this));
	}
	std::wstring get_string_or_nothing(const std::wstring &key) const{
		auto i = this->map.find(key);
		return i == this->map.end() ? L"" : i->second;
	}
	std::wstring album(){
		return this->get_string_or_nothing(ALBUM);
	}
	std::wstring track_title();
	std::wstring track_number(){
		return this->get_string_or_nothing(TRACKNUMBER);
	}
	std::wstring track_artist(){
		return this->get_string_or_nothing(ARTIST);
	}
	std::wstring date(){
		return this->get_string_or_nothing(DATE);
	}
	bool picture(unsigned char *&buffer, size_t &length);
	bool track_gain(double &);
	bool track_peak(double &);
	bool album_gain(double &);
	bool album_peak(double &);
};

class Mp3Metadata : public GenericMetadata{
	friend class Mp3Decoder;
	std::wstring id3_title;
	std::wstring id3_artist;
	std::wstring id3_album;
	std::wstring id3_year;
	std::wstring id3_genre;
	std::wstring id3_comment;
	std::map<std::wstring, std::wstring> comments;
	std::map<std::wstring, std::wstring> texts;
	std::vector<unsigned char> id3_picture;
public:
	Mp3Metadata(const std::wstring &path): GenericMetadata(path){}
	void add_mp3_text(const void *text);
	void add_mp3_extra(const void *text);
	void add_picture(const void *buffer, size_t length);
	boost::shared_ptr<Mp3Metadata> clone(){
		return boost::shared_ptr<Mp3Metadata>(new Mp3Metadata(*this));
	}
	std::wstring album(){
		return this->id3_album;
	}
	std::wstring track_title(){
		return this->id3_title;
	}
	std::wstring track_number();
	std::wstring track_artist(){
		return this->id3_artist;
	}
	std::wstring date(){
		return this->id3_year;
	}
	bool picture(unsigned char *&buffer, size_t &length);
	bool track_gain(double &);
	bool track_peak(double &);
	bool album_gain(double &);
	bool album_peak(double &);
};

struct ReplayGainSettings{
	bool prefer_album_gain;
	double preamp_db;
	bool apply_preamp;
	bool apply_peak;
	double fallback_preamp_db;
	ReplayGainSettings():
		prefer_album_gain(1),
		preamp_db(0),
		apply_preamp(0),
		apply_peak(1),
		fallback_preamp_db(-6){}
};

double replaygain_get_multiplier(GenericMetadata &, const ReplayGainSettings &);

#endif
