#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D environment;
uniform float exposure = 1.0f;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(scene, TexCoords).rgb;

	if(hdrColor.r > 0.8f)
	{
		FragColor = vec4(hdrColor, 1.0);
		return;
	}

	vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
	hdrColor += bloomColor; // additive blending
	// tone mapping
	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	// also gamma correct while we're at it
	result = pow(result, vec3(1.0 / gamma));

	vec3 envColor = texture(environment, TexCoords).rgb;
	FragColor = vec4(result + envColor, 1.0);
}
