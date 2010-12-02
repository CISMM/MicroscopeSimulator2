/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkGatherFluorescencePolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkGatherFluorescencePolyDataMapper.h"

#include "vtkMatrix4x4.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGL3DTexture.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

#include <vector>


#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkGatherFluorescencePolyDataMapper, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkGatherFluorescencePolyDataMapper);
#endif


// Construct empty object.
vtkGatherFluorescencePolyDataMapper::vtkGatherFluorescencePolyDataMapper() {
  this->ListId = 0;
  this->Renderer = NULL;

  this->SetSimulatedRegion(0, 1, 0, 1);
  this->FocalPlaneDepth = 0.0;
  this->ExtensionsLoaded = 0;
  this->PtsLastTimePointsModified = 0;

  // Set up texture memory for storing point data.
  this->PointTextureID = 0;
  this->PointTextureDimension = 0;
  this->PointTextureTarget = vtkgl::TEXTURE_RECTANGLE_ARB;

  this->ShaderProgramHandle = 0;
  this->VertexProgramHandle = 0;
  this->FragmentProgramHandle = 0;

  // Set default number of points to render per pass.
  this->PointsPerPass = 3000;

  // Set the default intensity scale factor.
  this->IntensityScale = 1.0;
}


// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkGatherFluorescencePolyDataMapper::~vtkGatherFluorescencePolyDataMapper() {
}


// Release the graphics resources used by this mapper.  In this case, release
// the display list if any as well as the texture holding the point positions.
void vtkGatherFluorescencePolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
  if (win) {
    win->MakeCurrent();
  }
  
  if (this->ListId && win) {
    glDeleteLists(this->ListId,1);
    this->ListId = 0;
  }
  if (this->PointTextureID && win) {
    this->DeletePointTexture();
  }
  this->LastWindow = NULL;

  if (vtkgl::DeleteShader) {
    vtkgl::DeleteShader(this->VertexProgramHandle);
    this->VertexProgramHandle = 0;

    vtkgl::DeleteShader(this->FragmentProgramHandle);
    this->FragmentProgramHandle = 0;

    vtkgl::DeleteProgram(this->ShaderProgramHandle);
    this->ShaderProgramHandle = 0;
  }
}


// Renders the points
void vtkGatherFluorescencePolyDataMapper::RenderPoints(vtkActor *actor, vtkRenderer* renderer) {
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);
  //this->PSFTexture->Load(renderer);

  if (!this->PSFTexture)
    return;
  
  this->PSFTexture->Render(renderer);

  this->LoadPointTexture();

  // We'll use the texture matrix to store the actor transformation
  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity();
  vtkMatrix4x4 *matrix = actor->GetMatrix();
  this->RenderMatrix(matrix);

  glBindTexture(this->PointTextureTarget, (GLuint) this->PointTextureID);
  glEnable(this->PointTextureTarget);
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  // Turn on blending. Hope it's floating-point. If floating-point blending
  // is not available, we'll have inaccurate images.
  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);
  
  // Disable a few things.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);

  // Set up window
  int *size = renderer->GetRenderWindow()->GetSize();
  int winW = size[0];
  int winH = size[1];
  glViewport(0, 0, winW, winH);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  double worldWinW = this->PixelSize[0] * winW;
  double worldWinH = this->PixelSize[1] * winH;
  glOrtho(0, worldWinW, 0, worldWinH, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  vtkOpenGL3DTexture *texture = this->PSFTexture;
  if (texture && texture->GetInput()) {
    vtkImageData* psfImage = texture->GetInput();
    int *psfDims = psfImage->GetDimensions();
    double psfSpacing[3], psfOrigin[3], psfMaxTexCoords[3], psfScale[3];
    psfImage->GetSpacing(psfSpacing);
    psfImage->GetOrigin(psfOrigin);
    texture->GetMaxTextureCoordinates(psfMaxTexCoords);
    for (int i = 0; i < 3; i++) {
      // I have defined the origin in VTK so that it falls in the center of
      // the voxel at index (0,0,0). To translate into texture coordinates,
      // it is more convenient to have the origin defined at the corner of the
      // voxel at (0,0,0). Do this by translating the origin by minus 
      // one-half pixel.
      psfOrigin[i] -= 0.5 * psfSpacing[i];
      psfScale[i] = psfMaxTexCoords[i] / (psfDims[i] * psfSpacing[i]);
    }

    if (this->ShaderProgramHandle) {
      vtkgl::UseProgram(this->ShaderProgramHandle);

      // Set shader parameters.
      this->SetUniform1i("psfSampler", 0);
      this->SetUniform1i("ptsSampler", 1);
      this->SetUniform1i ("pointTexDim", this->PointTextureDimension);
      this->SetUniform3dv("psfOrigin", psfOrigin);
      this->SetUniform3dv("psfScale", psfScale);
      this->SetUniform3dv("psfMaxTexCoords", psfMaxTexCoords);
      this->SetUniform1f("focalDepth", this->FocalPlaneDepth);
      this->SetUniform1f("gain", this->Gain);
      this->SetUniform1f("shearInX", this->Shear[0]);
      this->SetUniform1f("shearInY", this->Shear[1]);
    }

    // Get bounding box of points to limit fragment programs
    vtkPoints *points = this->GetInput()->GetPoints();
    double *bb = points->GetBounds();
    double quad[4];

    double *imgBB = texture->GetInput()->GetBounds();
    double xPad = 0.5*(imgBB[1] - imgBB[0]);
    double yPad = 0.5*(imgBB[3] - imgBB[2]);
    this->ComputeBoundingQuad(bb, matrix, xPad, yPad, quad);

    while(glGetError());

    // Outer loop required because fragment programs are limited to 2^16 instructions.
    // We'll generally have far more points than 2^16, so we need to draw several
    // quads, gathering from a different set of points in each one.
    int numPoints = points->GetNumberOfPoints();
    int increment = this->PointsPerPass;
    for (int startIndex = 0; startIndex < numPoints; startIndex += increment) {
      int endIndex = startIndex + increment;
      if (endIndex > numPoints)
        endIndex = numPoints;
      
      this->SetUniform1i("startIndex", startIndex);
      this->SetUniform1i("endIndex", endIndex);

      // Draw quad that bounds all contributions from the fluorophores
      glBegin(GL_QUADS);
        glVertex2f(quad[0], quad[2]);
        glVertex2f(quad[0], quad[3]);
        glVertex2f(quad[1], quad[3]);
        glVertex2f(quad[1], quad[2]);
      glEnd();
    }
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glDisable(this->PointTextureTarget);

  // Switch back to texture unit 0
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  // Make sure to disable any 3D textures
  //glDisable(vtkgl::TEXTURE_3D);

  this->PSFTexture->PostRender(renderer);

  // Turn blending back off
  glDisable(GL_BLEND);

  vtkgl::UseProgram(0);
}

// Send matrix to OpenGL.
void vtkGatherFluorescencePolyDataMapper::RenderMatrix(vtkMatrix4x4 *matrix) {
  double *mat = matrix->Element[0];
  double mat2[16];
  // Transpose the matrix to the format expected by OpenGL.
  mat2[ 0] = mat[0];  mat2[ 1] = mat[4];  mat2[ 2] = mat[ 8];  mat2[ 3] = mat[12];  
  mat2[ 4] = mat[1];  mat2[ 5] = mat[5];  mat2[ 6] = mat[ 9];  mat2[ 7] = mat[13];
  mat2[ 8] = mat[2];  mat2[ 9] = mat[6];  mat2[10] = mat[10];  mat2[11] = mat[14];
  mat2[12] = mat[3];  mat2[13] = mat[7];  mat2[14] = mat[11];  mat2[15] = mat[15];
  glMultMatrixd(mat2);
}

// Draw method for OpenGL.
int vtkGatherFluorescencePolyDataMapper::Draw(vtkRenderer *aren, vtkActor *act) {
  this->LoadExtensions(aren->GetRenderWindow());
  this->LoadShaderProgram(aren->GetRenderWindow());

  this->RenderPoints(act, aren);
  this->UpdateProgress(1.0);
  return 1;
}

// Loads the point texture.
void vtkGatherFluorescencePolyDataMapper::LoadPointTexture() {
  if (!this->ExtensionsLoaded) {
    return;
  }

  int ptsModifiedTime = this->GetInput()->GetPoints()->GetMTime();
  if (ptsModifiedTime != this->PtsLastTimePointsModified ||
    this->PointTextureID == 0) {
    this->DeletePointTexture();

    vtkPoints *points = this->GetInput()->GetPoints();
    int numPoints = points->GetNumberOfPoints();
    this->PointTextureDimension = ceil(sqrt((double) numPoints));

    vtkPointData *pointData = this->GetInput()->GetPointData();
    vtkFloatArray *intensities = vtkFloatArray::SafeDownCast
      (pointData->GetScalars("Intensity"));

    // Copy and cast point locations to texture memory.
    int pointDataLength = this->PointTextureDimension * this->PointTextureDimension;
    int tupleSize = 4;
    GLfloat *textureData = new GLfloat[pointDataLength * tupleSize];
    for (int i = 0; i < numPoints; i++) {
      double *tmp = points->GetPoint(i);
      textureData[i*tupleSize + 0] = (GLfloat) tmp[0];
      textureData[i*tupleSize + 1] = (GLfloat) tmp[1];
      textureData[i*tupleSize + 2] = (GLfloat) tmp[2];
      if (intensities) {
        textureData[i*tupleSize + 3] = 
          this->IntensityScale * intensities->GetValue(i);
      } else {
        textureData[i*tupleSize + 3] = this->IntensityScale;
      }
    }

    // Fill the rest of the array (these points will not be used).
    for (int i = numPoints; i < pointDataLength; i++) {
      textureData[i*tupleSize + 0] = 0.0f;
      textureData[i*tupleSize + 1] = 0.0f;
      textureData[i*tupleSize + 2] = 0.0f;
      textureData[i*tupleSize + 3] = 0.0f;
    }

    this->PtsLastTimePointsModified = ptsModifiedTime;

    vtkgl::ActiveTexture(vtkgl::TEXTURE1);

    // Update point data in texture map
    glGenTextures(1, (GLuint *) &this->PointTextureID);

    glBindTexture(this->PointTextureTarget, (GLuint) this->PointTextureID);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
    glTexParameteri(this->PointTextureTarget, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(this->PointTextureTarget, 0, vtkgl::RGBA_FLOAT32_ATI, this->PointTextureDimension, 
      this->PointTextureDimension, 0, GL_RGBA, GL_FLOAT, textureData);

    delete[] textureData;

    vtkgl::ActiveTexture(vtkgl::TEXTURE0);
  }
}

// Deletes the point texture.
void vtkGatherFluorescencePolyDataMapper::DeletePointTexture() {
  if (this->PointTextureID != 0) {
    glDeleteTextures(1, (GLuint *) &this->PointTextureID);
    this->PointTextureID = 0;
  }
}

// Compute bounding quad.
void vtkGatherFluorescencePolyDataMapper::ComputeBoundingQuad(double bounds[6], vtkMatrix4x4 *matrix, 
                                                              double padX, double padY, double quad[4]) {
  // List of point indices into bounding box
  int bb2pt[8][3] = {
    {0, 2, 4},
    {1, 2, 4},
    {0, 3, 4},
    {1, 3, 4},
    {0, 2, 5},
    {1, 2, 5},
    {0, 3, 5},
    {1, 3, 5}};
  double point[4];
  double boundingQuad[4];

  boundingQuad[0] = boundingQuad[2] =  1e9;
  boundingQuad[1] = boundingQuad[3] = -1e9;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 3; j++) {
      point[j] = bounds[bb2pt[i][j]];
    }
    point[3] = 1.0;
    matrix->MultiplyPoint(point, point);
    
    if (point[0] < boundingQuad[0]) boundingQuad[0] = point[0];
    if (point[0] > boundingQuad[1]) boundingQuad[1] = point[0];
    if (point[1] < boundingQuad[2]) boundingQuad[2] = point[1];
    if (point[1] > boundingQuad[3]) boundingQuad[3] = point[1];
  }

  // Pad with PSF size
  boundingQuad[0] -= padX;
  boundingQuad[1] += padX;
  boundingQuad[2] -= padY;
  boundingQuad[3] += padY;

  // Output final result
  for (int i = 0; i < 4; i++) 
    quad[i] = boundingQuad[i];
}


void vtkGatherFluorescencePolyDataMapper::SetUniform1i(const char* name, int value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform1i(handle, value);
}


void vtkGatherFluorescencePolyDataMapper::SetUniform1f(const char* name, float value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform1f(handle, value);
}


void vtkGatherFluorescencePolyDataMapper::SetUniform3dv(const char* name, double* value) {
  GLint handle = vtkgl::GetUniformLocation(this->ShaderProgramHandle, name);
  vtkgl::Uniform3f(handle, (float) value[0], (float) value[1], (float) value[2]);
}


// Calls the superclass update method.
void vtkGatherFluorescencePolyDataMapper::Update() {
  this->Superclass::Update();
}


int vtkGatherFluorescencePolyDataMapper::LoadExtensions(vtkRenderWindow *renWin) {
  if (this->ExtensionsLoaded)
    return 1;

  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(renWin);

  std::vector<std::string> versions;
  versions.push_back("GL_VERSION_1_1");
  versions.push_back("GL_VERSION_1_2");
  versions.push_back("GL_VERSION_1_3");
  versions.push_back("GL_VERSION_1_4");
  versions.push_back("GL_VERSION_2_0");
  
  for (unsigned int i = 0; i < versions.size(); i++) {
    manager->LoadSupportedExtension(versions[i].c_str());
  }

  std::vector<std::string> extensions;
  extensions.push_back("GL_ARB_texture_rectangle");
  extensions.push_back("GL_ARB_imaging");

  for (unsigned int i = 0; i < extensions.size(); i++) {
    int supported = manager->LoadSupportedExtension(extensions[i].c_str());
    if (!supported) {
      vtkErrorMacro(<< extensions[i] << " not supported on your system!");
      return 0;
    }
  }

  this->ExtensionsLoaded = 1;

  // Dispose of the manager.
  manager->Delete();
  return 1;
}


int vtkGatherFluorescencePolyDataMapper::LoadShaderProgram(vtkRenderWindow *renWin) {
  if (this->ShaderProgramHandle)
    return 1;

  if (!this->LoadExtensions(renWin))
    return 0;

  std::string gatherVertexProgram =
#include "vtkGatherFluorescencePolyDataMapper_vertex.glsl"
	  ;

  std::string gatherFragmentProgram =
#include "vtkGatherFluorescencePolyDataMapper_fragment.glsl"
	  ;

  this->VertexProgramHandle = vtkgl::CreateShader(vtkgl::VERTEX_SHADER);
  const vtkgl::GLchar* vertexProgram = gatherVertexProgram.c_str();
  vtkgl::ShaderSource(this->VertexProgramHandle, 1, &vertexProgram, NULL);
  vtkgl::CompileShader(this->VertexProgramHandle);

  this->FragmentProgramHandle = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  const vtkgl::GLchar* fragmentProgram = gatherFragmentProgram.c_str();
  vtkgl::ShaderSource(this->FragmentProgramHandle, 1, &fragmentProgram, NULL);
  vtkgl::CompileShader(this->FragmentProgramHandle);

  // Check for errors
  GLint status;
  char infoLog[256];
  GLint length;
  vtkgl::GetShaderiv(this->VertexProgramHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro("Failed to compile vertex program");
    vtkgl::GetShaderInfoLog(this->VertexProgramHandle, 256, &length, infoLog);
    vtkGraphicErrorMacro(renWin, infoLog);
    return 0;
  }
  vtkgl::GetShaderiv(this->FragmentProgramHandle, vtkgl::COMPILE_STATUS, &status);
  if (!status) {
    vtkErrorMacro("Failed to compile fragment program");
    vtkgl::GetShaderInfoLog(this->FragmentProgramHandle, 256, &length, infoLog);
    vtkGraphicErrorMacro(renWin, infoLog);
    return 0;
  }

  this->ShaderProgramHandle = vtkgl::CreateProgram();
  vtkgl::AttachShader(this->ShaderProgramHandle, this->VertexProgramHandle);
  vtkgl::AttachShader(this->ShaderProgramHandle, this->FragmentProgramHandle);

  vtkgl::LinkProgram(this->ShaderProgramHandle);

  vtkgl::GetProgramiv(this->ShaderProgramHandle, vtkgl::LINK_STATUS, &status);
  if (!status) {
    vtkGraphicErrorMacro(renWin, "Failed to link shader program");
    vtkgl::GetProgramInfoLog(this->ShaderProgramHandle, 256, &length, infoLog);
    vtkGraphicErrorMacro(renWin, infoLog);
  }

  return 1;
}


void vtkGatherFluorescencePolyDataMapper::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}
