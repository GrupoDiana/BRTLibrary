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
#include <ServiceModules/ServicesBase.hpp>

namespace BRTServices {


#ifndef PI 
#define PI 3.14159265
#endif
#ifndef DEFAULT_RESAMPLING_STEP
#define DEFAULT_GRIDSAMPLING_STEP 5
#endif

#ifndef DEFAULT_HRTF_MEASURED_DISTANCE
#define DEFAULT_HRTF_MEASURED_DISTANCE 1.95f
#endif

#ifndef DEFAULT_EXTRAPOLATION_STEP
#define DEFAULT_EXTRAPOLATION_STEP 10
#endif

	///** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
	//*/
	//struct THRIRPartitionedStruct {
	//	uint64_t leftDelay;				///< Left delay, in number of samples
	//	uint64_t rightDelay;			///< Right delay, in number of samples
	//	std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
	//	std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data

	//	THRIRPartitionedStruct() : leftDelay{ 0 }, rightDelay{ 0 } {}
	//};

	/** \brief Type definition for an impulse response with the ITD removed and stored in a specific struct field
	*/
	struct TOneEarHRIR_struct {
		uint64_t delay;				///< Delay, in number of samples
		CMonoBuffer<float> HRIR;	///< Impulse response data
	};

	/** \brief Type definition for an impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	//struct TOneEarHRIRPartitionedStruct {
	//	std::vector<CMonoBuffer<float>> HRIR_Partitioned;	///< Partitioned impulse response data
	//	uint64_t delay;										///< Delay, in number of samples
	//	
	//	TOneEarHRIRPartitionedStruct() {
	//		delay = 0;
	//	}
	//};


	/** \brief Type definition for table HRTF, this is the one read from the SOFA file.
	*/
	typedef std::unordered_map<orientation, BRTServices::THRIRStruct> T_HRTFTable;

	/** \brief Type definition for the HRTF table, this is the one our grid has and is the one used for rendering.
	*/
	typedef std::unordered_map<orientation, THRIRPartitionedStruct> T_HRTFPartitionedTable;

	/**
	 * @brief Type definition for table HRBRIR, this is the one read from the SOFA file.
	 */
	typedef std::unordered_map<TVector3, T_HRTFTable> T_HRBRIRTable;

	/**
	 * @brief Type definition for the HRBRIR table, this is the one our grid has and is the one used for rendering.
	 */
	typedef std::unordered_map<TVector3, T_HRTFPartitionedTable> T_HRBRIRPartitionedTable;

}
#endif