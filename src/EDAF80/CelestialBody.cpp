#include "CelestialBody.hpp"
#include "glm/gtc/matrix_transform.hpp"

CelestialBody::CelestialBody(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id)
{
	_body.set_geometry(shape);
	_body.set_program(program);
	_body.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);



}

glm::mat4 CelestialBody::render(std::chrono::microseconds ellapsed_time, glm::mat4 const& view_projection, glm::mat4 const& parent_transform)
{
	//Axis vectors
	glm::vec3 AxisX = glm::vec3(1, 0, 0);
	glm::vec3 AxisY = glm::vec3(0, 1, 0);
	glm::vec3 AxisZ = glm::vec3(0, 0, 1);

	glm::vec3 RingV = glm::vec3(_ring_scale, 1);

	//time counter
	std::chrono::duration<float> const ellapsed_time_s = ellapsed_time;

	//Anglespeed
	_spin_angle += ellapsed_time_s.count() * _spin_speed;
	_orbit_angle += ellapsed_time_s.count() * _orbit_speed;

	//Body matrices
	glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), _body_scale);
	glm::mat4 spin_matrix = glm::rotate(glm::mat4(1.0f), _spin_angle, AxisY);
	glm::mat4 tilt_matrix = glm::rotate(glm::mat4(1.0f), _spin_inclination, AxisZ);

	glm::mat4 ring_scale_matrix = glm::scale(glm::mat4(1.0f), RingV);


	//Orbit matrices
	glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), _orbit_angle, AxisY);

	//Translation matrix
	glm::mat4 translation_matrix = glm::mat4(1.0f);
	translation_matrix[3][0] = _orbit_radius;

	glm::mat4 tilt_rotation_matrix = glm::rotate(glm::identity<glm::mat4>(), _orbit_inclanation, AxisZ);

	//Scaled and tilted orbit matrices
	glm::mat4 tilted_orbit_matrix = tilt_rotation_matrix * rotation_matrix * translation_matrix;

	//Transform matrix
	glm::mat4 _transform_matrix = parent_transform * tilted_orbit_matrix;

	glm::mat4 matrix = _transform_matrix * tilt_matrix * spin_matrix * scaling_matrix;

	glm::mat4 ring_matrix = _transform_matrix * tilt_matrix * spin_matrix * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0)) * ring_scale_matrix;



	_body.render(view_projection, matrix);

	_ring.render(view_projection, ring_matrix);

	return _transform_matrix;
}



void CelestialBody::set_scale(glm::vec3 const& scale)
{
	_body_scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
	_spin_inclination = configuration.inclination;
	_spin_speed = configuration.speed;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
	_orbit_radius = configuration.radius;
	_orbit_inclanation = configuration.inclination;
	_orbit_speed = configuration.speed;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id, glm::vec2 const& scale)
{
	_ring.set_geometry(shape);
	_ring.set_program(program);
	_ring.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_ring_scale = scale;


}

void CelestialBody::add_child(CelestialBody* child)
{
	_children.push_back(child);
}


std::vector<CelestialBody*> const& CelestialBody::get_children() const
{
	return _children;
}






