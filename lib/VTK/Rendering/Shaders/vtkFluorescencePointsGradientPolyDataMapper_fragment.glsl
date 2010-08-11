// Every fragment computes the gradient for one point.
#extension GL_ARB_texture_rectangle : enable
#pragma optimize(on)

uniform sampler3D     psfSampler; // TexUnit0
uniform sampler2DRect ptsSampler; // TexUnit1
uniform sampler2DRect syntheticImageSampler;    // TexUnit2
uniform sampler3D     experimentalImageSampler; // TexUnit3
uniform int   pointTexDim;
uniform int   psfRowLength;
uniform vec3  psfOrigin;
uniform vec3  psfScale;
uniform vec3  psfSpacing;
uniform vec3  psfMaxTexCoords;
uniform vec3  expOrigin;
uniform vec3  expScale;
uniform vec3  expMaxTexCoords;
uniform vec3  synthOrigin;
uniform vec3  synthScale;
uniform vec3  synthMaxTexCoords;
uniform float focalDepth;
uniform float gain;
uniform int   startRow;
uniform int   endRow;

varying vec4 screenPosition;
uniform float shearInX;
uniform float shearInY;


void main() {
   vec3 sum = vec3(0.0, 0.0, 0.0);

   // Use the screen position as a lookup into the points texture
   vec2 shear = vec2(shearInX*focalDepth, shearInY*focalDepth);
   vec4 point = vec4(texture2DRect(ptsSampler, screenPosition.xy).rgb, 1.0);

   // Transform according to the modelview matrix, which is held in the
   // texture matrix for tex unit 1.
   point = (gl_TextureMatrix[1] * point);

   // 3-vectors for checking whether texture coordinate is valid
   vec3 zero = vec3(0.0);

   // Initialize counter
   int row = startRow;
   int col = 0;
   bool done = false;
   while (!done) {
      while (!done) {

         // Sample at the voxel centers of the PSF. psfOrigin here is defined at the bottom corner of
         // voxel (0, 0, 0).
         vec2 samplePositionXY = (vec2(float(col), float(row))+0.5) * psfSpacing.xy + psfOrigin.xy + point.xy;
         vec3 samplePosition = vec3(samplePositionXY - shear, focalDepth);

         // Now that we have the sample position, find texture coordinates
         // for each of the PSF, synthetic, and experimental images.
         vec3 psfTexCoord  = (samplePosition - (psfOrigin + point.xyz)) * psfScale;
         vec3 synthTexCoord = (samplePosition - synthOrigin)  * synthScale;
         vec3 expTexCoord = (samplePosition - expOrigin) * expScale;

         // Ready to do operations for gradient

         // Restricted to images synthesized in the red channel
         bool computeSample = 
           all(greaterThanEqual(psfTexCoord, zero)) && 
           all(lessThanEqual(psfTexCoord, psfMaxTexCoords)) && 
           all(greaterThanEqual(synthTexCoord, zero)) && 
           all(lessThanEqual(synthTexCoord.xy, synthMaxTexCoords.xy)) && 
           all(greaterThanEqual(expTexCoord, zero)) && 
           all(lessThanEqual(expTexCoord, expMaxTexCoords));
         if (computeSample) {
           vec3 psfVal    = (texture3D(psfSampler, psfTexCoord)).rgb;
           float synthVal = (texture2DRect(syntheticImageSampler, synthTexCoord.rg)).r;
           float expVal   = (texture3D(experimentalImageSampler, expTexCoord)).r;
           sum += (synthVal - expVal) * psfVal;
         }

         col = col + 1;
         if (col >= psfRowLength) {
           row = row + 1;
           col = 0;
         }
         done = row >= endRow;
      }
   }
   gl_FragColor.rgb = sum;
   gl_FragColor.a = 1.0;
}
