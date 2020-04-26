// This is a shader to display 3d texture.
#version 420
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_shader_image_load_store : enable
#pragma IGLU_DISABLE BLENDING

in vec4 esFragPos; //eye space fragment position
in vec3 texCoord;

out vec4 result;

coherent uniform layout(RGBA32F) readonly image3D texImg3D;

uniform mat4x4 invViewModel; //inverse of View matrix

uniform float texWidth;
uniform float marchStepNum;

bool IsInsideCube(vec3 pos, vec3 minV, vec3 maxV)
{
	if ( pos.x > minV.x && pos.y > minV.y && pos.z > minV.z
	  && pos.x < maxV.x && pos.y < maxV.y && pos.z < maxV.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void main( void ) {
	// Compute the view ray in object space
	vec3 objFragPos = (invViewModel * vec4(esFragPos.xyz, 1)).xyz;
	vec3 objEyePos = (invViewModel * vec4(0, 0, 0, 1)).xyz;

	// Ray direction
	vec3 objRayDir = normalize(objFragPos - objEyePos);

	// Ray matching and stop when Ray is outside of the cube box
	memoryBarrier();
	ivec3 coord = ivec3(texCoord*texWidth);
	vec4 accColor = imageLoad(texImg3D,coord);
	//vec4 accColor = texture(tex3D,texCoord);
	float tmpMarchStepNum = max(marchStepNum, texWidth*4);

	float cellLength = 2.0*1.73/tmpMarchStepNum;
	vec3 currentPos = objFragPos + cellLength * objRayDir;

	int i;
	for(i = 0; i < marchStepNum; ++ i)
	{
		if (!IsInsideCube(currentPos, vec3(-1), vec3(1)) )
			break;
		// Get the color and accumulate
		//accColor = accColor + texture(tex3D, (currentPos+1)/2.0);
		accColor = accColor + imageLoad(texImg3D, ivec3((currentPos+1)/2.0*texWidth) );
		// Keep matching
		currentPos = currentPos + cellLength * objRayDir;
	}

		
	result = accColor/(marchStepNum);

	//if(i <= 2)
		//result = vec4(1,0,0,1);

}