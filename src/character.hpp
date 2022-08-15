#ifndef __CHARACTER_HPP__
#define __CHARACTER_HPP__

#include <map>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glw.hpp"

class map;

enum class item_t {
	empty = -1,
	dynamite = 0,
	barrier,
	wood,
	coal,
	sulphur,
	iron_ore,
	iron,
	steel,
	engine,
	booster,
	fuel,
	bridge,
};

class character {
public:
	constexpr static float character_speed = 1.f;
	constexpr static float booster_strength = 25.f;
	glm::vec2 pos;
	glm::vec2 boosterpush;

	character(float x, float y);
	virtual void tick(GLFWwindow *w, float dt, const map &map) = 0;
protected:
	void move(float dt, const map &map, const glm::vec2 &dir);
};
class ai : public character {
public:
	constexpr static float process_speed = 2.f;
	enum slot_id {
		slot_single_in, slot_single_out, slot_double_in_a, slot_double_in_b, slot_double_out
	};
	glw::vao pathvao;
	size_t pathsize;
	float battery, batterymax;
	item_t slots[5];
	float process_single, process_double;

	ai(float x, float y);
	void tick(GLFWwindow *w, float dt, const map &map) override;
private:
	static std::map<item_t, item_t> single_recipes;
	static std::map<std::pair<item_t, item_t>, item_t> double_recipes;
	std::vector<glm::vec2> path;
	glw::vbo pathvbo;
	float a_star_retry_timer;
	float path_abort_timer;

	void a_star(const map &map);
	void build_path();
};
class app;
class player : public character {
public:
	constexpr static float player_speed_mult = 5.f;
	item_t hand;
	bool walking;

	player(float x, float y);
	void tick(GLFWwindow *w, float dt, const map &map) override;
	void click(app &a, map &m, unsigned int x, unsigned int y);
private:
};

#endif
