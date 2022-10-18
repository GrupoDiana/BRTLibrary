/**
* \class CVector3
*
* \brief  Declaration of CVector3 class interface.
* \date	July 2016
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#ifndef _CVECTOR3_H_
#define _CVECTOR3_H_

//#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??	// TODO not working in visual 2019 //TO FIXME
#include <cmath>
#include <iostream>
#include <Common/ErrorHandler.h>
#include <Common/Conventions.h>
#include <string>

const double PI_D = 3.141592653589793238463;
const float  PI_F = 3.14159265358979f;
//const float  M_PI = 3.14159265358979f;
#define M_PI 3.14159265358979			// TODO change this //TO FIXME
constexpr float _2PI = 2.0f * M_PI;


namespace Common {

	/** \brief Acos without risk*/
	inline double SafeAcos(double x)
	{
		if (x < -1.0) x = -1.0;
		else if (x > 1.0) x = 1.0;
		return std::acos(x);
	}

	/** \details This class declares the vars and methods for handling 3D Vectors.
	*/
	class CVector3
	{
		// METHODS
	public:

		//static const CVector3 ZERO;		///< Predefined ZERO vector (0, 0, 0)

	//
	// Predefined rotation axis for rotating in basic directions, using angle-axis rotation
	//

		//static const CVector3 TO_LEFT;       ///< Predefined Left rotation axis, for angle-axis rotation
		//static const CVector3 TO_RIGHT;      ///< Predefined Right rotation axis, for angle-axis rotation
		//static const CVector3 TO_UP;         ///< Predefined Up rotation axis, for angle-axis rotation
		//static const CVector3 TO_DOWN;       ///< Predefined Down rotation axis, for angle-axis rotation

		//static const CVector3 TO_ROLL_LEFT;  ///< Predefined forward rotation axis, for angle-axis rotation
		//static const CVector3 TO_ROLL_RIGHT; ///< Predefined backward rotation axis, for angle-axis rotation

	//
	// Constructors/Destructors
	//

		/** \brief Default constructor
		*	\details By default, sets \link ZERO \endlink vector
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3()
		{
			*this = CVector3::ZERO();
		}

		/** \brief Constructor from array
		*	\param [in] _xyzArray array with the 3 vector components (x, y, z)
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3(float _xyzArray[3])
		{
			x = _xyzArray[0];
			y = _xyzArray[1];
			z = _xyzArray[2];
		}

		/** \brief Constructor from components
		*	\param [in] _x x vector component
		*	\param [in] _y y vector component
		*	\param [in] _z z vector component
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		//
		// Get methods
		//

		/** \brief Get distance (vector modulus)
		*	\retval distance vector modulus
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetDistance() const
		{
			// Error handler: trust in GetSqrDistance
			// Sqrt may set errno if the argument is negative, but we have full control on this argument and we know that it can never be negative		
			return std::sqrt(GetSqrDistance());
		}

		/** \brief Get squared distance
		*	\details To avoid computing square roots
		*	\retval distance squared vector modulus
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetSqrDistance() const
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Distance computed succesfully.");

			//return (x*x + y*y + z*z);
			return (x * x + y * y + z * z);
		}

		/** \brief Get elevation in radians
		*	\details Elevation to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.
		*	\retval elevation elevation, in radians
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetElevationRadians() const
		{
			// Error handler:
			float distance = GetDistance();
			if (distance == 0.0f)
			{
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
				return 0.0f;
			}
			//else
			//	SET_RESULT(RESULT_OK, "Elevation computed from vector succesfully");	// No more possible errors. 

			// 0=front; 90=up; -90=down
			//float cosAngle = *upAxis / GetDistance(); // Error check: division by zero
			//float angle = SafeAcos(cosAngle);
			//return (M_PI / 2.0f) - angle;

			// 0=front; 90=up; 270=down (LISTEN)
			float cosAngle = GetAxis(UP_AXIS) / distance;
			float angle = SafeAcos(cosAngle);
			float adjustedAngle = (M_PI * 2.5f) - angle;

			// Check limits (always return 0 instead of 2PI)
			if (adjustedAngle >= _2PI)
				adjustedAngle = std::fmod(adjustedAngle, (float)_2PI);

			return adjustedAngle;
		}

		/** \brief Get azimuth in radians
		*	\details Azimuth to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for for azimuth angles: anti-clockwise full circle starting with 0º in front.
		*	\retval azimuth azimuth, in radians
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAzimuthRadians() const
		{
			// Error handler:
			float rightAxis = GetAxis(RIGHT_AXIS);
			float forwardAxis = GetAxis(FORWARD_AXIS);
			if ((rightAxis == 0.0f) && (forwardAxis == 0.0f))
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Azimuth cannot be computed for a (0,0,z) vector. 0.0 is returned");
				return 0.0f;
			}

			// front=0; left=-90; right=90
			//return atan2(*rightAxis, *forwardAxis);		

			//front=0; left=90; right=270 (LISTEN)
			float angle = std::atan2(GetAxis(RIGHT_AXIS), GetAxis(FORWARD_AXIS));
			float adjustedAngle = std::fmod((float)(_2PI - angle), (float)_2PI);

			// Check limits (always return 0 instead of 2PI)
			if (adjustedAngle >= _2PI)
				adjustedAngle = std::fmod(adjustedAngle, (float)_2PI);

			return adjustedAngle;
		}

		/** \brief Get elevation in degrees
		*	\details Elevation to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.
		*	\retval elevation elevation, in degrees
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetElevationDegrees() const
		{
			// Error handler:
			// Trust in GetElevationRadians for setting result

			return GetElevationRadians() * (180.0f / M_PI);
		}

		/** \brief Get azimuth in degrees
		*	\details Azimuth to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for for azimuth angles: anti-clockwise full circle starting with 0º in front.
		*	\retval azimuth azimuth, in degrees
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAzimuthDegrees() const
		{
			// Error handler:
			// Trust in GetAzimuthRadians for setting result

			return GetAzimuthRadians() * (180.0f / M_PI);
		}
		/** \brief Set the x,y,z coordinates from azimuth, elevation and distance.
		*    Currently uses LISTEN database convention for azimuth angles : anti - clockwise full circle starting with 0 degrees in front.
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromAED(float azimuth, float elevation, float distance)
		{
			azimuth = azimuth * (M_PI / 180.0f);
			elevation = elevation * (M_PI / 180.0f);

			float up = std::sin(elevation);

			float pd = std::cos(elevation); // pd -> projected distance (on the horizontal plane).

			float right = -pd * std::sin(azimuth);   // minus sign to fit the LISTEN database convention
			float forward = pd * std::cos(azimuth);

			SetAxis(UP_AXIS, up * distance);
			SetAxis(RIGHT_AXIS, right * distance);
			SetAxis(FORWARD_AXIS, forward * distance);
		}

		/** \brief Get the interaural azimuth angle in radians, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural azimuth angle of a vector (sometimes called interaural angle) is the angle between that vector and the sagittal (median) plane.
		*	\retval interauralAzimuth interaural azimuth in radians, from -PI/2 (left) to PI/2 degrees (right). 0 radians means the sagittal (median) plane
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetInterauralAzimuthRadians() const
		{
			float distance = GetDistance();
			if (distance == 0.0f)
			{
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
				return 0.0f;
			}
			//else
			//	SET_RESULT(RESULT_OK, "Interaural azimuth computed from vector succesfully");	// No more possible errors. 

			float f = GetAxis(FORWARD_AXIS);
			float u = GetAxis(UP_AXIS);
			float r = GetAxis(RIGHT_AXIS);
			float angle = SafeAcos(std::sqrt(f * f + u * u) / distance);

			return r > 0 ? angle : -angle;
		}

		/** \brief Get the interaural elevation angle in radians, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural elevation angle of a vector (sometimes called polar angle) is the rotation angle around the intearural axis.
		*	\retval interauralElevation interaural elevation in radians, from 0 to 2*PI.
		*	0 radians means the forward axial (horizontal) semiplane.
		*   PI/2 means the upper coronal (frontal) semiplane.
		*   PI means the backward axial (horizontal) semiplane.
		*   3/2*PI means the lower coronal (frontal) semiplane.
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetInterauralElevationRadians() const
		{
			float distance = GetDistance();
			if (distance == 0.0f)
			{
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
				return 0.0f;
			}
			//else
			//	SET_RESULT(RESULT_OK, "Interaural elevation computed from vector succesfully");	// No more possible errors. 

			float angle = std::atan2(GetAxis(UP_AXIS), GetAxis(FORWARD_AXIS));
			return angle >= 0 ? angle : angle + 2.0 * M_PI;
		}

		/** \brief Get the interaural azimuth angle in degrees, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural azimuth angle of a vector (sometimes called interaural angle) is the angle between that vector and the sagittal (median) plane.
		*	\retval interauralAzimuth interaural azimuth from -90 degrees (left) to +90 degrees (right). 0 degrees means the sagittal (median) plane
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetInterauralAzimuthDegrees() const
		{
			// Error handler:
			// Trust in GetInterauralAzimutRadians for setting result
			return GetInterauralAzimuthRadians() * (180.0f / M_PI);
		}

		/** \brief Get the interaural elevation angle in degrees, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural elevation angle of a vector (sometimes called polar angle) is the rotation angle around the intearural axis.
		*	\retval interauralElevation interaural elevation in degrees, from 0 to 360.
		*	0 degrees means the forward axial (horizontal) semiplane.
		*   90 degrees means the upper coronal (frontal) semiplane.
		*   180 degrees means the backward axial (horizontal) semiplane.
		*   270 degrees means the lower coronal (frontal) semiplane.
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetInterauralElevationDegrees() const
		{
			// Error handler:
			// Trust in GetInterauralElevationRadians for setting result
			return GetInterauralElevationRadians() * (180.0f / M_PI);
		}

		/** \brief Get the angle that this vector keeps with the forward axis.
		*	\details
		*	\retval This angle in degrees.
		*   \throws May throw errors to debugger
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAngleToForwardAxisDegrees() const
		{
			return GetAngleToForwardAxisRadians() * (180.0f / M_PI);
		}

		/** \brief Get the angle that this vector keeps with the forward axis.
		*	\details
		*	\retval This angle in radians.
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetAngleToForwardAxisRadians() const
		{
			float distance = GetDistance();
			if (distance == 0.0f)
			{
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
				return 0.0f;
			}

			float f = GetAxis(FORWARD_AXIS);
			float angle = SafeAcos(f / distance);

			return f >= 0 ? angle : angle + M_PI * 0.5;
		}

		/** \brief Get the value of a given axis, in accordance with the axis convention
		*	\details This method is convention-safe
		*	\param [in] _axis which axis
		*	\retval value value of axis component
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetAxis(TAxis _axis) const
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Succesfully got axis from convention.");

			switch (_axis)
			{
			case AXIS_X: return x; break;
			case AXIS_Y: return y; break;
			case AXIS_Z: return z; break;
			case AXIS_MINUS_X: return -x; break;
			case AXIS_MINUS_Y: return -y; break;
			case AXIS_MINUS_Z: return -z; break;
			default: SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to get an axis which name is not defined");  return 0.0f;
			}
		}

		/** \brief Set the value of a given axis, in accordance with the axis convention
		*	\details This method is convention-safe
		*	\param [in] _axis which axis
		*	\param [in] value value of axis component
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetAxis(TAxis _axis, float value)
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Succesfully set axis from convention.");

			switch (_axis)
			{
			case AXIS_X: x = value; break;
			case AXIS_Y: y = value; break;
			case AXIS_Z: z = value; break;
			case AXIS_MINUS_X: x = -value;	break;
			case AXIS_MINUS_Y: y = -value;	break;
			case AXIS_MINUS_Z: z = -value;	break;
			default: SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to set an axis which name is not defined");
			}
		}

		/** \brief Set the three components of the vector
		*	\param [in] _x x component
		*	\param [in] _y y component
		*	\param [in] _z z component
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCoordinates(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		//
		// Basic vector operators
		//

		/** \brief Component-wise substraction
		*/
		CVector3 operator-(CVector3 const _rightHand)
		{
			return CVector3(x - _rightHand.x, y - _rightHand.y, z - _rightHand.z);
		}

		/** \brief Component-wise addition
		*/
		const CVector3 operator+(CVector3 const _rightHand) const
		{
			return CVector3(x + _rightHand.x, y + _rightHand.y, z + _rightHand.z);
		}

		/** \brief Computes the vector dot product
		*	\param [in] _rightHand other vector
		*	\retval product dot product of this vector with other vector
		*   \eh Nothing is reported to the error handler.
		*/
		float DotProduct(CVector3 _rightHand)
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Dot product computed succesfully");

			return (x * _rightHand.x + y * _rightHand.y + z * _rightHand.z);
		}

		/** \brief Computes the vector cross product
		*	\param [in] _rightHand other vector
		*	\retval product cross product of this vector with other vector
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3 CrossProduct(CVector3 _rightHand)
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Cross product computed sucessfully");

			CVector3 result;

			result.x = y * _rightHand.z - z * _rightHand.y;
			result.y = z * _rightHand.x - x * _rightHand.z;
			result.z = x * _rightHand.y - y * _rightHand.x;

			return result;
		}
			
		static CVector3 ZERO() { return CVector3(0.0f, 0.0f, 0.0f); }

		//////////////////////////////////////////////
		// Predefined rotation axis for rotating in basic directions, using angle-axis rotation
		//////////////////////////////////////////////
#if AZIMUTH_MOTION == ANTICLOCKWISE
#if UP_AXIS == AXIS_Y		
		//const CVector3 CVector3::TO_LEFT(0.0f, -1.0f, 0.0f);
		//const CVector3 CVector3::TO_RIGHT(0.0f, 1.0f, 0.0f);
		static CVector3 TO_LEFT() { return CVector3(0.0f, -1.0f, 0.0f); }
		static CVector3 TO_RIGHT() { return CVector(0.0f, 1.0f, 0.0f); }
#elif UP_AXIS == AXIS_X
		//const CVector3 CVector3::TO_LEFT(1.0f, 0.0f, 0.0f);
		//const CVector3 CVector3::TO_RIGHT(-1.0f, 0.0f, 0.0f);
		static CVector3 TO_LEFT() { return CVector3(1.0f, 0.0f, 0.0f); }
		static CVector3 TO_RIGHT() { return CVector3(-1.0f, 0.0f, 0.0f); }
#elif UP_AXIS == AXIS_Z
		//	const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, 1.0f);
		//	const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, -1.0f);
		static CVector3 TO_LEFT() { return CVector3(0.0f, 0.0f, 1.0f); }
		static CVector3 TO_RIGHT() { return CVector3(0.0f, 0.0f, -1.0f); }
#elif UP_AXIS == AXIS_MINUS_Z
		//const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, -1.0f);
		//const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, 1.0f);
		static CVector3 TO_LEFT() { return CVector3(0.0f, 0.0f, -1.0f); }
		static CVector3 TO_RIGHT() { return CVector3(0.0f, 0.0f, 1.0f); }
#endif
		// TO DO: cases for -X and -Y
#elif AZIMUTH_MOTION == CLOCKWISE
#if UP_AXIS == AXIS_Y
		//const CVector3 CVector3::TO_LEFT(0.0f, -1.0f, 0.0f);
		//const CVector3 CVector3::TO_RIGHT(0.0f, 1.0f, 0.0f);
		static CVector3 TO_LEFT() { return CVector3(0.0f, -1.0f, 0.0f); }
		static CVector3 TO_RIGHT() { return CVector3(0.0f, 1.0f, 0.0f); }
#elif UP_AXIS == AXIS_X
		//const CVector3 CVector3::TO_LEFT(-1.0f, 0.0f, 0.0f);
		///const CVector3 CVector3::TO_RIGHT(1.0f, 0.0f, 0.0f);
		static CVector3 TO_LEFT() { return CVector3(-1.0f, 0.0f, 0.0f); }
		static CVector3 TO_RIGHT() {
			return CVector3(1.0f, 0.0f, 0.0f);
#elif UP_AXIS == AXIS_Z
		//const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, -1.0f);
		//const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, 1.0f);
		static  CVector3 TO_LEFT() { return CVector3(0.0f, 0.0f, -1.0f); }
		static  CVector3 TO_RIGHT() { return CVector3(0.0f, 0.0f, 1.0f); }
#endif
		// TO DO: more cases
#endif

#if ELEVATION_MOTION == ANTICLOCKWISE
#if RIGHT_AXIS == AXIS_X
		//const CVector3 CVector3::TO_UP(1.0f, 0.0f, 0.0f);
		//const CVector3 CVector3::TO_DOWN(-1.0f, 0.0f, 0.0f);
		static CVector3 TO_UP() { return CVector3(1.0f, 0.0f, 0.0f); }
		static CVector3 TO_DOWN() { return CVector3(-1.0f, 0.0f, 0.0f); }
#elif RIGHT_AXIS == AXIS_Y
		//const CVector3 CVector3::TO_UP(0.0f, 1.0f, 0.0f);
		//const CVector3 CVector3::TO_DOWN(0.0f, -1.0f, 0.0f);
		static CVector3 TO_UP() { return CVector3(0.0f, 1.0f, 0.0f); }
		static CVector3 TO_DOWN() { return CVector3(0.0f, -1.0f, 0.0f); }
#elif RIGHT_AXIS == AXIS_Z
		//const CVector3 CVector3::TO_UP(0.0f, 0.0f, 1.0f);
		//const CVector3 CVector3::TO_DOWN(0.0f, 0.0f, -1.0f);
		static CVector3 TO_UP() {	return CVector3(0.0f, 0.0f, 1.0f);	}
		static CVector3 TO_DOWN() {	return CVector3(0.0f, 0.0f, -1.0f);	}
#elif RIGHT_AXIS == AXIS_MINUS_Y
		//const CVector3 CVector3::TO_UP(0.0f, -1.0f, 0.0f);
		//const CVector3 CVector3::TO_DOWN(0.0f, 1.0f, 0.0f);
		static CVector3 TO_UP() {	return CVector3(0.0f, -1.0f, 0.0f); }
		static CVector3 TO_DOWN() {	return CVector3(0.0f, 1.0f, 0.0f);	}
#endif
#elif ELEVATION_MOTION == CLOCKWISE
#if RIGHT_AXIS == AXIS_X
		//const CVector3 CVector3::TO_UP(-1.0f, 0.0f, 0.0f);
		//const CVector3 CVector3::TO_DOWN(1.0f, 0.0f, 0.0f);
		static CVector3 TO_UP() {	return CVector3(-1.0f, 0.0f, 0.0f);	}
		static CVector3 TO_DOWN() {	return CVector3(1.0f, 0.0f, 0.0f);	}
#elif RIGHT_AXIS == AXIS_Y
		//const CVector3 CVector3::TO_UP(0.0f, -1.0f, 0.0f);
		//const CVector3 CVector3::TO_DOWN(0.0f, 1.0f, 0.0f);
		static CVector3 TO_UP() {	return CVector3(0.0f, -1.0f, 0.0f);	}
		static CVector3 TO_DOWN() {	return CVector3(0.0f, 1.0f, 0.0f);	}
#elif RIGHT_AXIS == AXIS_Z
		//const CVector3 CVector3::TO_UP(0.0f, 0.0f, -1.0f);
		//const CVector3 CVector3::TO_DOWN(0.0f, 0.0f, 1.0f);
		static CVector3 TO_UP() {	return CVector3(0.0f, 0.0f, -1.0f);	}
		static CVector3 TO_DOWN() {	return CVector3(0.0f, 0.0f, 1.0f);	}
#endif
#endif
		
#if AZIMUTH_MOTION == ANTICLOCKWISE
#if FORWARD_AXIS == AXIS_MINUS_Y
		//const CVector3 CVector3::TO_ROLL_LEFT(0.0f, -1.0f, 0.0f);
		//const CVector3 CVector3::TO_ROLL_RIGHT(0.0f, 1.0f, 0.0f);
		static CVector3 TO_ROLL_LEFT() {	return CVector3(0.0f, -1.0f, 0.0f); }
		static CVector3 TO_ROLL_RIGHT() {	return CVector3(0.0f, 1.0f, 0.0f); }
#elif FORWARD_AXIS == AXIS_X
		//const CVector3 CVector3::TO_ROLL_LEFT(1.0f, 0.0f, 0.0f);
		//const CVector3 CVector3::TO_ROLL_RIGHT(-1.0f, 0.0f, 0.0f);
		static CVector3 TO_ROLL_LEFT() {	return CVector3(1.0f, 0.0f, 0.0f);	}
		static CVector3 TO_ROLL_RIGHT() {	return CVector3(-1.0f, 0.0f, 0.0f);	}
#endif
		// TO DO: more cases 
#elif AZIMUTH_MOTION == CLOCKWISE
		// TO DO: more cases
#endif

		// ATTRIBUTES
	public:
		float x;   ///< x component of the vector
		float y;   ///< y component of the vector
		float z;   ///< z component of the vector	
	};

	/** \brief Formatted stream output of vectors for debugging */
	inline std::ostream& operator<<(std::ostream& out, const CVector3& v)
	{
		out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return out;
	}

}//end namespace Common
#endif