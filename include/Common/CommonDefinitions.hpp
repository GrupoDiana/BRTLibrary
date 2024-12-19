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
#include <Common/Vector3.hpp>

namespace Common {

	//inline constexpr std::string_view COMMAND_EXIT_POINT_ID{ "command" };
	//inline constexpr std::string_view COMMAND_ENTRY_POINT_ID{ "command" };
	const char COMMAND_EXIT_POINT_ID[] = "command";
	const char COMMAND_ENTRY_POINT_ID[] = "command";


	//----------------------------------------------------------------------
	/** \brief Type definition for specifying one ear
	*/
	enum T_ear {
		LEFT = 0,	///< Left ear
		RIGHT = 1,	///< Right ear
		BOTH = 2,	///< Both ears
		NONE = 3	///< No ear
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
	class CEarPair
	{
	public:
		T left;		///< left channel
		T right;	///< right channel
	};

	static bool AreSameDouble(double a, double b, double epsilon)
	{
		//float absA = fabs(a);
		//float absB = fabs(b);
		float diff = std::fabs(a - b);

		return diff < epsilon;
	}

	static bool AreSame(float a, float b, float epsilon)
	{
		//float absA = fabs(a);
		//float absB = fabs(b);
		float diff = std::fabs(a - b);

		return diff < epsilon;
	}

	static bool AreSame(Common::CVector3 a, Common::CVector3 b, float epsilon)
	{
		return AreSame(a.x, b.x, epsilon) && AreSame(a.y, b.y, epsilon) && AreSame(a.z, b.z, epsilon);
	}

	/** \brief This method check if a number is a power of 2
		*	\param [in] integer to check
		*	\param [out] return true if the number is power of two
		*/
	static bool CalculateIsPowerOfTwo(int x)
	{
		return (x != 0) && ((x & (x - 1)) == 0);
	}

	
	/**
	 * @brief This method Round up to the next highest power of 2 
	 * @param integer to check 
	 * @return next highest power of 2
	*/
	static int CalculateNextPowerOfTwo(int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
}
#endif