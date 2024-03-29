#version 330

const int colors = 256;

// Input fragment attributes (from fragment shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform ivec3 palette[colors];

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord)*fragColor;

    // Convert the (normalized) texel color RED component (GB would work, too)
    // to the palette index by scaling up from [0, 1] to [0, 255].
    int index = int(texelColor.r*255.0);
    ivec3 color = palette[index];

    // Calculate final fragment color. Note that the palette color components
    // are defined in the range [0, 255] and need to be normalized to [0, 1]
    // for OpenGL to work.

    finalColor = vec4(color/255.0, texelColor.a);
}
