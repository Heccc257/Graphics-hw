#version 330 core
in vec2 TexCoords;
in vec4 ParticleColor;
in vec3 FragPos;
out vec4 color;

uniform vec3 lightPos;
uniform vec3 lightShootDir;
uniform vec3 lightColor;
uniform vec3 Normal;

uniform vec3 viewPos;
uniform vec3 viewFront;

uniform sampler2D sprite;

void main()
{
    vec4 objectColor = texture(sprite, TexCoords) * ParticleColor;
    // vec4 objectColor = ParticleColor;
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
   // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float mid = 0.58;
    diff = 0.55 + 4.0 * (diff-mid);
    diff = max(0, diff);
    diff = min(1.0, diff);
    // if(diff > 0.7) {
    //     objectColor = vec4(100.0, 0.0, 0.0, 1.0);
    // }
    // if(diff < 0.3) {
    //     objectColor = vec4(0.0, 100.0, 0.0, 1.0);
    // }
    vec3 diffuse = 1.5 * diff * lightColor;

   // specular
    float specularStrength = 0.1;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);

    vec3 specular = specularStrength * spec * lightColor;

    // float shootStrength = max(dot(normalize(lightShootDir), lightDir), 0.0);\n"
    
    // float viewStrength = max(dot(normalize(viewFront), viewDir), 0.0);
    // diffuse = viewStrength * diffuse;
    // specular = viewStrength * specular;




    vec4 result = vec4((ambient + diffuse + specular), 1.0) * objectColor;

    // if(result.x>1.0) {
    //     result = vec4(1.0, 0.0, 0.0, 1.0);
    // }
    // if(result.y>1.0) {
    //     result = vec4(1.0, 0.0, 0.0, 1.0);
    // }
    // if(result.z>1.0) {
    //     result = vec4(1.0, 0.0, 0.0, 1.0);
    // }

    color = result;
    // color = objectColor * vec4((specular), 1.0);

    // color = objectColor * vec4((specular), 1.0);
    // color = (texture(sprite, TexCoords) * ParticleColor);
}
