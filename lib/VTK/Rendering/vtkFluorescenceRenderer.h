/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFluorescenceRenderer.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFluorescenceRenderer - OpenGL renderer
// .SECTION Description
// vtkFluorescenceRenderer is a subclass of vtkFramebufferObjectRenderer that serves
// as a base for all fluorescence microscope renderers. Provides a method for adding
// noise to the generated image.
//

#ifndef __vtkFluorescenceRenderer_h
#define __vtkFluorescenceRenderer_h

#include "vtkCollection.h"
#include "vtkOpenGLRenderer.h"
#include "vtkFramebufferObjectRenderer.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

class vtkFluorescenceRenderer : public vtkFramebufferObjectRenderer
{

public:
  static vtkFluorescenceRenderer *New();
  vtkTypeMacro(vtkFluorescenceRenderer,vtkFramebufferObjectRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Render method that calls the superclass DeviceRender()
  // then optionally adds in noise to the result.
  void DeviceRender(void);

  // Description:
  // Sets/gets the flag controlling generation of Gaussian noise.
  vtkGetMacro(GenerateNoise, int);
  vtkSetMacro(GenerateNoise, int);
  vtkBooleanMacro(GenerateNoise, int);

  // Description:
  // Sets/gets the standard deviation of the Gaussian noise added to the
  // fluorescence rendering.
  vtkGetMacro(NoiseStdDev, double);
  vtkSetMacro(NoiseStdDev, double);

  // Description:
  // Sets/gets the mean of the Gaussian noise added to the fluorescence rendering.
  vtkGetMacro(NoiseMean, double);
  vtkSetMacro(NoiseMean, double);

  // Description:
  // Sets/gets the background intensity added to the simulated fluorescence image
  vtkGetMacro(BackgroundIntensity, double);
  vtkSetMacro(BackgroundIntensity, double);
  
protected:
  vtkFluorescenceRenderer();
  ~vtkFluorescenceRenderer();

  // Description:
  // Add noise to image?
  int GenerateNoise;

  // Description:
  // Standard deviation of Gaussian noise to be added to a fluorescence rendering.
  double NoiseStdDev;

  // Description:
  // Mean of Gaussian noise to be added to a fluorescence rendering.
  double NoiseMean;

  // Description:
  // Background intensity to add to the simulated fluorescence image
  double BackgroundIntensity;
  
  // Description:
  // Loads fragment program for remapping texture values to displayable values.
  void LoadNoiseFragmentProgram(void);

  // Description:
  // Loads fragment program for adding background intensity.
  void LoadBackgroundIntensityProgram(void);

  // Description:
  // Renders noise into the active framebuffer texture.
  void RenderNoise();

  // Description:
  // Renders noise into the framebuffer texture given by index.
  void RenderNoise(int index);

  // Description:
  // Renders the background intensity into the active framebuffer texture.
  void RenderBackgroundIntensity();

private:
  vtkFluorescenceRenderer(const vtkFluorescenceRenderer&);  // Not implemented.
  void operator=(const vtkFluorescenceRenderer&);  // Not implemented.

  // Description:
  // Scramble key for hashing program that generates noise.
  unsigned int ScrambleKey;
  
  // Description:
  // Handle for program.
  GLuint NoiseProgramHandle;

  // Description:
  // Handle for fragment shader.
  GLuint NoiseFragmentShaderHandle;

  // Description:
  GLuint BackgroundIntensityProgramHandle;

  // Description:
  GLuint BackgroundIntensityFragmentShaderHandle;
  
};

#endif
