/*
 particles
 Basic example of a particle animation
 Adaped from the particle animation tutorial from:
 http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/

  Iain Martin November 2014
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "soil.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <vector>
#include <algorithm>



/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Include the hacked version of SOIL */
#include "soil.h"

#include "particle_object.h"

/* Define buffer object indices */
GLuint quad_vbo, quad_normals, quad_colours, quad_tex_coords;

/* Define textureID*/
GLuint texID, particle_texID;
GLuint tex_location;

GLuint program;		/* Identifier for the shader prgoram */
GLuint particle_program;		/* Identifier for the particle shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat px, py, pz;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;
GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

particle_object particleObject;

using namespace glm;
using namespace std;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0;
	y = 0;
	z = -50.f;
	px, py, pz = 0;
	angle_y = angle_z = 0;
	angle_x = -20.f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("particles.vert", "particles.frag");
		particle_program = glw->LoadShader("particle_object.vert", "particle_object.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");

	/* Create our quad and texture */
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	// Create dat for our quad with vertices, normals and texturee coordinates 
	static const GLfloat quad_data[] =
	{
		// Vertex positions
		0.75f, 0, -0.75f,
		-0.75f, 0, -0.75f,
		-0.75f, 0, 0.75f,
		0.75f, 0, 0.75f,

		// Normals
		0, 1.f, 0,
		0, 1.f, 0,
		0, 1.f, 0,
		0, 1.f, 0,

		// Texture coordinates. Note we only need two per vertex but have a
		// redundant third to fit the texture coords in the same buffer for this simple object
		0.0f, 0.0f, 0,
		1.0f, 0.0f, 0,
		1.0f, 1.0f, 0,
		0.0f, 1.0f, 0,
	};

	// Copy the data into the buffer. See how this example combines the vertices, normals and texture
	// coordinates in the same buffer and uses the last parameter of  glVertexAttribPointer() to
	// specify the byte offset into the buffer for each vertex attribute set.
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	try
	{
		/* Not actually needed if using one texture at a time */
		glActiveTexture(GL_TEXTURE0);

		/* load an image file directly as a new OpenGL texture */
		texID = SOIL_load_OGL_texture("..\\..\\images\\img.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		/* check for an error during the load process */
		if (texID == 0)
		{
			printf("TexID SOIL loading error: '%s'\n", SOIL_last_result());
		}

		/* Standard bit of code to enable a uniform sampler for our texture */
		int loc = glGetUniformLocation(program, "tex1");
		if (loc >= 0) glUniform1i(loc, 0);
	}
	catch (exception &e)
	{
		printf("\nImage file loading failed.");
	}

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	particleObject.create(particle_program);
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(vao);
	
	/* Make the surface texture shader program current */
	glUseProgram(program);

	// Define the model transformations 
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x, y, z));
	model = scale(model, vec3(scaler*5.f, scaler*5.f, scaler*5.f));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1.f, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1.f, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1.f)); //rotating in clockwise direction around z-axis

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 Projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 View = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &Projection[0][0]);

	/* Draw our textured quad*/
	glBindTexture(GL_TEXTURE_2D, texID);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;

	mat4 ParticleView = lookAt(
		vec3(0, 0, 4.f), 
		vec3(-px, -py, -pz), 
		vec3(0, 1.f, 0)
		);
	/* Update and draw our particle animation */
	particleObject.drawParticles(Projection, ParticleView);
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= 0.05f;
	if (key == 'W') angle_inc_x += 0.05f;
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'A') scaler -= 0.02f;
	if (key == 'S') scaler += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;
	if (key == 'O') px -= 0.05f;
	if (key == 'P') px += 0.05f;
	if (key == 'L') py -= 0.05f;
	if (key == ';') py += 0.05f;
	if (key == ',') pz -= 0.05f;
	if (key == '.') pz += 0.05f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Texture image example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

