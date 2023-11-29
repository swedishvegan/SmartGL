#version 430 core

const vec2 vertices[] = vec2[]( 
	vec2(0.0, 0.0), 
	vec2(1.0, 0.0), 
	vec2(1.0, 1.0), 
	vec2(1.0, 1.0), 
	vec2(0.0, 1.0), 
	vec2(0.0, 0.0) 
); 

out vec2 texCoords;

void main() {

    texCoords = vertices[gl_VertexID];
    gl_Position = vec4(mix(vec2(-1.0), vec2(1.0), texCoords), 0.0, 1.0);

}