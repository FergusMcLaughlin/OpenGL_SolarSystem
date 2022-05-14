/** Basic Point class to use in an example particle animation
Iain Martin
November 2018
*/
#pragma once

#include <glm/glm.hpp>
#include "wrapper_glfw.h"

class points
{
public:

	points(GLuint number, GLfloat dist, GLfloat sp);
	~points();

	void create();
	void draw();
	void animate();
	void updateParams(GLfloat dist, GLfloat sp);

	glm::vec3* vertices;//star thing ?
	glm::vec3* colours;
	glm::vec3* velocity;

	GLuint numpoints;
	GLuint vertex_buffer;
	GLuint colour_buffer;

	GLfloat particle_speed;

	GLfloat maxdist;// max dist away before we go back
};

