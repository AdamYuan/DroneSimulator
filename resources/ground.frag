#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform float half_size;

void main()
{
    FragColor = texture(image, TexCoords);
    vec2 absCoord = abs(TexCoords);
    if(absCoord.x < half_size && absCoord.y < half_size)
        FragColor = mix(FragColor, vec4(1, 1, 0, 1), 0.3f);
}
