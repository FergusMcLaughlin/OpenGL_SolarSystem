/** Basic Point class to use in an example particle animation 
 Iain Martin
 November 2018
 */

#include "points.h"
#include "glm/gtc/random.hpp"

/* Constructor, set initial parameters*/
points::points(GLuint number, GLfloat dist, GLfloat sp)
{
	numpoints = number;
	maxdist = dist;
	particle_speed = sp;
}


points::~points()
{
	delete [] colours;
	delete[] vertices;
}

void points::updateParams(GLfloat dist, GLfloat sp)
{
	maxdist = dist;
	particle_speed = sp;
}


void  points::create()
{
	vertices = new glm::vec3[numpoints];
	colours = new glm::vec3[numpoints];
	velocity = new glm::vec3[numpoints];

	/* Define random position and velocity */
	for (int i = 0; i < numpoints; i++)
	{
		vertices[i] = glm::ballRand(1.f);
		colours[i] = glm::ballRand(1.f);
		velocity[i] = glm::vec3(glm::ballRand(glm::linearRand(0.0, 0.01)));
	}

	/* Create the vertex buffer object */
	/* and the vertex buffer positions */
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colour_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), colours, GL_STATIC_DRAW);
}


void points::draw()
{
	/* Bind  vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Draw our points*/
	glDrawArrays(GL_POINTS, 0, numpoints);
}


void points::animate()
{
	for (int i = 0; i < numpoints; i++)
	{
		// Add velocity to the vertices 
		vertices[i] += velocity[i];

		// Calculate distance to the origin
		GLfloat dist = glm::length(vertices[i]);

		// If we are near the origin then we introduce a new random direction
		if (dist < 0.01f) velocity[i] = glm::vec3(glm::ballRand(glm::linearRand(0.0, 0.02)));

		// If we are too far away then change direction back to the origin
		if (dist > maxdist) velocity[i] = -vertices[i] / 500.f * particle_speed;
	}

	// Update the vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);
}



