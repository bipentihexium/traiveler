#include "character.hpp"

#include <algorithm>
#include <map>
#include <glm/gtx/norm.hpp>
#include "app.hpp"
#include "map.hpp"

constexpr inline int floor(float f) {
	return static_cast<int>(f) > f ? static_cast<int>(f) - 1 : static_cast<int>(f);
}

character::character(float x, float y) : pos(x, y), boosterpush(0.f, 0.f) { }
void character::move(float dt, const map &map, const glm::vec2 &dir) {
	if (map.tiles[floor(pos.x)][floor(pos.y)].t == tile_t::booster_right) { boosterpush = glm::vec2(booster_strength, 0.f); }
	else if (map.tiles[floor(pos.x)][floor(pos.y)].t == tile_t::booster_left) { boosterpush = glm::vec2(-booster_strength, 0.f); }
	else if (map.tiles[floor(pos.x)][floor(pos.y)].t == tile_t::booster_up) { boosterpush = glm::vec2(0.f, -booster_strength); }
	else if (map.tiles[floor(pos.x)][floor(pos.y)].t == tile_t::booster_down) { boosterpush = glm::vec2(0.f, booster_strength); }
	pos += dt * (boosterpush + dir * character_speed);
	boosterpush *= glm::pow(.01f, dt);
	float x1 = floor(pos.x - .5f) + .5f;
	float y1 = floor(pos.y - .5f) + .5f;
	std::vector<glm::vec2> centers({ glm::vec2(x1,y1), glm::vec2(x1+1,y1), glm::vec2(x1,y1+1), glm::vec2(x1+1,y1+1) });
	// remove passable
	centers.erase(std::remove_if(centers.begin(), centers.end(), [&map](const glm::vec2 &c){
		return map.passable(floor(c.x), floor(c.y)); }), centers.end());
	std::sort(centers.begin(), centers.end(), [this](const glm::vec2 &a, const glm::vec2 &b){
		return glm::length2(pos - a) < glm::length2(pos - b); });
	for (const glm::vec2 &block : centers) {
		glm::vec2 overlap(glm::vec2(.9f, .9f) - glm::abs(pos - block));
		if (overlap.x > overlap.y) {
			if (overlap.y > 0) {
				if (pos.y > block.y)
					pos.y += overlap.y;
				else
					pos.y -= overlap.y;
			}
		} else {
			if (overlap.x > 0) {
				if (pos.x > block.x)
					pos.x += overlap.x;
				else
					pos.x -= overlap.x;
			}
		}
	}
}

std::map<item_t, item_t> ai::single_recipes{
	{ item_t::wood, item_t::barrier },
	{ item_t::iron_ore, item_t::iron },
	{ item_t::coal, item_t::fuel },
	{ item_t::iron, item_t::bridge },
};
std::map<std::pair<item_t, item_t>, item_t> ai::double_recipes{
	{ { item_t::coal, item_t::sulphur }, item_t::dynamite },
	{ { item_t::sulphur, item_t::coal }, item_t::dynamite },
	{ { item_t::iron_ore, item_t::coal }, item_t::steel },
	{ { item_t::coal, item_t::iron_ore }, item_t::steel },
	{ { item_t::steel, item_t::iron }, item_t::engine },
	{ { item_t::iron, item_t::steel }, item_t::engine },
	{ { item_t::engine, item_t::fuel }, item_t::booster },
	{ { item_t::fuel, item_t::engine }, item_t::booster },
};

ai::ai(float x, float y) : character(x, y), slots{item_t::empty,item_t::empty,item_t::empty,item_t::empty,item_t::empty},
	process_single(0), process_double(0), a_star_retry_timer(0) { }
void ai::tick(GLFWwindow *w, float dt, const map &map) {
	(void)w;
	if (slots[slot_single_in] != item_t::empty && slots[slot_single_out] == item_t::empty) {
		auto it = single_recipes.find(slots[slot_single_in]);
		if (it != single_recipes.end()) {
			process_single += dt * process_speed;
			if (process_single >= 1.f) {
				process_single = 0.f;
				slots[slot_single_in] = item_t::empty;
				slots[slot_single_out] = it->second;
			}
		} else {
			process_single = 0;
		}
	} else {
		process_single = 0;
	}
	if (slots[slot_double_in_a] != item_t::empty && slots[slot_double_in_b] != item_t::empty && slots[slot_double_out] == item_t::empty) {
		auto it = double_recipes.find(std::make_pair(slots[slot_double_in_a], slots[slot_double_in_b]));
		if (it != double_recipes.end()) {
			process_double += dt * process_speed;
			if (process_double >= 1.f) {
				process_double = 0.f;
				slots[slot_double_in_a] = item_t::empty;
				slots[slot_double_in_b] = item_t::empty;
				slots[slot_double_out] = it->second;
			}
		} else {
			process_double = 0;
		}
	} else {
		process_double = 0;
	}
	battery -= dt;
	if (path.empty()) {
		if (a_star_retry_timer > 0) {
			a_star_retry_timer -= dt;
			move(dt, map, glm::vec2(0.f, 0.f));
			return;
		}
		a_star(map);
		if (path.empty()) {
			a_star_retry_timer = .5f;
			move(dt, map, glm::vec2(0.f, 0.f));
			return;
		} else {
			build_path();
			path_abort_timer = 3.f;
			a_star_retry_timer = 4.f;
		}
	}
	a_star_retry_timer -= dt;
	if (a_star_retry_timer <= 0) {
		a_star(map);
		if (path.empty()) {
			a_star_retry_timer = .5f;
			move(dt, map, glm::vec2(0.f, 0.f));
			return;
		} else {
			build_path();
			path_abort_timer = 3.f;
			a_star_retry_timer = 4.f;
		}
	}
	if (glm::length2(path.back()-pos) <= character_speed * character_speed * 0.01f ) {
		path_abort_timer = 3.f;
		path.pop_back();
		build_path();
	}
	path_abort_timer -= dt;
	if (path_abort_timer <= 0 || (!path.empty() && glm::length2(pos - path.back()) > 4.f)) {
		path.clear();
		if (pathvao.exists) pathvao.del();
		if (pathvbo.exists) pathvbo.del();
		a_star_retry_timer = 0.f;
	}
	if (!path.empty()) {
		move(dt, map, glm::normalize(path.back()-pos));
	} else {
		move(dt, map, glm::vec2(0.f, 0.f));
	}
}
void ai::a_star(const map &map) {
	const auto hfunc = [&map](unsigned int x, unsigned int y) -> unsigned int {
		unsigned int distx = std::abs(static_cast<int>(map.goalx) - static_cast<int>(x));
		unsigned int disty = std::abs(static_cast<int>(map.goaly) - static_cast<int>(y));
		return std::max(distx, disty) * 9 + std::min(distx, disty) * 3;
	};
	unsigned int checked = 0;
	unsigned int startx = floor(pos.x);
	unsigned int starty = floor(pos.y);
	if (startx == map.goalx && starty == map.goaly) {
		path.clear();
		path.emplace_back(map.goalx + .5f, map.goaly + .5f);
		return;
	}
	unsigned int startnode = startx + starty * map.w;
	unsigned int starth = hfunc(startx, starty);
	std::multimap<unsigned int, std::pair<unsigned int, unsigned int>> to_search;
	to_search.insert(std::make_pair(starth, std::make_pair(startnode, 0)));
	std::vector<unsigned char> paths(map.w * map.h, 0xff);
	paths[startnode] = 0x90;
	unsigned int goal = map.goalx + map.w * map.goaly;
	const int ox[4] = { 0, -1,1, 0 };
	const int oy[4] = { -1, 0,0, 1 };
	const int on[4] = { -static_cast<int>(map.w), -1,1, static_cast<int>(map.w) };
	const int dx[4] = { -1,1,  -1,1 };
	const int dy[4] = { -1,-1,  1,1 };
	const int dn[4] = { -static_cast<int>(map.w)-1,-static_cast<int>(map.w)+1, static_cast<int>(map.w)-1,static_cast<int>(map.w)+1 };
	const int an[8] = {
		-static_cast<int>(map.w), -1,1, static_cast<int>(map.w), // on
		-static_cast<int>(map.w)-1,-static_cast<int>(map.w)+1, static_cast<int>(map.w)-1,static_cast<int>(map.w)+1 // dn
	};
	while (!to_search.empty()) {
		checked++;
		unsigned int node = to_search.begin()->second.first;
		if (node == goal) {
			//std::cout << "found path! (" << checked << " nodes checked)" << std::endl;
			path.clear();
			path.reserve(map.w);
			unsigned int at = node;
			paths[node] -= 0x80;
			while (paths[at] != 0x10) {
				unsigned int x = at % map.w;
				unsigned int y = at / map.w;
				path.emplace_back(x + .5f, y + .5f);
				at -= an[paths[at]];
			}
			return;
		}
		unsigned int g = to_search.begin()->second.second + 10;
		to_search.erase(to_search.begin());
		paths[node] -= 0x80;
		unsigned int nx = node % map.w;
		unsigned int ny = node / map.w;
		for (unsigned int i = 0; i < 4; i++) {
			{
				unsigned int orth = node + on[i];
				int x = static_cast<int>(node % map.w) + ox[i];
				int y = static_cast<int>(node / map.w) + oy[i];
				if (x >= 0 && y >= 0 && x < static_cast<int>(map.w) && y < static_cast<int>(map.h) && map.tiles[x][y].accessible &&
					paths[orth] >= 0x80) {
					unsigned int h = hfunc(x, y);
					if (paths[orth] != 0xff) {
						auto it = std::find_if(to_search.lower_bound(g + h + 1), to_search.end(),
							[&node](const std::pair<unsigned int, std::pair<unsigned int, unsigned int>> &n){ return n.second.first == node; });
						if (it == to_search.end()) {
							goto skip_orth;
						}
						to_search.erase(it);
					}
					paths[orth] = 0x80 + i;
					to_search.insert(std::make_pair(g + h, std::make_pair(orth, g)));
				}
			}
		skip_orth:
			{
				unsigned int diag = node + dn[i];
				int x = static_cast<int>(node % map.w) + dx[i];
				int y = static_cast<int>(node / map.w) + dy[i];
				if (x >= 0 && y >= 0 && x < static_cast<int>(map.w) && y < static_cast<int>(map.h) &&
					map.tiles[x][y].accessible && (map.tiles[nx][y].accessible || map.tiles[x][ny].accessible) &&
					paths[diag] >= 0x80) {
					unsigned int h = hfunc(x, y);
					if (paths[diag] != 0xff) {
						auto it = std::find_if(to_search.lower_bound(g + h + 5), to_search.end(),
							[&node](const std::pair<unsigned int, std::pair<unsigned int, unsigned int>> &n){ return n.second.first == node; });
						if (it == to_search.end()) {
							continue;
						}
						to_search.erase(it);
					}
					paths[diag] = 0x84 + i;
					to_search.insert(std::make_pair(g + h + 4, std::make_pair(diag, g + 4)));
				}
			}
		}
	}
	// path not found!
	//std::cout << "could not find path! (" << checked << " nodes checked)" << std::endl;
}
void ai::build_path() {
	if (path.empty())
		return;
	std::vector<float> verts;
	verts.reserve(3 * path.size() + 3);
	for (size_t i = 0; i < path.size(); i++) {
		verts.insert(verts.end(), { path[i].x, path[i].y, static_cast<float>(path.size() - i) });
	}
	verts.insert(verts.end(), { pos.x, pos.y, 0.f });
	if (pathvao.exists) pathvao.del();
	if (pathvbo.exists) pathvbo.del();
	pathsize = verts.size() / 3;
	glw::initVaoVbo(pathvao, pathvbo, verts.data(), sizeof(float) * (verts.size() + 3), sizeof(float) * 3, { glw::vap(3) });
}

player::player(float x, float y) : character(x, y), walking(false) { }
void player::tick(GLFWwindow *w, float dt, const map &map) {
	glm::vec2 dir(0.f, 0.f);
#define PRESSED(k) (glfwGetKey(w, GLFW_KEY_ ## k) == GLFW_PRESS)
	if (PRESSED(W) || PRESSED(UP)) { dir.y -= player_speed_mult; }
	if (PRESSED(S) || PRESSED(DOWN)) { dir.y += player_speed_mult; }
	if (PRESSED(A) || PRESSED(LEFT)) { dir.x -= player_speed_mult; }
	if (PRESSED(D) || PRESSED(RIGHT)) { dir.x += player_speed_mult; }
#undef PRESSED
	move(dt, map, dir);
	walking = glm::length2(dir) > 0.1f;
}
void player::click(app &a, map &m, unsigned int x, unsigned int y) {
	if (glm::length2(pos - glm::vec2(x + .5f, y + .5f)) > 25.f) {
		return;
	}
#define PICKUP(item) case tile_t::item: hand = item_t::item; m.set_tile(x, y, tile_t::empty); break;
#define PLACE(item) case item_t::item: if (m.tiles[x][y].t == tile_t::empty) { m.set_tile(x, y, tile_t::item); hand = item_t::empty; } break;
	switch (hand) {
	case item_t::empty:
		switch (m.tiles[x][y].t) {
		case tile_t::tut_dynamite_pile: hand = item_t::dynamite; break;
		PICKUP(dynamite)
		PICKUP(barrier)
		PICKUP(wood)
		PICKUP(coal)
		PICKUP(sulphur)
		PICKUP(iron_ore)
		PICKUP(iron)
		PICKUP(steel)
		PICKUP(engine)
		PICKUP(fuel)
		PICKUP(booster)
		PICKUP(bridge)
		default: break;
		}
		break;
	case item_t::dynamite:
		m.set_tile_accessible(x, y);
		a.spawn_explosion(x, y);
		hand = item_t::empty;
		break;
	case item_t::barrier:
		m.set_tile_accessible(x, y, false);
		hand = item_t::empty;
		break;
	PLACE(wood)
	PLACE(coal)
	PLACE(sulphur)
	PLACE(iron_ore)
	PLACE(iron)
	PLACE(steel)
	PLACE(engine)
	PLACE(fuel)
	case item_t::booster:
		if (m.tiles[x][y].t == tile_t::empty) {
			hand = item_t::empty;
			if (glm::abs(pos.x-x-.5f) > glm::abs(pos.y-y-.5f)) {
				if (pos.x > x) {
					m.set_tile(x, y, tile_t::booster_left);
				} else {
					m.set_tile(x, y, tile_t::booster_right);
				}
			} else {
				if (pos.y > y) {
					m.set_tile(x, y, tile_t::booster_up);
				} else {
					m.set_tile(x, y, tile_t::booster_down);
				}
			}
		}
		break;
	case item_t::bridge:
		if (m.tiles[x][y].t >= tile_t::stream_down && m.tiles[x][y].t <= tile_t::stream_ul) {
			hand = item_t::empty;
			m.set_tile(x, y, tile_t::built_bridge);
		}
		break;
	default:
		break;
	}
#undef PICKUP
#undef PLACE
}
