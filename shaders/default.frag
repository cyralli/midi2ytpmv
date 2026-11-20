#version 330 core

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2DArray video;
uniform int time;

void main() {
	vec4 textureVideo = texture(
		video, vec3(texCoord.x, 1.0 - texCoord.y, float(time))
	);
	if (time != -1)
		fragColor = textureVideo;
	else fragColor = textureVideo * vec4(0.5, 0.5, 0.5, 1.0);
}
