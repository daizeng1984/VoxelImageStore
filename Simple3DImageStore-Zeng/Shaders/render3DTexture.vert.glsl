// This is a shader that display 3d texture in a cube
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
layout(location = IGLU_NORMAL) in vec3 normal;   // Sends normal data (if any) from models here

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

out vec4 esFragNormal; 
out vec4 esFragPos;
out vec3 texCoord;

void main( void )
{  
	// Store the eye-space position of this vertex, send to frag shader
	esFragPos = view * model * vec4( vertex, 1.0f );
	
	// Transform vertex normal to eye-space, send to frag shader
	esFragNormal = inverse( transpose( view * model ) ) * vec4( normal, 0.0 );	

	// Our input is a cube and this cube will have [-1, 1] x [-1, 1] x [-1, 1], and
	//    it can be used for texture coordinates by convert from [-1, 1]to[0, 1]
	texCoord = (vertex + 1 )/2.0;


    // Transform vertex into clip space
	gl_Position = proj * ( view * model * vec4( vertex, 1.0f ) );	
}