#ifndef __SOUND_HPP__
#define __SOUND_HPP__

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <SFML/Audio.hpp>

class sound_manager {
public:
	sound_manager(const std::vector<std::string> &sfx_files, const std::vector<std::pair<std::string, std::string>> &music_files_names);
	const std::string &music_playing() const;
	void next_music();
	void tick();
	void play_sfx(unsigned int index);
	void sfx_volume(int vol);
	void music_volume(int vol);
private:
	static std::string nonestr;
	std::vector<std::optional<sf::SoundBuffer>> sfx;
	std::vector<sf::Sound> playing_sfx;
	sf::Music music;
	unsigned int music_index;
	std::vector<std::string> music_files;
	std::vector<std::string> music_names;
	int sfx_vol;
};

#endif
