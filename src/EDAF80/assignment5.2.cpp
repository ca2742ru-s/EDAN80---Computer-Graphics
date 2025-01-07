#include "assignment5.hpp"
#include "parametric_shapes.hpp"


#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/ShaderProgramManager.hpp"
#include "core/node.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <array>

#include <clocale>
#include <cstdlib>
#include <stdexcept>
#include <string>



edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
		0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)


{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment5::run()
{
	// Set up the camera

	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 0.0f)); //Camera position
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 3.0f; // 3 m/s => 10.8 km/h

	//Try to load object
	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects(config::resources_path("Player.obj"));
	if (objects.empty()) {
		printf("Failed to load the TieFighter geometry: exiting.\n");
	}

	bonobo::mesh_data const& tiefighter = objects.front();

	std::vector<bonobo::mesh_data> const object_crate = bonobo::loadObjects(config::resources_path("Crate/Crate1.obj"));
	if (object_crate.empty()) {
		printf("Failed to load the crate geometry: exiting.\n");
	}

	bonobo::mesh_data const& object_shape_crate = object_crate.front();


	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "EDAF80/fallback.vert" },
		  { ShaderType::fragment, "EDAF80/fallback.frag" } },
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint outlines_shader = 0u;
	program_manager.CreateAndRegisterProgram("Outlines shader",
		{ { ShaderType::vertex, "EDAF80/outlines.vert" },
		  { ShaderType::fragment, "EDAF80/outlines.frag" } },
		outlines_shader);

	if (outlines_shader == 0u) {
		LogError("Failed to load Outlines shader");
		return;
	}


	GLuint celestial_body_shader = 0u;
	program_manager.CreateAndRegisterProgram("Celestial Shader",
		{ { ShaderType::vertex, "EDAF80/default.vert" },
		  { ShaderType::fragment, "EDAF80/default.frag" } },
		celestial_body_shader);

	if (celestial_body_shader == 0u) {
		LogError("Failed to load Celestial shader");
		return;
	}

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		{ ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u) {
		LogError("Failed to load skybox shader");
		return;
	}

	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Water",
		{ { ShaderType::vertex, "EDAF80/water.vert" },
		  { ShaderType::fragment, "EDAF80/water.frag" } },
		water_shader);
	if (water_shader == 0u) {
		LogError("Failed to load water shader");
		return;
	}

	GLuint torus_shader = 0u;
	program_manager.CreateAndRegisterProgram("Torus",
		{ { ShaderType::vertex, "EDAF80/torus.vert" },
		  { ShaderType::fragment, "EDAF80/torus.frag" } },
		torus_shader);
	if (torus_shader == 0u) {
		LogError("Failed to load torus shader");
		return;
	}

	GLuint texture_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture",
		{ { ShaderType::vertex, "EDAF80/texcoord.vert" },
		  { ShaderType::fragment, "EDAF80/texcoord.frag" } },
		texture_shader);
	if (texture_shader == 0u) {
		LogError("Failed to load torus shader");
		return;
	}

	//Set up uniforms sent in to the shader
	float ellapsed_time_s = 0.0f;
	auto light_position = glm::vec3(0.0f, 300.0f, 0.0f);
	auto camera_position = mCamera.mWorld.GetTranslation();
	const bool outline_color = true;
	const bool no_outline_color = false;

	auto const object_uniforms = [&light_position, &ellapsed_time_s, &camera_position, &no_outline_color](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "ellapsed_time_s"), ellapsed_time_s);
		glUniform1i(glGetUniformLocation(program, "outline_color"), no_outline_color ? 1 : 0);

	};

	auto const outline_uniforms = [&light_position, &ellapsed_time_s, &camera_position, &outline_color](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "ellapsed_time_s"), ellapsed_time_s);
		glUniform1i(glGetUniformLocation(program, "outline_color"), outline_color ? 1 : 0);
	};

	auto const set_uniforms = [&light_position, &camera_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	auto const water_uniforms = [&light_position, &ellapsed_time_s, &camera_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "ellapsed_time_s"), ellapsed_time_s);
	};

	auto my_torus_id = bonobo::loadTexture2D(config::resources_path("planets/2k_earth_daymap.jpg"));

	auto water_normal_id = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

	auto my_cube_map_id = bonobo::loadTextureCubeMap(config::resources_path("cubemaps/bkg/blue/left.png"),
		config::resources_path("cubemaps/bkg/blue/right.png"),
		config::resources_path("cubemaps/bkg/blue/top.png"),
		config::resources_path("cubemaps/bkg/blue/bottom.png"),
		config::resources_path("cubemaps/bkg/blue/back.png"),
		config::resources_path("cubemaps/bkg/blue/front.png"),
		true);

	auto player_map_id = bonobo::loadTexture2D(config::resources_path("cubemaps/bkg/red/bkg1_back6.png"), true);

	auto grid_map_id = bonobo::loadTexture2D(config::resources_path("cubemaps/white-bathroom-tiles.jpg"), true);

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//

	auto const sphere_shape = parametric_shapes::createSphere(10.0f, 10u, 10u);
	if (sphere_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the sphere");
		return;
	}

	auto const player_shape = parametric_shapes::createSphere(0.3f, 100u, 100u);
	if (player_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the player");
		return;
	}

	auto const torus_shape = parametric_shapes::createTorus(15.0f, 1.0f, 100u, 100u);
	if (player_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the torus");
		return;
	}

	auto const skybox_shape = parametric_shapes::createSphere(100.0f, 1000u, 1000u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	auto const floor_shape = parametric_shapes::createQuad(10, 25, 100, 100);
	if (floor_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the floor");
		return;
	}

	auto const wall_shape = parametric_shapes::createQuad(10, 25, 100, 100);
	if (wall_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the wall");
		return;
	}

	auto const object_shape_sphere = parametric_shapes::createSphere(2.0f, 100u, 100u);
	if (sphere_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	auto const object_shape_cube = parametric_shapes::createSphere(2.0f, 100u, 100u);
	if (sphere_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}


	//
	// Todo: Load your geometry

	std::array<glm::vec3, 10> torus_locations = {
		glm::vec3(0.0f, 0.0f, 300.0f),
		glm::vec3(-10.0f, 10.0f, 250.0f),
		glm::vec3(25.0f, -10.0f, 200.0f),
		glm::vec3(-20.0f, 25.0f, 150.0f),
		glm::vec3(25.0f, -25.0f, 100.0f),
		glm::vec3(-25.0f, 0.0f, 50.0f),
		glm::vec3(-20.0f, 25.0f, 0.0f),
		glm::vec3(25.0f, -25.0f, -100.0f),
		glm::vec3(-25.0f, 0.0f, -200.0f),
		glm::vec3(-25.0f, 0.0f, -250.0f)
	};

	std::array<glm::vec3, 10> torus_dir = {
		glm::vec3(0.0f, 0.1f, 0.0f),
		glm::vec3(0.0f, -0.1f, 0.0f),
		glm::vec3(0.1f, 0.1f, 0.0f),
		glm::vec3(-0.1f, -0.1f, 0.0f),
		glm::vec3(0.0f, 0.1f, 0.0f),
		glm::vec3(0.1f, 0.0f, 0.0f),
		glm::vec3(0.1f, 0.0f, 0.0f),
		glm::vec3(0.1f, 0.0f, 0.0f),
		glm::vec3(0.1f, 0.0f, 0.0f),
		glm::vec3(0.1f, 0.0f, 0.0f)
	};

	std::array<float, 10> scales = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };



	std::array<Node, torus_locations.size()> torus_points;
	std::array<Node, torus_locations.size()> control_points;

	for (std::size_t i = 0; i < torus_locations.size(); ++i) {
		auto& torus = torus_points[i];
		torus.set_geometry(torus_shape);
		torus.set_program(&torus_shader);
		torus.add_texture("torus_texture", my_torus_id, GL_TEXTURE_2D);
		torus.get_transform().SetTranslate(torus_locations[i]);
		torus.get_transform().RotateX(glm::half_pi<float>());


		auto& control_point = control_points[i];
		control_point.set_geometry(sphere_shape);
		control_point.set_program(&water_shader, water_uniforms);
		control_point.add_texture("skybox", my_cube_map_id, GL_TEXTURE_CUBE_MAP);
		control_point.add_texture("normal_map", water_normal_id, GL_TEXTURE_2D);
		control_point.get_transform().SetTranslate(torus_locations[i]);

	}

	std::array<bool, torus_locations.size()> states;
	for (std::size_t i = 0; i < torus_locations.size(); ++i) {
		states[i] = true;
	}

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&fallback_shader, set_uniforms);
	skybox.add_texture("skybox", my_cube_map_id, GL_TEXTURE_CUBE_MAP);
	//skybox.get_transform().SetTranslate(glm::vec3(0, 0, 0));

	Node player;
	player.set_geometry(tiefighter);
	player.set_program(&celestial_body_shader);
	//player.add_texture("diffuse_texture", grid_map_id , GL_TEXTURE_2D);
	player.get_transform().Scale(0.005);

	Node wall1;
	wall1.set_geometry(wall_shape);
	wall1.set_program(&fallback_shader);
	//wall1.add_texture("diffuse_texture", grid_map_id, GL_TEXTURE_2D);
	wall1.get_transform().RotateZ(glm::pi<float>()/2);

	Node wall2;
	wall2.set_geometry(wall_shape);
	wall2.set_program(&fallback_shader);
	//wall2.add_texture("diffuse_texture", grid_map_id, GL_TEXTURE_2D);
	//wall2.get_transform().RotateZ(glm::pi<float>()/2);
	//wall2.get_transform().RotateY(glm::pi<float>()/2);
	wall2.get_transform().Scale(glm::vec3(1, 1, 1/2.5));
	wall2.get_transform().RotateX(-glm::pi<float>()/2);

	Node floor;
	floor.set_geometry(floor_shape);
	floor.set_program(&fallback_shader);
	floor.get_transform().RotateZ(glm::pi<float>());
	floor.get_transform().Translate(glm::vec3(9.89, 0, 0));
	//floor.add_texture("diffuse_texture", grid_map_id, GL_TEXTURE_2D);

	Node node_object_sphere;
	node_object_sphere.set_geometry(object_shape_sphere);
	node_object_sphere.set_program(&outlines_shader, object_uniforms);
	node_object_sphere.get_transform().Translate(glm::vec3(5, 5, 10));

	Node node_object_crate;
	node_object_crate.set_geometry(object_shape_crate);
	node_object_crate.set_program(&outlines_shader, object_uniforms);
	node_object_crate.get_transform().Translate(glm::vec3(5, 5, 20));

	Node node_object_crate_outline;
	node_object_crate_outline.set_geometry(object_shape_crate);
	node_object_crate_outline.set_program(&outlines_shader, outline_uniforms);
	node_object_crate_outline.get_transform().Translate(glm::vec3(5, 5, 20));
	node_object_crate_outline.get_transform().Scale(1.1);






	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);


	auto lastTime = std::chrono::high_resolution_clock::now();

	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = false;
	bool show_gui = true;
	bool shader_reload_failed = false;

	float speed = 0.0f;
	float yaw = -90.0f;
	float pitch = 0.0f;

	std::array<float, 10> scores = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int score = 0;

	player.get_transform().SetTranslate(glm::vec3(0, 2, 0));

	glm::vec3 direction = player.get_transform().GetTranslation() + glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 position = player.get_transform().GetTranslation();

	mCamera.mWorld.SetTranslate(glm::vec3(25, 15, 25));
	mCamera.mWorld.RotateX(-0.46);
	mCamera.mWorld.RotateY(1.1);
	

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		ellapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();




		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();


		if ((inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED)) {
			pitch += 1;
			if (pitch > 89) {
				pitch = 89;
			}

		}

		if ((inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED)) {
			pitch -= 1;
			if (pitch < -89) {
				pitch = -89;
			}

		}

		if ((inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED) && std::chrono::duration<float>(deltaTimeUs).count() == 1000) {
			yaw -= 0.5;

		}

		if ((inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED)) {
			yaw += 0.5;

		}



		glm::vec3 camRotation = { -5 * cos(glm::radians(yaw)),
								  6 * sin(glm::radians(pitch)),
								  -6 * sin(glm::radians(yaw))
		};

		glm::vec3 direction = { cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
								-sin(glm::radians(pitch)),
								sin(glm::radians(yaw)) * cos(glm::radians(pitch))
		};

		direction = normalize(direction);

		if ((inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & PRESSED)) {
			speed += 0.001;
			if (speed > 0.5) {
				speed = 0.5;
			}


		}

		if ((inputHandler.GetKeycodeState(GLFW_KEY_E) & PRESSED)) {
			speed -= 0.001;
			if (speed < -0.5) {
				speed = -0.5;
			}
		}

		if (speed < 0) {
			speed += 0.0005;
		}
		if (speed > 0) {
			speed -= 0.0005;
		}


		//Angle controllers
		glm::vec3 playerPos = player.get_transform().GetTranslation();
		glm::vec3 playerUp = player.get_transform().GetUp();

		player.get_transform().Translate(direction * speed);
		player.get_transform().LookAt((playerPos + direction), glm::vec3(0, 1, 0));
		player.get_transform().Rotate(glm::radians(90.0f), player.get_transform().GetLeft());

		//mCamera.mWorld.SetTranslate(player.get_transform().GetTranslation() + camRotation);
		//mCamera.mWorld.LookAt(floor.get_transform().GetFront());


		
		mCamera.Update(deltaTimeUs, inputHandler);
		
		if (distance(player.get_transform().GetTranslation(), glm::vec3(0, 0, 0)) > 350) {
			player.get_transform().SetTranslate(glm::vec3(0, 0, 340));
			speed = 0;
			direction = glm::vec3(0, 0, 1);
		}


		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
					"An error occurred while reloading shader programs; see the logs for details.\n"
					"Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
					"error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F8) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);


		//
		// Todo: If you need to handle inputs, you can do it here
		//


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//

			//skybox.render(mCamera.GetWorldToClipMatrix());
			//player.render(mCamera.GetWorldToClipMatrix());
			floor.render(mCamera.GetWorldToClipMatrix());
			wall1.render(mCamera.GetWorldToClipMatrix());
			wall2.render(mCamera.GetWorldToClipMatrix());
			node_object_sphere.render(mCamera.GetWorldToClipMatrix());
			node_object_crate.render(mCamera.GetWorldToClipMatrix());
			//node_object_crate_outline.render(mCamera.GetWorldToClipMatrix());


			for (int i = 0; i < torus_points.size(); i++) {
				if (states[i] == true) {
					//torus_points[i].render(mCamera.GetWorldToClipMatrix());
					//control_points[i].render(mCamera.GetWorldToClipMatrix());
				}
				if (distance(player.get_transform().GetTranslation(), control_points[i].get_transform().GetTranslation()) < 13) {
					control_points[i].get_transform().Scale(0.9 * scales[i]);
					//states[i] = false;
					scales[i] -= 0.036;
					if (scales[i] < 0.01) {
						states[i] = false;
						scores[i] = 1;
					}
				}

				torus_points[i].get_transform().Translate(torus_dir[i]);
				control_points[i].get_transform().Translate(torus_dir[i]);
				if (distance(torus_points[i].get_transform().GetTranslation(), torus_locations[i]) > 80) {
					torus_dir[i] *= -1;
				}

			}


		}
		score = 0;
		for (int i = 0; i < torus_points.size(); i++) {
			score += scores[i];
		}

		camera_position = mCamera.mWorld.GetTranslation();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//

		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			//bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			ImGui::SliderInt("Score", &score, 0, 10);
		}
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			mWindowManager.RenderImGuiFrame();

		glfwSwapBuffers(window);


	}
}


int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}


