#version 330

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoords;

uniform mat4 u_worldMatrix;

layout(std140) uniform ViewProj
{
	mat4 u_viewMatrix;
	mat4 u_projectionMatrix;
};

out Vertex
{
	vec3 normal;
	vec2 texcoords;
} OUT;

void main(void)
{
	vec3 N = mat3(u_worldMatrix) * a_normal;
	OUT.normal = N;
	OUT.texcoords = a_texcoords;
	gl_Position = u_projectionMatrix * u_viewMatrix * u_worldMatrix * a_position;

}