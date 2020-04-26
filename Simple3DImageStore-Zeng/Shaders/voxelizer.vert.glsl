// This is a shader that display 3d texture in a cube, right now we just need vertex position.
//    You need to make sure the input vertex is in [-1, 1]x[-1, 1]x[-1, 1]. It's simple with
//    Obj reader set to IGLU_OBJ_CENTER | IGLU_OBJ_UNITIZE
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable

#pragma IGLU_DISABLE  DEPTH_TEST
#pragma IGLU_DISABLE BLENDING

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here
//layout(location = IGLU_NORMAL) in vec3 normal;   // Sends normal data (if any) from models here


void main( void )
{
    // Transform vertex into clip space
	gl_Position = vec4( vertex, 1.0f );	
}