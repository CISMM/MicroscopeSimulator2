/*=========================================================================

  Written by: Chris Weigle, University of North Carolina at Chapel Hill
  Email Contact: weigle@cs.unc.edu

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE
  OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF NORTH
  CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
  ON AN "AS IS" BASIS, AND THE UNIVERSITY OF NORTH CAROLINA HAS NO
  OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.

=========================================================================*/
#ifndef __vtkOpenGLShadowRenderer_h
#define __vtkOpenGLShadowRenderer_h

#ifndef VTK_IMPLEMENT_MESA_CXX
#include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

#include "vtkOpenGLRenderer.h"


class vtkOpenGLShadowRenderer : public vtkOpenGLRenderer
{
protected:
   int NumberOfLightsBound;

public:
   static vtkOpenGLShadowRenderer *New();
   vtkTypeRevisionMacro(vtkOpenGLShadowRenderer,vtkOpenGLRenderer);
   void PrintSelf(ostream& os, vtkIndent indent);

   // Description:
   // Concrete open gl render method.
   void DeviceRender(void);

   vtkSetMacro(CastShadows,int);
   vtkGetMacro(CastShadows,int);
   vtkBooleanMacro(CastShadows,int);

protected:
   vtkOpenGLShadowRenderer();
   ~vtkOpenGLShadowRenderer();

   //BTX
   void PushLightScaleBiasMatrix();
   void PushLightProjectionMatrix();
   void PushLightViewMatrix();

   void InitializeOpenGL();
   void InitializeShadow();

   void TwoPassRender();
   int OverlayPass;

   GLuint ShadowSize;
   GLuint ShadowFBO;
   GLuint ShadowTex;

   int ShadowInitialized;
   int OpenGLInitialized;
   //ETX

   int CastShadows;

private:
   vtkOpenGLShadowRenderer(const vtkOpenGLShadowRenderer&);  // Not implemented.
   void operator=(const vtkOpenGLShadowRenderer&);           // Not implemented.
};

#endif
