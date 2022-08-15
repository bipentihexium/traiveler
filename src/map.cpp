#include "map.hpp"

#include <algorithm>
#include <fstream>
#include <limits>

tile::tile(tile_t t, bool accessible) : t(t), accessible(accessible) { }

map::map(const std::string &file, const glw::high::tiledTextureAtlas<8, 8> *a, float &bat, size_t &hand, std::string &tip) : atlas(a) {
	std::ifstream f(file);
	if (!f.good()) {
		tiles = std::vector<std::vector<tile>>{
			{ tile(tile_t::empty), tile(tile_t::empty), tile(tile_t::empty), },
			{ tile(tile_t::empty), tile(tile_t::empty), tile(tile_t::empty), },
			{ tile(tile_t::empty), tile(tile_t::empty), tile(tile_t::empty), },
			{ tile(tile_t::empty, false), tile(tile_t::empty, false), tile(tile_t::empty, false), },
			{ tile(tile_t::win_y), tile(tile_t::win_w), tile(tile_t::goal, false), },
			{ tile(tile_t::win_o), tile(tile_t::win_o2), tile(tile_t::empty, false), },
			{ tile(tile_t::win_u), tile(tile_t::win_n), tile(tile_t::empty, false), },
		};
		startx = 1;
		starty = 1;
		goalx = 4;
		goaly = 2;
		w = tiles.size();
		h = tiles[0].size();
		gen_meshes();
		bat = std::numeric_limits<float>::max();
		hand = -1;
		tip = "you won!";
		return;
	}
	std::string buff;
	std::getline(f, buff);
	w = buff.size();
	tiles = std::vector<std::vector<tile>>(w);
	for (unsigned int i = 0; i < w; i++) {
		tiles[i].reserve(64); // wasting space shouldn't be a big problem :)
	}
	unsigned int y = 0;
	do {
		for (unsigned int i = 0; i < w; i++) {
			switch (buff[i]) {
			case 'S': tiles[i].emplace_back(tile_t::empty); startx = i; starty = y; break;
			case 'g': tiles[i].emplace_back(tile_t::goal, false); goalx = i; goaly = y; break;
			case 'G': tiles[i].emplace_back(tile_t::goal); goalx = i; goaly = y; break;
			case '^': tiles[i].emplace_back(tile_t::stream_up); break;
			case 'V': tiles[i].emplace_back(tile_t::stream_down); break;
			case 'v': tiles[i].emplace_back(tile_t::stream_down, false); break;
			case '<': tiles[i].emplace_back(tile_t::stream_left); break;
			case '>': tiles[i].emplace_back(tile_t::stream_right); break;
			case '=': tiles[i].emplace_back(tile_t::stream_right, false); break;
			case 'L': tiles[i].emplace_back(tile_t::stream_ur); break;
			case 'l': tiles[i].emplace_back(tile_t::stream_ru); break;
			case 'P': tiles[i].emplace_back(tile_t::stream_rd); break;
			case 'p': tiles[i].emplace_back(tile_t::stream_dr); break;
			case 'Q': tiles[i].emplace_back(tile_t::stream_dl); break;
			case 'q': tiles[i].emplace_back(tile_t::stream_ld); break;
			case 'J': tiles[i].emplace_back(tile_t::stream_lu); break;
			case 'j': tiles[i].emplace_back(tile_t::stream_ul); break;
			case 'D': tiles[i].emplace_back(tile_t::dynamite); break;
			case 'W': tiles[i].emplace_back(tile_t::wood); break;
			case 'C': tiles[i].emplace_back(tile_t::coal); break;
			case 's': tiles[i].emplace_back(tile_t::sulphur); break;
			case 'i': tiles[i].emplace_back(tile_t::iron); break;
			case 'I': tiles[i].emplace_back(tile_t::iron_ore); break;
			case '_': tiles[i].emplace_back(tile_t::steel); break;
			case 'E': tiles[i].emplace_back(tile_t::engine); break;
			case 'b': tiles[i].emplace_back(tile_t::booster); break;
			case 'F': tiles[i].emplace_back(tile_t::fuel); break;
			case 'B': tiles[i].emplace_back(tile_t::bridge); break;
			case '{': tiles[i].emplace_back(tile_t::booster_left); break;
			case '}': tiles[i].emplace_back(tile_t::booster_right); break;
			case '*': tiles[i].emplace_back(tile_t::booster_up); break;
			case '+': tiles[i].emplace_back(tile_t::booster_down); break;
			case '!': tiles[i].emplace_back(tile_t::spikes); break;
			case '0': tiles[i].emplace_back(tile_t::tut_broken_ai_stream); break;
			case '1': tiles[i].emplace_back(tile_t::tut_fake_goal); break;
			case '2': tiles[i].emplace_back(tile_t::tut_happy_ai); break;
			case '3': tiles[i].emplace_back(tile_t::tut_blown_up); break;
			case '4': tiles[i].emplace_back(tile_t::tut_dynamite_pile); break;
			case '5': tiles[i].emplace_back(tile_t::tut_slot_wood); break;
			case '6': tiles[i].emplace_back(tile_t::tut_slot_process_arrow); break;
			case '7': tiles[i].emplace_back(tile_t::tut_slot_barrier); break;
			case '8': tiles[i].emplace_back(tile_t::tut_slot_coal); break;
			case '9': tiles[i].emplace_back(tile_t::tut_slot_sulphur); break;
			case '?': tiles[i].emplace_back(tile_t::tut_slot_dynamite); break;
			case 'X': tiles[i].emplace_back(tile_t::tut_slot_bridge); break;
			case 'x': tiles[i].emplace_back(tile_t::tut_slot_iron); break;
			case 'Y': tiles[i].emplace_back(tile_t::tut_slot_steel); break;
			case 'y': tiles[i].emplace_back(tile_t::tut_slot_engine); break;
			case 'Z': tiles[i].emplace_back(tile_t::tut_slot_booster); break;
			case 'z': tiles[i].emplace_back(tile_t::tut_slot_fuel); break;
			case 'w': tiles[i].emplace_back(tile_t::tut_slot_iron_ore); break;
			case ':': tiles[i].emplace_back(tile_t::empty, false); break;
			default: tiles[i].emplace_back(tile_t::empty); break; // case '.':
			}
		}
		y++;
	} while (std::getline(f, buff) && buff.size() > 0);
	h = tiles[0].size();
	gen_meshes();
	f >> bat >> hand;
	std::getline(f, tip); // skip rest
	std::getline(f, tip);
	if(!tip.empty()) {
		size_t start_pos = 0;
		while((start_pos = tip.find("\\n", start_pos)) != std::string::npos) {
			tip.replace(start_pos, 2, "\n");
			start_pos++;
		}
	}
}
bool map::passable(int x, int y) const {
	if (x < 0 || x >= static_cast<int>(w) || y < 0 || y >= static_cast<int>(h)) {
		return false;
	}
	return tiles[x][y].accessible;
}
void map::set_tile_accessible(unsigned int x, unsigned int y, bool access_new) {
	tiles[x][y].accessible = access_new;
	gen_mesh(x / chunk_size, y / chunk_size);
}
void map::set_tile(unsigned int x, unsigned int y, tile_t type_new) {
	tiles[x][y].t = type_new;
	gen_mesh(x / chunk_size, y / chunk_size);
}

void map::gen_meshes() {
	meshes.clear();
	meshesvbo.clear();
	meshesebo.clear();
	meshsizes.clear();
	unsigned int mw = (w + chunk_size - 1) / chunk_size;
	unsigned int mh = (h + chunk_size - 1) / chunk_size;
	meshes.reserve(mw);
	meshesvbo.reserve(mw);
	meshesebo.reserve(mw);
	meshsizes.reserve(mw);
	for (unsigned int x = 0; x < mw; x++) {
		meshes.emplace_back();
		meshesvbo.emplace_back();
		meshesebo.emplace_back();
		meshsizes.emplace_back();
		meshes.back().reserve(mh);
		meshesvbo.back().reserve(mh);
		meshesebo.back().reserve(mh);
		meshsizes.back().reserve(mh);
		for (unsigned int y = 0; y < mh; y++) {
			meshes.back().emplace_back();
			meshesvbo.back().emplace_back();
			meshesebo.back().emplace_back();
			meshsizes.back().emplace_back();
			gen_mesh(x, y);
		}
	}
}
void map::gen_mesh(unsigned int x, unsigned int y) {
	std::vector<float> verts;
	std::vector<unsigned int> indices;
	unsigned int tx_begin = x * chunk_size;
	unsigned int ty_begin = y * chunk_size;
	unsigned int tx_end = std::min(x * chunk_size + chunk_size, w);
	unsigned int ty_end = std::min(y * chunk_size + chunk_size, h);
	verts.reserve(24 * chunk_size * chunk_size);
	indices.reserve(9 * chunk_size * chunk_size);
	for (unsigned int tx = tx_begin; tx != tx_end; tx++) {
		for (unsigned int ty = ty_begin; ty != ty_end; ty++) {
			unsigned int vertindex = verts.size() / 4;
			float u1, v1, u2, v2;
			size_t texid = static_cast<size_t>(tiles[tx][ty].t);
			atlas->getuv(texid, u1, v1, u2, v2);
			verts.insert(verts.end(), {
				static_cast<float>(tx),   static_cast<float>(ty),   u1, v1,
				static_cast<float>(tx+1), static_cast<float>(ty),   u2, v1,
				static_cast<float>(tx+1), static_cast<float>(ty+1), u2, v2,
				static_cast<float>(tx),   static_cast<float>(ty+1), u1, v2,
			});
			indices.insert(indices.end(), {
				vertindex + 0, vertindex + 1, vertindex + 3, // the + 0 is just to look nicer
				vertindex + 1, vertindex + 2, vertindex + 3
			});
			if (!tiles[tx][ty].accessible) {
				atlas->getuv(63, u1, v1, u2, v2);
				verts.insert(verts.end(), {
					static_cast<float>(tx),   static_cast<float>(ty),   u1, v1,
					static_cast<float>(tx+1), static_cast<float>(ty),   u2, v1,
					static_cast<float>(tx+1), static_cast<float>(ty+1), u2, v2,
					static_cast<float>(tx),   static_cast<float>(ty+1), u1, v2,
				});
				indices.insert(indices.end(), {
					vertindex + 4, vertindex + 5, vertindex + 7, // the + 0 is just to look nicer
					vertindex + 5, vertindex + 6, vertindex + 7
				});
			}
		}
	}
	glw::initVaoVboEbo(meshes[x][y], meshesvbo[x][y], meshesebo[x][y], verts.data(), sizeof(float) * verts.size(),
		indices.data(), sizeof(unsigned int) * indices.size(), sizeof(float) * 4, { glw::vap(4) });
	meshsizes[x][y] = indices.size();
}
