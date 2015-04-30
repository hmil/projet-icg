#version 400 core

in vec2 position;
out vec2 vPosition;
out vec2 vUV;

uniform sampler2D tex;

void main()
{
  // Texturing
  vUV = (position + vec2(2.0, 2.0)) * 0.25;
  vPosition = position;
}
