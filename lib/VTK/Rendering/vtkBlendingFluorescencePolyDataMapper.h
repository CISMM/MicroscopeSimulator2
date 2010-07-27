/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBlendingFluorescencePolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBlendingFluorescencePolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkBlendingFluorescencePolyDataMapper is a subclass of vtkOpenGLPolyDataMapper.
// vtkBlendingFluorescencePolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkBlendingFluorescencePolyDataMapper_h
#define __vtkBlendingFluorescencePolyDataMapper_h

#include <math.h>

#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLTexture.h"
#include "vtkFluorescencePolyDataMapper.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkBlendingFluorescencePolyDataMapper : public vtkFluorescencePolyDataMapper
{
public:
  static vtkBlendingFluorescencePolyDataMapper *New();
  vtkTypeRevisionMacro(vtkBlendingFluorescencePolyDataMapper,vtkFluorescencePolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Set the renderer used to render the fluorescence image
  vtkGetObjectMacro(Renderer, vtkRenderer);
  void SetRenderer(vtkRenderer *renderer);

  // Description:
  // Sets/gets the vtkImageData object representing the PSF.
  vtkGetObjectMacro(ImageData, vtkImageData);
  vtkSetObjectMacro(ImageData, vtkImageData);

  // Description:
  // Sets/gets the focal plane position (in nanometers);
  vtkGetMacro(FocalPlaneDepth, double);
  vtkSetMacro(FocalPlaneDepth, double);

  // Description:
  // Set the size of the simulated region.
  void SetSimulatedRegion(double xBegin, double xEnd, double yBegin, double yEnd);

  // Description:
  // Loads required extensions for vtkBlendingFluorescencePolyDataMappers.
  int LoadExtensions(vtkRenderer *renderer);

  // Description:
  // Updates the zoom factor
  virtual void Update();
  
protected:
  vtkBlendingFluorescencePolyDataMapper();
  ~vtkBlendingFluorescencePolyDataMapper();

  // Description:
  // Renders points in the polygonal data
  void RenderPoints(vtkActor *actor);

  // Description:
  // Draws PSF sprites as point sprites. Called when the screen size of the point 
  // sprite is supported by the graphics card.
  void RenderPointsAsPoints(vtkActor *actor, double width);

  // Description:
  // Draws PSF sprites as rects. Called when the screen size of the point sprite
  // is not suppoted by the graphics card.
  void RenderPointsAsRectsCPU(vtkActor *actor, double width);

  // Description:
  // Same functionality as RenderPointsAsRectsCPU, but calculates billboard
  // transformation on the GPU.
  void RenderPointsAsRectsGPU(vtkActor *actor, double width);

  // Description:
  // Returns the focal plane depth transformed according to the scale and position
  // of the actor passed in as an argument.
  double GetTransformedFocalPlaneDepth(vtkActor *actor);
  
  // Description:
  // Pointer to the renderer in which this data mapper operates. The renderer
  // gives access to the render window, which contains viewport dimensions.
  vtkRenderer *Renderer;

  // Description:
  // The PSF image data.
  vtkImageData *ImageData;

  // Description:
  // Designates the region of the experiment. Numbers are in nanometers.
  double SimulatedRegion[4];

  // Description:
  // Scaling factor for determining the length (pixels in screen space) of a
  // distance in world space (in nanometers)
  double WorldToScreen;

  // Description:
  // Maximum point size, after which we need to switch back to drawing quads
  GLfloat maxPointSize;

  // Description:
  // Depth of the focal plane (in nanometers)
  double FocalPlaneDepth;

  // Description:
  // Indicates whether NVIDIA point sprites are supported.
  int NVPointSpriteSupported;

private:
  vtkBlendingFluorescencePolyDataMapper(const vtkBlendingFluorescencePolyDataMapper&);  // Not implemented.
  void operator=(const vtkBlendingFluorescencePolyDataMapper&);  // Not implemented.

  // Description:
  // Flag indicating whether the extensions have been loaded yet.
  int extensionsLoaded;

};

#endif
