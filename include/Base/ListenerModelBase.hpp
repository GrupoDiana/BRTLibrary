/**
* \class CListenerModelBase
*
* \brief Declaration of CListenerModelBase class
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

#ifndef _CLISTENER_MODEL_BASE_H_
#define _CLISTENER_MODEL_BASE_H_

#include <memory>
//#include <Base/EntryPointManager.hpp>
//#include <Base/CommandEntryPointManager.hpp>
#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/NFCFilters.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTSourceModel {
	class CSourceSimpleModel;
	class CSourceDirectivityModel;
}

namespace BRTBase {

	//enum class TListenerType { ListenerHRFTModel, ListenerAmbisonicHRTFModel, ListenerEnviromentBRIRModel };

	/**
	 * @brief This class looks for a method of identifying each model, so that we can then know which methods are called. 
	 * It is actually a provisional solution.
	 */
	class TListenerModelcharacteristics {
		bool supportHRTF;
		bool supportBRIR;
		bool ambisonic;
		
		bool nearFieldCompensation;
		bool parallaxCorrection;
		bool itdSimulation;

		bool configurableSpatialisation;
		bool configurableInterpolation;
	
	public:		
		TListenerModelcharacteristics() : supportHRTF{ false }, supportBRIR{ false }, ambisonic{ false }, nearFieldCompensation{ false }, 
			parallaxCorrection{ false }, itdSimulation{ false }, configurableSpatialisation{ false }, configurableInterpolation{ false } {}
		
		TListenerModelcharacteristics(bool _supportHRTF, bool _supportBRIR, bool _ambisonic, bool _nearFieldCompensation, bool _parallaxCorrection, bool _itdSimulation, bool _configurableSpatialisation, bool _configurableInterpolation) :
			supportHRTF{ _supportHRTF }, supportBRIR{ _supportBRIR }, ambisonic{ _ambisonic }, nearFieldCompensation{ _nearFieldCompensation }, parallaxCorrection{ _parallaxCorrection },
			itdSimulation{ _itdSimulation }, configurableSpatialisation{ _configurableSpatialisation }, configurableInterpolation{ _configurableInterpolation } {}

		bool SupportHRTF() { return supportHRTF; }
		bool SupportBRIR() { return supportBRIR; }
		bool IsAmbisonic() { return ambisonic; }
		bool SupportNearFieldCompensation() { return nearFieldCompensation; }
		bool SupportParallaxCorrection() { return parallaxCorrection; }
		bool SupportITDSimulation() { return itdSimulation; }
		bool SupportConfigurableSpatialisation() { return configurableSpatialisation; }
		bool SupportConfigurableInterpolation() { return configurableInterpolation; }
	};


	class CListenerModelBase: public CAdvancedEntryPointManager, public CExitPointManager/*, public CCommandEntryPointManager, public CEntryPointManager */{
	public:

		// Public Attributes
		bool enableModel;

		// Virtual Methods

		virtual ~CListenerModelBase() {}
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;		
		
		virtual bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) { return false; };				
		virtual std::shared_ptr < BRTServices::CHRTF> GetHRTF() const { return nullptr; }
		virtual void RemoveHRTF() {};
		
		virtual bool SetNearFieldCompensationFilters(std::shared_ptr< BRTServices::CNearFieldCompensationFilters > _listenerILD) { return false; };
		virtual std::shared_ptr < BRTServices::CNearFieldCompensationFilters> GetNearFieldCompensationFilters() const { return nullptr; }
		virtual void RemoveNearFierldCompensationFilters() {};
				
		virtual bool SetHRBRIR(std::shared_ptr< BRTServices::CHRBRIR > _listenerBRIR) { return false; };		        
		virtual std::shared_ptr < BRTServices::CHRBRIR> GetHRBRIR() const { return nullptr; };
		virtual void RemoveHRBRIR() {};		


		
		virtual void EnableModel() {  };
		virtual void DisableModel() {};
		virtual bool IsModelEnabled() { return enableModel; }


		virtual void EnableITDSimulation() {};
		virtual void DisableITDSimulation() {};
		virtual bool IsITDSimulationEnabled() { return false; }

		virtual void EnableNearFieldEffect() {};
		virtual void DisableNearFieldEffect() {};
		virtual bool IsNearFieldEffectEnabled() { return false; }
		
		virtual void EnableSpatialization() {};
		virtual void DisableSpatialization() {};
		virtual bool IsSpatializationEnabled() { return false; }

		virtual void EnableInterpolation() {};
		virtual void DisableInterpolation() {};
		virtual bool IsInterpolationEnabled() { return false; }

		virtual void EnableParallaxCorrection() {};
		virtual void DisableParallaxCorrection() {};
		virtual bool IsParallaxCorrectionEnabled() { return false; }

		virtual bool SetAmbisonicOrder(int _ambisonicOrder) { return false; }
		virtual int GetAmbisonicOrder() const { return 0; }
		virtual bool SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) { return false; }
		virtual bool SetAmbisonicNormalization(std::string _ambisonicNormalization) {return false;}
		virtual Common::TAmbisonicNormalization GetAmbisonicNormalization() const { return Common::TAmbisonicNormalization::none; }

		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel > _source) = 0;
		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel > _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) = 0;

		virtual bool ConnectListenerTransform(const std::string _listenerID) {return false; }
		virtual bool DisconnectListenerTransform(const std::string _listenerID) { return false; }

		// Class Methods

		CListenerModelBase(std::string _listenerModelID, TListenerModelcharacteristics _listenerCharacteristics) : listenerModelID {_listenerModelID},
			listenerCharacteristics{ _listenerCharacteristics },
			leftDataReady{ false }, rightDataReady{ false } {
												
			CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");									
			CreateTransformExitPoint();			
			CreateIDExitPoint();
			
			CreateSamplesExitPoint("leftEar");
			CreateSamplesExitPoint("rightEar");
			CreateIDEntryPoint("listenerID");
			GetIDExitPoint()->sendData(listenerModelID);						
			CreateCommandEntryPoint();
		}
						

		/**
		 * @brief Get listener ID
		 * @return Return listener identificator
		*/
		std::string GetID() { return listenerModelID; }
			
		/**
		* @brief Set listener type
		* @param _listenerType Listener type
		*/
		//TListenerType GetListenerModelType() const { return listenerModelType; }		
		
		/**
		* @brief Get listener model characteristics
		* @return Return characteristics
		*/
		TListenerModelcharacteristics GetListenerModelCharacteristics() const { return listenerCharacteristics; }
		
		
		bool IsConnectedToListener() { 
			std::string _listenerID = GetIDEntryPoint("listenerID")->GetData();
			if (_listenerID != "") {
				return true;
			}
			return false; 
		}

		/////////////////////		
		// Update Callbacks
		/////////////////////
				
		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		 * @param _entryPointId entryPoint ID
		*/
		
		void OneEntryPointOneDataReceived(std::string _entryPointId) {
						
			if (_entryPointId == "leftEar") {				
				if (!leftDataReady) { InitBuffer(leftBuffer); }				
				CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("leftEar")->GetData();				
				leftDataReady = MixEarBuffers(leftBuffer, newBuffer);									
			}
			else if (_entryPointId == "rightEar") {
				if (!rightDataReady) { InitBuffer(rightBuffer); }
				CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("rightEar")->GetData();				
				rightDataReady = MixEarBuffers(rightBuffer, newBuffer);				
			}			
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() {
			
			GetSamplesExitPoint("leftEar")->sendData(leftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(rightBuffer);
			leftDataReady = false;
			rightDataReady = false;
						           
		}
		
		

	private:
		//TListenerType listenerModelType;
		TListenerModelcharacteristics listenerCharacteristics;
		std::string listenerModelID;						// Store unique listener ID		
		
		
		Common::CGlobalParameters globalParameters;		
		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	

		
		//////////////////////////
		// Private Methods
		/////////////////////////
				
		
		///**
		// * @brief Mix the new buffer received with the contents of the buffer.
		//*/				
		bool MixEarBuffers(CMonoBuffer<float>& buffer, const CMonoBuffer<float>& newBuffer) {			
			if (newBuffer.size() != 0) {
				buffer += newBuffer;
				return true;
			}
			return false;
		}

		void InitBuffer(CMonoBuffer<float>& buffer) {
			buffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		}
	};
}
#endif