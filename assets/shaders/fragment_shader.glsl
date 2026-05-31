#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D ourTexture;

void main()
{
    vec4 texColor = texture(ourTexture, texCoord);

    // Grass top tint: atlas cell (col=0, row=0) → u in [0, 1/16], v in [15/16, 1]
    float cs = 1.0 / 16.0;
    if (texCoord.x < cs && texCoord.y > 15.0 * cs) {
        texColor.rgb *= vec3(0.72, 0.90, 0.50);
    }

    FragColor = texColor;
}