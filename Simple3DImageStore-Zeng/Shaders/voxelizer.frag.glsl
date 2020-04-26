// This is a shader to display 3d texture.
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable

#pragma IGLU_DISABLE  DEPTH_TEST
#pragma IGLU_DISABLE BLENDING

in vec3 vertexPos;

// 3D texture as our output buffer
coherent uniform layout(RGBA32F) image3D texImg3D;

// 3D texture's size, cube: texWidth x texWidth x texWidth
uniform float texWidth;


void main( void ) {
	// Directly write the color
	//memoryBarrier();
	ivec3 coord = ivec3((vertexPos+1)/2.0*(texWidth-1) + 0.5/texWidth);
	//vec4 accColor = imageLoad(texImg3D,coord);
	//memoryBarrier();
	imageStore(texImg3D, coord, vec4(0, 1, 0,1));

}