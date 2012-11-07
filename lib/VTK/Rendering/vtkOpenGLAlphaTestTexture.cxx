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
#include "vtkOpenGLAlphaTestTexture.h"

#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderWindow.h"

#include "vtkOpenGL.h"

#include <math.h>

vtkStandardNewMacro(vtkOpenGLAlphaTestTexture);

// Initializes an instance, generates a unique index.
vtkOpenGLAlphaTestTexture::vtkOpenGLAlphaTestTexture() : vtkOpenGLTexture()
{
}

vtkOpenGLAlphaTestTexture::~vtkOpenGLAlphaTestTexture()
{
}

// Implement base class method.
void vtkOpenGLAlphaTestTexture::Load(vtkRenderer *ren)
{
  GLenum format = GL_LUMINANCE;
  vtkImageData *input = this->GetInput();
  
  // need to reload the texture
  if (this->GetMTime() > this->LoadTime.GetMTime() ||
      input->GetMTime() > this->LoadTime.GetMTime() ||
      (this->GetLookupTable() && this->GetLookupTable()->GetMTime () >  
       this->LoadTime.GetMTime()) || 
       ren->GetRenderWindow() != this->RenderWindow ||
       ren->GetRenderWindow()->GetMTime() > this->LoadTime.GetMTime())
    {
    int bytesPerPixel;
    int *size;
    vtkDataArray *scalars;
    unsigned char *dataPtr;
    int rowLength;
    unsigned char *resultData=NULL;
    int xsize, ysize;
    unsigned short xs,ys;
    GLuint tempIndex=0;

    // get some info
    size = input->GetDimensions();
    scalars = input->GetPointData()->GetScalars();

    // make sure scalars are non null
    if (!scalars) 
      {
      vtkErrorMacro(<< "No scalar values found for texture input!");
      return;
      }

    bytesPerPixel = scalars->GetNumberOfComponents();

    // make sure using unsigned char data of color scalars type
    if (this->MapColorScalarsThroughLookupTable ||
       scalars->GetDataType() != VTK_UNSIGNED_CHAR )
      {
      dataPtr = this->MapScalarsToColors (scalars);
      bytesPerPixel = 4;
      }
    else
      {
      dataPtr = static_cast<vtkUnsignedCharArray *>(scalars)->GetPointer(0);
      }

    // we only support 2d texture maps right now
    // so one of the three sizes must be 1, but it 
    // could be any of them, so lets find it
    if (size[0] == 1)
      {
      xsize = size[1]; ysize = size[2];
      }
    else
      {
      xsize = size[0];
      if (size[1] == 1)
        {
        ysize = size[2];
        }
      else
        {
        ysize = size[1];
        if (size[2] != 1)
          {
          vtkErrorMacro(<< "3D texture maps currently are not supported!");
          return;
          }
        }
      }

    // xsize and ysize must be a power of 2 in OpenGL
    xs = (unsigned short)xsize;
    ys = (unsigned short)ysize;
    while (!(xs & 0x01))
      {
      xs = xs >> 1;
      }
    while (!(ys & 0x01))
      {
      ys = ys >> 1;
      }

    // -- decide whether the texture needs to be resampled --
    int resampleNeeded = 0;
    // if not a power of two then resampling is required
    if ((xs > 1)||(ys > 1))
      {
      resampleNeeded = 1;
      }
    GLint maxDimGL;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDimGL);
    // if larger than permitted by the graphics library then must resample
    if ( xsize > maxDimGL || ysize > maxDimGL )
      {
      vtkDebugMacro( "Texture too big for gl, maximum is " << maxDimGL);
      resampleNeeded = 1;
      }

    if ( resampleNeeded )
      {
      vtkDebugMacro(<< "Resampling texture to power of two for OpenGL");
      resultData = this->ResampleToPowerOfTwo(xsize, ysize, dataPtr, 
                                              bytesPerPixel);
      }

    // format the data so that it can be sent to opengl
    // each row must be a multiple of 4 bytes in length
    // the best idea is to make your size a multiple of 4
    // so that this conversion will never be done.
    rowLength = ((xsize*bytesPerPixel +3 )/4)*4;
    if (rowLength == xsize*bytesPerPixel)
      {
      if ( resultData == NULL )
        {
        resultData = dataPtr;
        }
      }
    else
      {
      int col;
      unsigned char *src,*dest;
      int srcLength;

      srcLength = xsize*bytesPerPixel;
      resultData = new unsigned char [rowLength*ysize];
      
      src = dataPtr;
      dest = resultData;

      for (col = 0; col < ysize; col++)
        {
        memcpy(dest,src,srcLength);
        src += srcLength;
        dest += rowLength;
        }
      }

    // free any old display lists (from the old context)
    if (this->RenderWindow)
      {
      this->ReleaseGraphicsResources(this->RenderWindow);
      }
    
     this->RenderWindow = ren->GetRenderWindow();
     
    // make the new context current before we mess with opengl
    this->RenderWindow->MakeCurrent();
 
    // define a display list for this texture
    // get a unique display list id
#ifdef GL_VERSION_1_1
    glGenTextures(1, &tempIndex);
    this->Index = (long) tempIndex;
    glBindTexture(GL_TEXTURE_2D, this->Index);
#else
    this->Index = glGenLists(1);
    glDeleteLists ((GLuint) this->Index, (GLsizei) 0);
    glNewList ((GLuint) this->Index, GL_COMPILE);
#endif

    ((vtkOpenGLRenderWindow *)(ren->GetRenderWindow()))->RegisterTextureResource( this->Index );
    
    if (this->Interpolate)
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       GL_LINEAR);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       GL_LINEAR );
      }
    else
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      }
    if (this->Repeat)
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT );
      }
    else
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP );
      }
    int internalFormat = bytesPerPixel;
    switch (bytesPerPixel)
      {
      case 1: format = GL_LUMINANCE; break;
      case 2: format = GL_LUMINANCE_ALPHA; break;
      case 3: format = GL_RGB; break;
      case 4: format = GL_RGBA; break;
      }
    // if we are using OpenGL 1.1, you can force 32 or16 bit textures
#ifdef GL_VERSION_1_1
    if (this->Quality == VTK_TEXTURE_QUALITY_32BIT)
      {
      switch (bytesPerPixel)
        {
        case 1: internalFormat = GL_LUMINANCE8; break;
        case 2: internalFormat = GL_LUMINANCE8_ALPHA8; break;
        case 3: internalFormat = GL_RGB8; break;
        case 4: internalFormat = GL_RGBA8; break;
        }
      }
    else if (this->Quality == VTK_TEXTURE_QUALITY_16BIT)
      {
      switch (bytesPerPixel)
        {
        case 1: internalFormat = GL_LUMINANCE4; break;
        case 2: internalFormat = GL_LUMINANCE4_ALPHA4; break;
        case 3: internalFormat = GL_RGB4; break;
        case 4: internalFormat = GL_RGBA4; break;
        }
      }
#endif
    glTexImage2D( GL_TEXTURE_2D, 0 , internalFormat,
                  xsize, ysize, 0, format, 
                  GL_UNSIGNED_BYTE, (const GLvoid *)resultData );
#ifndef GL_VERSION_1_1
    glEndList ();
#endif
    // modify the load time to the current time
    this->LoadTime.Modified();
    
    // free memory
    if (resultData != dataPtr)
      {
      delete [] resultData;
      }
    }

  // execute the display list that uses creates the texture
#ifdef GL_VERSION_1_1
  glBindTexture(GL_TEXTURE_2D, this->Index);
#else
  glCallList ((GLuint) this->Index);
#endif
  
  // don't accept fragments if they have zero opacity. this will stop the
  // zbuffer from be blocked by totally transparent texture fragments.

  // actually, I want that behavior in some cases. ok, allow the VTK default behavior
  // if the alpha test function hasn't been changed from the VTK default of GL_GREATER,
  // otherwise, assume it was changed on purpose and leave it that way.
  int func;
  float ref;
  glGetIntegerv(GL_ALPHA_TEST_FUNC, &func);
  glGetFloatv(GL_ALPHA_TEST_REF, &ref);
  if (func == GL_ALWAYS)
     glAlphaFunc (GL_GREATER, (GLclampf)0);
  glEnable (GL_ALPHA_TEST);

  // now bind it 
  glEnable(GL_TEXTURE_2D);
}
