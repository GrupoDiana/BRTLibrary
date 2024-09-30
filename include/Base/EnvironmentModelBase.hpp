/**
* \class CEnviromentVirtualSourceBaseModel
*
* \brief Declaration of CEnviromentVirtualSourceBaseModel class
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

#ifndef _CENVIRONMENT_MODEL_BASE_H_
#define _CENVIRONMENT_MODEL_BASE_H_

#include <memory>
#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Base/SourceModelBase.hpp>
#include <SourceModels/VirtualSourceModel.hpp>

namespace BRTSourceModel {
	class CSourceSimpleModel;
	class CSourceDirectivityModel;
}

namespace BRTBase {


	class CEnviromentModelBase : public CAdvancedEntryPointManager, public CExitPointManager{
	public:
		// Public Attributes
		bool enableModel;

		// Virtual Methods
		virtual ~CEnviromentModelBase() { }
		virtual void Update(std::string entryPointID) = 0;
		//virtual void UpdateCommand() = 0;


		virtual void EnableModel() {};
		virtual void DisableModel() {};
		virtual bool IsModelEnabled() { return enableModel; }

		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) = 0;
		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) = 0;
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) = 0;
		
		//virtual bool ConnectListenerTransform(const std::string _listenerID) { return false; }
		//virtual bool DisconnectListenerTransform(const std::string _listenerID) { return false; }


		CEnviromentModelBase(const std::string & _environmentModelID)
			: environmentModelID { _environmentModelID }
			, enableModel { true } {
			//CreateSamplesEntryPoint("inputSamples");
			//CreatePositionEntryPoint("sourcePosition");			
			//CreateIDEntryPoint("sourceID");
			
			CreateIDExitPoint();
			//CreatePositionEntryPoint("listenerPosition");
			CreateIDEntryPoint("listenerID");
			CreateIDEntryPoint("listenerModelID");
			GetIDExitPoint()->sendData(environmentModelID);
			CreateCommandEntryPoint();
		}


		/**
		 * @brief Get listener ID
		 * @return Return listener identificator
		*/
		std::string GetID() { return environmentModelID; }

		/**
		 * @brief Check if this environment is already connected to a listener model
		 * @return 
		 */
		bool IsConnectedToListenerModel() {
			std::string _listenerModelID = GetIDEntryPoint("listenerModelID")->GetData();
			return (_listenerModelID != "");			
		}

		///**
		// * @brief Create a new virtual source
		// * @param _virtualSourceID ID of the virtual source
		//*/
		//void CreateVirtualSource(std::string _virtualSourceID) {
		//	std::shared_ptr<BRTSourceModel::CVirtualSourceModel> _virtualSource;
		//	_virtualSource = brtManager->CreateSoundSource<BRTSourceModel::CVirtualSourceModel>(_virtualSourceID);
		//	
		//	virtualSources.push_back(_virtualSource);
		//}


		/**
		 * @brief Connect a new source to this environment
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		//*/
		//template <typename T>
		//bool ConnectSoundSource(std::shared_ptr<T> _source) {
		//				
		//	//CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);
		//	
		//	bool control = brtManager->ConnectModuleTransform(_source, this, "sourcePosition");
		//	control = control && brtManager->ConnectModuleID(_source, this, "sourceID");
		//	control = control && brtManager->ConnectModulesSamples(_source, "samples", this, "inputSamples");
		//	
		//	if (control) {				
		//		SetOriginSourceID(_source->GetID());
		//		return true;
		//	}
		//	return false;
		//}

		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		/*template <typename T>
		bool ConnectToListener(std::shared_ptr<T> _listener) {
			
			bool control = brtManager->ConnectModuleTransform(_listener, this, "listenerPosition");
			control = control && brtManager->ConnectModuleID(_listener, this, "listenerID");
			
			for (auto _virtualSource : virtualSources) {
				control = control && _listener->ConnectSoundSource(_virtualSource);
			}
			return control;
		}*/

		/*template <typename T>
		bool ConnectToListener(T* _listener) {

			bool control = brtManager->ConnectModuleTransform(_listener, this, "listenerPosition");
			control = control && brtManager->ConnectModuleID(_listener, this, "listenerID");

			for (auto _virtualSource : virtualSources) {
				control = control && _listener->ConnectSoundSource(_virtualSource);
			}
			return control;
		}*/




		/*void SetVirtualSourceBuffer(std::string _virtualSourceID, CMonoBuffer<float>& _buffer) {
			
			auto it = std::find_if(virtualSources.begin(), virtualSources.end(), [&_virtualSourceID](std::shared_ptr<BRTSourceModel::CVirtualSourceModel> virtualSource) { return virtualSource->GetID() == _virtualSourceID; });
			if (it != virtualSources.end()) {					
				it[0]->SetBuffer(_buffer);								
			}
			else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There is no virtual source with that name.");
			}			
		}


		void SetVirtualSourcePosition(std::string _virtualSourceID, Common::CTransform _sourcePosition) {
			auto it = std::find_if(virtualSources.begin(), virtualSources.end(), [&_virtualSourceID](std::shared_ptr<BRTSourceModel::CVirtualSourceModel> virtualSource) { return virtualSource->GetID() == _virtualSourceID; });
			if (it != virtualSources.end()) {
				it[0]->SetSourceTransform(_sourcePosition);
			}
			else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There is no virtual source with that name.");
			}
		}*/

				
		/////////////////////
		// Update Callbacks
		/////////////////////

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		 * @param _entryPointId entryPoint ID
		*/

		//void OneEntryPointOneDataReceived(std::string _entryPointId) override{

		//	/*if (_entryPointId == "leftEar") {
		//		if (!leftDataReady) {
		//			InitBuffer(leftBuffer);
		//		}
		//		CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("leftEar")->GetData();
		//		leftDataReady = MixEarBuffers(leftBuffer, newBuffer);
		//	} else if (_entryPointId == "rightEar") {
		//		if (!rightDataReady) {
		//			InitBuffer(rightBuffer);
		//		}
		//		CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("rightEar")->GetData();
		//		rightDataReady = MixEarBuffers(rightBuffer, newBuffer);
		//	}*/
		//}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {

			/*GetSamplesExitPoint("leftEar")->sendData(leftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(rightBuffer);
			leftDataReady = false;
			rightDataReady = false;*/
		}
		
		void UpdateCommand() override {
			// Nothing to do
		}
		/*void updateFromCommandEntryPoint(std::string entryPointID) override {
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}*/

	private:
		std::string environmentModelID;															// Store unique enviroment ID	
		//std::vector<std::shared_ptr<BRTSourceModel::CVirtualSourceModel>> virtualSources;		// Store a list of virtual sources
		//BRTBase::CBRTManager* brtManager;




		/*void SetOriginSourceID(std::string _originSourceID) {
			for (auto it : virtualSources) {
				it->SetOriginSourceID(_originSourceID);
			}
		}*/
	};
}
#endif