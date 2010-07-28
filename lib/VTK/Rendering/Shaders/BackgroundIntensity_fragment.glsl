#extension GL_ARB_texture_rectangle : enable
#pragma optimize(on)

uniform float intensity;

void main() {
  gl_FragColor.rgb = vec3(intensity);
  gl_FragColor.a = 1.0;
}
