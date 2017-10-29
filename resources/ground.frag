#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform float half_size;

void main()
{
    FragColor = texture(image, TexCoords);
    if(-half_size <= TexCoords.x && TexCoords.x <= half_size - 1.0f
    && -half_size <= TexCoords.y && TexCoords.y <= half_size - 1.0f)
        FragColor = mix(FragColor, vec4(0.8, 0, 0, 1), 0.3f);
}
