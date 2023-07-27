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

#ifndef _CENVIRONMENT_MODEL_BASE_H_
#define _CENVIRONMENT_MODEL_BASE_H_

#include <memory>
#include <Base/EntryPointManager.hpp>
#include <Base/CommandEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <SourceModels/VirtualSourceModel.hpp>

namespace BRTBase {


	class CEnviromentVirtualSourceBaseModel : public CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager {
	public:
		virtual ~CEnviromentVirtualSourceBaseModel() {}
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;

		CEnviromentVirtualSourceBaseModel(BRTBase::CBRTManager* _brtManager): brtManager{ _brtManager } {			
			CreateSamplesEntryPoint("inputSamples");
			CreatePositionEntryPoint("sourcePosition");			
			CreateIDEntryPoint("sourceID");

			CreatePositionEntryPoint("listenerPosition");
			CreateIDEntryPoint("listenerID");
			
			CreateCommandEntryPoint();
		}

		void CreateVirtualSource(std::string _virtualSourceID) {
			std::shared_ptr<BRTSourceModel::CVirtualSourceModel> virtualSource;
			virtualSource = brtManager->CreateSoundSource<BRTSourceModel::CVirtualSourceModel>(_virtualSourceID);
			
			virtualSourcesList.push_back(virtualSource);
		}


		/**
		 * @brief Connect a new source to this environment
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectSoundSource(std::shared_ptr<T> _source) {
						
			//CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);
			
			bool control = brtManager->ConnectModuleTransform(_source, this, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, this, "sourceID");
			control = control && brtManager->ConnectModulesSamples(_source, "samples", this, "inputSamples");

			///
			/*bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.binauralConvolverProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.nearFieldEffectProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.binauralConvolverProcessor, "sourceID");

			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRTF");
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(this, _newSourceProcessors.binauralConvolverProcessor, "listenerID");

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.binauralConvolverProcessor, "inputSamples");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "leftEar", _newSourceProcessors.nearFieldEffectProcessor, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "rightEar", _newSourceProcessors.nearFieldEffectProcessor, "rightEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "rightEar", this, "rightEar");*/

			if (control) {
				//sourcesConnectedProcessors.push_back(std::move(_newSourceProcessors));
				return true;
			}
			return false;
		}

		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectToListener(std::shared_ptr<T> _listener) {
			
			bool control = control && brtManager->ConnectModuleTransform(listener, this, "listenerPosition");
			control = control && brtManager->ConnectModuleID(listener, this, "listenerID");
			
			for (auto _source : exitSources) {
				_listener()->ConnectSoundSource(_source);
			}						
		}


	private:
		std::string environmentID;																// Store unique enviroment ID	
		std::vector<std::shared_ptr<BRTSourceModel::CVirtualSourceModel>> virtualSourcesList;	// Store a list of virtual sources
		BRTBase::CBRTManager* brtManager;
	};
}
#endif