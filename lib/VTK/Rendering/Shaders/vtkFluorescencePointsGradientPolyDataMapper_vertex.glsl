varying vec4 screenPosition;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    screenPosition = gl_Vertex;
    gl_FrontColor = gl_Color;
}
