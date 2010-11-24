/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHaeberlePSFImageSource.h,v $
  Language:  C++
  Date:      $Date: 2010/04/19 18:50:02 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkHaeberlePSFImageSource_h
#define __itkHaeberlePSFImageSource_h

#include <complex>

#include "itkParametricImageSource.h"
#include "itkNumericTraits.h"

namespace itk
{

/** \class HaeberlePSFImageSource
 * \brief Generate a synthetic point-spread function according to the
 * Haeberle model.
 *
 * The Haeberle point-spread function model is based on the vectorial
 * model of light propagation in widefield fluorescence
 * microscopes. This image source generates images according to this
 * IMPORTANT: Please pay attention to the units each method
 * expects. Some take nanometers, some take micrometers, and some
 * take millimeters.
 *
 * \ingroup DataSources Multithreaded
 */
template <class TOutputImage>
class ITK_EXPORT HaeberlePSFImageSource :
  public ParametricImageSource<TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef HaeberlePSFImageSource              Self;
  typedef ParametricImageSource<TOutputImage> Superclass;
  typedef SmartPointer<Self>                  Pointer;
  typedef SmartPointer<const Self>            ConstPointer;

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

  /** Typedef for complex type. */
  typedef std::complex<double> ComplexType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(HaeberlePSFImageSource, ParametricImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;

  /** Specify the size of the output image. */
  virtual void SetSize(const SizeType & size)
  {
    if (size != m_Size)
      {
      m_Size = size;
      this->Modified();
      }
  }

  /** Get the size of the output image. */
  itkGetConstReferenceMacro(Size, SizeType);

  /** Specify the spacing of the output image (in nanometers). */
  virtual void SetSpacing(const SpacingType & spacing)
  {
    if (spacing != m_Spacing)
      {
      m_Spacing = spacing;
      this->Modified();
      }
  }

  /** Get the spacing of the output image (in nanometers). */
  itkGetConstReferenceMacro(Spacing, SpacingType);

  /** Specify the origin of the output image (in nanometers). */
  virtual void SetOrigin(const PointType & origin)
  {
    if (origin != m_Origin)
      {
      m_Origin = origin;
      this->Modified();
      }
  }

  /** Get the origin of the output image (in nanometers). */
  itkGetConstReferenceMacro(Origin, PointType);

  /** Specify the point source center (in nanometers). */
  virtual void SetPointCenter(const PointType & center)
  {
    if (center != m_PointCenter)
      {
      m_PointCenter = center;
      this->Modified();
      }
  }

  /** Get the point source center (in nanometers). */
  itkGetConstReferenceMacro(PointCenter, PointType);

  /** Specify the X shear. */
  itkSetMacro(ShearX, double);

  /** Get the X shear. */
  itkGetConstMacro(ShearX, double);

  /** Specify the Y shear. */
  itkSetMacro(ShearY, double);

  /** Get the Y shear. */
  itkGetConstMacro(ShearY, double);

  /** Specify the emission wavelength (in nanometers). */
  itkSetMacro(EmissionWavelength, double);

  /** Get the emission wavelength (in nanometers). */
  itkGetConstMacro(EmissionWavelength, double);

  /** Specify the numerical aperture (unitless). */
  itkSetMacro(NumericalAperture, double);

  /** Get the numerical aperture (unitless). */
  itkGetConstMacro(NumericalAperture, double);

  /** Specify the magnification (unitless). */
  itkSetMacro(Magnification, double);

  /** Get the magnification (unitless). */
  itkGetConstMacro(Magnification, double);

  /** Specify the cover slip refractive index (unitless). */
  itkSetMacro(CoverSlipRefractiveIndex, double);

  /** Get the cover slip refractive index (unitless). */
  itkGetConstMacro(CoverSlipRefractiveIndex, double);

  /** Specify the cover slip thickness (in micrometers). */
  itkSetMacro(CoverSlipThickness, double);

  /** Get the cover slip thickness (in micrometers). */
  itkGetConstMacro(CoverSlipThickness, double);

  /** Specify the immersion oil refractive index (unitless). */
  itkSetMacro(ImmersionOilRefractiveIndex, double);

  /** Get the immersion oil refractive index (unitless). */
  itkGetConstMacro(ImmersionOilRefractiveIndex, double);

  /** Specify the immersion oil thickness (in micrometers). */
  itkSetMacro(ImmersionOilThickness, double);

  /** Get the immersion oil refractive index (in micrometers). */
  itkGetConstMacro(ImmersionOilThickness, double);

  /** Specify the specimen layer refractive index (unitless). */
  itkSetMacro(SpecimenLayerRefractiveIndex, double);

  /** Get the specimen layer refractive index (unitless). */
  itkGetConstMacro(SpecimenLayerRefractiveIndex, double);

  /** Expects the parameters argument to contain values for ALL parameters. */
  virtual void SetParameters(const ParametersType& parameters);

  /** Gets the full parameters list. */
  virtual ParametersType GetParameters() const;

  /** Gets the total number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

protected:
  HaeberlePSFImageSource();
  ~HaeberlePSFImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  virtual void
  ThreadedGenerateData(const RegionType& outputRegionForThread, int threadId );
  virtual void GenerateOutputInformation();

  /** Computes the light intensity at a specified point. */
  double ComputeSampleValue(PointType& point);

  /** Computes the integrated light intensity over a CCD pixel centered at
      point. */
  double ComputeIntegratedPixelValue(PointType& point);

private:
  HaeberlePSFImageSource(const HaeberlePSFImageSource&); //purposely not implemented
  void operator=(const HaeberlePSFImageSource&); //purposely not implemented

  SizeType    m_Size;        // size of the output image

  SpacingType m_Spacing;     // spacing
  PointType   m_Origin;      // origin
  PointType   m_PointCenter; // the center of the point source
  double      m_ShearX;      // Shear in the x-direction with respect to z
  double      m_ShearY;      // Shear in the y-direction with respect to z

  /** Point-spread function model parameters. */
  double m_EmissionWavelength;
  double m_NumericalAperture;
  double m_Magnification;
  double m_CoverSlipRefractiveIndex;
  double m_CoverSlipThickness;
  double m_ImmersionOilRefractiveIndex;
  double m_ImmersionOilThickness;
  double m_SpecimenLayerRefractiveIndex;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHaeberlePSFImageSource.txx"
#endif

#endif
