#extension GL_ARB_texture_rectangle : enable
#pragma optimize(on)

uniform sampler3D     psfSampler; // TexUnit0
uniform sampler2DRect ptsSampler; // TexUnit1
uniform int startIndex;
uniform int endIndex;
uniform int pointTexDim;
uniform vec3 psfShift;
uniform vec3 psfScale;
uniform float focalDepth;
uniform float exposure;

varying vec4 screenPosition;

void main() {
   vec3 sum = vec3(0.0, 0.0, 0.0);
  
   // Get pixel position in world space
   vec3 pixelPos = vec3(screenPosition.xy, focalDepth);
   
   // 3-vectors for checking whether texture coordinate is valid
   vec3 zero = vec3(0.0);
   vec3 one  = vec3(1.0);
  
   // Initialize counter
   int index = startIndex;
   bool done = false;
   while (!done) {
      while (!done) {
         float div = float(index) / float(pointTexDim);
         vec2 ptTexCoord = vec2(fract(div) * float(pointTexDim), floor(div));
         vec4 texPt = texture2DRect(ptsSampler, ptTexCoord);
      
         // Get PSF origin
         vec3 origin = (gl_TextureMatrix[1] * texPt).xyz + psfShift;
         
         // Get texture coordinate in PSF. If we are outside
         // the texture, we should get black.
         vec3 texCoord = (pixelPos - origin) * psfScale;

         // Lookup PSF value in texture
         if (all(greaterThanEqual(texCoord, zero)) && all(lessThanEqual(texCoord, one))) {
            vec3 sampleValue = texture3D(psfSampler, texCoord).rgb;
            sum += sampleValue;
         }
            
         index = index + 1;
         done = index >= endIndex;
      }
   }
   gl_FragColor.rgb = sum * exposure * gl_Color.rgb;
   gl_FragColor.a = 1.0;
}
