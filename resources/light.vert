#version 330 core
layout(location=0) in vec4 position;
uniform mat4 view, projection;

void main()
{
	gl_Position = projection * view * position;
	gl_PointSize = max(100.0f / gl_Position.z, 2.0f);
}
