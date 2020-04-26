// This is a shader that display 3d texture in a cube
// The geometry shader part is used to check the normal
//    then choose the dominant projection direction
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable

#pragma IGLU_DISABLE  DEPTH_TEST
#pragma IGLU_DISABLE BLENDING

// Information about this geometry shader. For each triangle we compute normal to find the dominant
//   projection direction
layout (triangles, invocations = 1)		in;
layout (triangle_strip, max_vertices = 3)		out;

const float sqrtOfTwo = 1.414213562373095;

out vec3 vertexPos;

void main( void )
{
	// Compute the normal
	vec3 v[3];
	for(int i = 0; i < 3; ++ i)
	{
		v[i] = gl_in[i].gl_Position.xyz;
	}

	// Now I assume that CW 0 1 2
	vec3 normal = cross( v[0] - v[1], v[0] - v[2] ); 
	normal.x = abs(normal.x);
	normal.y = abs(normal.y);
	normal.z = abs(normal.z);
	// Check the n.v_xyz as in chapter 22 in OpenGL Insights
	float maxComp = max(normal.x, max(normal.y, normal.z));
	if( normal.x == maxComp )
	{
		// You rasterize it with the x as project direction
		for(int i = 0; i < 3; ++ i)
		{
			vec3 temp = gl_in[i].gl_Position.xyz;
			gl_Position = vec4(temp.z, temp.y, temp.x, 1);
			vertexPos = temp;
			EmitVertex();
		}


	}
	else if (normal.y == maxComp)
	{
		// You rasterize it with the x as project direction
		for(int i = 0; i < 3; ++ i)
		{ 
			vec3 temp = gl_in[i].gl_Position.xyz;
			gl_Position = vec4(temp.x, temp.z, temp.y, 1);
			vertexPos = temp;
			EmitVertex();
		}
	}
	else
	{
		//normally, projection direction is Z
		for(int i = 0; i < 3; ++ i)
		{ 
			gl_Position = vec4(gl_in[i].gl_Position.xyz, 1);
			vertexPos = gl_Position.xyz;
			EmitVertex();
		}
	}

	

	EndPrimitive();	
	
}