 #version 330

 // Direction VERS la source lumineuse exprimee dans le repere WORLD
const vec3 L = vec3(0.0, 0.0, 1.0);

uniform sampler2D u_sampler;

in Vertex
{
	vec3 normal;
	vec2 texcoords;
	float useTransparency;
	vec3 lightDirection;
} IN;

out vec4 Fragment;

void main(void)
{
    vec4 texColor = texture(u_sampler, IN.texcoords);

	if(IN.useTransparency < 0.5) {
		// calcul du cosinus de l'angle entre les deux vecteurs
		// Lumière fixe
		// float NdotL = max(dot(normalize(IN.normal), L), 0.0);
		// Lumière bougeable
		float NdotL = max(dot(normalize(IN.normal), -IN.lightDirection), 0.0);
		// Equation de Lambert : Intensite Reflechie = Intensite Incidente * N.L
		Fragment = texColor * NdotL;
	} else {
		Fragment = vec4(texColor.xyz, 0.5);
	}
}