#version 130
#extension GL_ARB_texture_rectangle : enable
#pragma optimize(on)

uniform sampler3D     psfSampler; // TexUnit0
uniform sampler2DRect ptsSampler; // TexUnit1
uniform int   pointTexDim;
uniform vec3  psfOrigin;
uniform vec3  psfScale;
uniform vec3  psfMaxTexCoords;
uniform float focalDepth;
uniform float gain;
uniform int   startIndex;
uniform int   endIndex;

varying vec4  screenPosition;

uniform float shearInX;
uniform float shearInY;

void main() {
   vec3 zero = vec3(0.0);
   vec3 sum = zero;
   vec3 err = zero;

   // Get pixel position in world space
   vec2 shear = vec2(shearInX*focalDepth, shearInY*focalDepth);
   vec3 samplePosition = vec3(screenPosition.xy - shear, focalDepth);

   // Initialize counter
   int index = startIndex;
   bool done = false;
   while (!done) {
      while (!done) {
         float div = float(index) / float(pointTexDim);
         vec2 ptTexCoord = vec2(fract(div) * float(pointTexDim), floor(div)) + 0.5;
         vec4 texPt = texture2DRect(ptsSampler, ptTexCoord);
         vec4 fluorophorePt = vec4(texPt.xyz, 1.0);
         float intensity = texPt.w;

         // Get PSF origin
         vec3 origin = (gl_TextureMatrix[1] * fluorophorePt).xyz + psfOrigin;

         // Get texture coordinate in PSF. If we are outside
         // the texture, we should get black.
         vec3 texCoord = (samplePosition - origin) * psfScale;

         // Lookup PSF value in texture
         if (all(greaterThanEqual(texCoord, zero)) && all(lessThanEqual(texCoord, psfMaxTexCoords))) {
            vec3 sampleValue = texture3D(psfSampler, texCoord).rgb * texPt.a;

            // Kahan summation
            vec3 y = sampleValue - err;
            vec3 t = sum + y;
            err = (t - sum) - y;
            sum = t;
         }

         index = index + 1;
         done = index >= endIndex;
      }
   }
   gl_FragColor.rgb = sum * gain * gl_Color.rgb;
   gl_FragColor.a = 1.0;
}
