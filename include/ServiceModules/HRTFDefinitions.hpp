/**
* \HRTF Definitions
*
* \brief Declaration of CHRTFAuxiliarMethods class interface
* \date	July 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga 2023. Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CHRTF_DEFINITIONS_HPP
#define _CHRTF_DEFINITIONS_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>

namespace BRTServices {


#ifndef PI 
#define PI 3.14159265
#endif
#ifndef DEFAULT_RESAMPLING_STEP
#define DEFAULT_RESAMPLING_STEP 5
#endif

#ifndef DEFAULT_HRTF_MEASURED_DISTANCE
#define DEFAULT_HRTF_MEASURED_DISTANCE 1.95f
#endif

#ifndef DEFAULT_EXTRAPOLATION_STEP
#define DEFAULT_EXTRAPOLATION_STEP 10
#endif

	/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct THRIRPartitionedStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
		std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data

		THRIRPartitionedStruct() : leftDelay{ 0 }, rightDelay{ 0 } {}
	};

	/** \brief Type definition for an impulse response with the ITD removed and stored in a specific struct field
	*/
	struct oneEarHRIR_struct {
		uint64_t delay;				///< Delay, in number of samples
		CMonoBuffer<float> HRIR;	///< Impulse response data
	};

	/** \brief Type definition for an impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct TOneEarHRIRPartitionedStruct {
		std::vector<CMonoBuffer<float>> HRIR_Partitioned;	///< Partitioned impulse response data
		uint64_t delay;				///< Delay, in number of samples
	};


	/** \brief Type definition for the HRTF table
	*/
	typedef std::unordered_map<orientation, BRTServices::THRIRStruct> T_HRTFTable;

	/** \brief Type definition for the HRTF partitioned table used when UPConvolution is activated
	*/
	typedef std::unordered_map<orientation, THRIRPartitionedStruct> T_HRTFPartitionedTable;


}
#endif