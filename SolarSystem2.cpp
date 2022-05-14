/* Assignment 2 - AC41001
A modified version of my Assignmnet 1
Built useing Labs, using Learn openGL - Graphics Programming by joey de vries
 and https://learnopengl.com/Advanced-OpenGL/.
 Fergus Mclaughlin December 2021
*/

//Link to static libraries.
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

//GLFW wrapper
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

//GLM core and matrix extensions.
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

//Image loader for textures.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tiny_loader.h"	//Obj loader.
#include "sphere_tex.h"		//Sphere for the planet_Parameterss.
#include "points.h"			//points

TinyObjLoader SpaceBody, Asteroids,
Phobos_obj, Voyager_obj;		// Object loaded instance.
Sphere sphere;					//Global object instance.
GLuint program;					//Identifier for the shader program.
GLuint points_program;			//Pointer shader.
GLuint light_emitter;			//Not used in favour of Vertex shader so that all planets are visable.
GLuint vao;						//Vertex array (Container) object, VAO container for buffer objects.
GLuint model_scale;				//For re-sizeing.
GLuint drawmode;				//Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;		//Define the resolution of the sphere object


//Uniforms
GLuint  modelID, viewID, projectionID, lightposID, normalmatrixID;
GLuint numspherevertices;
GLfloat aspect_ratio;
//points uniforms
GLuint points_modelID, points_viewID, points_projectionID, points_sizeID;
//points params
points* point_anim;
GLfloat point_speed;
GLfloat maxdist;
GLfloat point_size;		

//Not Used for the lighting anymore.
GLfloat light_x, light_y, light_z;
GLuint emitmode;
GLuint attenuationmode;
GLuint emitmodeID, attenuationmodeID;
GLfloat speed;
GLfloat vx, vy, vz;

GLuint Cam_Pos; //For directed Camera options.
GLfloat fov = 42; //Max zoom distance

//Define texture ID value (identifier for a specific texture).
GLuint	Sun_texture, Mercury_texture, Venus_texture,
		Earth_texture, Mars_texture, Jupiter_texture,
		Saturn_texture, Uranus_texture, Neptune_texture,
		Stars_texture, Moon_texture, Pluto_texture, Europa_texture, 
		Titan_texture, Triton_texture,point_textureID;

//planet_Parameters class
class planet_Parameters {
public:
	float size, distance, orbit, orbit_inc, orbit_speed, spin, spin_speed;
	planet_Parameters(float _size, float _distance, float _orbit, float _orbitInc, float _orbitSpeed, float _spin, float _spin_speed)
	{
		size = _size;
		distance = _distance;
		orbit = _orbit;
		orbit_inc = _orbitInc;
		orbit_speed = _orbitSpeed;
		spin = _spin;
		spin_speed = _spin_speed;
	}
};
/* Create our planet_Parameters objects.
planet_Parameters Name(Size, Distance, Orbit, Orbit increese, Orbit Speed, Spin, spin_speed) */
planet_Parameters Sun_param(3.f, 0, 0, 0, 0, 0, 0.05);
planet_Parameters Mercury_param(24.f,0.6f, 0, 0, 2.00f, 0, 22.6);
planet_Parameters Venus_param(16.f, 0.9f, 0, 0, 1.05f, 0, 38.0);
planet_Parameters Earth_param(16.5f, 1.2f, 0, 0, 0.75f, 0, 9.9);
planet_Parameters Mars_param(17.f, 1.6f, 0, 0, 0.65f, 0, 10.3);
planet_Parameters Jupiter_param(8.f, 2.5f, 0, 0, 0.35f, 0, 4.1);
planet_Parameters Saturn_param(10.f, 3.0f, 0, 0, 0.25f, 0, 4.5);
planet_Parameters Uranus_param(12.f, 3.3f, 0, 0, 0.16f, 0, 7.2);
planet_Parameters Neptune_param(13.f, 3.7f, 0, 0, 0.12f, 0, 6.7);
planet_Parameters Pluto_param(26.f, 4.0f, 0, 0, 0.10f, 0, 6.0);
//Moons
planet_Parameters Luna_param(6.0f, 0, 0, 0, 0.75f, 0, 7.0);//Earth
planet_Parameters Phobos_param(60.f, 3.3f, 0, 0, 0.12f, 0, 6.7);//Mars
planet_Parameters Europa_param(12.f, 3.0f, 0, 0, 0.16f, 0, 7.2);//Jupiter
planet_Parameters Titan_param(10.f, 2.8f, 0, 0, 0.25f, 0, 4.5);//Saturn
planet_Parameters Triton_param(10.f, 2.8f, 0, 0, 0.25f, 0, 4.5); //Neptune
//Other
planet_Parameters Voyager_param(200.f, 6.0f, 0, 0, 0.5f, 0, 0); 
planet_Parameters Belt_param(8.5f, 3.0f, 0, 0, 0.5f, 0.25f, 4.5);//Saturn

//Individual Planet models
Sphere	Sun_object, Mercury_object, Venus_object, Earth_object,
		Mars_object, Jupiter_object, Saturn_object, Uranus_object, 
		Neptune_object, Pluto_object, Titan_object, Europa_object, 
		Triton_object, Stars_object;

using namespace std;
using namespace glm;

//Image Loader 
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps)
{
	glGenTextures(1, &texID);		 												//Obtain an unused texture identifier. 
	int width, height, nrChannels; 													//Local image parameters.
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0); 	//Load an image file using stb_image.

	if (data)		// Check that the pixel formates are correct.
	{
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
		// texID will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);
		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}

//Function to initialisation stuff, called before the display loop.
void init(GLWrapper* glw)
{									

	model_scale = 1.f;				//Used for resizeing and to keep Planets and Moons Uniform
	aspect_ratio = 1024.f / 768.f;	//Initial aspect ratio from window size (variables would be better!)
	numlats = 20;					//Number of latitudes in our sphere
	numlongs = 20;					//Number of longitudes in our sphere
	Cam_Pos = 1;					//Directed Camera toggle
	
	/* Define the Blending function */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	//Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("main_shader.vert", "main_shader.frag");
		points_program = glw->LoadShader("point_shader.vert", "point_shader.frag");
		//light_emitter = glw->LoadShader("SolarS.vert", "SolarS.frag"); // Not used.

	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	// Define uniforms to send to point sprites program shaders.
	points_modelID = glGetUniformLocation(points_program, "model");
	points_sizeID = glGetUniformLocation(points_program, "size");
	points_viewID = glGetUniformLocation(points_program, "view");
	points_projectionID = glGetUniformLocation(points_program, "projection");

	//Defualt uniforms.
	modelID = glGetUniformLocation(program, "model");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");

	//Define light uniforms (Not used anymore).
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");
	lightposID = glGetUniformLocation(program, "lightpos");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	attenuationmodeID = glGetUniformLocation(program, "attenuationmode");

	//Create the Sphere object.
	Stars_object.makeSphere(numlats, numlongs);		Saturn_object.makeSphere(numlats, numlongs);
	Sun_object.makeSphere(numlats, numlongs);		Titan_object.makeSphere(numlats, numlongs);
	Mercury_object.makeSphere(numlats, numlongs);	Triton_object.makeSphere(numlats, numlongs);
	Venus_object.makeSphere(numlats, numlongs);		Europa_object.makeSphere(numlats, numlongs);
	Earth_object.makeSphere(numlats, numlongs);		Pluto_object.makeSphere(numlats, numlongs);
	Mars_object.makeSphere(numlats, numlongs);		Neptune_object.makeSphere(numlats, numlongs);
	Jupiter_object.makeSphere(numlats, numlongs);	Uranus_object.makeSphere(numlats, numlongs);

	//Define the image files that will be used for the textures
	const char* Sun_image = "Textures\\sun.jpg";		const char* Mercury_image = "Textures\\mercury.jpg";
	const char* Venus_image = "Textures\\venus.jpg";	const char* Earth_image = "Textures\\earth.jpg";
	const char* Mars_image = "Textures\\mars.jpg";		const char* Jupiter_image = "Textures\\jupiter.jpg";
	const char* Saturn_image = "Textures\\saturn.jpg";	const char* Uranus_image = "Textures\\uranus.jpg";
	const char* Neptune_image = "Textures\\neptune.jpg";const char* Stars_image = "Textures\\stars.jpg";
	const char* Moon_image = "Textures\\moon.jpg";		const char* Pluto_image = "Textures\\pluto.jpg";
	const char* Europa_image = "Textures\\europa.jpg";	const char* Triton_image = "Textures\\triton.jpg";
	const char* Titan_image = "Textures\\titan.jpg";	const char* point_texture_file = "Sprites\\rock.png";

	stbi_set_flip_vertically_on_load(true); // flip texture

	//load all the spesified textures
	load_texture(Sun_image, Sun_texture, true);			load_texture(Mercury_image, Mercury_texture, true);
	load_texture(Venus_image, Venus_texture, true);		load_texture(Earth_image, Earth_texture, true);
	load_texture(Mars_image, Mars_texture, true);		load_texture(Jupiter_image, Jupiter_texture, true);
	load_texture(Saturn_image, Saturn_texture, true);	load_texture(Uranus_image, Uranus_texture, true);
	load_texture(Neptune_image, Neptune_texture, true);	load_texture(Stars_image, Stars_texture, true);
	load_texture(Moon_image, Moon_texture, true);		load_texture(Pluto_image, Pluto_texture, true);
	load_texture(Europa_image, Europa_texture, true);	load_texture(Triton_image, Triton_texture, true);
	load_texture(Titan_image, Titan_texture, true);

	//Checks for errors in loading the textures
	if (!load_texture(Sun_image, Sun_texture, true)			|| !load_texture(Mercury_image, Mercury_texture, true) ||
		!load_texture(Venus_image, Venus_texture, true)		|| !load_texture(Earth_image, Earth_texture, true) ||
		!load_texture(Mars_image, Mars_texture, true)		|| !load_texture(Jupiter_image, Jupiter_texture, true) ||
		!load_texture(Saturn_image, Saturn_texture, true)	|| !load_texture(Uranus_image, Uranus_texture, true) ||
		!load_texture(Neptune_image, Neptune_texture, true) || !load_texture(Stars_image, Stars_texture, true) ||
		!load_texture(Pluto_image, Pluto_texture, true)		|| !load_texture(Europa_image, Europa_texture, true) ||
		!load_texture(Titan_image, Titan_texture, true)		|| !load_texture(Triton_image, Triton_texture, true) ||
		!load_texture(Moon_image, Moon_texture, true))
	{
		cout << "Error Loading Textures" << endl;
		exit(0);
	}

	// This is the location of the texture object (TEXTURE0), i.e. tex1 will be the name
	// of the sampler in the fragment shader
	int loc = glGetUniformLocation(program, "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	// Define the texture uniform for the points shader program
	int loc_points = glGetUniformLocation(points_program, "tex1");
	if (loc_points >= 0) glUniform1i(loc_points, 0);

	// SET Texture MAG_FILTER to linear which will blur the texture if we
	// zoom too close in
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Point sprites variables.
	point_speed = 0.1f;
	maxdist = 3.f;
	point_anim = new points(1200, maxdist, speed);
	point_anim->create();
	point_size = 2;

	//Blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//load obj's and set thier colour.
	SpaceBody.load_obj("Models\\Moon_2k.obj");
	SpaceBody.overrideColour(vec4(0.5f, 0, 1.0, 1.f));
	Asteroids.load_obj("Models\\Asteroid.obj");
	Asteroids.overrideColour(vec4(1.0f, 1.0, 1.0, 1.f));
	Phobos_obj.load_obj("Models\\Phobos.obj");
	Phobos_obj.overrideColour(vec4(1.0f, 1.0, 1.0, 1.f));
	Voyager_obj.load_obj("Models\\Voyager.obj");
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

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Make the compiled shader program current */
	glUseProgram(program);

	//lighting normalmatrix (not used anymore)
	mat3 normalmatrix;
	
	//Camera, fov can be altered by the mouse so it zooms in.
	mat4 projection = perspective(radians(fov), aspect_ratio, 0.1f, 100.0f);

	//Diffrent views
	mat4 sun_cam = lookAt(vec3(Sun_param.distance, 4, 2),
						vec3(Sun_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 mercury_cam = lookAt(vec3(Mercury_param.distance, 4, 2),
						vec3(Mercury_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 venus_cam =lookAt(vec3(Venus_param.distance, 4, 2),
						vec3(Venus_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 earth_cam = lookAt(vec3(Earth_param.distance, 4, 2),
						vec3(Earth_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 mars_cam = lookAt(vec3(Mars_param.distance, 4, 2),
						vec3(Mars_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 jupiter_cam = lookAt(vec3(Jupiter_param.distance, 4, 2),
						vec3(Jupiter_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 saturn_cam = lookAt(vec3(Saturn_param.distance, 4, 2),
						vec3(Saturn_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 neptune_cam = lookAt(vec3(Neptune_param.distance, 4, 2),
						vec3(Neptune_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 uranus_cam = lookAt(vec3(Uranus_param.distance , 4, 2),
						vec3(Uranus_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 pluto_cam = lookAt(vec3(Pluto_param.distance, 4, 2),
						vec3(Pluto_param.distance, 0, 0),
						vec3(0, -1, 0));
	mat4 voyager_cam = lookAt(vec3(Voyager_param.distance, 4, 2),
						vec3(Voyager_param.distance, 0, 0),
						vec3(0, -1, 0));

	//Makes sure the camera follows its target.
	sun_cam = rotate(sun_cam, -radians(Sun_param.orbit), vec3(0, 0, -1));
	mercury_cam = rotate(mercury_cam, -radians(Mercury_param.orbit), vec3(0, 0, -1));
	venus_cam = rotate(venus_cam, -radians(Venus_param.orbit), vec3(0, 0, -1));
	earth_cam = rotate(earth_cam, -radians(Earth_param.orbit), vec3(0, 0, -1));
	mars_cam = rotate(mars_cam, -radians(Mars_param.orbit), vec3(0, 0, -1));
	jupiter_cam = rotate(jupiter_cam, -radians(Jupiter_param.orbit), vec3(0, 0, -1));
	saturn_cam = rotate(saturn_cam, -radians(Saturn_param.orbit), vec3(0, 0, -1));
	neptune_cam = rotate(neptune_cam, -radians(Neptune_param.orbit), vec3(0, 0, -1));
	uranus_cam = rotate(uranus_cam, -radians(Uranus_param.orbit), vec3(0, 0, -1));
	pluto_cam = rotate(pluto_cam, -radians(Pluto_param.orbit), vec3(0, 0, -1));
	voyager_cam = rotate(voyager_cam, -radians(Voyager_param.orbit), vec3(0, 0, -1));

	//Create view.
	mat4 view;

	//Camera swapper 
	switch (Cam_Pos) {
	case 1:
		view = sun_cam;
		break;
	case 2:
		view = mercury_cam;
		break;
	case 3:
		view = venus_cam;
		break;
	case 4:
		view = earth_cam;
		break;
	case 5:
		view = mars_cam;
		break;
	case 6:
		view = jupiter_cam;
		break;
	case 7:
		view = saturn_cam;
		break;
	case 8:
		view = uranus_cam;
		break;
	case 9:
		view = neptune_cam;
		break;
	case 10:
		view = pluto_cam;
		break;
	case 11:
		view = voyager_cam;
		break;
	default:
		view = sun_cam;
	}

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

	// Define our model transformation in a stack and push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0); //Not used anymore.

	//Create the model matrices.
	mat4 sun = mat4(1.0f);
	mat4 mercury = mat4(1.0f);
	mat4 venus = mat4(1.0f);
	mat4 earth = mat4(1.0f);
	mat4 mars = mat4(1.0f);
	mat4 jupiter = mat4(1.0f);
	mat4 saturn = mat4(1.0f);
	mat4 uranus = mat4(1.0f);
	mat4 neptune = mat4(1.0f);
	mat4 pluto = mat4(1.0f);
	mat4 stars = mat4(1.0f);
	mat4 luna = mat4(1.0f);

	mat4 voyager = mat4(1.0f);
	mat4 sprite = mat4(1.0f);
	mat4 asteroid = mat4(1.0f);

	

	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis

	//Orbits
	mercury = rotate(mercury, -radians(Mercury_param.orbit), vec3(0, 0, 1));
	venus = rotate(venus, -radians(Venus_param.orbit), vec3(0, 0, 1));
	earth = rotate(earth, -radians(Earth_param.orbit), vec3(0, 0, 1));
	mars = rotate(mars, -radians(Mars_param.orbit), vec3(0, 0, 1));
	jupiter = rotate(jupiter, -radians(Jupiter_param.orbit), vec3(0, 0, 1));
	saturn = rotate(saturn, -radians(Saturn_param.orbit), vec3(0, 0, 1));
	uranus = rotate(uranus, -radians(Uranus_param.orbit), vec3(0, 0, 1));
	neptune = rotate(neptune, -radians(Neptune_param.orbit), vec3(0, 0, 1));
	pluto = rotate(pluto, -radians(Pluto_param.orbit), vec3(0, 0, 1));

	voyager = rotate(voyager, -radians(Voyager_param.orbit), vec3(0, 0, 2));
	sprite = rotate(sprite, -radians(Saturn_param.orbit), vec3(0, 0, 1));


	//Draw out planets, moons, sky, spacecraft and particals :

	//Draw Sun.___________________________________________________________________________________________________________________________________
	sun = translate(sun, vec3(Sun_param.distance, 0, 0));
	sun = scale(sun, vec3(model_scale / Sun_param.size, model_scale / Sun_param.size, model_scale / Sun_param.size));
	sun = rotate(sun, -radians(Sun_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Sun_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(sun[0][0]));

	//Not used anymore.
	normalmatrix = transpose(inverse(mat3(view * sun)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	emitmode = 1;
	glUniform1ui(emitmodeID, emitmode);

	Sun_object.drawSphere(drawmode);

	//Not used anymore.
	emitmode = 0;
	glUniform1ui(emitmodeID, emitmode);

	
//Draw Mercury._______________________________________________________________________________________________________________________________

	mercury = translate(mercury, vec3(Mercury_param.distance, 0, 0));
	mercury = scale(mercury, vec3(model_scale / Mercury_param.size, model_scale / Mercury_param.size, model_scale / Mercury_param.size));
	mercury = rotate(mercury, -radians(Mercury_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Mercury_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(mercury[0][0]));
	Mercury_object.drawSphere(drawmode);

//Draw Venus._________________________________________________________________________________________________________________________________

	venus = translate(venus, vec3(Venus_param.distance, 0, 0));
	venus = scale(venus, vec3(model_scale / Venus_param.size, model_scale / Venus_param.size, model_scale / Venus_param.size));
	venus = rotate(venus, -radians(Venus_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Venus_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(venus[0][0]));
	Venus_object.drawSphere(drawmode);

//Draw Earth._________________________________________________________________________________________________________________________________

	earth = translate(earth, vec3(Earth_param.distance, 0, 0));
	earth = scale(earth, vec3(model_scale / Earth_param.size, model_scale / Earth_param.size, model_scale / Earth_param.size));
	earth = rotate(earth, -radians(Earth_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Earth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(earth[0][0]));
	Earth_object.drawSphere(drawmode);

//Draw Earth_Moon.____________________________________________________________________________________________________________________________
		
	earth = translate(earth, vec3(Earth_param.distance + 1, 0, 0));
	earth = scale(earth, vec3(model_scale / Luna_param.size , model_scale / Luna_param.size , model_scale / Luna_param.size ));
	earth = rotate(earth, -radians(Luna_param.spin), vec3(0, 0, -1));
	glBindTexture(GL_TEXTURE_2D, Moon_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(earth[0][0]));

	SpaceBody.drawObject(drawmode);
		

//Draw Mars.__________________________________________________________________________________________________________________________________

	mars = translate(mars, vec3(Mars_param.distance, 0, 0));
	mars = scale(mars, vec3(model_scale / Mars_param.size, model_scale / Mars_param.size, model_scale / Mars_param.size));
	mars = rotate(mars, -radians(Mars_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Mars_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(mars[0][0]));
	Mars_object.drawSphere(drawmode);

//Draw Phobos.____________________________________________________________________________________________________________________________

	mars = translate(mars, vec3(Mars_param.distance + 0.5, 0, 0));
	mars = scale(mars, vec3(model_scale / Phobos_param.size, model_scale / Phobos_param.size, model_scale / Phobos_param.size));
	mars = rotate(mars, -radians(Phobos_param.spin), vec3(0, 0, -1));
	glBindTexture(GL_TEXTURE_2D, Moon_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(mars[0][0]));
	Phobos_obj.drawObject(drawmode);

//Draw Jupiter._______________________________________________________________________________________________________________________________
	
	jupiter = translate(jupiter, vec3(Jupiter_param.distance, 0, 0));
	jupiter = scale(jupiter, vec3(model_scale / Jupiter_param.size, model_scale / Jupiter_param.size, model_scale / Jupiter_param.size));
	jupiter = rotate(jupiter, -radians(Jupiter_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Jupiter_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(jupiter[0][0]));
	Jupiter_object.drawSphere(drawmode);

//Draw Europa.____________________________________________________________________________________________________________________________

	jupiter = translate(jupiter, vec3(Jupiter_param.distance - 1, 0, 0));
	jupiter = scale(jupiter, vec3(model_scale / Europa_param.size, model_scale / Europa_param.size, model_scale / Europa_param.size));
	jupiter = rotate(jupiter, -radians(Europa_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Europa_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(jupiter[0][0]));
	Europa_object.drawSphere(drawmode);

//Draw Saturn.________________________________________________________________________________________________________________________________

	saturn = translate(saturn, vec3(Saturn_param.distance, 0, 0));
	saturn = scale(saturn, vec3(model_scale / Saturn_param.size, model_scale / Saturn_param.size, model_scale / Saturn_param.size));
	saturn = rotate(saturn, -radians(Saturn_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Saturn_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(saturn[0][0]));
	Saturn_object.drawSphere(drawmode);


//Draw Titan.________________________________________________________________________________________________________________________________

	saturn = translate(saturn, vec3(Saturn_param.distance - 1.0, 0, 0));
	saturn = scale(saturn, vec3(model_scale / Titan_param.size, model_scale / Titan_param.size, model_scale / Titan_param.size));
	saturn = rotate(saturn, -radians(Titan_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Titan_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(saturn[0][0]));
	Titan_object.drawSphere(drawmode);

//Draw Uranus.________________________________________________________________________________________________________________________________

	uranus = translate(uranus, vec3(Uranus_param.distance, 0, 0));
	uranus = scale(uranus, vec3(model_scale / Uranus_param.size, model_scale / Uranus_param.size, model_scale / Uranus_param.size));
	uranus = rotate(uranus, -radians(Uranus_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Uranus_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(uranus[0][0]));
	Uranus_object.drawSphere(drawmode);

//Draw Neptune._______________________________________________________________________________________________________________________________

	neptune = translate(neptune, vec3(Neptune_param.distance, 0, 0));
	neptune = scale(neptune, vec3(model_scale / Neptune_param.size, model_scale / Neptune_param.size, model_scale / Neptune_param.size));
	neptune = rotate(neptune, -radians(Neptune_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Neptune_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(neptune[0][0]));
	Neptune_object.drawSphere(drawmode);

//Draw Triton._______________________________________________________________________________________________________________________________

	neptune = translate(neptune, vec3(Neptune_param.distance - 2, 0, 0));
	neptune = scale(neptune, vec3(model_scale / Triton_param.size, model_scale / Triton_param.size, model_scale / Triton_param.size));
	neptune = rotate(neptune, -radians(Triton_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Triton_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(neptune[0][0]));
	Triton_object.drawSphere(drawmode);

//Draw Pluto.________________________________________________________________________________________________________________________________

	pluto = translate(pluto, vec3(Pluto_param.distance, 0, 0));
	pluto = scale(pluto, vec3(model_scale / Pluto_param.size, model_scale / Pluto_param.size, model_scale / Pluto_param.size));
	pluto = rotate(pluto, -radians(Pluto_param.spin), vec3(0, 0, 1));

	glBindTexture(GL_TEXTURE_2D, Pluto_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(pluto[0][0]));
	Pluto_object.drawSphere(drawmode);

//Draw Voyager.________________________________________________________________________________________________________________________________

	voyager = translate(voyager, vec3(Voyager_param.distance, 0, 0));
	voyager = scale(voyager, vec3(model_scale / Voyager_param.size, model_scale / Voyager_param.size, model_scale / Voyager_param.size));
	

	glBindTexture(GL_TEXTURE_2D, Pluto_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(voyager[0][0]));
	Voyager_obj.drawObject(drawmode);

//Draw Stars._________________________________________________________________________________________________________________________________


	stars = translate(stars, vec3(0, 0, 0));
	stars = scale(stars, vec3(14.2f, 14.2f, 14.2f));

	glBindTexture(GL_TEXTURE_2D, Stars_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(stars[0][0]));
	Stars_object.drawSphere(drawmode);

//Draw Asteroid Belt.__________________________________________________________________________________________________________________________

	glUseProgram(points_program); //Change to point_shaders

	sprite = translate(sprite, vec3(Saturn_param.distance, 0, 0));
	sprite = scale(sprite, vec3(0.06,0.06,0));
	sprite = rotate(sprite, -radians(Sun_param.spin), vec3(0, 0, -1));

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(points_modelID, 1, GL_FALSE, &sprite[0][0]);
	glUniform1f(points_sizeID, point_size);
	glUniformMatrix4fv(points_viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(points_projectionID, 1, GL_FALSE, &projection[0][0]);

	// Enable gl_PointSize
	glEnable(GL_PROGRAM_POINT_SIZE);
	//Enable Blending for the analytic point sprite */
	glEnable(GL_BLEND);

	point_anim->draw();
	point_anim->animate();
//______________________________________________________________________________________________________________________________________________

	glDisableVertexAttribArray(0);
	glUseProgram(0); //remove shader program.

	// Modify our animation variables 
	Mercury_param.orbit += Mercury_param.orbit_inc;
	Venus_param.orbit += Venus_param.orbit_inc;
	Earth_param.orbit += Earth_param.orbit_inc;
	Mars_param.orbit += Mars_param.orbit_inc;
	Jupiter_param.orbit += Jupiter_param.orbit_inc;
	Saturn_param.orbit += Saturn_param.orbit_inc;
	Uranus_param.orbit += Uranus_param.orbit_inc;
	Neptune_param.orbit += Neptune_param.orbit_inc;
	Pluto_param.orbit += Pluto_param.orbit_inc;
	Voyager_param.orbit += Voyager_param.orbit_inc;
	Luna_param.orbit += Luna_param.orbit_inc;

	//planet_Parameterss spinning on their own axis
	Mercury_param.spin += Mercury_param.spin_speed;
	Venus_param.spin += Venus_param.spin_speed;
	Earth_param.spin += Earth_param.spin_speed;
	Mars_param.spin += Mars_param.spin_speed;
	Jupiter_param.spin += Jupiter_param.spin_speed;
	Saturn_param.spin += Saturn_param.spin_speed;
	Uranus_param.spin += Uranus_param.spin_speed;
	Neptune_param.spin += Neptune_param.spin_speed;
	Sun_param.spin += Sun_param.spin_speed;
	Pluto_param.spin += Pluto_param.spin_speed;
	Luna_param.spin += Luna_param.spin_speed;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	// Store aspect ratio to use for our perspective projection
	aspect_ratio = float(w) / float(h);
}

//Mouse wheele zoom.
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 10.0f)
		fov = 10.0f;
	if (fov >= 10.0f)
		fov -= 0.2f;
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//Fast forward
	if (key == GLFW_KEY_RIGHT)
		Mercury_param.orbit_inc += Mercury_param.orbit_speed,
		Venus_param.orbit_inc += Venus_param.orbit_speed,
		Earth_param.orbit_inc += Earth_param.orbit_speed,
		Mars_param.orbit_inc += Mars_param.orbit_speed,
		Jupiter_param.orbit_inc += Jupiter_param.orbit_speed,
		Saturn_param.orbit_inc += Saturn_param.orbit_speed,
		Uranus_param.orbit_inc += Uranus_param.orbit_speed,
		Neptune_param.orbit_inc += Neptune_param.orbit_speed,
		Pluto_param.orbit_inc += Pluto_param.orbit_speed,
		Voyager_param.orbit_inc += Voyager_param.orbit_speed,
		Luna_param.orbit_inc += Luna_param.orbit_speed;

	//Rewind
	if (key == GLFW_KEY_LEFT)
		Mercury_param.orbit_inc -= Mercury_param.orbit_speed,
		Venus_param.orbit_inc -= Venus_param.orbit_speed,
		Earth_param.orbit_inc -= Earth_param.orbit_speed,
		Mars_param.orbit_inc -= Mars_param.orbit_speed,
		Jupiter_param.orbit_inc -= Jupiter_param.orbit_speed,
		Saturn_param.orbit_inc -= Saturn_param.orbit_speed,
		Uranus_param.orbit_inc -= Uranus_param.orbit_speed,
		Neptune_param.orbit_inc -= Neptune_param.orbit_speed,
		Pluto_param.orbit_inc -= Pluto_param.orbit_speed,
		Voyager_param.orbit_inc -= Voyager_param.orbit_speed,
		Luna_param.orbit_inc -= Luna_param.orbit_speed;



	//Start
	if (key == GLFW_KEY_SPACE)
		Mercury_param.orbit_inc = Mercury_param.orbit_speed,
		Venus_param.orbit_inc = Venus_param.orbit_speed,
		Earth_param.orbit_inc = Earth_param.orbit_speed,
		Mars_param.orbit_inc = Mars_param.orbit_speed,
		Jupiter_param.orbit_inc = Jupiter_param.orbit_speed,
		Saturn_param.orbit_inc = Saturn_param.orbit_speed,
		Uranus_param.orbit_inc = Uranus_param.orbit_speed,
		Neptune_param.orbit_inc = Neptune_param.orbit_speed,
		Pluto_param.orbit_inc = Pluto_param.orbit_speed,
		Luna_param.orbit_inc = Luna_param.orbit_speed,
		Voyager_param.orbit_inc = Voyager_param.orbit_speed,
		Europa_param.orbit_inc = Europa_param.orbit_speed;

	//Stop
	if (key == GLFW_KEY_BACKSPACE){
		Mercury_param.orbit_inc = 0,
		Venus_param.orbit_inc = 0,
		Earth_param.orbit_inc = 0,
		Mars_param.orbit_inc = 0,
		Jupiter_param.orbit_inc = 0,
		Saturn_param.orbit_inc = 0,
		Uranus_param.orbit_inc = 0,
		Neptune_param.orbit_inc = 0;
		Pluto_param.orbit_inc = 0;
		Voyager_param.orbit_inc = 0;
	}

	//Change camera.
	if (key == '0' && action == GLFW_PRESS) Cam_Pos = 1, cout << "The Sun" << endl;
	if (key == '1' && action == GLFW_PRESS) Cam_Pos = 2, cout << "Mercury"  << endl;
	if (key == '2' && action == GLFW_PRESS) Cam_Pos = 3, cout << "Venus" << endl;
	if (key == '3' && action == GLFW_PRESS) Cam_Pos = 4, cout << "Earth" << endl;
	if (key == '4' && action == GLFW_PRESS) Cam_Pos = 5, cout << "Mars" << endl;
	if (key == '5' && action == GLFW_PRESS) Cam_Pos = 6, cout << "Jupiter" << endl;
	if (key == '6' && action == GLFW_PRESS) Cam_Pos = 7, cout << "Saturn" << endl;
	if (key == '7' && action == GLFW_PRESS) Cam_Pos = 8 , cout << "Uranus" << endl;
	if (key == '8' && action == GLFW_PRESS) Cam_Pos = 9, cout << "Neptune" << endl;
	if (key == '9' && action == GLFW_PRESS)	Cam_Pos = 10, cout << "Pluto" << endl;
	if (key == '`' && action == GLFW_PRESS)	Cam_Pos = 11, cout << "Voyager" << endl;
	if (key == 'V' && action == GLFW_PRESS) Cam_Pos += 1 ;

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'D' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}

}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Lab3 start example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	// Register the callback functions
	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);
	glw->setScrollCallback(scroll_callback);
	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}
