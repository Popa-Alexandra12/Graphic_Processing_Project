#version 410 core

layout (location = 0) in vec3 vPosition;
layout (location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;
out vec3 textureCoordinates;
out vec3 normal;
out vec4 fragPosEye;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	fragPosEye = view * model * vec4(vPosition, 1.0f);
	normal = vNormal;
    vec4 tempPos = projection * view * model * vec4(vPosition, 1.0);
    gl_Position = tempPos.xyww;
    textureCoordinates = vPosition;
}
