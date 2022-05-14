/* tiny_loader2.h
Example class to demonstrate the use of TinyObjectLoader to load an obj (WaveFront)
object file and copy the date into vertex, normal and element buffers.
This is incomplete: I've tested it with vertices, normals and elements but not
with texture coordinates.
Iain Martin November 2018
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class TinyObjLoader
{
public:
	TinyObjLoader();
	~TinyObjLoader();

	void load_obj(std::string inputfile, bool debugPrint = false);
	void drawObject(int drawmode);
	void overrideColour(glm::vec4 c);

private:
	// Define vertex buffer object names (e.g as globals)
	GLuint positionBufferObject;
	GLuint colourBufferObject;
	GLuint normalBufferObject;
	GLuint elementBufferObject;
	GLuint texCoordsObject;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;
	GLuint attribute_v_texcoord;

	int drawmode;
	GLuint numVertices;
	GLuint numNormals;
	GLint  numTexCoords;
	GLuint numPIndexes;
	

};
