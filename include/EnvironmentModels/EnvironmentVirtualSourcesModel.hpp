/**
* \class CEnviromentVirtualSourcesModel
*
* \brief Declaration of CEnviromentVirtualSourcesModel class
* \date	June 2023
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
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _CENVIRONMENT_MODEL_BASE_HPP_
#define _CENVIRONMENT_MODEL_BASE_HPP_

#include <memory>
#include <Base/EnvironmentModelBase.hpp>


namespace BRTEnvironmentModel {

	class CEnviromentVirtualSourceExampleModel : public BRTBase::CEnviromentVirtualSourceBaseModel {
	public:
		
		CEnviromentVirtualSourceExampleModel(BRTBase::CBRTManager* _brtManager)  : BRTBase::CEnviromentVirtualSourceBaseModel(_brtManager){
		
			/*CreateVirtualSource("virtual1");
			CreateVirtualSource("virtual2");
			CreateVirtualSource("virtual3");
			CreateVirtualSource("virtual4");
			CreateVirtualSource("virtual5");
			CreateVirtualSource("virtual6");*/
		}


		void Update(std::string _entryPointID) {
			if (_entryPointID == "inputSamples") {
				std::cout << "Process" << std::endl;
			}
		}


		
		void UpdateCommand() {

		}

	private:



	};



};
#endif