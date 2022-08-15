#version 400 core
in float alpha;

out vec4 outcol;

void main() {
	if (alpha <= 0.f)
		discard;
	outcol = vec4(0, 1, 1, alpha);
}
