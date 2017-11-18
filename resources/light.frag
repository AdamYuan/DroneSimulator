#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 LightColor;
uniform vec4 color;
uniform bool light;
void main()
{
    FragColor = color;
	if(light)
		LightColor = color;
	else
		LightColor = vec4(0.0f);
}
