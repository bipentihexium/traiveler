#ifndef __MAP_HPP__
#define __MAP_HPP__

#include <string>
#include <vector>
#include "glw.hpp"

enum class tile_t {
	empty=0,
	goal,
	stream_down, stream_up,
	stream_left, stream_right,
	stream_ur, stream_ru,
	stream_rd, stream_dr,
	stream_dl, stream_ld,
	stream_lu, stream_ul,
	dynamite, barrier,
	wood, coal, sulphur,
	iron_ore, iron, steel,
	engine, booster, fuel, bridge,
	built_bridge,
	booster_right,
	booster_down,
	booster_left,
	booster_up,
	spikes,
	tut_slot_bridge=39,
	tut_slot_coal,
	tut_slot_sulphur,
	tut_slot_dynamite,
	tut_slot_iron,
	tut_slot_steel,
	tut_slot_engine,
	tut_slot_booster,
	tut_slot_fuel,
	tut_slot_wood,
	win_y,
	win_o,
	win_u,
	win_w,
	win_o2,
	win_n,
	tut_slot_iron_ore,
	tut_broken_ai_stream,
	tut_fake_goal,
	tut_happy_ai,
	tut_blown_up,
	tut_dynamite_pile,
	tut_slot_barrier,
	tut_slot_process_arrow,
};
struct tile {
	tile_t t;
	bool accessible;

	tile(tile_t t, bool accessible=true);
};
class map {
public:
	std::vector<std::vector<tile>> tiles;
	const glw::high::tiledTextureAtlas<8, 8> *atlas;
	unsigned int w;
	unsigned int h;
	constexpr static unsigned int chunk_size = 16;
	std::vector<std::vector<glw::vao>> meshes;
	std::vector<std::vector<unsigned int>> meshsizes;
	unsigned int startx, starty;
	unsigned int goalx, goaly;

	map(const std::string &file, const glw::high::tiledTextureAtlas<8, 8> *a, float &bat, size_t &hand, std::string &tip);
	bool passable(int x, int y) const;
	void set_tile_accessible(unsigned int x, unsigned int y, bool access_new=true);
	void set_tile(unsigned int x, unsigned int y, tile_t type_new);
private:
	std::vector<std::vector<glw::vbo>> meshesvbo;
	std::vector<std::vector<glw::ebo>> meshesebo;
	
	void gen_meshes();
	void gen_mesh(unsigned int x, unsigned int y);
};

#endif
