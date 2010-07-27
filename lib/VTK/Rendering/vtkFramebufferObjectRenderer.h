/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFramebufferObjectRenderer - OpenGL renderer
// .SECTION Description
// vtkFramebufferObjectRenderer is a subclass of vtkOpenGLRenderer that enables
// capabilities not necessarily found in the standard framebuffer, such as floating-point
// rendering.
//
// KNOWN BUGS
// - Transparent objects fail to appear in the rendered image.

#ifndef __vtkFramebufferObjectRenderer_h
#define __vtkFramebufferObjectRenderer_h

#include "vtkCollection.h"
#include "vtkOpenGLRenderer.h"
#include "vtkFramebufferObjectTexture.h"

#if TIME
#include "stopwatch.h"
#endif // TIME

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkFramebufferObjectRenderer : public vtkOpenGLRenderer
{

public:
  static vtkFramebufferObjectRenderer *New();
  vtkTypeRevisionMacro(vtkFramebufferObjectRenderer,vtkOpenGLRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Concrete open gl render method.
  void DeviceRender(void);

  // Description:
  // Get the number of framebuffer textures.
  int GetNumberOfFramebufferTextures();
  
  // Description:
  // Setter/getter for the default rendering target texture.
  void SetFramebufferTexture(vtkFramebufferObjectTexture *texture);
  vtkFramebufferObjectTexture * GetFramebufferTexture();

  // Description:
  // Get the rendering target texture with the given index.
  vtkFramebufferObjectTexture * GetFramebufferTexture(int index);

  // Description:
  // Gets the active rendering target texture.
  vtkFramebufferObjectTexture * GetActiveFramebufferTexture();

  // Description:
  // Add a rendering target texture.
  void AddFramebufferTexture(vtkFramebufferObjectTexture *texture);

  // Description:
  // Activates the framebuffer texture with index.
  void SetActiveFramebufferTexture(int index);
  
  // Description:
  // These methods can be used to define how values in the framebuffer
  // texture get mapped to values displayable on the screen.
  vtkSetMacro(MapsToZero, double);
  vtkGetMacro(MapsToZero, double);
  vtkSetMacro(MapsToOne, double);
  vtkGetMacro(MapsToOne, double);
  
  // Description:
  // Renders the frame buffer object texture to a rectangle in the displayed frame buffer.
  void TextureRender(void);
  void TextureRender(int index);
  
protected:
  vtkFramebufferObjectRenderer();
  ~vtkFramebufferObjectRenderer();

  // Description:
  // Value that will map to zero.
  double MapsToZero;
  
  // Description:
  // Value that will map to one.
  double MapsToOne;

  // Description:
  // Handle for program.
  GLuint ProgramHandle;

  // Description:
  // Handle for fragment shader.
  GLuint FragmentShaderHandle;

  // Description:
  // Collection of framebuffer object textures that can,
  // individually, serve as targets for the framebuffer renderer.
  vtkCollection *FramebufferTextureCollection;

  // Description:
  // Index of active framebuffer object texture.
  int ActiveFramebufferTextureIndex;

  // Description:
  // Checks the framebuffer status, reporting errors if they are encountered.
  void CheckFrameBufferStatus(void);
  
  // Description:
  // Loads fragment program for remapping texture values to displayable values.
  void LoadFragmentProgram(void);

  // Description:
  // Applies scale and bias to map rendered texture to display
  // range. The parameter index is the index of the
  // framebuffer object texture to render.
  void RemapAndDisplay();
  void RemapAndDisplay(int index);

#if TIME
  // Description:
  // Stopwatch handle
  unsigned int Stopwatch;
#endif // TIME

  // Description:
  // Resets, starts, and ends timer.
  void ResetTimer();
  void StartTimer();
  void StopTimer();
  void PrintTime();

private:
  vtkFramebufferObjectRenderer(const vtkFramebufferObjectRenderer&);  // Not implemented.
  void operator=(const vtkFramebufferObjectRenderer&);  // Not implemented.

};

#endif
