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

#ifndef _CENVIRONMENT_MODEL_BASE_HPP_
#define _CENVIRONMENT_MODEL_BASE_HPP_

#include <memory>
#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Base/SourceModelBase.hpp>
#include <SourceModels/VirtualSourceModel.hpp>
#include <Common/Room.hpp>

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

		virtual void UpdatedRoom() = 0;

		CEnviromentModelBase(const std::string & _environmentModelID)
			: environmentModelID { _environmentModelID }
			, enableModel { true } {

			
			CreateIDExitPoint();
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

		/**
		 * @brief Set up a shoebox room
		 * @param length extension of the room along the X axis
		 * @param width extension of the room along the Y axis.
		 * @param height extension of the room along the Z axis
		 */
		void SetupShoeBoxRoom(float length, float width, float height) {
			roomDefinition.SetupShoeBox(length, width, height);
			UpdatedRoom();
		}

		Common::CRoom GetRoom() {
			return roomDefinition;
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
		std::string environmentModelID;															// Store unique enviroment ID	
		Common::CRoom roomDefinition;
	};
}
#endif