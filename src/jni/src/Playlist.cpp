#include "stdafx.h"
#include "Playlist.h"
#include "File.h"
#include "Decoder.h"
#include "CommonFunctions.h"
#ifndef HAVE_PRECOMPILED_HEADERS
#include <fstream>
#endif

void Playlist::clear(){
	this->tracks.clear();
	this->shuffle_vector.clear();
	this->current_track = -1;
}

void Playlist::insert(const std::vector<std::wstring> &v, size_t p){
	p %= (this->tracks.size() + 1);
	const auto n = v.size();
	if (this->shuffle){
		for (auto &i : this->shuffle_vector)
			if (i >= p)
				i += (int)n;
		auto previous_size = this->shuffle_vector.size();
		this->shuffle_vector.resize(previous_size + n);
		for (size_t i = 0; i < n; i++)
			this->shuffle_vector[previous_size + i] = (int)(i + p);
		std::random_shuffle(this->shuffle_vector.begin() + previous_size, this->shuffle_vector.end());
	}else if (this->current_track < 0)
		this->current_track = 0;
	else if (this->current_track >= p)
		this->current_track += (int)n;
	this->tracks.insert(this->tracks.begin() + p, v.begin(), v.end());
}

void Playlist::toggle_shuffle(){
	if (this->shuffle){
		if (this->shuffle_vector.size()){
			this->current_track = this->shuffle_vector[this->current_track];
			this->shuffle_vector.clear();
		}
	}else{
		if (this->tracks.size()){
			this->shuffle_vector.resize(this->tracks.size());
			for (size_t i = 0; i < this->shuffle_vector.size(); i++)
				this->shuffle_vector[i] = (int)i;
			std::swap(this->shuffle_vector[0], this->shuffle_vector[this->current_track]);
			this->current_track = 0;
			std::random_shuffle(this->shuffle_vector.begin() + 1, this->shuffle_vector.end());
		}
	}
	this->shuffle = !this->shuffle;
}

bool Playlist::get_current_track(std::wstring &dst){
	if (this->at_null_position())
		return 0;
	size_t index = !this->shuffle ? this->current_track : this->shuffle_vector[this->current_track];
	dst = this->tracks[index];
	return 1;
}

bool Playlist::next(){
	if (this->at_null_position())
		return 0;
	switch (this->mode){
		case PlaybackMode::SINGLE:
			this->current_track++;
			break;
		case PlaybackMode::REPEAT_LIST:
			this->current_track = (this->current_track + 1) % this->tracks.size();
			if (!this->current_track && this->shuffle)
				std::random_shuffle(this->shuffle_vector.begin(), this->shuffle_vector.end());
			break;
		case PlaybackMode::REPEAT_TRACK:
			break;
	}
	return 1;
}

bool Playlist::back(){
	if (!this->is_back_possible())
		return 0;
	auto n = this->tracks.size();
	this->current_track = (int)(((size_t)this->current_track + (n - 1)) % n);
	return 1;
}

bool Playlist::is_back_possible() const{
	if (this->at_null_position())
		return 0;
	if (!this->current_track && (this->shuffle || this->mode == PlaybackMode::SINGLE))
		return 0;
	return 1;
}

Playlist::PlaybackMode Playlist::cycle_mode(){
	this->mode = (PlaybackMode)(((int)this->mode + 1) % (int)PlaybackMode::COUNT);
	if (this->mode == PlaybackMode::REPEAT_LIST && this->tracks.size())
		this->current_track %= this->tracks.size();
	return this->mode;
}

void Playlist::load(bool file, const std::wstring &path){
	this->clear();
	std::vector<std::wstring> list;
	if (file){
		auto ext = get_extension(path);
		if (ext == L"pls"){
			this->load_playlist(path);
			return;
		}
		if (!format_is_supported(path))
			return;
		list.push_back(path);
	}else{
		std::vector<std::wstring> files;
		find_files_recursively(files, path);
		filter_list_by_supported_formats(list, files);
	}
	this->insert(list, 0);
}

void Playlist::append(bool file, const std::wstring &path){
}

void Playlist::load_playlist(const std::wstring &path){
	auto utf8 = string_to_utf8(path);
	std::ifstream file(utf8.c_str());
	std::vector<std::wstring> paths;
	auto container = get_container(path);
	while (1){
		std::string line;
		std::getline(file, line);
		if (!file || !line.size() || line[0] == '#')
			continue;
		std::wstring wide = utf8_to_string(line);
		if (path_is_rooted(wide))
			paths.push_back(wide);
		else
			paths.push_back(container + wide);
	}
	this->insert(paths, 0);
}
