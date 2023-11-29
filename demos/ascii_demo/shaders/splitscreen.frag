#version 430 core

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D lh;
uniform sampler2D rh;

void main() {

    if (texCoords.x <= 0.5) fragColor.rgb = texture(lh, texCoords * vec2(2.0, 1.0)).rgb;
    else fragColor.rgb = texture(rh, (texCoords - vec2(0.5, 0.0)) * vec2(2.0, 1.0)).rgb;

    fragColor.a = 1.0;

}