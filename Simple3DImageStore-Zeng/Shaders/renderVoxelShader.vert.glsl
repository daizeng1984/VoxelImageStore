// This is a shader that display 3d texture in a cube
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable

layout(location = IGLU_VERTEX) in vec3 vertex;   // Sends vertex data from models here

uniform mat4 model;	 // Transforms: model space -> world space
uniform mat4 view;   // Transforms: world space -> eye space
uniform mat4 proj;   // Transforms: eye space   -> clip space

uniform sampler3D tex3D;

out vec3 color;

uniform float voxWidth;

void main( void )
{
	// Get the voxel position based on the ID of the cube
	int k = gl_InstanceID/int(voxWidth*voxWidth);
	int s = gl_InstanceID%int(voxWidth*voxWidth);
	int j = s/int(voxWidth);
	int i = s%int(voxWidth);
	vec3 pos = vec3(i, j, k)/voxWidth + vec3(0.5)/voxWidth;
	
	// Store the eye-space position of this vertex, send to frag shader
	gl_Position = proj * view * model * vec4( vertex/voxWidth + 2*(pos-0.5), 1.0f );

	color = texture(tex3D, pos).rgb;
	vec3 offset[8];
	offset[0] = vec3(1, 1, 1);
	offset[1] = vec3(-1, -1, 1);
	offset[2] = vec3(1, -1, 1);
	offset[3] = vec3(-1, 1, 1);
	offset[4] = vec3(1, 1, -1);
	offset[5] = vec3(-1, -1, -1);
	offset[6] = vec3(1, -1, -1);
	offset[7] = vec3(-1, 1, -1);

	if( length(color) < 0.00001)
	{
		gl_Position = vec4(0,0,0,1);
	}
	else
	{
		float occlusion = 0;
		//Compute the ambient occlusion
		for(int i = 0; i < 8; ++ i)
		{
			vec3 temp = vertex.xyz/(2.0*voxWidth) + pos + offset[i]/(2.0*voxWidth);
			temp = texture(tex3D, temp).rgb;
			if(length(temp) > 0.00001)
			{
				occlusion += 1.0; 
			}
		}
		occlusion = occlusion/8.0;

		color = (1 - occlusion)*color;
	}
}