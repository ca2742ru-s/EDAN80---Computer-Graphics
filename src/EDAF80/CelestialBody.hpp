#pragma once

#include "assignment1.hpp"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "glm/vec3.hpp"

class CelestialBody
{
private:

	Node _body;
	Node _ring;
	std::vector<CelestialBody*> _children;

	//Body Attributes
	float _spin_inclination = 0;
	float _spin_angle = 0;
	float _spin_speed = 0;


	//Orbit Attributes
	float _orbit_radius = 0;
	float _orbit_inclanation = 0;
	float _orbit_speed = 0;
	float _orbit_angle = 0;

	//Child Attributes


	glm::vec3 _body_scale = glm::vec3(1, 1, 1);
	glm::vec2 _ring_scale = glm::vec2(1, 1);
	glm::mat4 _transform_matrix;

public:

	CelestialBody(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id);

	glm::mat4 render(std::chrono::microseconds ellapsed_time, glm::mat4 const& view_projection, glm::mat4 const& parent_transform = glm::mat4(1.0f));

	void set_scale(glm::vec3 const& scale);
	void set_spin(SpinConfiguration const& configuration);
	void set_orbit(OrbitConfiguration const& configuration);

	void set_ring(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id, glm::vec2 const& scale = glm::vec2(1.0f));

	void add_child(CelestialBody* children);

	std::vector<CelestialBody*> const& get_children() const;

};
