#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	glm::vec2 b(1, x);
	glm::mat2x2 M(glm::vec2(1, -1), glm::vec2(0, 1));
	glm::vec2 a = b * M; //(1-x, x)

	glm::mat3x2 p(glm::vec2(p0[0], p1[0]),
		glm::vec2(p0[1], p1[1]),
		glm::vec2(p0[2], p1[2])); // (p0[0], p1[0],
				 //  p0[1], p1[1],
				 //  p0[2], p1[2])



	return b * M * p;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 const& p3,
	float const t, float const x)
{
	//! \todo Implement this function
	glm::vec4 b(1, x, pow(x, 2), pow(x, 3)); //(1, x, x^2, x^3)

	glm::mat4 M(glm::vec4(0, 1, 0, 0),
		glm::vec4(-t, 0, t, 0),
		glm::vec4(2 * t, (t - 3), (3 - 2 * t), -t),
		glm::vec4(-t, (2 - t), (t - 2), t));

	glm::mat4 trans = glm::transpose(M);

	glm::mat3x4 p(glm::vec4(p0[0], p1[0], p2[0], p3[0]),
		glm::vec4(p0[1], p1[1], p2[1], p3[1]),
		glm::vec4(p0[2], p1[2], p2[2], p3[2]));


	return b * trans * p;
}
