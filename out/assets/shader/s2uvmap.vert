#version 400 core
layout (location = 0) in vec4 data;

out vec2 tpos;

uniform mat4 proj;
uniform vec4 uvuv;

void main() {
	gl_Position = proj * vec4(data.xy, 0, 1);
	tpos = data.zw * (uvuv.zw - uvuv.xy) + uvuv.xy;
}
