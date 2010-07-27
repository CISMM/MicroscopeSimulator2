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
#include "vtkOpenGLShadowRenderer.h"

#include "vtkCuller.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkOpenGLProperty.h"
#include "vtkRenderWindow.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"

#include "vtkOpenGLExtensionManager.h"
#include "vtkgl.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
#include "vtkOpenGL.h"
#endif

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkOpenGLShadowRenderer, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkOpenGLShadowRenderer);
#endif

#define VTK_MAX_LIGHTS 8

// this class is pretty standard shadowmap stuff.

void CheckFBO()
{
   GLenum status = vtkgl::CheckFramebufferStatusEXT(vtkgl::FRAMEBUFFER_EXT);
#define LogDebug(X) std::cerr << "FBO info: " << X << std::endl;
#define LogError(X) std::cerr << "FBO error: " << X << std::endl;
   switch(status)
   {
		case vtkgl::FRAMEBUFFER_COMPLETE_EXT:
			LogDebug("Framebuffer complete");
			break;
		case vtkgl::FRAMEBUFFER_UNSUPPORTED_EXT:
			LogError("Unsupported framebuffer format");
			break;
		case vtkgl::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			LogError("Framebuffer incomplete, missing attachment");
			break;
		case vtkgl::FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			LogError("Framebuffer incomplete, attached images must have same dimensions");
			break;
		case vtkgl::FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			LogError("Framebuffer incomplete, attached images must have same format");
			break;
		case vtkgl::FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			LogError("Framebuffer incomplete, missing draw buffer");
			break;
		case vtkgl::FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			LogError("Framebuffer incomplete, missing read buffer");
			break;
		default:
			LogError("Unknown framebuffer status!");
			break;
   }
}

const GLubyte* gluErrorString( GLenum errorCode )
{
   static char tess_error[][128] = {
      "missing gluBeginPolygon",
      "missing gluBeginContour",
      "missing gluEndPolygon",
      "missing gluEndContour",
      "misoriented or self-intersecting loops",
      "coincident vertices",
      "colinear vertices",
      "FIST recovery process fatal error"
   };
   static char nurbs_error[][128] = {
      "spline order un-supported",
      "too few knots",
      "valid knot range is empty",
      "decreasing knot sequence knot",
      "knot multiplicity greater than order of spline",
      "endcurve() must follow bgncurve()",
      "bgncurve() must precede endcurve()",
      "missing or extra geometric data",
      "can't draw pwlcurves",
      "missing bgncurve()",
      "missing bgnsurface()",
      "endtrim() must precede endsurface()",
      "bgnsurface() must precede endsurface()",
      "curve of improper type passed as trim curve",
      "bgnsurface() must precede bgntrim()",
      "endtrim() must follow bgntrim()",
      "bgntrim() must precede endtrim()",
      "invalid or missing trim curve",
      "bgntrim() must precede pwlcurve()",
      "pwlcurve referenced twice",
      "pwlcurve and nurbscurve mixed",
      "improper usage of trim data type",
      "nurbscurve referenced twice",
      "nurbscurve and pwlcurve mixed",
      "nurbssurface referenced twice",
      "invalid property",
      "endsurface() must follow bgnsurface()",
      "misoriented trim curves",
      "intersecting trim curves",
      "UNUSED",
      "unconnected trim curves",
      "unknown knot error",
      "negative vertex count encountered",
      "negative byte-stride encountered",
      "unknown type descriptor",
      "null control array or knot vector",
      "duplicate point on pwlcurve"
   };

   /* GL Errors */
   if (errorCode==GL_NO_ERROR) {
      return (GLubyte *) "no error";
   }
   else if (errorCode==GL_INVALID_VALUE) {
      return (GLubyte *) "invalid value";
   }
   else if (errorCode==GL_INVALID_ENUM) {
      return (GLubyte *) "invalid enum";
   }
   else if (errorCode==GL_INVALID_OPERATION) {
      return (GLubyte *) "invalid operation";
   }
   else if (errorCode==GL_STACK_OVERFLOW) {
      return (GLubyte *) "stack overflow";
   }
   else if (errorCode==GL_STACK_UNDERFLOW) {
      return (GLubyte *) "stack underflow";
   }
   else if (errorCode==GL_OUT_OF_MEMORY) {
      return (GLubyte *) "out of memory";
   }
   else {
      return (GLubyte *) "unspecified";
   }
}
void CheckOGL(const char* text = 0)
{
   GLenum errCode;
   const GLubyte *errStr;

   if ((errCode = glGetError()) != GL_NO_ERROR) 
   {
      errStr = gluErrorString(errCode);
      if (text) std::cerr << text << " : ";
      std::cerr << "OGL Error: " << errStr << std::endl;
   }
}

vtkOpenGLShadowRenderer::vtkOpenGLShadowRenderer() : vtkOpenGLRenderer()
{
   this->OpenGLInitialized = 0;
   this->ShadowInitialized = 0;

   this->CastShadows = 0;

   this->ShadowFBO = 0;
   this->ShadowTex = 0;
   this->ShadowSize = 750;
   // 750 is totally arbitrary, works for my development app, and should actually
   // change to match the renderer
}

void vtkOpenGLShadowRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os,indent);
}

vtkOpenGLShadowRenderer::~vtkOpenGLShadowRenderer()
{
   if (this->ShadowFBO) vtkgl::DeleteFramebuffersEXT(1, &this->ShadowFBO);
   if (this->ShadowTex) glDeleteTextures(1, &this->ShadowTex);
}

void vtkOpenGLShadowRenderer::PushLightScaleBiasMatrix()
{
   glScalef(this->ShadowSize, this->ShadowSize, 1.);
   glTranslatef(.5, .5, .5);
   glScalef(.5, .5, .5);
}

void vtkOpenGLShadowRenderer::PushLightProjectionMatrix()
{
   double bounds[6];
   this->ComputeVisiblePropBounds(bounds);
   double box[3] = {bounds[1]-bounds[0], bounds[3]-bounds[2], bounds[5]-bounds[4]};
   double l = vtkMath::Norm(box) * .5;
   glOrtho(-l, l, -l, l, 1, 4.*l);
}

void vtkOpenGLShadowRenderer::PushLightViewMatrix()
{
   double bounds[6];
   this->ComputeVisiblePropBounds(bounds);
   double box[3] = {bounds[1]-bounds[0], bounds[3]-bounds[2], bounds[5]-bounds[4]};
   double l = vtkMath::Norm(box);

   vtkCollectionSimpleIterator sit;
   this->Lights->InitTraversal(sit);
   vtkLight *light = this->Lights->GetNextLight(sit);

   double pos[3], foc[3], up[3];

   light->GetTransformedPosition(pos);
   light->GetTransformedFocalPoint(foc);
   this->ActiveCamera->GetViewUp(up);

   GLdouble m[16];
   GLdouble side[3], dir[3];

   dir[0] = foc[0] - pos[0];
   dir[1] = foc[1] - pos[1];
   dir[2] = foc[2] - pos[2];
   vtkMath::Normalize(dir);

   foc[0] = (bounds[0]+bounds[1]) * .5;
   foc[1] = (bounds[2]+bounds[3]) * .5;
   foc[2] = (bounds[4]+bounds[5]) * .5;

   pos[0] = foc[0] - l * dir[0];
   pos[1] = foc[1] - l * dir[1];
   pos[2] = foc[2] - l * dir[2];

   vtkMath::Cross(dir, up, side);
   vtkMath::Normalize(side);

   vtkMath::Cross(side, dir, up);
   vtkMath::Normalize(up);

#define M(row,col)  m[col*4+row]
   M(0,0) = side[0];  M(0,1) = side[1];  M(0,2) = side[2];  M(0,3) = 0.0;
   M(1,0) =   up[0];  M(1,1) =   up[1];  M(1,2) =   up[2];  M(1,3) = 0.0;
   M(2,0) = -dir[0];  M(2,1) = -dir[1];  M(2,2) = -dir[2];  M(2,3) = 0.0;
   M(3,0) =     0.0;  M(3,1) =     0.0;  M(3,2) =     0.0;  M(3,3) = 1.0;
#undef M

   glMultMatrixd(m);
   glTranslated(-pos[0], -pos[1], -pos[2]);
}

void vtkOpenGLShadowRenderer::TwoPassRender()
{
  CheckOGL("vtk scene pre-init");
   this->ClearLights();
   this->UpdateCamera();
   this->UpdateLightGeometry();
   this->UpdateLights();
   CheckOGL("vtk scene post-init");

   glMatrixMode(GL_MODELVIEW);
   double t[16], m[16];
   glGetDoublev(GL_MODELVIEW_MATRIX, t); CheckOGL("get modelview");
   vtkMatrix4x4::Invert(t, m);

   vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, this->ShadowFBO); CheckOGL("bind depth FBO");
   glClearDepth(1.); CheckOGL("set clear depth");
   glClear(GL_DEPTH_BUFFER_BIT); CheckOGL("depth clear");
   vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, 0); CheckOGL("bind window");
   CheckOGL("clear depth fbo");

   if (this->CastShadows)
   {
      vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, this->ShadowFBO);

      glPushAttrib(GL_VIEWPORT_BIT); // save the viewport
      glViewport(0, 0, this->ShadowSize, this->ShadowSize); CheckOGL("push viewport");

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      this->PushLightProjectionMatrix();

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      this->PushLightViewMatrix();

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2., 1.1);

      // set matrix mode for actors 
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();

      vtkgl::ActiveTextureARB(vtkgl::TEXTURE0);

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      int func;
      float ref;
      glGetIntegerv(GL_ALPHA_TEST_FUNC, &func);
      glGetFloatv(GL_ALPHA_TEST_REF, &ref);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, .5); CheckOGL("before update");
      this->UpdateGeometry();
      glDisable(GL_ALPHA_TEST); CheckOGL("after update");
      glAlphaFunc(func, ref);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();

      glDisable(GL_POLYGON_OFFSET_FILL);
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

      glPopAttrib(); CheckOGL("pop viewport");

      vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, 0);
   }

  /* vtkgl::ActiveTextureARB(vtkgl::TEXTURE1); CheckOGL("texture1 active");
   glEnable(vtkgl::TEXTURE_RECTANGLE_ARB);
   glBindTexture(vtkgl::TEXTURE_RECTANGLE_ARB, this->ShadowTex); CheckOGL("bind shadow");

   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   this->PushLightScaleBiasMatrix();
   this->PushLightProjectionMatrix();
   this->PushLightViewMatrix();
   glMultMatrixd(m); CheckOGL("setup texture matrix");

   vtkgl::ActiveTextureARB(vtkgl::TEXTURE0); CheckOGL("texture0 active");*/

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   this->UpdateGeometry();
   glDisable(GL_BLEND);
   CheckOGL("draw geometry");

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glPopMatrix(); // pushed by UpdateCamera();

   //vtkgl::ActiveTextureARB(vtkgl::TEXTURE1);
   //glMatrixMode(GL_TEXTURE);
   //glPopMatrix();
   //glDisable(vtkgl::TEXTURE_RECTANGLE_ARB);
   //vtkgl::ActiveTextureARB(vtkgl::TEXTURE0); CheckOGL("texture0 active");
}

void vtkOpenGLShadowRenderer::InitializeOpenGL()
{
   vtkOpenGLExtensionManager *extensions = vtkOpenGLExtensionManager::New();
   extensions->SetRenderWindow(this->RenderWindow);

   if (!extensions->ExtensionSupported("GL_VERSION_2_0"))
   {
      vtkErrorMacro("OpenGL version 1.5 not supported (GL_VERSION_1_5)!");
   }
   extensions->LoadExtension("GL_VERSION_2_0");

   if (!extensions->ExtensionSupported("GL_ARB_multitexture"))
   {
      vtkErrorMacro("Multitexture not supported (GL_ARB_multitexture)!");
   }
   extensions->LoadExtension("GL_ARB_multitexture");

   if (!extensions->ExtensionSupported("GL_ARB_depth_texture"))
   {
      vtkErrorMacro("Depth textures not supported (GL_ARB_depth_texture)!");
   }
   extensions->LoadExtension("GL_ARB_depth_texture");

   if (!extensions->ExtensionSupported("GL_ARB_shadow"))
   {
      vtkErrorMacro("Shadows not supported (GL_ARB_shadow)!");
   }
   extensions->LoadExtension("GL_ARB_shadow");

   if (!extensions->ExtensionSupported("GL_EXT_framebuffer_object"))
   {
      vtkErrorMacro("FrameBuffer Objects not supported (GL_EXT_framebuffer_object)!");
   }
   extensions->LoadExtension("GL_EXT_framebuffer_object");

   if (!extensions->ExtensionSupported("GL_ARB_texture_rectangle"))
   {
      vtkErrorMacro("Rectangle textures not supported (GL_ARB_texture_rectangle)!");
   }
   extensions->LoadExtension("GL_ARB_texture_rectangle");
   // NOTES: vtk fiddles with GL_TEXTURE_2D and the active texture in many classes,
   // preventing easy use of that texture state for shadows w/o changing those classes.
   // so I'm using the texture_rectangle state. problem ... ati didn't implement this in
   // their drivers, so this is nvidia only. a little searching on the web suggests ati
   // never intended to implement it, but maybe intel will overrule them one day?

   extensions->Delete();

   OpenGLInitialized = 1;
}

void vtkOpenGLShadowRenderer::InitializeShadow()
{
   // create the texture we'll use for the shadow map
   glGenTextures(1, &this->ShadowTex);
   CheckOGL();
   glEnable(vtkgl::TEXTURE_RECTANGLE_ARB);
   
   vtkgl::ActiveTextureARB(vtkgl::TEXTURE1);   
   glBindTexture(vtkgl::TEXTURE_RECTANGLE_ARB, this->ShadowTex);
   CheckOGL();
   glTexImage2D(vtkgl::TEXTURE_RECTANGLE_ARB, 0, vtkgl::DEPTH_COMPONENT24, this->ShadowSize, this->ShadowSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
   CheckOGL("create depth texture");
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, vtkgl::TEXTURE_COMPARE_MODE, vtkgl::COMPARE_R_TO_TEXTURE);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, vtkgl::TEXTURE_COMPARE_FUNC, GL_LEQUAL);
   glTexParameteri(vtkgl::TEXTURE_RECTANGLE_ARB, vtkgl::DEPTH_TEXTURE_MODE, GL_INTENSITY);
   CheckOGL("parameters");

    // create fbo and bind it
   vtkgl::GenFramebuffersEXT(1, &this->ShadowFBO);
   vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, this->ShadowFBO);
   CheckFBO();

   // attach texture to framebuffer
   vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT, vtkgl::DEPTH_ATTACHMENT_EXT, vtkgl::TEXTURE_RECTANGLE_ARB, this->ShadowTex, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   CheckOGL(); CheckFBO();

   // verify all is well and restore state
   vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, 0);
   CheckFBO();

   vtkgl::ActiveTextureARB(vtkgl::TEXTURE0);

   ShadowInitialized = 1;
}

// Concrete open gl render method.
void vtkOpenGLShadowRenderer::DeviceRender()
{
   // Do not remove this MakeCurrent! Due to Start / End methods on
   // some objects which get executed during a pipeline update, 
   // other windows might get rendered since the last time
   // a MakeCurrent was called.
   this->RenderWindow->MakeCurrent();

   if (!OpenGLInitialized) InitializeOpenGL();
   if (OpenGLInitialized && !ShadowInitialized) InitializeShadow();

   TwoPassRender();
}
