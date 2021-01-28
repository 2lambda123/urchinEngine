#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 texCoord;

out vec2 textCoordinates;
invariant gl_Position;

void main() {
    textCoordinates = texCoord;
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
