/**
* \class CSourceSimpleModel
*
* \brief Declaration of CSourceSimpleModel class
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
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SOUND_SOURCE_BASIC_MODEL_HPP
#define _SOUND_SOURCE_BASIC_MODEL_HPP

#include <SourceModels/SourceModelBase.hpp>
#include <vector>

namespace BRTSourceModel {
	class CSourceSimpleModel : public CSourceModelBase {

	public:			
		CSourceSimpleModel(std::string _sourceID)
			: CSourceModelBase(_sourceID, TSourceType::Simple) {			
		}


	private:		
		/**
		 * @brief Actions when the entry points are ready
		 * @param _entryPointID 
		 */
		void Update(std::string _entryPointID) override {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {
				CMonoBuffer<float> buffer = GetBuffer();
				SendData(buffer);
			}
		}

		/**
		* @brief Implementation of the virtual method for processing the received commands
		* The SourceModelBase class already handles the common commands. Here you have to manage the specific ones.
		*/
		void UpdateCommandSource() override {
			// Nothing extra to do
		}
	};
}
#endif