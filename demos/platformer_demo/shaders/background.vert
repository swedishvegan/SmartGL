#version 430 core

const vec2 coords[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0),
	vec2(-1.0, -1.0)
);

out vec3 fray;

uniform mat3 rot;

void main() {
	
	gl_Position = vec4(coords[gl_VertexID], 0.0, 1.0);
	fray = rot * vec3(coords[gl_VertexID], -1.0);

}