/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkGatherFluorescencePolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGatherFluorescencePolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkGatherFluorescencePolyDataMapper is a subclass of vtkOpenGLPolyDataMapper.
// vtkGatherFluorescencePolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkGatherFluorescencePolyDataMapper_h
#define __vtkGatherFluorescencePolyDataMapper_h

#include <math.h>

#include "vtkImageData.h"
#include "vtkFluorescencePolyDataMapper.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkGatherFluorescencePolyDataMapper : public vtkFluorescencePolyDataMapper
{
public:
  static vtkGatherFluorescencePolyDataMapper *New();
  vtkTypeRevisionMacro(vtkGatherFluorescencePolyDataMapper,vtkFluorescencePolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets/gets number of points to render per pass.
  vtkSetMacro(PointsPerPass, int);
  vtkGetMacro(PointsPerPass, int);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Loads required extensions for vtkGatherFluorescencePolyDataMappers.
  int LoadExtensions(vtkRenderWindow *renWin);

  // Description:
  // Loads the shader program for the gather convolution algorithm
  int LoadShaderProgram(vtkRenderWindow *renWin);

  // Description:
  // Updates the zoom factor
  virtual void Update();
  
protected:
  vtkGatherFluorescencePolyDataMapper();
  ~vtkGatherFluorescencePolyDataMapper();

  // Description:
  // Renders points in the polygonal data
  void RenderPoints(vtkActor *actor, vtkRenderer* renderer);

  // Description:
  // Sends the modelview matrix to OpenGL
  void RenderMatrix(vtkMatrix4x4 *matrix);

  // Description:
  // Returns the focal plane depth transformed according to the scale and position
  // of the actor passed in as an argument.
  double GetTransformedFocalPlaneDepth(vtkActor *actor);

  // Description:
  // Loads the point coordinate texture.
  void LoadPointTexture();

  // Description:
  // Deletes the point coordinate texture.
  void DeletePointTexture();

  // Description:
  // Compute quad that bounds the actor points with padding added for PSF. Output parameter
  // quad contains quadruple of screen bounds {xmin, xmax, ymin, ymax}.
  void ComputeBoundingQuad(double bounds[6], vtkMatrix4x4 *matrix, double xPad, double yPad, double quad[4]);

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

private:
  vtkGatherFluorescencePolyDataMapper(const vtkGatherFluorescencePolyDataMapper&);  // Not implemented.
  void operator=(const vtkGatherFluorescencePolyDataMapper&);  // Not implemented.

  // Description:
  // Flag indicating whether the extensions have been loaded yet.
  int ExtensionsLoaded;

  // Description:
  // Keeps track of last time points were modified.
  int PtsLastTimePointsModified;

  // Description:
  // Number of points to render in a bunch. On older hardware, this number
  // needs to be kept fairly low to avoid overrunning instruction limits in executed
  // shader programs.
  int PointsPerPass;

};

#endif
