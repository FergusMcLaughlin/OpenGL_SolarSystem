// Basic fragment shader to demosntrate combining texturing with 
// vertex lighting
// Iain Martin 2019

#version 400

in vec4 fcolour;
in vec2 ftexcoord;
out vec4 outputColor;
in float fdistance;

// Fog parameters, could make them uniforms and pass them into the shader
float fog_maxdist = 50.0;
float fog_mindist = 20;
vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
const float fogDensity = 0.2;

uniform sampler2D tex1;

void main()
{
	float fog_factor;
	float dist = abs(fdistance);
	// exponential fog
	fog_factor = (fog_maxdist-dist) / (fog_maxdist - fog_mindist);
	// Limit fog factor to range 0 to 1
	fog_factor = clamp(fog_factor, 0.0, 1.0);

	vec4 texcolour = texture(tex1, ftexcoord);

	//outputColor = fcolour * texcolour;

	vec4 newcolour = fcolour * texcolour;

		outputColor = mix(fog_colour, newcolour, fog_factor);
}