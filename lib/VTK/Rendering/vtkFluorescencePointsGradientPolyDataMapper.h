/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePointsGradientPolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFluorescencePointsGradientPolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkFluorescencePointsGradientPolyDataMapper is meant to be used to 
// calculate the gradient of point positions in a point-sampled geometric
// model.

#ifndef __vtkFluorescencePointsGradientPolyDataMapper_h
#define __vtkFluorescencePointsGradientPolyDataMapper_h

#include "vtkFluorescencePolyDataMapper.h"

// Forward declarations
class vtkFluorescencePointsGradientRenderer;
class vtkFramebufferObjectTexture;
class vtkImageData;
class vtkRenderer;

class vtkFluorescencePointsGradientPolyDataMapper : public vtkFluorescencePolyDataMapper
{
public:
  static vtkFluorescencePointsGradientPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkFluorescencePointsGradientPolyDataMapper,vtkFluorescencePolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets/gets number of points to render per pass.
  vtkSetMacro(PixelsPerPass, int);
  vtkGetMacro(PixelsPerPass, int);

  // Description:
  // Sets/gets clearing of the texture storing the partially computed gradients
  vtkSetMacro(ClearTextureTarget, int);
  vtkGetMacro(ClearTextureTarget, int);
  vtkBooleanMacro(ClearTextureTarget, int);

  // Description:
  // Gets the points gradient calculated by this mapper
  float* GetPointsGradient(int& numPoints);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Loads required extensions for vtkFluorescencePointsGradientPolyDataMappers.
  int LoadExtensions(vtkRenderWindow *renWin);

  // Description:
  // Loads the shader program for the gather convolution algorithm
  int LoadShaderProgram(vtkRenderWindow *renWin);

  // Description:
  // Updates the zoom factor
  virtual void Update();
  
protected:
  vtkFluorescencePointsGradientPolyDataMapper();
  ~vtkFluorescencePointsGradientPolyDataMapper();

  // Description:
  // Renders points in the polygonal data
  void RenderPoints(vtkActor *actor, vtkRenderer* renderer);

  // Description:
  // Sends the modelview matrix to OpenGL
  void RenderMatrix(vtkMatrix4x4 *matrix);

  // Description:
  // Loads the point coordinate texture.
  void LoadPointTexture();

  // Description:
  // Deletes the point coordinate texture.
  void DeletePointTexture();

  // Description:
  // Binds the shader program and sets its uniform variables
  int  EnableShaderProgram(vtkFluorescencePointsGradientRenderer* renderer);
  void DisableShaderProgram();

  // Description:
  // Convenience methods for setting shader program uniform variables.
  void SetUniform1i(const char* name, int value);
  void SetUniform1f(const char* name, float value);
  void SetUniform3dv(const char* name, double* values);

  // Description:
  // Integer handle for texture storing point data from the mapped object.
  unsigned int PointTextureID;

  // Description:
  // Integer dimension of texture used to store point locations.
  int PointTextureDimension;

  // Description:
  // Texture target.
  int PointTextureTarget;

  // Description:
  // Shader program handle
  int ShaderProgramHandle;

  // Description:
  // Vertex and fragment shader handles
  int VertexProgramHandle;
  int FragmentProgramHandle;

  // A mapper computes the gradient for the points that it maps and stores
  // them in a texture bound to a framebuffer object
  vtkFramebufferObjectTexture* TextureTarget;

private:
  vtkFluorescencePointsGradientPolyDataMapper(const vtkFluorescencePointsGradientPolyDataMapper&);  // Not implemented.
  void operator=(const vtkFluorescencePointsGradientPolyDataMapper&);  // Not implemented.

  // Description:
  // Flag indicating whether the extensions have been loaded yet.
  int ExtensionsLoaded;

  // Description:
  // Keeps track of last time points were modified.
  int PtsLastTimePointsModified;

  // Description:
  // Max number of pixels to render per shader program invocation.
  // On older hardware, this number needs to be kept fairly low to 
  // avoid overrunning instruction limits in executed
  // shader programs.
  int PixelsPerPass;

  // Description:
  // Tells whether the mapper should clear (set to 0) the colors in the
  // texture target prior to rendering.
  int ClearTextureTarget;

};

#endif
