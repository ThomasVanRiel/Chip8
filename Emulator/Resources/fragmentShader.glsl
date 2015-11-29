#version 330 core
in vec2 UV;
out vec4 color;
uniform sampler2D texSampler;

float distortion = 0.2f;


vec2 BarrelDistort(vec2 uv){
	vec2 newUV = uv - vec2(0.5f, 0.5f);
	float rd = length(newUV);
	float ru = rd * (1 + distortion * rd * rd);
	newUV = normalize(newUV) * ru;
	newUV += vec2(0.5f, 0.5f);
	newUV = clamp(newUV, 0, 1);
	return newUV;
}

void main() {
	vec2 newUV = BarrelDistort(UV);
	//newUV = UV;

	color = texture(texSampler, newUV);
	color = (color / 1.2f) + 0.15f;
	color *= 0.1f * sin(newUV.y * 32 * 40) * 2 + 0.9f;
	color *= clamp((pow(sin(newUV.x * 3.1415f) * sin(newUV.y * 3.1415f) , 0.25f)), 0, 1);
	
	//color *= vec4(0, 0.8f, 0, 1);
}