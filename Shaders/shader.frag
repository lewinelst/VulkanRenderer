#version 450

layout(location = 0) in vec3 fragPos;

layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 4) in vec3 lightPos;
layout(location = 5) in vec3 viewPos;
layout(location = 6) in vec3 lightColor;

layout(binding = 1) uniform sampler2D baseColorSampler;

layout(binding = 2) uniform sampler2D roughnessSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    // 

    vec3 color = texture(baseColorSampler, fragTexCoord).rgb;
    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 normalNormalized = normalize(normal);
    float diff = max(dot(lightDir, normalNormalized), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normalNormalized);
    float spec = 0.0;
 
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normalNormalized, halfwayDir), 0.0), 32.0);
    vec3 specular = texture(roughnessSampler, fragTexCoord).rgb * spec; 

	outColor = vec4(ambient + diffuse + specular, 1.0);

}