#version 400 core
in vec2 tpos;

out vec4 outcol;

uniform sampler2D tex;

void main() {
	outcol = texture(tex, tpos);
	if (outcol.a < 0.5)
		discard;
}
