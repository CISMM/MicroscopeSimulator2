#ifndef _VTK_MODEL_OBJECT_FLUORESCENCE_REPRESENTATION_H_
#define _VTK_MODEL_OBJECT_FLUORESCENCE_REPRESENTATION_H_

#include <vtkSmartPointer.h>

#include <vtkModelObjectRepresentation.h>
#include <FluorophoreModelObjectProperty.h>

class vtkActor;
class vtkBlendingFluorescencePolyDataMapper;
class vtkGatherFluorescencePolyDataMapper;
class vtkInformation;
class vtkInformationVector;
class vtkOpenGL3DTexture;
class vtkFluorescencePointsGradientPolyDataMapper;
class vtkProperty;
class vtkRenderView;
class vtkTransformFilter;
class vtkUniformPointSampler;


class vtkModelObjectFluorescenceRepresentation : public vtkModelObjectRepresentation {
 public:
  static vtkModelObjectFluorescenceRepresentation *New();
  vtkTypeRevisionMacro(vtkModelObjectFluorescenceRepresentation, vtkModelObjectRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the ModelObject.
  void SetModelObject(ModelObjectPtr mo);
  ModelObjectPtr GetModelObject();

  void SetFluorophoreModelObjectProperty(FluorophoreModelObjectProperty* property);
  FluorophoreModelObjectProperty* GetFluorophoreModelObjectProperty();

  typedef enum {
   GATHER_MAPPER,
   BLENDING_MAPPER,
   BINNING_MAPPER
  } Mapper_t;


  void UseGatherMapper();
  void UseBlendingMapper();
  // TODO - void UseBinningMapper();

  Mapper_t GetMapperType();

  // Description:
  // Gets the vtkActor used to represent the ModelObject.
  vtkActor* GetActor();

  // Description:
  // Gets the vtkActor used to represent points in the gradient computation
  vtkActor* GetGradientActor();

  // Description:
  // Gets points gradient from the gradient actor. The return value
  // contains a copy of the fluorophore point locations as well as a
  // data array containing the gradient.
  vtkPolyData* GetPointsGradient();

 protected:
  vtkModelObjectFluorescenceRepresentation();
  ~vtkModelObjectFluorescenceRepresentation();

  ModelObjectPtr ModelObject;
  FluorophoreModelObjectProperty* FluorophoreProperty;

  Mapper_t MapperType;

  vtkSmartPointer<vtkGatherFluorescencePolyDataMapper>   GatherMapper;
  vtkSmartPointer<vtkBlendingFluorescencePolyDataMapper> BlendingMapper; 

  // Mapper for computing point set gradient
  vtkSmartPointer<vtkFluorescencePointsGradientPolyDataMapper> GradientMapper;

  // This is the actor for generating the synthetic image
  vtkSmartPointer<vtkActor> Actor;

  // This is the actor for generating the points gradient
  vtkSmartPointer<vtkActor> GradientActor;                   

  // Description:
  // Sets the input pipeline connection to this representation.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);


  virtual void PrepareForRendering(vtkView* view);

  virtual void UpdateRepresentation();

  virtual void SetPosition(double position[3]);
  virtual void SetRotationWXYZ(double rotation[4]);

  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);

  friend class vtkFluorescenceRenderView;

 private:
  vtkModelObjectFluorescenceRepresentation(const vtkModelObjectFluorescenceRepresentation&); // Purposely not implemented.
  void operator=(const vtkModelObjectFluorescenceRepresentation&); // Purposely not implemented
};


#endif // _VTK_MODEL_OBJECT_FLUORESCENCE_REPRESENTATION_H_
