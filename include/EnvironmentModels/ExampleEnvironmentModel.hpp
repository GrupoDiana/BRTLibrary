/**
* \class CExampleEnvironmentModel
*
* \brief Declaration of CExampleEnvironmentModel class
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
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _CEXAMPLE_ENVIRONMENT_MODE_HPP_
#define _CEXAMPLE_ENVIRONMENT_MODE_HPP_

#include <memory>
#include <Base/EnvironmentModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <EnvironmentModels/ExampleEnvironment.hpp>

namespace BRTEnvironmentModel {

	class CExampleEnvironmentModel : public BRTBase::CEnviromentVirtualSourceBaseModel, CExampleEnvironment {
	public:
		
		CExampleEnvironmentModel(BRTBase::CBRTManager* _brtManager)  : BRTBase::CEnviromentVirtualSourceBaseModel(_brtManager){
		

			SetDelayBuffer(globalParameters.GetBufferSize() * 5);

			CreateVirtualSource("virtual1");
			CreateVirtualSource("virtual2");
			CreateVirtualSource("virtual3");
			CreateVirtualSource("virtual4");
			CreateVirtualSource("virtual5");
			CreateVirtualSource("virtual6");
			
		}


		void Update(std::string _entryPointID) {

			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "inputSamples") {
				// Get data from entry points
				CMonoBuffer<float> inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
				std::vector<CMonoBuffer<float>> virtualSourceBuffers;
				std::vector<Common::CTransform> virtualSourcePositions;

				if (inBuffer.size() != 0) {

					// Call your processing algorithm, as an example we have put it in another class although it could be put in this class. 
					Process(inBuffer, sourcePosition, listenerPosition, virtualSourceBuffers, virtualSourcePositions);

					// It's a mess, but you get the idea, right?
					SetVirtualSourceBuffer("virtual1", virtualSourceBuffers[0]);
					SetVirtualSourcePosition("virtual1", virtualSourcePositions[0]);

					/*SetVirtualSourceBuffer("virtual2", virtualSourceBuffers[1]);
					SetVirtualSourcePosition("virtual2", virtualSourcePositions[1]);

					SetVirtualSourceBuffer("virtual3", virtualSourceBuffers[2]);
					SetVirtualSourcePosition("virtual3", virtualSourcePositions[2]);

					SetVirtualSourceBuffer("virtual4", virtualSourceBuffers[3]);
					SetVirtualSourcePosition("virtual4", virtualSourcePositions[3]);

					SetVirtualSourceBuffer("virtual5", virtualSourceBuffers[4]);
					SetVirtualSourcePosition("virtual5", virtualSourcePositions[4]);

					SetVirtualSourceBuffer("virtual6", virtualSourceBuffers[5]);
					SetVirtualSourcePosition("virtual6", virtualSourcePositions[5]);*/
				}
				//this->resetUpdatingStack();
			}
		}


		
		void UpdateCommand() {

		}

	private:
		mutable std::mutex mutex;
		Common::CGlobalParameters globalParameters;


	};



};
#endif