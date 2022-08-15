#include "sound.hpp"

#include <algorithm>
#include <iostream>

sound_manager::sound_manager(const std::vector<std::string> &sfx_files, const std::vector<std::pair<std::string, std::string>> &music_files_names) :
	music_index(static_cast<unsigned int>(-1)), sfx_vol(100) {
	sfx.reserve(sfx_files.size());
	for (const auto &sfx_f : sfx_files) {
		sfx.push_back(sf::SoundBuffer());
		if (!sfx.back()->loadFromFile(sfx_f)) {
			std::cout << "failed to open sound " << sfx_f << "\n";
			sfx.back().reset();
		}
	}
	music_files.reserve(music_files_names.size());
	music_names.reserve(music_files_names.size());
	for (const auto &music_f_n : music_files_names) {
		music_files.emplace_back(music_f_n.first);
		music_names.emplace_back(music_f_n.second);
	}
	next_music();
}
void sound_manager::next_music() {
	if (!music_files.empty()) {
		music_index = (music_index + 1) % music_files.size();
		if (!music.openFromFile(music_files[music_index])) {
			std::cout << "failed to open music file " << music_files[music_index] << "\n";
		} else {
			music.play();
		}
	}
}
std::string sound_manager::nonestr("none");
const std::string &sound_manager::music_playing() const {
	if (music_names.empty())
		return nonestr;
	return music_names[music_index];
}
void sound_manager::play_sfx(unsigned int index) {
	if (sfx[index].has_value()) {
		playing_sfx.emplace_back(sfx[index].value());
		playing_sfx.back().setVolume(sfx_vol);
		playing_sfx.back().play();
	}
}
void sound_manager::tick() {
	if (music.getStatus() == sf::SoundSource::Stopped) {
		next_music();
	}
	playing_sfx.erase(std::remove_if(playing_sfx.begin(), playing_sfx.end(),
		[](const sf::Sound &s){ return s.getStatus() == sf::SoundSource::Stopped; }), playing_sfx.end());
}
void sound_manager::sfx_volume(int vol) {
	sfx_vol = vol;
	for (auto &s : playing_sfx) {
		s.setVolume(sfx_vol);
	}
}
void sound_manager::music_volume(int vol) {
	music.setVolume(vol);
}
