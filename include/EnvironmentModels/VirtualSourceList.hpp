/**
* \class CVirtualSourcesModel
*
* \brief Declaration of CVirtualSourcesModel class
* \date	Sep 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#ifndef _CVIRTUAL_SOURCEs_MODEL_HPP_
#define _CVIRTUAL_SOURCEs_MODEL_HPP_

#include <memory>
#include <Base/BRTManager.hpp>
//#include <SourceModels/SourceModelBase.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <SourceModels/VirtualSourceModel.hpp>

namespace BRTEnvironmentModel {


	class CVirtualSourceList  {
	public:
						
		CVirtualSourceList(BRTBase::CBRTManager * _brtManager)
			: brtManager { _brtManager } {												
		}

		/**
		 * @brief Create a new virtual source
		 * @param _virtualSourceID ID of the virtual source
		*/
		void CreateVirtualSource(const std::string& _virtualSourceID, const std::string _originalSourceID) {			
			std::shared_ptr<BRTSourceModel::CVirtualSourceModel> _virtualSource;
			_virtualSource = brtManager->CreateSoundSource<BRTSourceModel::CVirtualSourceModel>(_virtualSourceID);
			_virtualSource->SetOriginSourceID(_originalSourceID);
			virtualSources.push_back(_virtualSource);
		}
		
		/**
		 * @brief Connect all the virtual sources to a listener model
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectVirtualSourcesToListenerModel(std::shared_ptr<T> _listenerModel) {						
			bool control = true;
			for (auto _virtualSource : virtualSources) {
				control = control && _listenerModel->ConnectSoundSource(_virtualSource);
			}
			return control;
		}

		/**
		 * @brief Disconnect all the virtual sources from a listener model
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
		*/
		template <typename T>
		bool DisconnectVirtualSourcesToListenerModel(std::shared_ptr<T> _listenerModel) {
			bool control = true;
			for (auto _virtualSource : virtualSources) {
				control = control && _listenerModel->DisconnectSoundSource(_virtualSource);
			}
			return control;
		}

		void SetOriginSourceID(std::string _originSourceID) {
			for (auto it : virtualSources) {
				it->SetOriginSourceID(_originSourceID);
			}
		}

		void SetVirtualSourceBuffer(std::string _virtualSourceID, CMonoBuffer<float>& _buffer) {
			
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
		}
		

	private:		
		std::vector<std::shared_ptr<BRTSourceModel::CVirtualSourceModel>> virtualSources;		// Store a list of virtual sources
		BRTBase::CBRTManager* brtManager;
	};
}
#endif