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

	enum class TListenerType { ListenerHRFTModel, ListenerAmbisonicHRTFModel, ListenerEnviromentBRIRModel };

	class CListenerModelBase: public CAdvancedEntryPointManager, public CExitPointManager/*, public CCommandEntryPointManager, public CEntryPointManager */{
	public:

		// Public Attributes
		bool enableITDSimulation;							// Enable ITD simulation 

		// Virtual Methods

		virtual ~CListenerModelBase() {}
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;		
		virtual bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) { return false; };
		virtual void SetILD(std::shared_ptr< BRTServices::CNearFieldCompensationFilters > _listenerILD) {};
		virtual std::shared_ptr < BRTServices::CHRTF> GetHRTF() const { return nullptr; }
		virtual void RemoveHRTF() {};
		virtual std::shared_ptr < BRTServices::CNearFieldCompensationFilters> GetILD() const { return nullptr; }
		virtual void RemoveILD() {};
		virtual std::shared_ptr < BRTServices::CHRBRIR> GetHRBRIR() const { return nullptr; };
		
		virtual bool SetHRBRIR(std::shared_ptr< BRTServices::CHRBRIR > _listenerBRIR) { return false; };		        
		virtual void RemoveHRBRIR() {};

		virtual void EnableITDSimulation()= 0;
		virtual void DisableITDSimulation() = 0;			
		virtual bool IsITDSimulationEnabled() { return enableITDSimulation; }
		

		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel > _source) = 0;
		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel > _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) = 0;


		// Class Methods

		CListenerModelBase(std::string _listenerModelID, TListenerType _listenerType) : listenerModelID{ _listenerModelID }, listenerModelType {_listenerType},
			leftDataReady{ false }, rightDataReady{ false }, enableITDSimulation{ true } {
												
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
		TListenerType GetListenerModelType() const { return listenerModelType; }
		
		
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
		
		/*void UpdateCommand() {
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
			
		}*/
		
		
		// old
		/*void UpdateEntryPointData(std::string id) {
			if (id == "leftEar") {
				UpdateLeftBuffer();
			}
			else if (id == "rightEar") {
				UpdateRightBuffer();
			}
		}*/
		/*void updateFromCommandEntryPoint(std::string entryPointID) {
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}*/


	private:
		TListenerType listenerModelType;
		std::string listenerModelID;						// Store unique listener ID		
		//bool connectedToListener;
		

		Common::CGlobalParameters globalParameters;		
		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	

		
		//////////////////////////
		// Private Methods
		/////////////////////////
		
		///**
		// * @brief Mix the new buffer received for the left ear with the contents of the buffer.
		//*/
		//void UpdateLeftBuffer() {
		//	if (!leftDataReady) {
		//		leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}			
		//	CMonoBuffer<float> buffer = GetSamplesEntryPoint("leftEar")->GetData();
		//	if (buffer.size() != 0) {
		//		leftBuffer += buffer;
		//		leftDataReady = true;
		//	}
		//}
		///**
		// * @brief Mix the new buffer received for the right ear with the contents of the buffer.
		//*/
		//void UpdateRightBuffer() {
		//	if (!rightDataReady) {
		//		rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}			
		//	CMonoBuffer<float> buffer = GetSamplesEntryPoint("rightEar")->GetData();
		//	if (buffer.size() != 0) {
		//		rightBuffer += buffer;
		//		rightDataReady = true;
		//	}
		//}	
		
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