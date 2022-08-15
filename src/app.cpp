#include "app.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "glw.hpp"

explosion::explosion(float x, float y) : x(x), y(y), frame(0) { }

extern app *a; // don't look here, ugly code cuz I need to be fast
void volume_bar::mousedown(int mb, float mx, float my) {
	(void)my;
	if (mb == GLFW_MOUSE_BUTTON_LEFT) {
		value = static_cast<int>(100.f * mx / (size.x - 1));
		a->update_volume();
	}
}
void volume_bar::render(const glm::mat4 &proj) const {
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", glm::scale(proj, glm::vec3(size, 1.f)));
	glgui::guish.uniform3f("col", colorbg);
	glgui::guish.uniform1i("usetex", 0);
	glgui::quad.bind();
	glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
	glgui::guish.uniformM4f("proj", glm::scale(proj, glm::vec3(size.x * value / 100.f, size.y, 1.f)));
	glgui::guish.uniform3f("col", colorfill);
	glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
}
void image::render(const glm::mat4 &proj) const {
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", glm::scale(proj, glm::vec3(size, 1.f)));
	glgui::guish.uniform3f("col", glm::vec3(1.f, 1.f, 1.f));
	glgui::guish.uniform1i("usetex", 1);
	img->bind();
	glgui::quad.bind();
	glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
}

void call_play(void *app_ptr) {
	((app *)app_ptr)->play();
}
void call_resume(void *app_ptr) {
	((app *)app_ptr)->resume();
}

app::app(int width, int height, const char *title) : width(width), height(height), clicked(false) {
	w = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!w) {
		throw std::runtime_error("Failed to create window!");
	}
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to load OpenGL! (It's version 4.0 core. Does your GPU support it?)");
	}
	resize(width, height);
#ifndef NDEBUG
	glw::checkError("preload", glw::justPrint);
#endif
	float qv[16] = {
		0, 0, 0, 0,
		1, 0, 1, 0,
		1, 1, 1, 1,
		0, 1, 0, 1
	};
	unsigned int qi[6] = { 0, 1, 3,   1, 2, 3 };
	glw::initVaoVboEbo(quad, quadvbo, quadebo, qv, sizeof(float) * 16, qi, sizeof(unsigned int) * 6, sizeof(float) * 4, { glw::vap(4) });
#ifndef NDEBUG
	glw::checkError("vaos", glw::justPrint);
#endif
	for (tilemap_frame = 0; tilemap_frame < 16; tilemap_frame++) {
		tilemap[tilemap_frame].fromFile(("./assets/texture/tiles" + std::to_string(tilemap_frame) + ".png").c_str(),
			glw::justPrint, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST);
	}
	itemmap.fromFile("./assets/texture/items.png", glw::justPrint, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST);
	explosionmap.fromFile("./assets/texture/explosion.png", glw::justPrint, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST);
	pltex.gen();
	pltex.bind();
	pltex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	pltex.fromFile("./assets/texture/player.png", glw::justPrint, "failed to load texture ");
	plwalk1tex.gen();
	plwalk1tex.bind();
	plwalk1tex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	plwalk1tex.fromFile("./assets/texture/player_walk1.png", glw::justPrint, "failed to load texture ");
	plwalk2tex.gen();
	plwalk2tex.bind();
	plwalk2tex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	plwalk2tex.fromFile("./assets/texture/player_walk2.png", glw::justPrint, "failed to load texture ");
	aitex.gen();
	aitex.bind();
	aitex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	aitex.fromFile("./assets/texture/ai.png", glw::justPrint, "failed to load texture ");
	ai2tex.gen();
	ai2tex.bind();
	ai2tex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	ai2tex.fromFile("./assets/texture/ai2.png", glw::justPrint, "failed to load texture ");
	ainobatterytex.gen();
	ainobatterytex.bind();
	ainobatterytex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	ainobatterytex.fromFile("./assets/texture/ai_low_battery.png", glw::justPrint, "failed to load texture ");
	cursortex.gen();
	cursortex.bind();
	cursortex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	cursortex.fromFile("./assets/texture/cursor.png", glw::justPrint, "failed to load texture ");
	slottex.gen();
	slottex.bind();
	slottex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	slottex.fromFile("./assets/texture/slot.png", glw::justPrint, "failed to load texture ");
	arrowtex.gen();
	arrowtex.bind();
	arrowtex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	arrowtex.fromFile("./assets/texture/arrow.png", glw::justPrint, "failed to load texture ");
	processarrowtex.gen();
	processarrowtex.bind();
	processarrowtex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	processarrowtex.fromFile("./assets/texture/arrow_processed.png", glw::justPrint, "failed to load texture ");
	slotcursortex.gen();
	slotcursortex.bind();
	slotcursortex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	slotcursortex.fromFile("./assets/texture/slot_cursor.png", glw::justPrint, "failed to load texture ");
	batterytex.gen();
	batterytex.bind();
	batterytex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	batterytex.fromFile("./assets/texture/battery.png", glw::justPrint, "failed to load texture ");
	fullbatterytex.gen();
	fullbatterytex.bind();
	fullbatterytex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	fullbatterytex.fromFile("./assets/texture/battery_full.png", glw::justPrint, "failed to load texture ");
	overlay.gen();
	overlay.bind();
	overlay.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_LINEAR);
	overlay.fromFile("./assets/texture/bg_overlay.png", glw::justPrint, "failed to load texture ");
	titletex.gen();
	titletex.bind();
	titletex.setWrapFilter({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE }, GL_LINEAR, GL_NEAREST);
	titletex.fromFile("./assets/texture/title.png", glw::justPrint, "failed to load texture ");
#ifndef NDEBUG
	glw::checkError("textures", glw::justPrint);
#endif
	glw::compileShaderFromFile(s2, "./assets/shader/s2", glw::default_shader_error_handler());
	s2.use();
	s2.uniform1i("tex", 0);
	glw::compileShaderFromFile(spath, "./assets/shader/path", glw::default_shader_error_handler());
	glw::compileShaderFromFile(s2uvmap, "./assets/shader/s2uvmap.vert", "./assets/shader/s2.frag", glw::default_shader_error_handler());
	s2uvmap.use();
	s2uvmap.uniform1i("tex", 0);
#ifndef NDEBUG
	glw::checkError("shaders", glw::justPrint);
#endif
	glgui::init("./assets/shader", "./assets/texture/font.png", glw::justPrint, glw::default_shader_error_handler());
	tiplabel.pos = glm::vec2(0, 10);
	tiplabel.anch = glgui::anchor::TOPMID;
	tiplabel.align = glgui::anchor::TOPMID;
	tiplabel.color = glm::vec3(1, 1, 1);
	tiplabel.outline = true;
	tiplabel.charsize = glm::vec2(12, 24);
	tiplabel.setText("");
	gui.controls.push_back(&tiplabel);
	playinglabel.pos = glm::vec2(-10, -10);
	playinglabel.anch = glgui::anchor::BOTRIGHT;
	playinglabel.align = glgui::anchor::BOTRIGHT;
	playinglabel.color = glm::vec3(1, 1, 1);
	playinglabel.outline = true;
	playinglabel.charsize = glm::vec2(12, 24);
	playinglabel.setText("playing: nothing yet");
	gui.controls.push_back(&playinglabel);
	titleimg.pos = glm::vec2(0, -100);
	titleimg.size = glm::vec2(512, 128);
	titleimg.anch = glgui::anchor::CENTER;
	titleimg.align = glgui::anchor::CENTER;
	titleimg.img = &titletex;
	gui.controls.push_back(&titleimg);
	playbutton.pos = glm::vec2(0, 50);
	playbutton.size = glm::vec2(200, 50);
	playbutton.anch = glgui::anchor::CENTER;
	playbutton.align = glgui::anchor::CENTER;
	playbutton.textalign = glgui::anchor::CENTER;
	playbutton.bgcolor = glm::vec3(.3f, 1.f, 1.f);
	playbutton.charsize = glm::vec2(24, 48);
	playbutton.setText("play");
	playbutton.data = this;
	playbutton.cb = call_play;
	gui.controls.push_back(&playbutton);
	continuebutton.pos = glm::vec2(0, 0);
	continuebutton.size = glm::vec2(200, 50);
	continuebutton.anch = glgui::anchor::CENTER;
	continuebutton.align = glgui::anchor::CENTER;
	continuebutton.textalign = glgui::anchor::CENTER;
	continuebutton.bgcolor = glm::vec3(.3f, 1.f, 1.f);
	continuebutton.charsize = glm::vec2(24, 48);
	continuebutton.setText("continue");
	continuebutton.data = this;
	continuebutton.cb = call_resume;
	continuebutton.visible = false;
	gui.controls.push_back(&continuebutton);
	volumemusiclabel.pos = glm::vec2(-110, 5);
	volumemusiclabel.anch = glgui::anchor::TOPRIGHT;
	volumemusiclabel.align = glgui::anchor::TOPRIGHT;
	volumemusiclabel.color = glm::vec3(1, 1, 1);
	volumemusiclabel.outline = true;
	volumemusiclabel.charsize = glm::vec2(10, 20);
	volumemusiclabel.setText("music volume");
	gui.controls.push_back(&volumemusiclabel);
	volumesfxlabel.pos = glm::vec2(-110, 30);
	volumesfxlabel.anch = glgui::anchor::TOPRIGHT;
	volumesfxlabel.align = glgui::anchor::TOPRIGHT;
	volumesfxlabel.color = glm::vec3(1, 1, 1);
	volumesfxlabel.outline = true;
	volumesfxlabel.charsize = glm::vec2(10, 20);
	volumesfxlabel.setText("sfx volume");
	gui.controls.push_back(&volumesfxlabel);
	musicvolumebar.pos = glm::vec2(-5, 5);
	musicvolumebar.size = glm::vec2(100, 20);
	musicvolumebar.anch = glgui::anchor::TOPRIGHT;
	musicvolumebar.align = glgui::anchor::TOPRIGHT;
	musicvolumebar.colorbg = glm::vec3(.2f, .2f, .2f);
	musicvolumebar.colorfill = glm::vec3(.3f, 1.f, 1.f);
	gui.controls.push_back(&musicvolumebar);
	sfxvolumebar.pos = glm::vec2(-5, 30);
	sfxvolumebar.size = glm::vec2(100, 20);
	sfxvolumebar.anch = glgui::anchor::TOPRIGHT;
	sfxvolumebar.align = glgui::anchor::TOPRIGHT;
	sfxvolumebar.colorbg = glm::vec3(.2f, .2f, .2f);
	sfxvolumebar.colorfill = glm::vec3(.3f, 1.f, 1.f);
	gui.controls.push_back(&sfxvolumebar);
	gui.pos = glm::vec2(0, 0);
	gui.size = glm::vec2(width, height);
	gui.anch = glgui::anchor::TOPLEFT;
	gui.align = glgui::anchor::TOPLEFT;
	gui.unfocus();
	gui.init(w);
#ifndef NDEBUG
	glw::checkError("glgui init", glw::justPrint);
#endif
	tilemap_frame = 0;
	std::vector<std::pair<std::string, std::string>> music;
	std::ifstream musicf("./assets/sound/music.txt");
	std::string mbuff;
	while (std::getline(musicf, mbuff)) {
		if (mbuff.empty() || mbuff[0] == '#') {
			continue;
		}
		size_t i = mbuff.find(':');
		music.emplace_back(std::make_pair("./assets/sound/" + mbuff.substr(0, i) + ".ogg", mbuff.substr(i + 1)));
	}
	sounds = std::unique_ptr<sound_manager>(new sound_manager({ "./assets/sound/explosion.wav" }, music));
	level = -1;
	stage = game_stage::MENU;
}
app::~app() {
	glgui::terminate();
}
void app::mainloop() {
#ifndef NDEBUG
	glw::checkError("pretick check", glw::justPrint);
#endif
	double prev = glfwGetTime();
	while (!glfwWindowShouldClose(w)) {
		double now = glfwGetTime();
		dt = static_cast<float>(now-prev);
		prev = now;
		glfwPollEvents();
		tick();
		glfwSwapBuffers(w);
#ifndef NDEBUG
		glw::checkError("tick check", glw::justPrint);
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
void app::resize(int width, int height) {
	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);
	gui.size = glm::ivec2(width, height);
	gui.resize();
}
void app::click(int button, int action, int mod) {
	(void)mod;
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	if (action == GLFW_PRESS) {
		gui.mousedown(button, static_cast<float>(mx), static_cast<float>(my));
		if (button == GLFW_MOUSE_BUTTON_LEFT && stage == game_stage::GAME)  {
			clicked = true;
		}
	} else if (action == GLFW_RELEASE) {
		gui.mouseup(button, static_cast<float>(mx), static_cast<float>(my));
	}
}
void app::scroll(double x, double y) {
	(void)x; (void)y;
}
void app::key(int key, int action, int mod) {
	(void)mod;
	if (stage == game_stage::GAME && action == GLFW_PRESS) {
		if (key == GLFW_KEY_R && current_map.has_value()) {
			load(level);
		}
		if (key == GLFW_KEY_ESCAPE) {
			pause();
		}
	} else {
		if (stage == game_stage::PAUSE && action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
			resume();
		}
	}
}
void app::write(unsigned int codepoint) {
	(void)codepoint;
}
void app::spawn_explosion(float x, float y) {
	explosions.emplace_back(x, y);
	sounds->play_sfx(0);
	if (pl.has_value() && glm::length2(pl->pos - glm::vec2(x + .5f, y + .5f)) < 1.44f) {
		player_dead = true;
		lose_timer += dt;
	}
}
void app::play() {
	titleimg.visible = false;
	playbutton.visible = false;
	volumemusiclabel.visible = false;
	volumesfxlabel.visible = false;
	musicvolumebar.visible = false;
	sfxvolumebar.visible = false;
	stage = game_stage::GAME;
	level = 0;
	load(level);
}
void app::pause() {
	stage = game_stage::PAUSE;
	continuebutton.visible = true;
	volumemusiclabel.visible = true;
	volumesfxlabel.visible = true;
	musicvolumebar.visible = true;
	sfxvolumebar.visible = true;
}
void app::resume() {
	continuebutton.visible = false;
	volumemusiclabel.visible = false;
	volumesfxlabel.visible = false;
	musicvolumebar.visible = false;
	sfxvolumebar.visible = false;
	stage = game_stage::GAME;
}
void app::update_volume() {
	sounds->sfx_volume(sfxvolumebar.value);
	sounds->music_volume(musicvolumebar.value);
}

void app::tick() {
	animcounter += dt;
	if (animcounter > .5f) {
		animcounter = .5f;
	}
	while (animcounter > .1f) {
		animcounter -= .1f;
		tick_anim();
	}
	float realdt = dt;
	if (stage != game_stage::GAME) {
		clicked = false;
		dt = 0.f;
	}
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	int mtx = -1, mty = -1;
	float mouse_map_x = 0.f, mouse_map_y = 0.f;
	bool mouse_in_bounds = false;
	if (current_map.has_value()) {
		if (pl.has_value()) {
			float tilesize = std::max(width / 20.f, height / 20.f);
			mouse_map_x = pl->pos.x + (static_cast<float>(mx) - width / 2) / tilesize;
			mouse_map_y = pl->pos.y + (static_cast<float>(my) - height / 2) / tilesize;
			mtx = static_cast<int>(glm::floor(mouse_map_x));
			mty = static_cast<int>(glm::floor(mouse_map_y));
			if (mtx >= 0 && mty >= 0 && mtx < static_cast<int>(current_map->w) && mty < static_cast<int>(current_map->h)) {
				mouse_in_bounds = true;
				/*if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) {
					current_map->set_tile_accessible(mtx, mty, true);
				} else if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) {
					current_map->set_tile_accessible(mtx, mty, false);
				}*/
			}

			if (!player_dead) {
				pl->tick(w, dt, current_map.value());
				if (current_map->tiles[static_cast<unsigned int>(pl->pos.x)][static_cast<unsigned int>(pl->pos.y)].t == tile_t::spikes) {
					player_dead = true;
					lose_timer += dt;
				}
			}
			cam = pl->pos;
		}
		if (lose_timer > 0.f) {
			lose_timer += dt;
			if (lose_timer > .5f) {
				load(level);
			}
		} else {
			if (aipl.has_value()) {
				aipl->tick(w, (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) ? dt * 2.5f : dt, current_map.value());
				tile_t under_ai = current_map->tiles[static_cast<size_t>(glm::floor(aipl->pos.x))][static_cast<size_t>(glm::floor(aipl->pos.y))].t;
				if (under_ai >= tile_t::stream_down && under_ai <= tile_t::stream_ul) {
					lose_timer += dt;
					spawn_explosion(aipl->pos.x - .5f, aipl->pos.y - .5f);
				} else if (aipl->battery < 0.f) {
					lose_timer += dt;
				}
			}
		}
	}

	glm::mat4 proj = glm::ortho<float>(0, width, height, 0);
	glClearColor(.2f, .2f, .2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	if (current_map.has_value()) {
		s2.use();
		glm::mat4 mat(proj);
		mat = glm::translate(mat, glm::vec3(width / 2.f, height / 2.f, 0.f));
		float scale = std::max(width / 20.f, height / 20.f);
		mat = glm::scale(mat, glm::vec3(scale, scale, 1.f));
		mat = glm::translate(mat, glm::vec3(-cam.x, -cam.y, 0.f));
		s2.uniformM4f("proj", mat);
		tilemap[tilemap_frame].atlas.bind(GL_TEXTURE0);
		for (unsigned int x = 0; x < (current_map->w + map::chunk_size - 1) / map::chunk_size; x++) { // TODO: optimize (tilemap frustrum culling)
			for (unsigned int y = 0; y < (current_map->h + map::chunk_size - 1) / map::chunk_size; y++) {
				current_map->meshes[x][y].bind();
				current_map->meshes[x][y].drawElements(current_map->meshsizes[x][y]);
			}
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (aipl.has_value() && aipl->pathvao.exists && lose_timer == 0.f) {
			glLineWidth(3.f);
			spath.use();
			spath.uniformM4f("proj", mat);
			aipl->pathvao.bind();
			aipl->pathvao.drawArrays(aipl->pathsize, GL_LINE_STRIP);
			glLineWidth(1.f);
		}
		s2.use();
		quad.bind();
		if (aipl.has_value() && lose_timer == 0.f && pl.has_value() && glm::length2(aipl->pos - pl->pos) <= 9.f) {
			slottex.bind(GL_TEXTURE0);
			glm::mat4 slotmat(glm::translate(mat, glm::vec3(aipl->pos.x - .5f, aipl->pos.y - .5f, 0.f)));
			slotmat = glm::translate(slotmat, glm::vec3(-1.f, -3.f, 0.f));
			auto slotfunc = [this, &mouse_map_x, &mouse_map_y, &mouse_in_bounds, &slotmat](float x, float y, ai::slot_id slot) -> bool {
				float mouse_ai_rel_x = mouse_map_x - aipl->pos.x + 1.5f;
				float mouse_ai_rel_y = mouse_map_y - aipl->pos.y + 3.5f;
				bool hover = mouse_ai_rel_x > x && mouse_ai_rel_x < x + 1 && mouse_ai_rel_y > y && mouse_ai_rel_y < y + 1;
				const glm::mat4 matrix = glm::translate(slotmat, glm::vec3(x, y, 0.f));
				s2.uniformM4f("proj", matrix);
				if (hover) {
					slotcursortex.bind(GL_TEXTURE0);
					quad.drawElements(6);
					slottex.bind(GL_TEXTURE0);
					mouse_in_bounds = false;
					if (clicked) {
						item_t tmp = aipl->slots[slot];
						aipl->slots[slot] = pl->hand;
						pl->hand = tmp;
						if (slot == ai::slot_single_in) {
							aipl->process_single = 0.f;
						}
						if (slot == ai::slot_double_in_a || slot == ai::slot_double_in_b) {
							aipl->process_double = 0.f;
						}
					}
				} else {
					quad.drawElements(6);
				}
				if (aipl->slots[slot] != item_t::empty) {
					s2uvmap.use();
					s2uvmap.uniformM4f("proj", matrix);
					float u1, v1, u2, v2;
					itemmap.getuv(static_cast<size_t>(aipl->slots[slot]), u1, v1, u2, v2);
					s2uvmap.uniform4f("uvuv", u1, v1, u2, v2);
					itemmap.atlas.bind(GL_TEXTURE0);
					quad.drawElements(6);
					s2.use();
					slottex.bind(GL_TEXTURE0);
				}
				return hover;
			};
			slotfunc(0, 0, ai::slot_single_in);
			slotfunc(1, 0, ai::slot_double_in_a);
			slotfunc(2, 0, ai::slot_double_in_b);
			slotfunc(0, 2, ai::slot_single_out);
			slotfunc(1.5f, 2, ai::slot_double_out);
			const glm::mat4 arrow1mat(glm::translate(slotmat, glm::vec3(0.f, 1.f, 0.f)));
			const glm::mat4 arrow2mat(glm::translate(slotmat, glm::vec3(1.5f, 1.f, 0.f)));
			const glm::mat4 batterymat(glm::translate(slotmat, glm::vec3(2.2f, 2.f, 0.f)));
			arrowtex.bind(GL_TEXTURE0);
			s2.uniformM4f("proj", arrow1mat);
			quad.drawElements(6);
			s2.uniformM4f("proj", arrow2mat);
			quad.drawElements(6);
			batterytex.bind(GL_TEXTURE0);
			s2.uniformM4f("proj", batterymat);
			quad.drawElements(6);
			s2uvmap.use();
			if (aipl->process_single > 0.f || aipl->process_double > 0.f) {
				processarrowtex.bind();
				if (aipl->process_single > 0.f) {
					s2uvmap.uniformM4f("proj", glm::scale(arrow1mat, glm::vec3(1.f, aipl->process_single, 1.f)));
					s2uvmap.uniform4f("uvuv", 0, 0, 1, aipl->process_single);
					quad.drawElements(6);
				}
				if (aipl->process_double > 0.f) {
					s2uvmap.uniformM4f("proj", glm::scale(arrow2mat, glm::vec3(1.f, aipl->process_double, 1.f)));
					s2uvmap.uniform4f("uvuv", 0, 0, 1, aipl->process_double);
					quad.drawElements(6);
				}
			}
			fullbatterytex.bind();
			float batteryratio = aipl->battery / aipl->batterymax;
			s2uvmap.uniformM4f("proj", glm::scale(
				glm::translate(batterymat, glm::vec3(0.f, 1.f - batteryratio, 0.f)),
				glm::vec3(1.f, batteryratio, 1.f)));
			s2uvmap.uniform4f("uvuv", 0.f, 1.f - batteryratio, 1.f, 1.f);
			quad.drawElements(6);
			s2.use();
		}
		if (pl.has_value() && mouse_in_bounds && glm::length2(pl->pos - glm::vec2(mtx + .5f, mty + .5f)) <= 25.f) {
			glm::mat4 curmat(glm::translate(mat, glm::vec3(mtx,  mty, 0.f)));
			s2.uniformM4f("proj", curmat);
			cursortex.bind(GL_TEXTURE0);
			quad.drawElements(6);
		}
		glDisable(GL_BLEND);
		if (aipl.has_value()) {
			glm::mat4 plmat = glm::translate(mat, glm::vec3(aipl->pos.x - .5f, aipl->pos.y - .5f, 0.f));
			s2.uniformM4f("proj", plmat);
			(aipl->battery > 0.f ? (ai_frame ? ai2tex : aitex) : ainobatterytex).bind(GL_TEXTURE0);
			quad.drawElements(6);
		}
		if (pl.has_value()) {
			glm::mat4 plmat = glm::translate(mat, glm::vec3(pl->pos.x - .5f, pl->pos.y - .5f, 0.f));
			if (player_dead) {
				plmat = glm::rotate(plmat, 1.57f, glm::vec3(0.f, 0.f, 1.f));
				plmat = glm::translate(plmat, glm::vec3(0.f, -1.f, 0.f));
				pltex.bind(GL_TEXTURE0);
			} else {
				if (player_walk_frame % 2 == 0)
					pltex.bind(GL_TEXTURE0);
				else if (player_walk_frame == 1)
					plwalk1tex.bind(GL_TEXTURE0);
				else
					plwalk2tex.bind(GL_TEXTURE0);
			}
			s2.uniformM4f("proj", plmat);
			quad.drawElements(6);
			if (pl->hand != item_t::empty) {
				s2uvmap.use();
				s2uvmap.uniformM4f("proj", plmat);
				float u1, v1, u2, v2;
				itemmap.getuv(static_cast<size_t>(pl->hand), u1, v1, u2, v2);
				s2uvmap.uniform4f("uvuv", u1, v1, u2, v2);
				itemmap.atlas.bind(GL_TEXTURE0);
				quad.drawElements(6);
			}
			if (mouse_in_bounds && clicked) {
				pl->click(*this, current_map.value(), mtx, mty);
			}
		}
		if (!explosions.empty()) {
			s2uvmap.use();
			explosionmap.atlas.bind(GL_TEXTURE0);
			for (const auto &e : explosions) {
				s2uvmap.uniformM4f("proj", glm::translate(mat, glm::vec3(e.x, e.y, 0.f)));
				float u1, v1, u2, v2;
				explosionmap.getuv(e.frame, u1, v1, u2, v2);
				s2uvmap.uniform4f("uvuv", u1, v1, u2, v2);
				quad.drawElements(6);
			}
		}
	}
	if (stage != game_stage::GAME) {
		glEnable(GL_BLEND);
		s2.use();
		s2.uniformM4f("proj", glm::scale(
			glm::translate(glm::mat4(1.f), glm::vec3(-1.f, -1.f, 0.f)),
			glm::vec3(2.f, 2.f, 1.f)));
		overlay.bind(GL_TEXTURE0);
		quad.bind();
		quad.drawElements(6);
		glDisable(GL_BLEND);
	}
	sounds->tick();
	if (playinglabel.text().substr(9) != sounds->music_playing()) {
		playinglabel.setText("playing: " + sounds->music_playing());
	}
	gui.render(glm::ortho<float>(0, width, height, 0));
	if (aipl.has_value() && glm::length2(aipl->pos - glm::vec2(current_map->goalx + .5f, current_map->goaly + .5f)) < 0.01f) {
		load(++level);
	}
	clicked = false;
	dt = realdt;
}
void app::load(int level_id) {
	float battery;
	size_t hand;
	std::string tip;
	current_map = map("./assets/level/l" + std::to_string(level_id) + ".txt", &tilemap[0], battery, hand, tip);
	aipl = ai(current_map->startx + .5f, current_map->starty + .5f);
	aipl->battery = aipl->batterymax = battery;
	pl = player(current_map->startx + 1.5f, current_map->starty + .5f);
	pl->hand = static_cast<item_t>(hand);
	player_dead = false;
	cam = pl->pos;
	explosions.clear();
	lose_timer = 0;
	tiplabel.setText(tip);
}
void app::tick_anim() {
	tilemap_frame = (tilemap_frame + 1) % 16;
	if (pl.has_value() && pl->walking)
		player_walk_frame = (player_walk_frame + 1) % 4;
	else
		player_walk_frame = 0;
	if (aipl.has_value() && aipl->pathvao.exists)
		ai_frame = (ai_frame + 1) % 2;
	for (auto &e : explosions) {
		e.frame++;
	}
	explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const explosion &e){ return e.frame >= explosion::frame_count; }), explosions.end());
}
