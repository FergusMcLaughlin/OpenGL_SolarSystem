/* sphere.cpp
 Example class to create a generic sphere object with normals, colours and texture coordinates
 Resolution can be controlled by setting the number of latitudes and longitudes.
 This version includes duplicated vertices along the longitudinal dateline to enable texture to
 be fully wrapped around the sphere and texture coordinates.
 Iain Martin November 2019
*/

#include "sphere_tex.h"

/* I don't like using namespaces in header files but have less issues with them in
seperate cpp files */
using namespace std;

/* Define the vertex attributes for vertex positions and normals.
Make these match your application and vertex shader
You might also want to add colours and texture coordinates */
Sphere::Sphere(bool useTexture)
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	attribute_v_texcoord = 3;
	numspherevertices = 0;		// We set this when we know the numlats and numlongs values in makeSphere
	drawmode = 0;

	// Initialise other member variables (good practice)
	sphereBufferObject = 0;
	sphereNormals = 0;
	sphereColours = 0;
	sphereTexCoords = 0;
	elementbuffer = 0;
	
	// Turn texture off if you're not handling texture coordinates in your shaders
	enableTexture = useTexture;
}

Sphere::~Sphere()
{
}


/* Make a sphere from two triangle fans (one at each pole) and triangle strips along latitudes */
/* This version uses indexed vertex buffers for both the fans at the poles and the latitude strips */
void Sphere::makeSphere(GLuint numlats, GLuint numlongs)
{
	GLuint i, j;
	/* Calculate the number of vertices required in sphere */
	GLuint numvertices = 2 + ((numlats - 1) * (numlongs));

	// Store the number of sphere vertices in an attribute because we need it later when drawing it
	numspherevertices = numvertices;
	this->numlats = numlats;
	this->numlongs = numlongs;

	// Create the temporary arrays to create the sphere vertex attributes
	GLfloat* pVertices = new GLfloat[numvertices * 3];
	GLfloat* pTexCoords = new GLfloat[numvertices * 2];
	GLfloat* pColours = new GLfloat[numvertices * 4];
	makeUnitSphere(pVertices, pTexCoords);

	/* Define colours as the x,y,z components of the sphere vertices */
	for (i = 0; i < numvertices; i++)
	{
		pColours[i * 4] = pVertices[i * 3];
		pColours[i * 4 + 1] = pVertices[i * 3 + 1];
		pColours[i * 4 + 2] = pVertices[i * 3 + 2];
		pColours[i * 4 + 3] = 1.f;
	}

	/* Generate the vertex buffer object */
	glGenBuffers(1, &sphereBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &sphereNormals);
	glBindBuffer(GL_ARRAY_BUFFER, sphereNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &sphereColours);
	glBindBuffer(GL_ARRAY_BUFFER, sphereColours);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 4, pColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the texture coords in a buffer object */
	/* Create the texture coordinate  buffer for the cube */
	if (enableTexture)
	{
		glGenBuffers(1, &sphereTexCoords);
		glBindBuffer(GL_ARRAY_BUFFER, sphereTexCoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 2, pTexCoords, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/* Calculate the number of indices in our index array and allocate memory for it */
	GLuint numindices = ((numlongs * 2)) * (numlats - 2) + ((numlongs + 1) * 2);
	GLuint* pindices = new GLuint[numindices];

	// fill "indices" to define triangle strips
	GLuint index = 0;		// Current index

	// Define indices for the first triangle fan for one pole
	for (i = 0; i < numlongs + 1; i++)
	{
		pindices[index++] = i;
	}

	GLuint start = 1;		// Start index for each latitude row
	for (j = 0; j < numlats - 2; j++)
	{
		for (i = 0; i < numlongs; i++)
		{
			pindices[index++] = start + i;
			pindices[index++] = start + i + numlongs;
		}
		start += numlongs;
	}

	// Define indices for the last triangle fan for the south pole region
	for (i = numvertices - 1; i > (numvertices - numlongs - 2); i--)
	{
		pindices[index++] = i;
	}

	// Generate a buffer for the indices
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numindices * sizeof(GLuint), pindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete [] pTexCoords;
	delete [] pindices;
	delete [] pColours;
	delete [] pVertices;
}


/* Define the vertex positions and texture coordinates for a sphere. 
  The array of vertices must have previously been created.
*/
void Sphere::makeUnitSphere(GLfloat *pVertices, GLfloat *pTexCoords)
{
	GLfloat DEG_TO_RADIANS = 3.141592f / 180.f;
	GLuint vnum = 0;
	GLfloat x, y, z, lat_radians, lon_radians;

	/* Define north pole */
	pVertices[0] = 0; pVertices[1] = 0; pVertices[2] = 1.f;
	pTexCoords[0] = 0.5;	pTexCoords[1] = 1.0;
	vnum++;

	GLfloat latstep = 180.f / numlats;
	GLfloat longstep = 360.f / (numlongs - 1);

	/* Define vertices along latitude lines */
	for (GLfloat lat = 90.f - latstep; lat > -90.f; lat -= latstep)
	{
		lat_radians = lat * DEG_TO_RADIANS;

		// This includes a quick hack to avoid a precion error.
		// lon should be limited to a max of 180.f but in practice this misses
		// out the dateline veretx duplication in some onstances (values on numlongs) so
		// I've increased  lon max to 180.f + longstep / 10.f to ensure that the last
		// set of vertices gets drawn. 
		for (GLfloat lon = -180.f; lon <= (180.f + longstep / 10.f); lon += longstep)
		{
			lon_radians = lon * DEG_TO_RADIANS;

			x = cos(lat_radians) * cos(lon_radians);
			y = cos(lat_radians) * sin(lon_radians);
			z = sin(lat_radians);

			/* Define the vertex */
			pVertices[vnum * 3] = x; pVertices[vnum * 3 + 1] = y; pVertices[vnum * 3 + 2] = z;

			/* Define the texture coordinates as normalised lat/long values */
			float u = (lon + 180.f) / 360.f;
			float v = (lat + 90.f) / 180.f;

			pTexCoords[vnum * 2] = u;
			pTexCoords[vnum * 2 + 1] = v;

			vnum++;
		}
	}
	/* Define south pole */
	pVertices[vnum * 3] = 0; pVertices[vnum * 3 + 1] = 0; pVertices[vnum * 3 + 2] = -1.f;
	pTexCoords[vnum * 2] = 0.5; pTexCoords[vnum * 2 + 1] = 0.f;
}

/* Draws the sphere form the previously defined vertex and index buffers */
void Sphere::drawSphere(int drawmode)
{
	GLuint i;

	/* Draw the vertices as GL_POINTS */
	glBindBuffer(GL_ARRAY_BUFFER, sphereBufferObject);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attribute_v_coord);

	/* Bind the sphere normals */
	glEnableVertexAttribArray(attribute_v_normal);
	glBindBuffer(GL_ARRAY_BUFFER, sphereNormals);
	glVertexAttribPointer(attribute_v_normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the sphere colours */
	glBindBuffer(GL_ARRAY_BUFFER, sphereColours);
	glVertexAttribPointer(attribute_v_colours, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attribute_v_colours);

	/* Bind the sphere texture coordinates */
	if (enableTexture)
	{
		glBindBuffer(GL_ARRAY_BUFFER, sphereTexCoords);
		glVertexAttribPointer(attribute_v_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attribute_v_texcoord);
	}

	// Define triangle winding as counter-clockwise
	glFrontFace(GL_CCW);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, numspherevertices);
	}
	else
	{
		/* Bind the indexed vertex buffer */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		/* Draw the north pole regions as a triangle  */
		glDrawElements(GL_TRIANGLE_FAN, numlongs + 1, GL_UNSIGNED_INT, (GLvoid*)(0));

		/* Calculate offsets into the indexed array. Note that we multiply offsets by 4
		because it is a memory offset the indices are type GLuint which is 4-bytes */
		GLuint lat_offset_jump = (numlongs * 2);
		GLuint lat_offset_start = numlongs + 1;
		GLuint lat_offset_current = lat_offset_start * 4;

		/* Draw the triangle strips of latitudes */
		for (i = 0; i < numlats - 2; i++)
		{
			glDrawElements(GL_TRIANGLE_STRIP, numlongs * 2, GL_UNSIGNED_INT, (GLvoid*)(lat_offset_current));
			lat_offset_current += (lat_offset_jump * 4);
		}
		/* Draw the south pole as a triangle fan */
		glDrawElements(GL_TRIANGLE_FAN, numlongs + 1, GL_UNSIGNED_INT, (GLvoid*)(lat_offset_current));
	}
}