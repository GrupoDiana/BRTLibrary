/**
* \class CEnviromentModelBase
*
* \brief Declaration of CEnviromentVirtualSourceBaseModel class
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

#ifndef _CENVIRONMENT_MODEL_BASE_HPP_
#define _CENVIRONMENT_MODEL_BASE_HPP_

#include <memory>
#include <Base/ModelBase.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <SourceModels/VirtualSourceModel.hpp>
#include <ServiceModules/Room.hpp>

namespace BRTSourceModel {
	class CSourceOmnidirectionalModel;
	class CSourceDirectivityModel;
}

namespace BRTEnvironmentModel {
	class CEnviromentModelBase : public BRTBase::CModelBase {
	public:
		
		// Virtual Methods
		virtual ~CEnviromentModelBase() { }		
				
		virtual void UpdateGain() = 0;

		virtual void EnableDirectPath() {};
		virtual void DisableDirectPath() {};
		virtual bool IsDirectPathEnabled() { return false; };

		virtual void EnableReverbPath() {};
		virtual void DisableReverbPath() {};
		virtual bool IsReverbPathEnabled() { return false; };		

		virtual void EnableDistanceAttenuation() {};
		virtual void DisableDistanceAttenuation() {};
		virtual bool IsDistanceAttenuationEnabled() { return false; };

		virtual void EnablePropagationDelay() {};
		virtual void DisablePropagationDelay() {};
		virtual bool IsPropagationDelayEnabled() { return false; };

		virtual bool SetRoom(std::shared_ptr<BRTServices::CRoom>) { return false; };
		virtual bool UpdateRoom() { return false; };
		virtual std::shared_ptr<BRTServices::CRoom> GetRoom() const { return nullptr; }		
		virtual void RemoveRoom() { };
						
		virtual bool SetDistanceAttenuationFactor(float _distanceAttenuationDB) { return false; };
		virtual float GetDistanceAttenuationFactor() { return 0; };
		
		virtual bool SetReferenceAttenuationDistance(float _referenceAttenuationDistance) { return false; };
		virtual float GetReferenceAttenuationDistance() { return 0; };


		virtual bool SetReflectionOrder(int _reflectionOrder) { return false; }
		virtual int GetReflectionOrder() { return 0; }

		virtual bool SetMaxDistanceSourcesToListener(float _maxDistanceSourcesToListener) { return false; }
		virtual float GetMaxDistanceSourcesToListener() { return 0; }

		virtual bool SetTransitionMeters(float _transitionMeters) { return false; }
		virtual float GetTransitionMeters() { return 0; }

		virtual size_t GetNumberOfVirtualSources() { return 0; }
		virtual size_t GetNumberOfActiveVirtualSources() { return 0; }

		virtual bool ConnectSoundSource(const std::string & _sourceID) { return false; }
		virtual bool DisconnectSoundSource(const std::string & _sourceID) { return false; }	

		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) { return false; }
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) { return false; }	
		
		CEnviromentModelBase(const std::string & _environmentModelID)
			: CModelBase(_environmentModelID) {

			
			CreateIDExitPoint();			
			CreateIDEntryPoint("listenerModelID");
			GetIDExitPoint()->sendData(modelID);
			CreateCommandEntryPoint();
		}
		
		/**
		 * @brief Set the gain of the model
		 * @param _gain New gain value
		 */
		void SetGain(float _gain) override = 0;
		/*void SetGain(float _gain) override {
			gain = _gain;
			UpdateGain();
		}*/

		/**
		 * @brief Check if this environment is already connected to a listener model
		 * @return 
		 */
		bool IsConnectedToListenerModel() {
			std::string _listenerModelID = GetIDEntryPoint("listenerModelID")->GetData();
			return (_listenerModelID != "");			
		}


		/////////////////////
		// Update Callbacks
		/////////////////////		

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {
			// Nothing to do
		}
		
		void UpdateCommand() override {
			// Nothing to do
		}		

	private:
		
	protected:
		//Common::CRoom roomDefinition;
	};
}
#endif