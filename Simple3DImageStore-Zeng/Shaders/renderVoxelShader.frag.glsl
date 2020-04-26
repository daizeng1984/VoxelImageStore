// This is a shader to display 3d texture.
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable
#pragma IGLU_DISABLE BLENDING

in vec3 color; //eye space fragment position

out vec4 result;

void main( void ) {
	//Just output color if there is color
	result = vec4(color,1);
}