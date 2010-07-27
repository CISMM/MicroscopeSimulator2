/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescencePointsGradientRenderer.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFluorescencePointsGradientRenderer - OpenGL renderer
// .SECTION Description
// vtkFluorescencePointsGradientRenderer is a subclass of vtkFramebufferObjectRenderer that serves
// as a base for all fluorescence microscope renderers. Provides a method for adding
// noise to the generated image.
//

#ifndef __vtkFluorescencePointsGradientRenderer_h
#define __vtkFluorescencePointsGradientRenderer_h

#include "vtkOpenGL3DTexture.h"
#include "vtkOpenGLRenderer.h"
#include "vtkFramebufferObjectRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class VTK_RENDERING_EXPORT vtkFluorescencePointsGradientRenderer : public vtkFramebufferObjectRenderer
{

public:
  static vtkFluorescencePointsGradientRenderer *New();
  vtkTypeRevisionMacro(vtkFluorescencePointsGradientRenderer,vtkFramebufferObjectRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Render method that calls the superclass DeviceRender()
  // then optionally adds in noise to the result.
  void DeviceRender(void);

  // Description:
  // Set/get the texture holding the experimental image
  vtkSetObjectMacro(ExperimentalImageTexture, vtkOpenGL3DTexture);
  vtkGetObjectMacro(ExperimentalImageTexture, vtkOpenGL3DTexture);
  
protected:
  vtkFluorescencePointsGradientRenderer();
  ~vtkFluorescencePointsGradientRenderer();

  int LoadExtensions(vtkRenderWindow *renWin);

private:
  vtkFluorescencePointsGradientRenderer(const vtkFluorescencePointsGradientRenderer&);  // Not implemented.
  void operator=(const vtkFluorescencePointsGradientRenderer&);  // Not implemented.

  // Description:
  // Texture containing experimental image
  vtkOpenGL3DTexture* ExperimentalImageTexture;

  int ExtensionsLoaded;

};

#endif
