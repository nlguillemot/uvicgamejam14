#version 130

in vec4 position;
in vec2 texcoord;

uniform vec2 PositionScale;

out vec2 ftexcoord;

void main()
{
    ftexcoord = texcoord;
    gl_Position = vec4(PositionScale,0,1) * position;
}
