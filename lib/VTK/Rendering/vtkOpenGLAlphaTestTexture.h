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
#ifndef __vtkOpenGLAlphaTestTexture_h
#define __vtkOpenGLAlphaTestTexture_h



#ifndef VTK_IMPLEMENT_MESA_CXX
#include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

#include "vtkOpenGLTexture.h"

class vtkOpenGLAlphaTestTexture : public vtkOpenGLTexture
{
public:
  static vtkOpenGLAlphaTestTexture *New();
  vtkTypeMacro(vtkOpenGLAlphaTestTexture,vtkOpenGLTexture);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);
  
protected:
  vtkOpenGLAlphaTestTexture();
  ~vtkOpenGLAlphaTestTexture();

private:
  vtkOpenGLAlphaTestTexture(const vtkOpenGLAlphaTestTexture&);  // Not implemented.
  void operator=(const vtkOpenGLAlphaTestTexture&);  // Not implemented.
};

#endif
