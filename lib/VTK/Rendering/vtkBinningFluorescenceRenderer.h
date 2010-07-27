/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBinningFluorescenceRenderer.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBinningFluorescenceRenderer - OpenGL renderer
// .SECTION Description
// vtkBinningFluorescenceRenderer is a subclass of vtkOpenGLRenderer that
// enables fluorescence simulation by binning points and then convolving
// them with a PSF.
//

#ifndef __vtkBinningFluorescenceRenderer_h
#define __vtkBinningFluorescenceRenderer_h

#include "vtkFluorescenceRenderer.h"
#include "vtkOpenGL3DTexture.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h"
#endif
#include "vtkgl.h"

#include "vtkBinningFluorescenceRendererCUDA.h"

// Data structure for storing data related to convolution
typedef struct _ConvolutionData {
  int width, height;         // Dimensions of image data
  size_t channelSize, channelComplexSize, imageSize;
  float *binR, *binG, *binB; // Individual color channels
  float4 *imagePtr;          // Buffer for RGBA binned image
  Complex *binComplexR, *binComplexG, *binComplexB, *psfComplex;
  cufftHandle forwardPlan, inversePlan; // CUFFT plans
} ConvolutionData;


class VTK_RENDERING_EXPORT vtkBinningFluorescenceRenderer : public vtkFluorescenceRenderer
{
public:
  static vtkBinningFluorescenceRenderer *New();
  vtkTypeRevisionMacro(vtkBinningFluorescenceRenderer,vtkFluorescenceRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Concrete open gl render method.
  void DeviceRender(void);

  // Description:
  // Set focal plane step.
  vtkGetMacro(FocalPlaneStep, double);
  vtkSetMacro(FocalPlaneStep, double);

  // Description:
  // Set focal plane range.
  vtkGetMacro(FocalPlaneMin, double);
  vtkSetMacro(FocalPlaneMin, double);
  vtkGetMacro(FocalPlaneMax, double);
  vtkSetMacro(FocalPlaneMax, double);

  // Description:
  // Set focal plane depth.
  vtkGetMacro(FocalPlaneDepth, double);
  vtkSetMacro(FocalPlaneDepth, double);

  // Description:
  // Sets/gets the 3D texture object containing the PSF.
  vtkGetObjectMacro(PSFTexture, vtkOpenGL3DTexture);
  vtkSetObjectMacro(PSFTexture, vtkOpenGL3DTexture);

  // Description:
  // Sets the bit depth of the render textures
  void SetQualityTo16Bit();
  void SetQualityTo32Bit();

  // Description:
  // Sets/gets the pixel size in the generated fluorescence image
  vtkGetMacro(PixelSize, double);
  vtkSetMacro(PixelSize, double);

  //BTX
  // Description:
  // Definitions of various render textures
  static const int ACCUMULATION_TEXTURE = 0;
  static const int BINNING_TEXTURE = 1;
  static const int PSF_RENDER_TEXTURE = 2;
  //ETX
  
protected:
  vtkBinningFluorescenceRenderer();
  ~vtkBinningFluorescenceRenderer();

  // Description:
  // Spacing between focal planes
  double FocalPlaneStep;

  // Description:
  // Minimum and maximum focal distances
  double FocalPlaneMin;
  double FocalPlaneMax;

  // Description:
  // Focal plane depth
  double FocalPlaneDepth;

  // Description:
  // PSF texture
  vtkOpenGL3DTexture *PSFTexture;

  // Description:
  double PixelSize;

  // Description:
  // Indicates whether extensions have been loaded
  int ExtensionsLoaded;

  // Description:
  // Pixel buffer objects that link OpenGL textures to buffers
  // in CUDA.
  GLuint BinningPBO;
  GLuint PSFPBO;

  // Description:
  // Sizes of the pixel buffer objects.
  int PBOBufferSize;

  // Description:
  // Loads required extensions for this class.
  int LoadExtensions();

  // Description:
  // Renders a slice of the PSF to the PSF framebuffer
  void RenderPSFSlice(double *psfSize, double distFromFocalPlane);

  // Description:
  // Creates/destroys pixel buffer objects.
  void LoadPBOs(int width, int height);
  void DeletePBOs();

  // Description:
  // Convolves the binned image and PSF image.
  void ComputeConvolution(ConvolutionData *bc);

  // Description:
  // Builds ConvolutionData structure.
  void CreateConvolutionData(ConvolutionData *cd, int width, int height);
  void DestroyConvolutionData(ConvolutionData *cd);

  // Description:
  // Checks for and reports OpenGL errors
  void ReportOpenGLErrors();

private:
  vtkBinningFluorescenceRenderer(const vtkBinningFluorescenceRenderer&);  // Not implemented.
  void operator=(const vtkBinningFluorescenceRenderer&);  // Not implemented.
  
};

#endif
