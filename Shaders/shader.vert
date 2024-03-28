#version 450

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;

	vec3 lightPos;
	vec3 viewPos;
	vec3 lightColor;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;

layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragTexCoord;

layout(location = 4) out vec3 lightPos;
layout(location = 5) out vec3 viewPos;
layout(location = 6) out vec3 lightColor;


void main()
{
	fragPos = vec3(ubo.model * vec4(inPosition, 1.0));

	gl_Position = ubo.proj * ubo.view * vec4(fragPos, 1.0);

	normal = mat3(transpose(inverse(ubo.model))) * inNormal;
	fragColor = inColor;
	fragTexCoord = inTexCoord;

	lightPos = ubo.lightPos;
	viewPos = ubo.viewPos;
	lightColor = ubo.lightColor;
}
