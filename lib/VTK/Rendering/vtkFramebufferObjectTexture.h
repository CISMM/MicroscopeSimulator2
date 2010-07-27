/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFramebufferObjectTexture.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFramebufferObjectTexture - OpenGL texture map whose data comes from a frame buffer
// .SECTION Description
// vtkFramebufferObjectTexture is a subclass of vtkOpenGLTexture. Instead of setting the texture's
// input as the texture data source, an instance of this class is passed to vtkFramebufferObjectRenderer
// to store the result of rendering.

#ifndef __vtkFramebufferObjectTexture_h
#define __vtkFramebufferObjectTexture_h

#include "vtkOpenGLTexture.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkWindow;
class vtkOpenGLRenderer;
class vtkUnsignedShortArray;
class vtkImageData;

class VTK_RENDERING_EXPORT vtkFramebufferObjectTexture : public vtkOpenGLTexture
{
public:
  static vtkFramebufferObjectTexture *New();
  vtkTypeRevisionMacro(vtkFramebufferObjectTexture, vtkOpenGLTexture);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Gets/sets whether texture dimensions are automatically set to match
  // the size of the renderer
  vtkGetMacro(AutomaticDimensions, int);
  vtkSetMacro(AutomaticDimensions, int);
  vtkBooleanMacro(AutomaticDimensions, int);

  // Description:
  // Gets dimensions of texture.
  vtkGetMacro(TextureWidth, int);
  void SetTextureWidth(int width);
  vtkGetMacro(TextureHeight, int);
  void SetTextureHeight(int height);

  // Description:
  // Sets texture format to RGBA or luminance. RGBA is default.
  void SetTextureFormatToLuminance();
  void SetTextureFormatToRGBA();
  
  // Description:
  // Gets maximum texture coordinates of framebuffer texture.
  vtkGetMacro(MaxCoordS, int);
  vtkGetMacro(MaxCoordT, int);
  
  // Description:
  // Load necessary OpenGL extensions. Returns 1 if successful, false otherwise.
  int LoadExtensions(void);

  // Description:
  // Setup/teardown of texture as a rendering target.
  void EnableTarget(vtkRenderer *renderer);
  void DisableTarget();

  // Description:
  // Sets the renderwindow associated with this texture
  void SetRenderWindow(vtkRenderWindow* renWin) {
    this->RenderWindow = renWin;
  }

  // Description:
  // Overrides base class method because the base class checks for non-null vtkImageData
  // input. We don't need to check because we are pulling from a frame buffer object
  // instead.
  void Render(vtkRenderer *ren);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);
  
  // Description:
  // Called after textured objects have been rendered.
  void PostRender();
  
  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release. Using the same texture object in multiple
  // render windows is NOT currently supported. 
  void ReleaseGraphicsResources(vtkWindow *);

  virtual int RequestInformation (vtkInformation *, vtkInformationVector **, 
                                  vtkInformationVector *);

protected:
  vtkFramebufferObjectTexture();
  ~vtkFramebufferObjectTexture();

  // Description:
  // Indicates whether texture dimensions are automatically set to the
  // dimensions of the renderer.
  int AutomaticDimensions;

  // Description:
  // Last known width and height of the render window.
  int TextureWidth;
  int TextureHeight;

  // Description:
  // Time when texture width and height were last modified
  vtkTimeStamp DimensionTime;

  // Description:
  // Frame buffer
  GLuint FrameBufferHandle;
  
  // Description:
  // Render buffer
  GLuint RenderBufferHandle;

  // Description:
  // Texture target (e.g., GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_NV, etc.)
  GLenum TextureTarget;

  // Description:
  // ID of the texture bound to the frame buffer object.
  GLuint Index;

  // Description:
  // Indicates number of channels in texture target. The value
  // of this variable affects the value stored in
  // TextureInternalFormat.
  GLenum TextureFormat;

  // Description:
  // Texture internal format
  GLenum TextureInternalFormat;

  // Description:
  // Filter mode.
  GLenum FilterMode;

  // Description:
  // Maximum s- and t-coordinate values.
  int MaxCoordS;
  int MaxCoordT;

  //virtual void ExecuteData(vtkDataObject *d);
  virtual int RequestData(vtkInformation* request, 
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkFramebufferObjectTexture(const vtkFramebufferObjectTexture&);  // Not implemented.
  void operator=(const vtkFramebufferObjectTexture&);  // Not implemented.
  
  void CheckFrameBufferStatus(void);
  
  void Clean(void);
};

#endif
