/**
* \class CommonDefinitions
*
* \brief Declaration of CommonDefinitions
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

#ifndef _COMMON_DEFINITIONS_HPP_
#define _COMMON_DEFINITIONS_HPP_

#include <cmath>

namespace Common {

//inline constexpr std::string_view COMMAND_EXIT_POINT_ID{ "command" };
//inline constexpr std::string_view COMMAND_ENTRY_POINT_ID{ "command" };
const char COMMAND_EXIT_POINT_ID[] = "command";
const char COMMAND_ENTRY_POINT_ID[] = "command";

constexpr double PI_D = 3.141592653589793238463;
constexpr float PI_F = 3.14159265358979f;
constexpr float _2PI_F = 2.0f * PI_F;

//----------------------------------------------------------------------
/** \brief Type definition for specifying one ear
	*/
enum T_ear {
	LEFT = 0, ///< Left ear
	RIGHT = 1, ///< Right ear
	BOTH = 2, ///< Both ears
	NONE = 3 ///< No ear
};

//----------------------------------------------------------------------

/* By default, the UPC algorithm is apllied. If the following defines are activated, the basic convolution will be applied. */
//#define USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC		//Fconvolver witouth UPC algorithms in the anechoic path
//#define USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB			//Fconvolver witouth UPC algorithms in the reverb path

//----------------------------------------------------------------------

/* \brief Declaration of CEarPair class to work with objects that must be duplicated in order to work with
	*        left and right channels.
	*/
template <class T>
class CEarPair {
public:
	T left; ///< left channel
	T right; ///< right channel
};

/**
 * @brief Defines normalization schemes for ambisonic audio signals.
 */
enum TAmbisonicNormalization { none,
	N3D,
	SN3D,
	maxN };

/** \brief This method check if a number is a power of 2
		*	\param [in] integer to check
		*	\param [out] return true if the number is power of two
		*/
static bool CalculateIsPowerOfTwo(int x) {
	return (x != 0) && ((x & (x - 1)) == 0);
}

/**
	 * @brief This method Round up to the next highest power of 2 
	 * @param integer to check 
	 * @return next highest power of 2
	*/
static int CalculateNextPowerOfTwo(int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

/**
	 * @brief This method checks if two floating point numbers are almost equal, considering both absolute and relative tolerances.
	 * @tparam type T
	 * @param a First floating point number.
	 * @param b Second floating point number.
	 * @param relEpsilon relative comparison tolerance
	 * @param absEpsilon absolute comparison tolerance
	 * @return 
	 */
template <typename T>
bool almostEqual(T a, T b, T relEpsilon, T absEpsilon) {
	T diff = std::fabs(a - b);

	// Absolute comparison (near 0)
	if (diff <= absEpsilon) return true;

	// Relative comparison (for large values)
	return diff <= relEpsilon * std::max(std::fabs(a), std::fabs(b));
}

/**
	 * @brief This method checks if two floating point numbers are almost equal, considering both absolute and relative tolerances.
	 * @details The method uses default tolerances based on the type of floating point number (float, double, long double).
	 * @tparam type T
	 * @param a First floating point number.
	 * @param b Second floating point number.
	 * @return true if the numbers are almost equal, false otherwise.
	 */
template <typename T>
bool almostEqual(T a, T b) {
	T relEpsilon;
	T absEpsilon;

	if constexpr (std::is_same_v<T, float>) {
		relEpsilon = 1e-5f;
		absEpsilon = 1e-8f;
	} else if constexpr (std::is_same_v<T, double>) {
		relEpsilon = 1e-12;
		absEpsilon = 1e-15;
	} else if constexpr (std::is_same_v<T, long double>) {
		relEpsilon = 1e-15L;
		absEpsilon = 1e-18L;
	} else {
		// generic fallback: use epsilon of the implementation
		relEpsilon = std::numeric_limits<T>::epsilon();
		absEpsilon = std::numeric_limits<T>::epsilon();
	}

	T diff = std::fabs(a - b);

	// Absolute comparison (for small values, close to 0)
	if (diff <= absEpsilon) {
		return true;
	}

	// Relative comparison (for large values)
	return diff <= relEpsilon * std::max(std::fabs(a), std::fabs(b));
}

/**
	 * @brief Determines if the first value is strictly greater than the second, excluding cases where they are almost equal.
	 * @tparam T The type of the values to compare.
	 * @param a The first value to compare.
	 * @param b The second value to compare.
	 * @return True if 'a' is greater than 'b' and they are not almost equal; otherwise, false.
	 */
template <typename T>
bool is_greater(T a, T b) {
	return (a > b) && !almostEqual(a, b);
}

/**
	 * @brief Determines if the first value is greater than or approximately equal to the second value.
	 * @tparam T The type of the values to compare.
	 * @param a The first value to compare.
	 * @param b The second value to compare.
	 * @return True if the first value is greater than or approximately equal to the second value; otherwise, false.
	 */
template <typename T>
bool is_greater_or_equal(T a, T b) {
	return (a > b) || almostEqual(a, b);
}

static bool AreSameDouble(double a, double b, double epsilon) {
	//float diff = std::fabs(a - b);
	//return diff < epsilon;
	return almostEqual<double>(a, b, 0.0, epsilon); // I only use absolute comparison to ensure that it works exactly the same as before.
}

static bool AreSame(float a, float b, float epsilon) {
	//float diff = std::fabs(a - b);
	//return diff < epsilon;
	return almostEqual(a, b, 0.0f, epsilon); //I only use absolute comparison to ensure that it works exactly the same as before.
}
}
#endif