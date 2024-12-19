/**
* \class CDirectivityTFDefinitions
*
* \brief Declaration of CDirectivityTFDefinitions class 
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
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/


#ifndef _CDIRECTIVITYTF_DEFINITIONS_HPP
#define _CDIRECTIVITYTF_DEFINITIONS_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <ServiceModules/ServicesBase.hpp>
//#include <ServiceModules/HRTFDefinitions.hpp>


namespace BRTServices
{
	//struct TDirectivityInterlacedTFStruct {
	//	CMonoBuffer<float> data;
	//};
	struct TDirectivityInterlacedTFStruct {
		std::vector<CMonoBuffer<float>> data;
	};

	/** \brief Type definition for the DirectivityTF table */
	typedef std::unordered_map<orientation, BRTServices::TDirectivityTFStruct> T_DirectivityTFTable;
	typedef std::unordered_map<orientation, BRTServices::TDirectivityInterlacedTFStruct> T_DirectivityTFInterlacedDataTable;

}


#endif