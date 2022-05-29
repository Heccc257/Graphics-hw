#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 ParticleColor;
out vec3 FragPos;

uniform mat4 projection;
uniform vec2 offset;
uniform vec4 color;
uniform float Life;
uniform float Z;
void main()
{

    float scale = 2.0f;
    if(Life < 0.4) {
        scale = scale * (0.5 + 0.5*Life/0.4);
    }
    TexCoords = vertex.zw;
    ParticleColor = color;

    // FragPos = vec3((vertex.xy * scale) + offset, 0.0);
    vec4 FPos = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
    
    // FPos.x+=100;;
    FragPos = vec3(FPos.x, FPos.y, FPos.z);
    FragPos.z = Z;

    gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);

}
