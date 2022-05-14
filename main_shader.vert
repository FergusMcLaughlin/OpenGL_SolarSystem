// Lab5 start vertex shader with Gouraud shading (vertex lighting). 
// You need to edit this fiel to handle the texture coordinates
// and to pass them through to the fragment shader

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;


// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
vec4 ambient = vec4(0, 0,0,0.0);
vec3 light_dir = vec3(0.0, 0.0, 10.0);

layout(location = 3) in vec2 texcoord;
// Output the texture coordinate - just pass it through
out vec2 ftexcoord;

void main()
{
	vec4 specular_colour = vec4(1.0,1.0,1.0,1.0);
	vec4 diffuse_colour = vec4(1.0,1.0,1.0,1.0);
	vec4 position_h = vec4(position, 1.0);
	float shininess = 20;
	
	ftexcoord = texcoord;



	ambient = diffuse_colour * 0.2;

	mat4 mv_matrix = view * model;
	mat3 n_matrix = transpose(inverse(mat3(mv_matrix)));
	vec3 N = normalize(n_matrix * normal);
	vec3 L = normalize(light_dir);

	vec4 diffuse = max(dot(N, L), 0.0) * diffuse_colour;

	vec4 P = mv_matrix * position_h;
	vec3 V = normalize(-P.xyz);						// Viewing vector is reverse of vertex position in eye space
	vec3 R = reflect(-L, N);						// Calculate the reflected beam, N defines the plane (see diagram on labsheet)
	vec4 specular = pow(max(dot(R, V), 0), shininess) * specular_colour;	// Calculate specular component

	// This is an alternative specular calculation: Blinn-Phong
	//vec3 half_vec = normalize(L - P.xyz);
//	vec4 specular = pow(max(dot(N, half_vec), 0.0), shininess) * specular_colour;

	// Define the vertex colour
	fcolour = diffuse + ambient + specular;

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}
