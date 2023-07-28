/**
* \class CExampleEnvironment
*
* \brief Declaration of CExampleEnvironment class
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

#ifndef _CEXAMPLE_ENVIRONMENT_HPP_
#define _CEXAMPLE_ENVIRONMENT_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>

namespace BRTEnvironmentModel {


	class CExampleEnvironment {
	public:

		CExampleEnvironment() {
			
		}

		void SetDelayBuffer(int _delayBufferSize) { 
			delayBuffer.clear();
			delayBuffer.resize(_delayBufferSize, 0);
		}
		void SetParemeter(int _whatever) { 
		
		}

		void Process(CMonoBuffer<float>& _inBuffer, Common::CTransform sourcePosition, Common::CTransform listenerPosition, std::vector<CMonoBuffer<float>>& _outBuffers, std::vector<Common::CTransform>& _virtualSourcePositions) {
			
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");


			//Dumb virtual source position 
			Common::CTransform virtualSourceTransform = sourcePosition;
			Common::CVector3 virtualSourcePosition = virtualSourceTransform.GetPosition();
			virtualSourcePosition.y *= -1;
			virtualSourceTransform.SetPosition(virtualSourcePosition);
			for (int i = 0; i < 6; i++) {
				_virtualSourcePositions.push_back(virtualSourceTransform);
			}
			
			

			//Dumb samples processing. I just delay the signals and copy them to all the virtual ones.
			
			CMonoBuffer<float> tempOutBuffer;
			tempOutBuffer.reserve(globalParameters.GetBufferSize());
			tempOutBuffer.insert(tempOutBuffer.begin(), delayBuffer.begin(), delayBuffer.begin() + globalParameters.GetBufferSize());			
			tempOutBuffer.ApplyGain(0.6f);
			for (int i = 0; i < 6; i++) {
				_outBuffers.push_back(tempOutBuffer);				
			}
			
			// Update delay Buffer
			std::vector<float> temp;
			temp.reserve(delayBuffer.size());
			//tempOutBuffer.insert(tempOutBuffer.begin(), delayBuffer.end() - (delayBuffer.size() - _inBuffer.size()), delayBuffer.end());
			temp.insert(temp.begin(), delayBuffer.begin() + _inBuffer.size(), delayBuffer.end());
			temp.insert(temp.end(), _inBuffer.begin(), _inBuffer.end());
			delayBuffer = temp;
			
		}

		


	private:
		std::vector<float> delayBuffer;

		Common::CGlobalParameters globalParameters;

	};

};
#endif