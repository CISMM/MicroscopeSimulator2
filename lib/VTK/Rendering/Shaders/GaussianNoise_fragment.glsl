#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable
#pragma optimize(on)

typedef unsigned int uint32;

uniform int scrambleKey;
uniform float frameWidth;
uniform float stdDev;
uniform float mean;


uint32 leftRotate(uint32 x, uint32 c) {
  return (x << c) | (x >> (32u - c));
}


uvec4 mix(uvec4 tD, uint32 f, int i, uint32 data, int r) {
  uint32 temp = tD.x;
  tD = tD.wyyz;
  tD.y += leftRotate(temp + f + uint32(floor(abs(sin(float(i) + 1.0))) * (~0u)) + data, r);
  return tD;
}


void md5(vec4 message, int key, out uvec4 digest) {
  ivec4  r = ivec4( 7, 12, 17, 22);
  ivec4 r2 = ivec4( 5,  9, 14, 20);
  ivec4 r3 = ivec4( 4, 11, 16, 23);
  ivec4 r4 = ivec4( 6, 10, 15, 21);

  //Pad message with zeros
  uint32 data[16];
  for (int i = 0; i < 12; i++) data[i   ] = 0u;

  // This is okay because the texture coordinates will
  // range over pixel coordinates.
  data[12] = uint32(message.x);
  data[13] = uint32(message.y);
  data[14] = uint32(message.z);
  data[15] = uint32(message.w);

  // Scramble data with key
  for (int i = 0; i < 16; i++) {
    data[i] = data[i] ^ key;
  }

  digest = uvec4(0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u);

  uvec4 tD = digest;

  // Round 1
  for (int i = 0; i < 16; i++) {
    uint32 f = (tD.y & tD.z) | ((~tD.y) & tD.w);
    int    g = i;
    tD = mix(tD, f, i, data[g], r.x);
    r  = r.yzwx;
  }

  // Round 2
  r = r2;
  for (int i = 16; i < 32; i++) {
    uint32 f = (tD.w & tD.y) | (tD.w & (~tD.z));
    int    g = (5*i + 1) % 16;
    tD = mix(tD, f, i, data[g], r.x);
    r  = r.yzwx;
  }

  // Round 3
  r = r3;
  for (int i = 32; i < 48; i++) {
    uint32 f = tD.y ^ tD.z ^ tD.w;
    int    g = (5*i + 1) % 16;
    tD = mix(tD, f, i, data[g], r.x);
    r  = r.yzwx;
  }

  // Round 4
  r = r4;
  for (int i = 48; i < 64; i++) {
    uint32 f = tD.z ^ (tD.y | (~tD.w));
    int    g = (3*i + 5) % 16;
    tD = mix(tD, f, i, data[g], r.x);
    r  = r.yzwx;
  }

  digest += tD;
}


float uniformToGaussian(uvec2 uniformRand) {
  float maxFloat = float(~0u);

  // The conversion from unsigned int to float used to appear as a
  // conversion from a signed int. Maybe it was a compiler bug? 
  // We used to bias the conversion accordingly, but that bug appears
  // now to be fixed.
  //vec2 x = (vec2(uniformRand) / maxFloat) + 0.5;
  vec2 x = (vec2(uniformRand) / maxFloat);
  float y = sqrt(-2.0f * log(x[0])) * cos(6.28318531f*x[1]);
  float z = stdDev * y + mean;

  return z;
}


void main() {
  // Get noise
  uvec4 digest;
  md5(gl_TexCoord[0], scrambleKey, digest);

  gl_FragColor.rgb = vec3(uniformToGaussian(digest.xy));
  gl_FragColor.a = 1.0f;
}
