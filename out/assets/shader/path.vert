#version 400 core
layout (location = 0) in vec3 data;
out float alpha;

uniform mat4 proj;

void main() {
	gl_Position = proj * vec4(data.xy, 0, 1);
	alpha = 1.f - data.z * 0.12f;
	alpha *= abs(alpha);
}
