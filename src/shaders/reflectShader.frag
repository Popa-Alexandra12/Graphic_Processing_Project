#version 410 core

in vec3 textureCoordinates;
in vec3 normal;
in vec4 fragPosEye;
out vec4 color;

uniform samplerCube skybox;

void main()
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 viewDirectionN = normalize(cameraPosEye - fragPosEye.xyz);
	vec3 normalN = normalize(normal);
	vec3 reflection = reflect(viewDirectionN, normalN);
	vec3 colorFromSkybox = vec3(texture(skybox, reflection));
	color = colorFromSkybox;
}
