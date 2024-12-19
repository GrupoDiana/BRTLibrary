/**
* \class CQuaternion
*
* \brief Definition of Quaternions (representation of orientation).
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


#ifndef _CQUATERNION_HPP_
#define _CQUATERNION_HPP_

//#include <Common/Vector3.h>
#include <iostream>

namespace Common {

	/** \details This class declares the necessary algorithms and vars for Quaternions (representation of orientation).
	*/
	class CQuaternion
	{
		// METHODS
	public:

		//static const CQuaternion ZERO;                         ///< Basic predefined ZERO quaternion (0, 0, 0, 0)
		//static const CQuaternion UNIT;                         ///< Basic predefined UNIT quaternion (1, 0, 0, 0)
				
	//
	// Constructors/Destructors
	//
		/** \brief Default constructor
		*	\details By default, sets a \link UNIT \endlink quaternion
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion()
		{
			//*this = CQuaternion::UNIT;
			*this = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}

		/** \brief Constructor from components
		*	\param [in] _w w component (scalar part)
		*	\param [in] _x x component (vector part)
		*	\param [in] _y y component (vector part)
		*	\param [in] _z z component (vector part)
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion(float _w, float _x, float _y, float _z)
		{
			w = _w;
			x = _x;
			y = _y;
			z = _z;
		}

		/** \brief Constructor from components
		*	\param [in] _w w component (scalar part)
		*	\param [in] _v vector part
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion(float _w, CVector3 _v)
		{
			w = _w;
			x = _v.x;
			y = _v.y;
			z = _v.z;
		}

		/** \brief Constructor from vector
		*	\details Builds a quaternion without scalar part
		*	\param [in] _vector vector components (x, y, z)
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion(CVector3 _vector)
		{
			w = 0.0f;
			x = _vector.x;
			y = _vector.y;
			z = _vector.z;
		}

		/** \brief Constructor from scalar
		*	\details Builds a quaternion without vector part
		*	\param [in] _scalar scalar component (w)
		*   \eh Nothing is reported to the error handler.
		*/		
		CQuaternion(float _scalar)
		{
			w = _scalar;
			x = y = z = 0.0f;
		}


		
		//
		// Conversions (We can implement more if we need them)
		//

			/** \brief Get a quaternion from an axis and angle representation
			*	\param [in] _axis rotation axis
			*	\param [in] _angle amount or angle of rotation, in radians
			*	\retval quaternion quaternion equivalent to the given axis-angle representation
			*   \eh Nothing is reported to the error handler.
			*/		
		static CQuaternion FromAxisAngle(CVector3& _axis, float _angle)
		{
			CVector3 _axisNormalized = _axis.Normalize();
			// Error handler:
			//SET_RESULT(RESULT_OK, "Conversion from axis-angle to quaternion was succesfull");

			float newW, newX, newY, newZ;

			float halfAngle = _angle * 0.5f;
			float halfSin = std::sin(halfAngle);
			float halfCos = std::cos(halfAngle);

			newW = halfCos;
			newX = halfSin * _axisNormalized.x;
			newY = halfSin * _axisNormalized.y;
			newZ = halfSin * _axisNormalized.z;

			return CQuaternion(newW, newX, newY, newZ);
		}

		/** \brief Get axis and angle representation from a quaternion
		*	\param [out] _axis rotation axis
		*	\param [out] _angle amount or angle of rotation, in radians
		*   \eh Warnings may be reported to the error handler
		*/		
		void ToAxisAngle(CVector3& _axis, float& _angle)
		{
			float sqrLength = x * x + y * y + z * z;
			if (sqrLength > 0.0f)
			{
				// Error handler:
				//SET_RESULT(RESULT_OK, "Conversion from quaternion to axis-angle was succesfull");

				float invLength = 1.0f / std::sqrt(sqrLength);
				_axis.SetCoordinates(x * invLength, y * invLength, z * invLength);
				_angle = 2.0f * std::acos(w);
			}
			else
			{
				// Error handler:
				SET_RESULT(RESULT_WARNING, "Converting to axis/angle from zero quaternion returns an arbitrary axis");

				_axis.SetCoordinates(1.0f, 0.0f, 0.0f); // Any axis is valid for a 0 angle
				_angle = 0.0f;
			}
		}

		/** \brief Get a quaternion from a roll-pitch-yaw representation
		*   \details This representation corresponds to intrinsic Tait-Bryan angles with sequence: yaw-pitch-roll
		*	\param [in] _roll roll angle in radians
		*	\param [in] _pitch pitch angle in radians
		*	\param [in] _yaw yaw angle in radians
		*	\retval quaternion quaternion equivalent to the given roll-pitch-yaw representation
		*   \eh Nothing is reported to the error handler.
		*/		
		static CQuaternion FromYawPitchRoll(float _yaw, float _pitch, float _roll)
		{
			double t0 = std::cos(_yaw * 0.5f);
			double t1 = std::sin(_yaw * 0.5f);
			double t2 = std::cos(_roll * 0.5f);
			double t3 = std::sin(_roll * 0.5f);
			double t4 = std::cos(_pitch * 0.5f);
			double t5 = std::sin(_pitch * 0.5f);

			float newW = t0 * t2 * t4 + t1 * t3 * t5;
			float newForward = t0 * t3 * t4 - t1 * t2 * t5;
			float newRight = t0 * t2 * t5 + t1 * t3 * t4;
			float newDown = t1 * t2 * t4 - t0 * t3 * t5;

			// Create vector part of quaternion, for convention-independent operations
			CVector3 vectorPart = CVector3::ZERO();
			vectorPart.SetAxis(UP_AXIS, -newDown);
			vectorPart.SetAxis(RIGHT_AXIS, newRight);
			vectorPart.SetAxis(FORWARD_AXIS, newForward);
			return CQuaternion(newW, vectorPart);
		}

		/** \brief Get roll-pitch-yaw representation from a quaternion
		*   \details This representation corresponds to intrinsic Tait-Bryan angles with sequence: yaw-pitch-roll
		*	\param [out] roll roll angle in radians
		*	\param [out] pitch pitch angle in radians
		*	\param [out] yaw yaw angle in radians
		*   \eh Nothing is reported to the error handler.
		*/		
		void ToYawPitchRoll(float& yaw, float& pitch, float& roll)
		{
			// Get vector part of quaternion and extract up, forward and right axis values
			CVector3 vectorPart = CVector3(x, y, z);
			float up = vectorPart.GetAxis(UP_AXIS);
			float right = vectorPart.GetAxis(RIGHT_AXIS);
			float forward = vectorPart.GetAxis(FORWARD_AXIS);
			float down = -up;

			// roll (forward-axis rotation)
			double t0 = 2.0f * (w * forward + right * down);
			double t1 = 1.0f - 2.0f * (forward * forward + right * right);
			roll = std::atan2(t0, t1);

			// pitch (right-axis rotation)
			double t2 = 2.0f * (w * right - down * forward);
			t2 = t2 > 1.0f ? 1.0f : t2;
			t2 = t2 < -1.0f ? -1.0f : t2;
			pitch = std::asin(t2);

			// yaw (up-axis rotation)
			double t3 = 2.0f * (w * down + forward * right);
			double t4 = 1.0f - 2.0f * (right * right + down * down);
			yaw = std::atan2(t3, t4);
		}
		
		//////////////////////
		// Basic operations
		//////////////////////

		/** \brief Rotate quaternion with another quaternion
		*	\details Equivalent to quaternion product
		*	\param [in] _rightHand other quaternion
		*   \eh Nothing is reported to the error handler.
		*/		
		void Rotate(CQuaternion _rightHand)
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Quaternion rotated succesfully");

			CQuaternion thisCopy = *this;
			*this = _rightHand * thisCopy;
		}

		/** \brief Rotate a vector with another quaternion
		*	\param [in] _vector vector to rotate
		*	\retval rotated rotated vector
		*   \eh Nothing is reported to the error handler.
		*/		
		const CVector3 RotateVector(CVector3 _vector) const
		{
			// Error handler:
			// Trust in Inverse for setting result

			// Convert vector into quaternion, forcing quaternion axis convention
			CQuaternion vectorQuaternion = CQuaternion(_vector);

			// Left product
			CQuaternion leftProduct = *this * vectorQuaternion;

			// Right product
			CQuaternion rightProduct = leftProduct * Inverse();

			// Convert result quaternion into vector
			CVector3 result = CVector3(rightProduct.x, rightProduct.y, rightProduct.z);

			return result;
		}

		//
		// Operands
		//

			/** \brief Quaternion product
			*	\details Quaternion product is not commutative. Used for rotating.
			*/		
		const CQuaternion operator* (const CQuaternion _rightHand) const
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Quaternion product operation succesfull.");

			float newW = w * _rightHand.w - x * _rightHand.x - y * _rightHand.y - z * _rightHand.z;
			float newX = w * _rightHand.x + x * _rightHand.w + y * _rightHand.z - z * _rightHand.y;
			float newY = w * _rightHand.y + y * _rightHand.w + z * _rightHand.x - x * _rightHand.z;
			float newZ = w * _rightHand.z + z * _rightHand.w + x * _rightHand.y - y * _rightHand.x;
			return CQuaternion(newW, newX, newY, newZ);
		}

		/** \brief Get the quaternion inverse
		*	\retval inverse inverse of quaternion
		*   \throws May throw warnings to debugger
		*   \eh Warnings may be reported to the error handler
		*/		
		const CQuaternion Inverse() const
		{
			// Error handler:
			float norm = SqrNorm();		// Not completely sure that we can use SqrNorm instead of Norm...
			if (norm == 0.0f)
			{
				SET_RESULT(RESULT_WARNING, "Computing inverse of quaternion with zero norm (returns ZERO quaternion)");
				//return CQuaternion::ZERO;
				return ZERO();
			}
			//else		
			//	SET_RESULT(RESULT_OK, "Inverse of quaternion was computed succesfully");

			float invNorm = 1.0f / norm;

			float newW = w * invNorm;
			float newX = -x * invNorm;
			float newY = -y * invNorm;
			float newZ = -z * invNorm;

			return CQuaternion(newW, newX, newY, newZ);
		}

		/** \brief Get the quaternion norm
		*	\retval norm norm of quaternion
		*   \eh Nothing is reported to the error handler.
		*/		
		float Norm()
		{
			// Error handler: 
			// Trust in SqrNorm for setting result

			return std::sqrt(SqrNorm());
		}

		/** \brief Get the squared quaternion norm
		*	\details To avoid computing square roots
		*	\retval sqrnorm squared norm of quaternion
		*   \eh Nothing is reported to the error handler.
		*/		
		const float SqrNorm() const
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Norm computed succesfully for quaternion");

			return w * w + x * x + y * y + z * z;
		}

		/** \brief Get the pitch angle in radians
		*	\retval pitch pitch angle in radians
		*   \eh Nothing is reported to the error handler.
		*/		
		float GetPitch()
		{
			return std::asin(-2.0 * (x * z - w * y));
		}

		/** \brief Get the roll angle in radians
		*	\retval roll roll angle in radians
		*   \eh Nothing is reported to the error handler.
		*/		
		float GetRoll()
		{
			return std::atan2(2.0 * (x * y + w * z), w * w + x * x - y * y - z * z);
		}

		static CQuaternion ZERO() { return CQuaternion(0.0f, 0.0f, 0.0f, 0.0f); }
		static CQuaternion UNIT() { return CQuaternion(1.0f, 0.0f, 0.0f, 0.0f); }

		// ATTRIBUTES
	public:
		float w;    ///< w component of the quaternion (scalar part)
		float x;    ///< x component of the quaternion (vector part)
		float y;    ///< y component of the quaternion (vector part)
		float z;    ///< z component of the quaternion (vector part)
	};

	/** \brief Formatted stream output of quaternions for debugging
	*/
	inline std::ostream & operator<<(std::ostream & out, const CQuaternion & q)
	{
		out << "<" << q.w << ", (" << q.x << ", " << q.y << ", " << q.z << ")>";
		return out;
	}


	//////////////////////////////////////////////
	// BASIC PREDEFINED QUATERNIONS
	//////////////////////////////////////////////	
	
	//const CQuaternion CQuaternion::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
	//const CQuaternion CQuaternion::UNIT(1.0f, 0.0f, 0.0f, 0.0f);
	//const CQuaternion CQuaternion::ZERO = CQuaternion(0.0f, 0.0f, 0.0f, 0.0f);///< Basic predefined ZERO quaternion (0, 0, 0, 0)	
	//const CQuaternion CQuaternion::UNIT = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);///< Basic predefined UNIT quaternion (1, 0, 0, 0)*/

}
#endif