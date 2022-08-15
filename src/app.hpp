#ifndef __APP_HPP__
#define __APP_HPP__

#include <memory>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glgui.hpp"
#include "character.hpp"
#include "map.hpp"
#include "sound.hpp"

enum class game_stage {
	MENU, GAME, PAUSE
};

struct explosion {
	constexpr static int frame_count = 4;
	float x, y;
	unsigned int frame;
	explosion(float x, float y);
};

class volume_bar : public glgui::control {
public:
	glm::fvec2 charsize;
	glm::vec3 colorfill;
	glm::vec3 colorbg;
	int value = 100;

	void mousedown(int mb, float mx, float my) override;
	void render(const glm::mat4 &proj) const override;
protected:
};
class image : public glgui::control {
public:
	glw::tex2 *img;

	void render(const glm::mat4 &proj) const override;
protected:
};

class app {
public:
	GLFWwindow *w;
	float dt;
	int width, height;

	app(int width, int height, const char *title);
	~app();
	void mainloop();
	void resize(int width, int height);
	void click(int button, int action, int mod);
	void scroll(double x, double y);
	void key(int key, int action, int mod);
	void write(unsigned int codepoint);
	void spawn_explosion(float x, float y);
	void play();
	void pause();
	void resume();
	void update_volume();
private:
	std::optional<map> current_map;
	std::optional<player> pl;
	std::optional<ai> aipl;
	glw::shader s2;
	glw::shader spath;
	glw::shader s2uvmap;
	glw::high::tiledTextureAtlas<8, 8> tilemap[16];
	glw::high::tiledTextureAtlas<4, 4> itemmap;
	glw::high::tiledTextureAtlas<1, 4> explosionmap;
	glw::tex2 pltex;
	glw::tex2 plwalk1tex;
	glw::tex2 plwalk2tex;
	glw::tex2 aitex;
	glw::tex2 ai2tex;
	glw::tex2 ainobatterytex;
	glw::tex2 cursortex;
	glw::tex2 slottex;
	glw::tex2 arrowtex;
	glw::tex2 processarrowtex;
	glw::tex2 slotcursortex;
	glw::tex2 batterytex;
	glw::tex2 fullbatterytex;
	glw::tex2 overlay;
	glw::tex2 titletex;
	glw::vao quad;
	glw::vbo quadvbo;
	glw::ebo quadebo;
	glgui::container gui;
	glgui::label tiplabel;
	glgui::label playinglabel;
	image titleimg;
	glgui::button playbutton;
	glgui::button continuebutton;
	glgui::label volumesfxlabel;
	glgui::label volumemusiclabel;
	volume_bar sfxvolumebar;
	volume_bar musicvolumebar;
	glm::vec2 cam;
	int level;
	bool clicked;
	float animcounter;
	unsigned int tilemap_frame;
	unsigned int player_walk_frame;
	unsigned int ai_frame;
	std::vector<explosion> explosions;
	float lose_timer;
	bool player_dead;
	game_stage stage;
	std::unique_ptr<sound_manager> sounds;

	void tick();
	void load(int level_id);
	void tick_anim();
};

#endif
