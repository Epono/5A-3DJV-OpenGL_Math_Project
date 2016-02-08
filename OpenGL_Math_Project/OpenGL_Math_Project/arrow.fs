 #version 330

uniform sampler2D u_sampler;

in Vertex
{
	vec3 normal;
	vec2 texcoords;
} IN;

out vec4 Fragment;

void main(void)
{
    vec4 texColor = texture(u_sampler, IN.texcoords);
	Fragment = vec4(0.941, 0.952, 0.384, 1.0);
}