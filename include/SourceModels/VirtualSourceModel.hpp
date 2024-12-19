/**
* \class CVirtualSourceModel
*
* \brief Declaration of CVirtualSourceModel class
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

#ifndef _SOUND_SOURCE_VIRTUAL_MODEL_HPP
#define _SOUND_SOURCE_VIRTUAL_MODEL_HPP

//#include <SourceModels/SourceModelBase.hpp>
#include <SourceModels/SourceSimpleModel.hpp>
#include <vector>

namespace BRTSourceModel {
	class CVirtualSourceModel : public CSourceSimpleModel {

	public:			

		CVirtualSourceModel(std::string _sourceID)
			: CSourceSimpleModel(_sourceID)
			, originSourceID {""} {
			SetSourceType(TSourceType::Virtual);
		}		
									
		void SetOriginSourceID(std::string _originSourceID) {
			if (originSourceID == "") {
				originSourceID = _originSourceID;
			}			
		}

		std::string GetOriginSourceID() {
			return originSourceID;
		}
	private:				
		std::string originSourceID;
	};
}
#endif