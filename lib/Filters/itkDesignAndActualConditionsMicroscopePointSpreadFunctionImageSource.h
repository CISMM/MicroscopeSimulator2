/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_h
#define __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_h

#include <complex>

#include "itkParametricImageSource.h"

namespace itk
{

namespace Functor
{

class DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand {
public:
  typedef std::complex<double> ComplexType;

public:
  template< class TSource >
  void CopySettings( const TSource* source )
  {
    this->m_EmissionWavelength                 = source->GetEmissionWavelength();
    this->m_NumericalAperture                  = source->GetNumericalAperture();
    this->m_Magnification                      = source->GetMagnification();
    this->m_DesignCoverSlipRefractiveIndex     = source->GetDesignCoverSlipRefractiveIndex();
    this->m_ActualCoverSlipRefractiveIndex     = source->GetActualCoverSlipRefractiveIndex();
    this->m_DesignCoverSlipThickness           = source->GetDesignCoverSlipThickness();
    this->m_ActualCoverSlipThickness           = source->GetActualCoverSlipThickness();
    this->m_DesignImmersionOilRefractiveIndex  = source->GetDesignImmersionOilRefractiveIndex();
    this->m_ActualImmersionOilRefractiveIndex  = source->GetActualImmersionOilRefractiveIndex();
    this->m_DesignImmersionOilThickness        = source->GetDesignImmersionOilThickness();
    this->m_DesignSpecimenLayerRefractiveIndex = source->GetDesignSpecimenLayerRefractiveIndex();
    this->m_ActualSpecimenLayerRefractiveIndex = source->GetActualSpecimenLayerRefractiveIndex();
    this->m_PointSourceDepthInSpecimenLayer    = source->GetPointSourceDepthInSpecimenLayer();

    this->m_K = 2.0 * itk::Math::pi / (this->m_EmissionWavelength * 1e-9);

    double NA = this->m_NumericalAperture;
    double M  = this->m_Magnification;

    // Assumes classical 160mm tube length.
    this->m_A = 0.160 * NA / sqrt(M*M - NA*NA);
  }

  /** Point-spread function model parameters. */
  double    m_EmissionWavelength;
  double    m_NumericalAperture;
  double    m_Magnification;
  double    m_DesignCoverSlipRefractiveIndex;
  double    m_ActualCoverSlipRefractiveIndex;
  double    m_DesignCoverSlipThickness;
  double    m_ActualCoverSlipThickness;
  double    m_DesignImmersionOilRefractiveIndex;
  double    m_ActualImmersionOilRefractiveIndex;
  double    m_DesignImmersionOilThickness;
  double    m_DesignSpecimenLayerRefractiveIndex;
  double    m_ActualSpecimenLayerRefractiveIndex;
  double    m_PointSourceDepthInSpecimenLayer;

  /** Precomputed values. */
  double m_K; // Wavenumber
  double m_A; // Radius of projection of the limiting aperture onto
              // the back focal plane of the objective lens

protected:
  /** Computes the optical path difference for a ray terminating at
  *   a normalized distance rho from the center of the back focal
  *   plane aperture. */
  inline ComplexType OPD(double rho, double dz) const
  {
    double NA      = this->m_NumericalAperture;
    double n_oil_d = this->m_DesignImmersionOilRefractiveIndex;
    double n_oil   = this->m_ActualImmersionOilRefractiveIndex;
    double t_oil_d = this->m_DesignImmersionOilThickness * 1e-6;
    double n_s     = this->m_ActualSpecimenLayerRefractiveIndex;
    double t_s     = this->m_PointSourceDepthInSpecimenLayer * 1e-6;
    double n_g_d   = this->m_DesignCoverSlipRefractiveIndex;
    double n_g     = this->m_ActualCoverSlipRefractiveIndex;
    double t_g_d   = this->m_DesignCoverSlipThickness * 1e-6;
    double t_g     = this->m_ActualCoverSlipThickness * 1e-6;

    ComplexType t1 = this->OPDTerm(rho, n_s,     t_s);
    ComplexType t2 = this->OPDTerm(rho, n_g,     t_g);
    ComplexType t3 = this->OPDTerm(rho, n_g_d,   t_g_d);
    ComplexType t4 = this->OPDTerm(rho, n_oil_d, t_oil_d);

    ComplexType c1 = n_oil * dz * sqrt(1.0 - ((NA*NA*rho*rho)/(n_oil*n_oil)));
    ComplexType result = c1 + t1 + t2 - t3 - t4;

    return result;
  }


  /** Common terms for computing the optical path difference term in
   *  point-spread function models descended from this class. */
  inline ComplexType OPDTerm(double rho, double n, double t) const
  {
    double NA    = this->m_NumericalAperture;
    double n_oil = this->m_ActualImmersionOilRefractiveIndex;
    double NA_rho_sq = NA*NA*rho*rho;
    double NA_rho_over_n_sq = NA_rho_sq/(n*n);
    double n_oil_over_n_sq = (n_oil*n_oil) / (n*n);
    double NA_rho_over_n_oil_sq = NA_rho_sq/(n_oil*n_oil);

    ComplexType sq1(1.0 - NA_rho_over_n_sq);
    sq1 = sqrt(sq1);

    ComplexType sq2(1.0 - NA_rho_over_n_oil_sq);
    sq2 = n_oil_over_n_sq * sqrt(sq2);

    ComplexType result = n*t*(sq1 - sq2);
    return result;
  }


};

} // end namespace Functor


/** \class DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource
 *
 *  \brief Base class for aberrated symmetric point-spread functions
 *  defined in terms of design and actual microscope parameters.
 *
 *  This class is the base class for point-spread function model
 *  sources that are rotationally symmetric about the z-axis and which are
 *  defined in terms of design parameters recommended by the
 *  microscope manufacturer and actual parameters reflecting the
 *  experimental conditions under which the point-spread function was
 *  acquired.
 *
 *  The output image must be 3D, and a float or double pixel type is
 *  recommended.
 *
 * \ingroup DataSources Multithreaded Microscopy
 */
template< class TOutputImage >
class ITK_EXPORT DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource :
    public ParametricImageSource< TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource Self;
  typedef ParametricImageSource< TOutputImage >            Superclass;
  typedef SmartPointer< Self >                             Pointer;
  typedef SmartPointer< const Self >                       ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef TOutputImage                             OutputImageType;
  typedef typename OutputImageType::PixelType      PixelType;
  typedef typename OutputImageType::IndexType      IndexType;
  typedef typename OutputImageType::RegionType     RegionType;
  typedef typename OutputImageType::PointType      PointType;
  typedef typename OutputImageType::PointValueType PointValueType;
  typedef typename OutputImageType::SpacingType    SpacingType;
  typedef typename OutputImageType::SizeType       SizeType;
  typedef typename OutputImageType::SizeValueType  SizeValueType;


  itkStaticConstMacro(ImageDimension, unsigned int,
		      TOutputImage::ImageDimension);

  /** Typedef for complex number. */
  typedef std::complex<double> ComplexType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource,
               ParametricImageSource);

  /** Type info for parameters. */
  typedef typename Superclass::ParametersType      ParametersType;
  typedef typename Superclass::ParametersValueType ParametersValueType;

  /** Type info for functor method. */
  typedef Functor::DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
    FunctorType;

  /** Set/get the size of the output image. */
  itkSetMacro(Size, SizeType);
  itkGetConstReferenceMacro(Size, SizeType);

  /** Set/get the spacing of the output image (in nanometers). */
  itkSetMacro(Spacing, SpacingType);
  itkGetConstReferenceMacro(Spacing, SpacingType);

  /** Set/get the origin of the output image (in nanometers). */
  itkSetMacro(Origin, PointType);
  itkGetConstReferenceMacro(Origin, PointType);

  /** Set/get the point center (in nanometers). */
  itkSetMacro(PointCenter, PointType);
  itkGetConstReferenceMacro(PointCenter,  PointType);

  /** Set/get the emission wavelength (in nanometers). */
  itkSetMacro(EmissionWavelength, double);
  itkGetConstMacro(EmissionWavelength, double);

  /** Set/get the numerical aperture (unitless). */
  itkSetMacro(NumericalAperture, double);
  itkGetConstMacro(NumericalAperture, double);

  /** Set/get the magnification (unitless). */
  itkSetMacro(Magnification, double);
  itkGetConstMacro(Magnification, double);

  /** Set/get the design cover slip refractive index (unitless). */
  itkSetMacro(DesignCoverSlipRefractiveIndex, double);
  itkGetConstMacro(DesignCoverSlipRefractiveIndex, double);

  /** Set/get the actual cover slip refractive index (unitless). */
  itkSetMacro(ActualCoverSlipRefractiveIndex, double);
  itkGetConstMacro(ActualCoverSlipRefractiveIndex, double);

  /** Set/get the design cover slip thickness (in micrometers). */
  itkSetMacro(DesignCoverSlipThickness, double);
  itkGetConstMacro(DesignCoverSlipThickness, double);

  /** Set/get the actual cover slip thickness (in micrometers). */
  itkSetMacro(ActualCoverSlipThickness, double);
  itkGetConstMacro(ActualCoverSlipThickness, double);

  /** Set/get the design immersion oil refractive index (unitless). */
  itkSetMacro(DesignImmersionOilRefractiveIndex, double);
  itkGetConstMacro(DesignImmersionOilRefractiveIndex, double);

  /** Set/get the actual immersion oil refractive index (unitless). */
  itkSetMacro(ActualImmersionOilRefractiveIndex, double);
  itkGetConstMacro(ActualImmersionOilRefractiveIndex, double);

  /** Set/get the design immersion oil thickness (in micrometers). */
  itkSetMacro(DesignImmersionOilThickness, double);
  itkGetConstMacro(DesignImmersionOilThickness, double);

  /** Set/get the design specimen layer refractive index (unitless). */
  itkSetMacro(DesignSpecimenLayerRefractiveIndex, double);
  itkGetConstMacro(DesignSpecimenLayerRefractiveIndex, double);

  /** Set/get the actual specimen layer refractive index (unitless). */
  itkSetMacro(ActualSpecimenLayerRefractiveIndex, double);
  itkGetConstMacro(ActualSpecimenLayerRefractiveIndex, double);

  /** Set/get the point source depth in the specimen layer (in
   *  microns). */
  itkSetMacro(PointSourceDepthInSpecimenLayer, double);
  itkGetConstMacro(PointSourceDepthInSpecimenLayer, double);

  /** Implementation of the SetParameters and GetParameters methods
   *  declared in the superclass. */
  virtual void SetParameters(const ParametersType& parameters);
  virtual ParametersType GetParameters() const;

  /** Get the total number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

protected:
  DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource();
  ~DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Integrates a one-dimensional function defined by a functor from
   *  a to b using 2*m+1 subdivisions. */
  template< class TFunctor >
  ComplexType IntegrateFunctor(const TFunctor& functor, double a, double b,
                               int m, double x, double y, double z)
  {
    int n = 2*m + 1;
    double h = 1.0 / static_cast<double>(n-1);

    double r = sqrt(x*x + y*y);

    // Initialize accumulator for integration.
    ComplexType sum(0.0, 0.0);

    // Compute initial terms in Simpson quadrature method.
    sum += functor(r, z, a);
    sum += functor(r, z, b);

    for (int k = 1; k <= m-1; k++)
      {
      sum += 2.0 * functor(r, z, (2*k)*h);
      }

    for (int k = 1; k <= m; k++)
      {
      sum += 4.0 * functor(r, z, (2*k-1)*h);
      }

    sum *= h / 3.0;

    return sum;
  }

  virtual void GenerateOutputInformation();

  /** Compute a sample of the point-spread function at a point. */
  virtual double ComputeSampleValue(const PointType& point) = 0;

  /** Approximate the integrated light intensity over a CCD pixel via
   *  point sampling. */
  double ComputeIntegratedPixelValue(const PointType& point);

protected:
  SizeType    m_Size;        // size of the output image
  SpacingType m_Spacing;     // spacing
  PointType   m_Origin;      // origin

  /** Point-spread function model parameters. */
  PointType m_PointCenter; // the center of the point source
  double    m_EmissionWavelength;
  double    m_NumericalAperture;
  double    m_Magnification;
  double    m_DesignCoverSlipRefractiveIndex;
  double    m_ActualCoverSlipRefractiveIndex;
  double    m_DesignCoverSlipThickness;
  double    m_ActualCoverSlipThickness;
  double    m_DesignImmersionOilRefractiveIndex;
  double    m_ActualImmersionOilRefractiveIndex;
  double    m_DesignImmersionOilThickness;
  double    m_DesignSpecimenLayerRefractiveIndex;
  double    m_ActualSpecimenLayerRefractiveIndex;
  double    m_PointSourceDepthInSpecimenLayer;

private:
  // purposely not implemented
  DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource(const DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource&);
  void operator=(const DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource&);

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource.txx"
#endif

#endif // __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_h
