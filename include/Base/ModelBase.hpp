/**
* \class CModelBase
*
* \brief This class contains all definitions and implementations common to BRT models.
* \date	Oct 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
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

#ifndef _C_MODEL_BASE_HPP_
#define _C_MODEL_BASE_HPP_

#include <string>
#include <Connectivity/BRTConnectivity.hpp>
#include "SourceModels/SourceOmnidirectionalModel.hpp"
#include <SourceModels/SourceDirectivityModel.hpp>
#include <SourceModels/VirtualSourceModel.hpp>


namespace BRTBase {
class CModelBase : public BRTConnectivity::CBRTConnectivity {
	
	protected:		
		std::string modelID; // Store unique model ID	
		bool enableModel; // Enable or disable the model
		float gain; // Output audio samples gain

	public:		

		CModelBase(const std::string _modelID)
			: modelID { _modelID }
			, enableModel { true }
			, gain { 1.0f } { }
		
		// Public Methods		
		std::string GetModelID() { return modelID; }

		// Virtual methods
		virtual void EnableModel() {};
		virtual void DisableModel() {};
		virtual bool IsModelEnabled() { return enableModel; }

		virtual void SetGain(float _gain) { gain = _gain; }
		virtual float GetGain() { return gain; }
		
		virtual bool IsConnectedToListener() { return false; }
		virtual bool IsConnectedToListenerModel() { return false; }

		std::vector<std::string> GetInputs() { return inputConnections; }
		std::vector<std::string> GetOutputs() { return outputConnections; }

		protected:

			void AddInputConnection(const std::string & _modelID) {
				inputConnections.push_back(_modelID);
			}
			void RemoveInputConnection(const std::string & _modelID) {
				inputConnections.erase(std::remove(inputConnections.begin(), inputConnections.end(), _modelID), inputConnections.end());
			}

			void AddOuputConnections(const std::string& _modelID) {
				outputConnections.push_back(_modelID);
			}
			void RemoveOutputConnections(const std::string& _modelID) {
				outputConnections.erase(std::remove(outputConnections.begin(), outputConnections.end(), _modelID), outputConnections.end());
			}

			std::vector<std::string> inputConnections;
			std::vector<std::string> outputConnections;
	};
}
#endif