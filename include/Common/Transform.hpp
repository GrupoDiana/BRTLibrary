/**
* \class CTransform
*
* \brief Definition of CTransform interfaces
* \date	June 2023
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

#ifndef _CTRANSFORM_HPP_
#define _CTRANSFORM_HPP_

#include <Common/Vector3.hpp>
#include <Common/Quaternion.hpp>
#include <Common/Conventions.hpp>

namespace Common {

	/** \details This class holds data and algorithms for rigid transformations (position and orientation).
	*/
	class CTransform
	{
		// METHODS
	public:

		////////////////////////////////
		// Constructors/Destructors
		///////////////////////////////

		/** \brief Default constructor
		*	\details By default, sets position to (0,0,0) and orientation towards the forward vector (front).
		*   \eh Nothing is reported to the error handler.
		*/		
		CTransform()
		{
			position = CVector3::ZERO();
			orientation = CQuaternion::UNIT();			
		}

		CTransform(CVector3 _position)
		{
			position = _position;			
		}

		////////////////////////////////
		// Get methods
		////////////////////////////////

			/** \brief Get a vector from "this" to target in "this" reference frame
			*	\param [in] target target transform
			*	\retval vector vector from this transform to target transform, in the reference frame of this transform
			*   \throws May throw warnings to debugger
			*   \eh Nothing is reported to the error handler.
			*/		
		const CVector3 GetVectorTo(CTransform target) const
		{
			// Error handler:
			// Trust in RotateVector for setting result

			// Get position of target in global reference frame
			CVector3 targetPositionGlobal = target.GetPosition();

			// Translate target until "this" reference frame has its origin in the global origin
			targetPositionGlobal = targetPositionGlobal - position;

			// Find new coordinates in "this" frame (rotated with respect to global)
			CVector3 targetPositionThis = orientation.Inverse().RotateVector(targetPositionGlobal);

			return targetPositionThis;
		}

		/** \brief Get the position component
		*	\retval position vector containing the position
		*   \eh Nothing is reported to the error handler.
		*/		
		CVector3 GetPosition() const
		{
			return position;
		}

		/** \brief Get the orientation component
		*	\retval orientation quaternion containing the orientation
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion GetOrientation() const
		{
			return orientation;
		}

		////////////////////////////////
		// Set methods
		////////////////////////////////

		/** \brief Set the position component
		*	\param [in] _position vector containing the position
		*   \eh Nothing is reported to the error handler.
		*/		
		void SetPosition(CVector3 _position)
		{
			position = _position;
		}

		/** \brief Set the orientation component
		*	\param [in] _orientation quaternion containing the orientation
		*   \eh Nothing is reported to the error handler.
		*/		
		void SetOrientation(CQuaternion _orientation)
		{
			orientation = _orientation;
		}

		////////////////////////////////
		// Transform methods
		////////////////////////////////

		/** \brief Applies a translation to the current position
		*	\param [in] _translation vector of translation
		*   \eh Nothing is reported to the error handler.
		*/		
		void Translate(CVector3 _translation)
		{
			position = position + _translation;
		}

		/** \brief Applies a rotation to the current orientation
		*	\details Rotation of a given angle with respect to a given axis
		*	\param [in] _axis axis of rotation
		*	\param [in] _angle amount or angle of rotation, in radians
		*   \eh Nothing is reported to the error handler.
		*/		
		void Rotate(CVector3 _axis, float _angle)
		{
			// Error handler:
			// Trust in FromAxisAngle and Rotate for setting result

			CQuaternion rotation = CQuaternion::FromAxisAngle(_axis, _angle);
			orientation.Rotate(rotation);
		}

		/** Returns a new transform with a local translation applied to the position
		*	\param [in] _translation vector of translation
		*	\retval translation position obtained after local translation
		*   \eh Nothing is reported to the error handler.
		*/		
		const CTransform GetLocalTranslation(CVector3 _translation) const
		{
			// Error handler: trust in called methods for setting result

			CTransform result;
			CVector3 newGlobalPosition = orientation.RotateVector(_translation) + position;
			result.SetPosition(newGlobalPosition);
			result.SetOrientation(orientation);
			return result;
		}

		// ATTRIBUTES
	private:
		CVector3 position;
		CQuaternion orientation;
	};
}//end namespace Common
#endif
