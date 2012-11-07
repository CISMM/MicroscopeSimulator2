/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBlendingFluorescencePolyDataMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBlendingFluorescencePolyDataMapper.h"

#include "vtkProperty.h"

vtkStandardNewMacro(vtkBlendingFluorescencePolyDataMapper);

// Some convenience macros
#define X 0
#define Y 1
#define Z 2

// Simple macro to translate add two vectors and scale the result
#define AddAndScaleMacro(dest, a, b, scale) { \
  dest[X] = (a[X] + b[X]) * scale; \
  dest[Y] = (a[Y] + b[Y]) * scale; \
  dest[Z] = (a[Z] + b[Z]) * scale; \
};

// Construct empty object.
vtkBlendingFluorescencePolyDataMapper::vtkBlendingFluorescencePolyDataMapper() {
  this->ListId = 0;
  this->Renderer = NULL;
  //this->PSFImage = NULL;

  this->SetSimulatedRegion(0, 1, 0, 1);
  this->WorldToScreen = 1.0;
  this->FocalPlaneDepth = 0.0;
  this->NVPointSpriteSupported = 0;
  this->extensionsLoaded = 0;
}

// Destructor (don't call ReleaseGraphicsResources() because it is virtual)
vtkBlendingFluorescencePolyDataMapper::~vtkBlendingFluorescencePolyDataMapper() {
  if (this->Renderer) {
    this->UnRegister(this->Renderer);
    this->Renderer = NULL;
  }

  //if (this->PSFImage) {
  //  this->UnRegister(this->PSFImage);
  //  this->PSFImage = NULL;
  //}

  if (this->LastWindow) {
    this->ReleaseGraphicsResources(this->LastWindow);
  }
}

// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
void vtkBlendingFluorescencePolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
  if (win) {
    win->MakeCurrent();
  }
  
  if (this->ListId && win) {
    glDeleteLists(this->ListId,1);
    this->ListId = 0;
  }
  this->LastWindow = NULL; 
}

// Renders the points
void vtkBlendingFluorescencePolyDataMapper::RenderPoints(vtkActor *actor) {

  // Figure out how large the NVIDIA point sprites can be
  GLfloat sizes[2];
  glGetFloatv(GL_POINT_SIZE_RANGE, sizes);
  this->maxPointSize = sizes[1];

  // Turn on blending. Hope it's floating-point. If floating-point blending
  // is not available, we'll have ugly images.
  glEnable(GL_BLEND);
  vtkgl::BlendEquation(vtkgl::FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);
  
  // Disable a few things.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  // Switch on custom PSF drawing routines based on requested screen size of
  // the PSF sprite.
  // WARNING. We are assuming radially symmetric PSFs here whose x and y
  // dimensions are the same, both in real space and pixel space.
  //double *spacing = this->PSFImage->GetSpacing();
  //int *dimensions = this->PSFImage->GetDimensions();
  double* spacing = actor->GetTexture()->GetInput()->GetSpacing();
  int* dimensions = actor->GetTexture()->GetInput()->GetDimensions();
  double width = static_cast<double>(dimensions[0]) * spacing[0];

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //if (pointSize > maxPointSize || !NVPointSpriteSupported) {
    //this->RenderPointsAsRectsCPU(actor, width);
    this->RenderPointsAsRectsGPU(actor, width);
  //} else {
  //  this->RenderPointsAsPoints(actor, width);
  //}

  // Turn blending back off
  glDisable(GL_BLEND);

}

// Use this method to render with NVIDIA point sprites. We choose NVIDIA
// points sprites because they let us manipulate the Z texture coordinate,
// which is crucial to choosing which slice in the PSF stack we display
// at each point.
void vtkBlendingFluorescencePolyDataMapper::RenderPointsAsPoints(vtkActor *actor, double width) {

  // Use the point-sprite function
  glEnable(vtkgl::POINT_SPRITE_NV);

  // Determine the R texture coordinate by the S texture coordinate
  // so we can use glTexCoord1d() instead of glTexCoord3d()
  vtkgl::PointParameteriNV(vtkgl::POINT_SPRITE_R_MODE_NV, GL_S);

  // and draw the points
  vtkPoints *points = this->GetInput()->GetPoints();
  int numPoints = points->GetNumberOfPoints();
  double transformedFocalPlaneDepth = this->GetTransformedFocalPlaneDepth(actor);
  double pointSize = width * this->WorldToScreen;

  glPointSize(pointSize);
  glBegin(GL_POINTS);
  for (int i = 0; i < numPoints; i++) {
    double point[3];
    points->GetPoint(i, point);
    glTexCoord1d(point[2] - transformedFocalPlaneDepth + 0.5);
    glVertex3d(point[0], point[1], point[2]);
  }
  glEnd();

  glDisable(vtkgl::POINT_SPRITE_NV);
}

// Renders points as billboards. The billboards are oriented with the
// rotation opposite of the camera orientation so that when the modelview
// matrix is applied, the billboard rotation is cancelled out, making them
// aligned with the image plane. This is done on the CPU. Unfortunately,
// this can only be called in Immediate mode rendering because the rotation
// of each billboard must be done for each frame. A faster way to do the
// individual rotations on the GPU in Cg is known, and will be implemented
// in a later release (i.e., when we need some numbers for a paper).
void vtkBlendingFluorescencePolyDataMapper::RenderPointsAsRectsCPU(vtkActor *actor, double width) {

  // Get info on the number of points
  double *scale = actor->GetScale();
  double offset = (0.5 * width) / (scale[0]);

  GLfloat modelView[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

  // Right vector
  double right[3] = {(double) modelView[0], (double) modelView[4], (double) modelView[8]};
  vtkMath::Normalize(right);

  // Up vector
  double up[3] = {(double) modelView[1], (double) modelView[5], (double) modelView[9]};
  vtkMath::Normalize(up);

  // We just need to precompute one rotated quad
  double ll[3], ul[3], ur[3], lr[3];
  AddAndScaleMacro(ll, -right, -up, offset); // Lower left
  AddAndScaleMacro(ul, -right,  up, offset); // Upper left
  AddAndScaleMacro(ur,  right,  up, offset); // Upper right
  AddAndScaleMacro(lr,  right, -up, offset); // Lower right

  vtkPoints *points = this->GetInput()->GetPoints();
  int numPoints = points->GetNumberOfPoints();

  // Get height of PSF
  //double psfHeight = this->PSFImage->GetDimensions()[Z] * this->PSFImage->GetSpacing()[Z];
  vtkImageData* psfImage = actor->GetTexture()->GetInput();
  double psfHeight = psfImage->GetDimensions()[Z] * psfImage->GetSpacing()[Z];
  
  // Get scaling factor for mapping a distance in world space to texture space,
  // which ranges from 0.0 to 1.0. It's okay if we go outside this range for now;
  // we'll cull later.
  double worldToTex = 1.0 / psfHeight;

  glBegin(GL_QUADS);
  for (int i = 0; i < numPoints; i++) {
    double point[4];
    points->GetPoint(i, point);
    point[3] = 1.0;

    vtkMatrix4x4 *mat = actor->GetMatrix();
    double worldPt[4];
    mat->MultiplyPoint(point, worldPt);
    double dist = this->FocalPlaneDepth - worldPt[Z];

    // Map z-distance from focal plane to point in world space to
    // distance along PSF
    double axialOffset = (worldToTex * dist) + 0.5;
    if (axialOffset < 0.0 || axialOffset > 1.0)
      continue;
    
    glTexCoord3d(0.0, 0.0, axialOffset); // Lower left
    glVertex3d(point[X]+ll[X], point[Y]+ll[Y], point[Z]+ll[Z]);
    glTexCoord3d(0.0, 1.0, axialOffset); // Upper left
    glVertex3d(point[X]+ul[X], point[Y]+ul[Y], point[Z]+ul[Z]);
    glTexCoord3d(1.0, 1.0, axialOffset); // Upper right
    glVertex3d(point[X]+ur[X], point[Y]+ur[Y], point[Z]+ur[Z]);
    glTexCoord3d(1.0, 0.0, axialOffset); // Lower right
    glVertex3d(point[X]+lr[X], point[Y]+lr[Y], point[Z]+lr[Z]);
  }
  glEnd();

}

// Renders points as billboards. The billboards are oriented with the
// rotation opposite of the camera orientation so that when the modelview
// matrix is applied, the billboard rotation is cancelled out, making them
// aligned with the image plane. This is done on the GPU using a vertex
// program. The 3D center of each quad is redundantly encoded in the four vertices
// that make up the quad; the w coordinate value is an index 0-3 that indicates
// which corner of the quad the vertex represents. With this info, the inverse
// orientation can be applied appropriately for the vertex.
void vtkBlendingFluorescencePolyDataMapper::RenderPointsAsRectsGPU(vtkActor *actor, double width) {

  vtkProperty *property = actor->GetProperty();
  vtkPoints *points = this->GetInput()->GetPoints();
  int numPoints = points->GetNumberOfPoints();

  // Get height of PSF
  //double psfHeight = this->PSFImage->GetDimensions()[Z] * this->PSFImage->GetSpacing()[Z];
  vtkImageData* psfImage = actor->GetTexture()->GetInput();
  double psfHeight = psfImage->GetDimensions()[Z] * psfImage->GetSpacing()[Z];
  
  // Get scaling factor for mapping a distance in world space to texture space,
  // which ranges from 0.0 to 1.0. It's okay if we go outside this range for now;
  // we'll cull later.
  float worldToTex = 1.0 / psfHeight;
  property->AddShaderVariable("worldToTex", worldToTex);
  property->AddShaderVariable("focalPlaneDepth", (float) this->FocalPlaneDepth);
  property->AddShaderVariable("screenOffset", 0.5 * width);

  glBegin(GL_QUADS);
  for (int i = 0; i < numPoints; i++) {
    double point[4];
    points->GetPoint(i, point);
    point[3] = 1.0;

    // w-coordinate in the vertex encodes the vertex ID
    glTexCoord3d(0.0, 0.0, 0.0); // Lower left
    glVertex4d(point[X], point[Y], point[Z], 0.0);
    glTexCoord3d(0.0, 1.0, 0.0); // Upper left
    glVertex4d(point[X], point[Y], point[Z], 1.0);
    glTexCoord3d(1.0, 1.0, 0.0); // Upper right
    glVertex4d(point[X], point[Y], point[Z], 2.0);
    glTexCoord3d(1.0, 0.0, 0.0); // Lower right
    glVertex4d(point[X], point[Y], point[Z], 3.0);
  }
  glEnd();

}

// We need to know the simulation region so that we know how to scale from world 
// space to screen space.
void vtkBlendingFluorescencePolyDataMapper::SetSimulatedRegion(double xBegin, 
                                                       double xEnd, 
                                                       double yBegin, 
                                                       double yEnd) {
  this->SimulatedRegion[0] = xBegin;
  this->SimulatedRegion[1] = xEnd;
  this->SimulatedRegion[2] = yBegin;
  this->SimulatedRegion[3] = yEnd;
}

// Computes the axial offset of the PSF. Returned value is between 0.0 and 1.0,
// and should be used as an index into the 3D PSF texture.
double vtkBlendingFluorescencePolyDataMapper::GetTransformedFocalPlaneDepth(vtkActor *actor) {
  double position[3], scale[3];
  actor->GetPosition(position);
  actor->GetScale(scale);

  // Shift the plane depth by the reverse of the actor translation
  double z = this->FocalPlaneDepth - position[Z];

  // Unscale the plane depth
  z /= scale[Z];

  return z;
}

// Draw method for OpenGL.
int vtkBlendingFluorescencePolyDataMapper::Draw(vtkRenderer *aren, vtkActor *act) {
  if (!this->extensionsLoaded) {
    this->LoadExtensions(aren);
  }

  //if (!this->PSFImage) {
  //  this->UpdateProgress(1.0);
  //  return 1;
  //}

  this->RenderPoints(act);
  this->UpdateProgress(1.0);
  return 1;
}

void vtkBlendingFluorescencePolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act) {

  // We need to apply the translation, scaling, and orientation transformations of the
  // actor to the point to get the z offset of the point from the focal plane.
  vtkMatrix4x4 *mat = act->GetMatrix();
  float zWorldTransform[4];
  for (int i = 0; i < 4; i++) {
    zWorldTransform[i] = (float) mat->GetElement(2, i);
  }
  vtkProperty *property = act->GetProperty();
  property->AddShaderVariable("zWorldTransform", 4, zWorldTransform);

  // Do the rendering for this actor.
  this->Superclass::RenderPiece(ren, act);
}

void vtkBlendingFluorescencePolyDataMapper::SetRenderer(vtkRenderer *renderer) {
  if (this->Renderer) {
    this->UnRegister(this->Renderer);
    this->Renderer = 0;
  }
  this->Renderer = renderer;
  this->Register(renderer);
}

// Calls the superclass update method, then updates the WorldToScreen member variable.
void vtkBlendingFluorescencePolyDataMapper::Update() {
  this->Superclass::Update();

  if (this->Renderer != NULL) {
    int *windowSize = this->Renderer->GetSize();
    double simWidth = SimulatedRegion[1] - SimulatedRegion[0];
    this->WorldToScreen = windowSize[0] / simWidth;
  }
}

int vtkBlendingFluorescencePolyDataMapper::LoadExtensions(vtkRenderer *renderer) {
  vtkOpenGLExtensionManager *manager = vtkOpenGLExtensionManager::New();
  manager->SetRenderWindow(renderer->GetRenderWindow());

  // Check for acceptable OpenGL versions
  int versionOK = 0;
  versionOK += manager->ExtensionSupported("GL_VERSION_1_2");
  manager->LoadExtension("GL_VERSION_1_2");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_3");
  manager->LoadExtension("GL_VERSION_1_3");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_4");
  manager->LoadExtension("GL_VERSION_1_4");
  versionOK += manager->ExtensionSupported("GL_VERSION_1_5");
  manager->LoadExtension("GL_VERSION_1_5");

  // Acceptable OpenGL version not found.
  if (!versionOK) {
    manager->Delete();
    return 0;
  }

  int supported = 1;
  this->NVPointSpriteSupported = manager->ExtensionSupported("GL_NV_point_sprite");
  supported *= NVPointSpriteSupported;
  if (supported)
    manager->LoadExtension("GL_NV_point_sprite");
  supported *= manager->ExtensionSupported("GL_ARB_imaging");
  if (supported)
    manager->LoadExtension("GL_ARB_imaging");

  // Dispose of the manager.
  manager->Delete();
  return supported;
}

void vtkBlendingFluorescencePolyDataMapper::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
}
