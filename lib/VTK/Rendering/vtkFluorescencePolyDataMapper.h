/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePolyDataMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFluorescencePolyDataMapper - superclass for fluorescence poly data mappers
// .SECTION Description
// vtkFluorescencePolyDataMapper is a subclass of vtkOpenGLPolyDataMapper.
// vtkFluorescencePolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkFluorescencePolyDataMapper_h
#define __vtkFluorescencePolyDataMapper_h

#include <math.h>

#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkOpenGL3DTexture.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
#include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkFluorescencePolyDataMapper : public vtkOpenGLPolyDataMapper
{
public:
  vtkTypeRevisionMacro(vtkFluorescencePolyDataMapper,vtkOpenGLPolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets/gets exposure.
  vtkSetMacro(Exposure, double);
  vtkGetMacro(Exposure, double);

  // Description:
  // Sets/gets pixel size.
  vtkSetVector2Macro(PixelSize, double);
  vtkGetVector2Macro(PixelSize, double);

  // Description:
  // Sets the shear parameters in X and Y. Shear is defined as a linear
  // function of Z. The shear parameters defined the slope of the
  // linear function in X and Y and define how objects appear to shift
  // due to the shear. The sample points in the image are actually
  // transformed by -Shear * z-position.
  vtkSetVector2Macro(Shear, double);
  vtkGetVector2Macro(Shear, double);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a) = 0;

  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Set the renderer used to render the fluorescence image
  vtkGetObjectMacro(Renderer, vtkRenderer);
  void SetRenderer(vtkRenderer *renderer);

  // Description:
  // Sets/gets the 3D texture object representing the PSF texture.
  vtkGetObjectMacro(PSFTexture, vtkOpenGL3DTexture);
  vtkSetObjectMacro(PSFTexture, vtkOpenGL3DTexture);

  // Description:
  // Sets/gets the focal plane position (in nanometers);
  vtkGetMacro(FocalPlaneDepth, double);
  vtkSetMacro(FocalPlaneDepth, double);

  // Description:
  // Set the size of the simulated region.
  // WARNING - Deprecated. Do not use.
  void SetSimulatedRegion(double xBegin, double xEnd, double yBegin, double yEnd);

protected:
  vtkFluorescencePolyDataMapper();
  ~vtkFluorescencePolyDataMapper();
  
  // Description:
  // Mimics exposure. Does not represent a physical quantity such as
  // photons per second.
  double Exposure;

  // Description:
  // Pointer to the renderer in which this data mapper operates. The renderer
  // gives access to the render window, which contains viewport dimensions.
  vtkRenderer *Renderer;

  // Description:
  // Designates the region of the experiment. Numbers are in nanometers.
  double SimulatedRegion[4];

  // Description:
  // Sets the pixel size. Numbers are in nanometers.
  double PixelSize[2];

  // Description:
  // Shear parameters in X and Y.
  double Shear[2];

  // Description:
  // Depth of the focal plane (in nanometers)
  double FocalPlaneDepth;

  // Description:
  // The 3D texture holding the PSF image data. Subclasses may choose to use
  // this to hold the gradient image of the PSF instead.
  vtkOpenGL3DTexture *PSFTexture;

private:
  vtkFluorescencePolyDataMapper(const vtkFluorescencePolyDataMapper&);  // Not implemented.
  void operator=(const vtkFluorescencePolyDataMapper&);  // Not implemented.

};

#endif
