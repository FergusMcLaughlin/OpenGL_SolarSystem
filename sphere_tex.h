/* sphere.h
 Example class to create a generic sphere object with texture coordinates
 Resolution can be controlled by setting number of latitudes and longitudes
 Iain Martin November 2019
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Sphere
{
public:
	Sphere(bool useTexture = true);
	~Sphere();

	void makeSphere(GLuint numlats, GLuint numlongs);
	void drawSphere(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint sphereBufferObject;
	GLuint sphereNormals;
	GLuint sphereColours;
	GLuint sphereTexCoords;
	GLuint elementbuffer;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;
	GLuint attribute_v_texcoord;

	unsigned int numspherevertices;
	unsigned int numlats;
	unsigned int numlongs;
	unsigned int drawmode;
	bool enableTexture;

private:
	void makeUnitSphere(GLfloat *pVertices, GLfloat *pTexCoords);
};
