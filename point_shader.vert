// Minimal vertex shader for Point Sprite object
// Adapted from Example 6.1 in the Redbook V4.3

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;


// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
uniform float size;

void main()
{
	vec4 colour_h = vec4(colour, 1.0);
	vec4 pos = vec4(position, 1.0);
	vec4 pos2 = model * pos;
	
	// Pass through the vertex colour
	fcolour = colour_h;

	// Define the vertex position
	gl_Position = projection * view * model * pos;

	gl_PointSize = (1.0 - pos2.z / pos2.w) * size;
}

