/**
* \class CSourceListenerRelativePositionCalculation
*
* \brief Declaration of CSourceListenerRelativePositionCalculation class interface.
* \date	November 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CSOURCE_LISTENER_RELATIVE_POSITION_CALCULATION_HPP_
#define _CSOURCE_LISTENER_RELATIVE_POSITION_CALCULATION_HPP_

#include <Common/Buffer.hpp>
#include <ServiceModules/HRTF.hpp>

#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace Common {
	class CSourceListenerRelativePositionCalculation {
	public:
				
		/**
		 * @brief Calculates the parameters derived from the source and listener position
		 * @param _sourceTransform source transform
		 * @param _listenerTransform listener transform
		 * @param _listenerHRTF listener HRTF
		 * @param leftElevation left ear elevation
		 * @param leftAzimuth left ear azimuth
		 * @param rightElevation right ear elevation
		 * @param rightAzimuth right ear azimuth
		 * @param centerElevation center head elevation
		 * @param centerAzimuth center head azimuth
		 * @param interauralAzimuth interaural azimuth
		*/
		static void CalculateSourceListenerRelativePositions(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, std::shared_ptr<BRTServices::CServicesBase>& _listenerHRTF, bool parallaxCorrection,float& leftElevation, float& leftAzimuth, float& rightElevation, float& rightAzimuth, float& centerElevation, float& centerAzimuth, float& interauralAzimuth)
		{

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();

			//Check listener and source are in the same position
			if (_distanceToListener <= MINIMUM_DISTANCE_SOURCE_LISTENER) {
				SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CHRTFConvolver");
				_distanceToListener = MINIMUM_DISTANCE_SOURCE_LISTENER;
			}
			
			// Calculate center head location			
			centerElevation = _vectorToListener.GetElevationDegrees();		//Get elevation from the head center
			if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, centerElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, centerElevation, EPSILON))
			{
				centerAzimuth = _vectorToListener.GetAzimuthDegrees();		//Get azimuth from the head center
			}

			interauralAzimuth = _vectorToListener.GetInterauralAzimuthDegrees();	//Get Interaural Azimuth

			// Calculate ears location
			if (parallaxCorrection) {
				Common::CVector3 leftEarLocalPosition = _listenerHRTF->GetEarLocalPosition(Common::T_ear::LEFT);
				Common::CVector3 rightEarLocalPosition = _listenerHRTF->GetEarLocalPosition(Common::T_ear::RIGHT);
				Common::CTransform leftEarTransform = _listenerTransform.GetLocalTranslation(leftEarLocalPosition);
				Common::CTransform rightEarTransform = _listenerTransform.GetLocalTranslation(rightEarLocalPosition);

				Common::CVector3 leftVectorTo = leftEarTransform.GetVectorTo(_sourceTransform);
				Common::CVector3 rightVectorTo = rightEarTransform.GetVectorTo(_sourceTransform);
				Common::CVector3 leftVectorTo_sphereProjection = GetSphereProjectionPosition(leftVectorTo, leftEarLocalPosition, _listenerHRTF->GetHRTFDistanceOfMeasurement());
				Common::CVector3 rightVectorTo_sphereProjection = GetSphereProjectionPosition(rightVectorTo, rightEarLocalPosition, _listenerHRTF->GetHRTFDistanceOfMeasurement());

				leftElevation = leftVectorTo_sphereProjection.GetElevationDegrees();	//Get left elevation
				if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, leftElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, leftElevation, EPSILON))
				{
					leftAzimuth = leftVectorTo_sphereProjection.GetAzimuthDegrees();	//Get left azimuth
				}

				rightElevation = rightVectorTo_sphereProjection.GetElevationDegrees();	//Get right elevation	
				if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, rightElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, rightElevation, EPSILON))
				{
					rightAzimuth = rightVectorTo_sphereProjection.GetAzimuthDegrees();		//Get right azimuth
				}
			}
			else {
				leftAzimuth = centerAzimuth;
				rightAzimuth = centerAzimuth;
				leftElevation = centerElevation;
				rightElevation = centerElevation;
			}

		}
	

		static float CalculateSourceListenerDistance(Common::CTransform _sourceTransform, Common::CTransform _listenerTransform) {			
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();
			return _distanceToListener;
		}
	
	private:

		// In orther to obtain the position where the HRIR is needed, this method calculate the projection of each ear in the sphere where the HRTF has been measured
		static const Common::CVector3 GetSphereProjectionPosition(Common::CVector3 vectorToEar, Common::CVector3 earLocalPosition, float distance)
		{
			//get axis according to the defined convention
			float rightAxis = vectorToEar.GetAxis(RIGHT_AXIS);
			float forwardAxis = vectorToEar.GetAxis(FORWARD_AXIS);
			float upAxis = vectorToEar.GetAxis(UP_AXIS);
			// Error handler:
			if ((rightAxis == 0.0f) && (forwardAxis == 0.0f) && (upAxis == 0.0f)) {
				ASSERT(false, RESULT_ERROR_DIVBYZERO, "Axes are not correctly set. Please, check axis conventions", "Azimuth computed from vector succesfully");
			}
			//get ear position in right axis
			float earRightAxis = earLocalPosition.GetAxis(RIGHT_AXIS);

			//Resolve a quadratic equation to get lambda, which is the parameter that define the line between the ear and the sphere, passing by the source
			// (x_sphere, y_sphere, z_sphere) = earLocalPosition + lambda * vectorToEar 
			// x_sphere^2 + y_sphere^2 + z_sphere^2 = distance^2


			float a = forwardAxis * forwardAxis + rightAxis * rightAxis + upAxis * upAxis;
			float b = 2.0f * earRightAxis * rightAxis;
			float c = earRightAxis * earRightAxis - distance * distance;
			float lambda = (-b + sqrt(b * b - 4.0f * a * c)) * 0.5f * (1 / a);

			Common::CVector3 cartesianposition;

			cartesianposition.SetAxis(FORWARD_AXIS, lambda * forwardAxis);
			cartesianposition.SetAxis(RIGHT_AXIS, (earRightAxis + lambda * rightAxis));
			cartesianposition.SetAxis(UP_AXIS, lambda * upAxis);

			return cartesianposition;
		}

	};
}

#endif