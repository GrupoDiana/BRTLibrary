/**
* \class CBRTManager
*
* \brief Declaration of CBRTManager class
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
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _BRT_MANAGER_
#define _BRT_MANAGER_

#include <thread>
#include "SourceModelBase.hpp"
#include "ListenerModelBase.hpp"
#include "third_party_libraries/nlohmann/json.hpp"

namespace BRTBase {
	using json = nlohmann::json;

	class CBRTManager {

	public:

		CBRTManager() : initialized{ false }, setupModeActivated{ false } {
			commandsExitPoint = std::make_shared<BRTBase::CExitPointCommand>(static_cast<std::string>(Common::COMMAND_EXIT_POINT_ID));
		}

		/**
		 * @brief Starts the configuration mode, where you can create/destroy and connect/disconnect modules.
		*/
		void BeginSetup() {
			setupModeActivated = true;
		}

		/**
		 * @brief Ends the library configuration mode. Out of this mode you must not create and connect modules. 
		 * This method is unfinished, it should check that all connections are correctly made. For now it is up to the user not to leave modules floating around.
		 * @return Right now always returns true
		*/
		bool EndSetup() {
			bool control = false;
			if (setupModeActivated) {
				//TODO Check the connections if they are OK return true
				control = true;
				initialized = true;
				setupModeActivated = false;
			}
			return control;
		}
		
		/**
		 * @brief Creates a new source and returns a pointer it. This pointer is also saved in a vector.
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param sourceID Source ID to be assigned to the source. It must be unique and cannot be changed.
		 * @return Returns the pointer to the source if it could be created, otherwise returns a null pointer.
		*/
		template <typename T>
		std::shared_ptr<T> CreateSoundSource(std::string _sourceID) {									
			try
			{
				if (!setupModeActivated) { 
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "BRT library is not in configuration mode");
					return nullptr; 				
				}
				auto it = std::find_if(audioSources.begin(), audioSources.end(), [&_sourceID](std::shared_ptr<CSourceModelBase>& sourceItem) { return sourceItem->GetID() == _sourceID; });
				if (it != audioSources.end()) {
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "A Source with such an ID already exists.");
					return nullptr;
				}
				std::shared_ptr<T> newSource(new T(_sourceID));
				ConnectModulesCommand(newSource);
				audioSources.push_back(newSource);
				SET_RESULT(RESULT_OK, "Sound source modelcreated succesfully");
				return newSource;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}
				
		/**
		 * @brief Creates a new listener and returns a pointer to it. This listener pointer is also saved in a vector.
		 * @tparam T It must be a listener model, i.e. a class that inherits from the CListenerModelBase class.
		 * @param _listenerID Listener ID to be assigned to the listener. It must be unique and cannot be changed.
		 * @return Returns the pointer to the listener if it could be created, otherwise returns a null pointer.
		*/
		template <typename T>
		std::shared_ptr<T> CreateListener(std::string _listenerID) {			
			try
			{
				if (!setupModeActivated) {
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "BRT library is not in configuration mode");
					return nullptr;
				}
				auto it = std::find_if(listeners.begin(), listeners.end(), [&_listenerID](std::shared_ptr<CListenerModelBase>& listenerItem) { return listenerItem->GetID() == _listenerID; });
				if (it != listeners.end()) {
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "A listener with such an ID already exists.");
					return nullptr;
				}

				std::shared_ptr<T> newListener = std::make_shared<T>(_listenerID, this);
				ConnectModulesCommand(newListener);
				listeners.push_back(newListener);
				SET_RESULT(RESULT_OK, "Listener created succesfully");
				return newListener;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		/**
		 * @brief Creates a new processor and returns a pointer to it. The brtmanager does NOT save the pointer.
		 * @tparam T It must be a procesor module, i.e. a class that inherits from the CProcessorBase class.
		 * @return Returns the pointer to the procesor if it could be created, otherwise returns a null pointer.
		*/
		template <typename T>
		std::shared_ptr<T> CreateProcessor() {
			if (!setupModeActivated) { return nullptr; }
			try
			{
				std::shared_ptr<T> newProcessor = std::make_shared<T>();
				ConnectModulesCommand(newProcessor);
				SET_RESULT(RESULT_OK, "Processor created succesfully");
				return newProcessor;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}
		

		/**
		 * @brief Creates a new processor and returns a pointer to it. The brtmanager does NOT save the pointer.
		 * @tparam T It must be a procesor module, i.e. a class that inherits from the CProcessorBase class.
		 * @return Returns the pointer to the procesor if it could be created, otherwise returns a null pointer.
		*/
		template <typename T, typename U>
		std::shared_ptr<T> CreateProcessor(U data) {
			if (!setupModeActivated) { return nullptr; }
			try
			{
				std::shared_ptr<T> newProcessor = std::make_shared<T>(data);
				ConnectModulesCommand(newProcessor);
				SET_RESULT(RESULT_OK, "Processor created succesfully");
				return newProcessor;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		/**
		 * @brief Delete a source
		 * @param _sourceID Identifier of the source to be deleted
		 * @return Returns true in case the source could have been deleted.
		*/
		bool RemoveSoundSource(std::string _sourceID) {
			if (!setupModeActivated) { return false; }
			auto it = std::find_if(audioSources.begin(), audioSources.end(), [&_sourceID](std::shared_ptr<CSourceModelBase>& sourceItem) { return sourceItem->GetID() == _sourceID; });
			if (it != audioSources.end()) { 
				DisconnectModulesCommand(*it);
				audioSources.erase(it); 				
				//it->reset();
				return true;
			}				
			return false;
		}
		
		/**
		 * @brief Delete a listener
		 * @param _listenerID Identifier of the listener to be deleted
		 * @return Returns true in case the listener could have been deleted.
		*/
		bool RemoveListener(std::string _listenerID) {
			if (!setupModeActivated) { return false; }
			auto it = std::find_if(listeners.begin(), listeners.end(), [&_listenerID](std::shared_ptr<CListenerModelBase>& listenerItem) { return listenerItem->GetID() == _listenerID; });
			if (it != listeners.end()) {
				DisconnectModulesCommand(*it);
				listeners.erase(it);
				it->reset();
				return true;
			}
			return false;
		}

		/**
		 * @brief Delete a processor
		 * @tparam T Processr type.
		 * @param _processor 
		 * @return Returns true in case the processor was deleted.
		*/
		template <typename T>
		bool RemoveProcessor(std::shared_ptr<T> _processor) {
			if (!setupModeActivated) { return false; }
			DisconnectModulesCommand(_processor);
			_processor.reset();
			return true;
		}

		
		///////////////////////////////////////////
		// MODULES CONNECTIONs
		///////////////////////////////////////////
		

		/**
		 * @brief Connects the Transform ExitPoint of one module to the Transform EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Transform exitpoint
		 * @param module2 Pointer to module having Transform entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModuleTransform(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(module1->GetTransformExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModuleTransform(T* module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(module1->GetTransformExitPoint(), entryPointID);
			return true;
		}

		/**
		 * @brief Disconnects the Transform ExitPoint of one module with the Transform EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Transform exitpoint
		 * @param module2 Pointer to module having Transform entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModuleTransform(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(module1->GetTransformExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool DisconnectModuleTransform(T* module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(module1->GetTransformExitPoint(), entryPointID);
			return true;
		}

		/**
		 * @brief Connects the HRTF ExitPoint of one module to the HRTF EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having HRTF exitpoint
		 * @param module2 Pointer to module having HRTF entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModuleHRTF(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectHRTFEntryTo(module1->GetHRTFExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModuleHRTF(T* module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectHRTFEntryTo(module1->GetHRTFExitPoint(), entryPointID);
			return true;
		}

		/**
		 * @brief Disconnects the HRTF ExitPoint of one module with the HRTF EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having HRTF exitpoint
		 * @param module2 Pointer to module having HRTF entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModuleHRTF(std::shared_ptr<T> module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectHRTFEntryTo(module1->GetHRTFExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool DisconnectModuleHRTF(T* module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectHRTFEntryTo(module1->GetHRTFExitPoint(), entryPointID);
			return true;
		}


		template <typename T, typename U>
		bool ConnectModuleABIR(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectABIREntryTo(module1->GetABIRExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModuleABIR(T* module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectABIREntryTo(module1->GetABIRExitPoint(), entryPointID);
			return true;
		}



		/**
		 * @brief Connects the ILD ExitPoint of one module to the ILD EntryPoint of another.
		 * @tparam T 
		 * @tparam U 
		 * @param module1 Module having ILD exitpoint
		 * @param module2 Module having ILD entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModuleILD(std::shared_ptr<T>& module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectILDEntryTo(module1->GetILDExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModuleILD(T* module1, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectILDEntryTo(module1->GetILDExitPoint(), entryPointID);
			return true;
		}
		/**
		 * @brief Disconnects the ILD ExitPoint of one module with the ILD EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having ILD exitpoint
		 * @param module2 Pointer to module having ILD entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModuleILD(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectILDEntryTo(module1->GetILDExitPoint(), entryPointID);
			return true;
		}		
		template <typename T, typename U>
		bool DisconnectModuleILD(T* module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectILDEntryTo(module1->GetILDExitPoint(), entryPointID);
			return true;
		}

		/**
		 * @brief Connects the ID ExitPoint of one module to the ID EntryPoint of another.
		 * @tparam T
		 * @tparam U
		 * @param module1 Module having ID exitpoint
		 * @param module2 Module having ID entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModuleID(std::shared_ptr<T> module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectIDEntryTo(module1->GetIDExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModuleID(T* module1, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectIDEntryTo(module1->GetIDExitPoint(), entryPointID);
			return true;
		}
		/**
		 * @brief Disconnects the ID ExitPoint of one module with the ID EntryPoint of another.
		 * @tparam T
		 * @tparam U
		 * @param module1 Module having ID exitpoint
		 * @param module2 Module having ID entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModuleID(std::shared_ptr<T> soundSourceModule, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectIDEntryTo(soundSourceModule->GetIDExitPoint(), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool DisconnectModuleID(T* soundSourceModule, std::shared_ptr<U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectIDEntryTo(soundSourceModule->GetIDExitPoint(), entryPointID);
			return true;
		}

		/**
		* @brief Connects the Samples ExitPoint of one module to the Samples EntryPoint of another.
		* @tparam T Type of module 1
		* @tparam U Type of module 2
		* @param module1 Pointer to module having Samples exitpoint
		* @param module2 Pointer to module having Samples entrypoint
		* @param entryPointID ID of entry point in module 2
		* @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModulesSamples(std::shared_ptr <T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModulesSamples(std::shared_ptr <T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		/**
		 * @brief Disconnects the Samples ExitPoint of one module with the Samples EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Samples exitpoint
		 * @param module2 Pointer to module having Samples entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModulesSamples(std::shared_ptr<T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}		
		template <typename T, typename U>
		bool DisconnectModulesSamples(std::shared_ptr<T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		/**
		 * @brief Connects the vector of multiples samples ExitPoint of one module to the vector of multiple samples EntryPoint of another.
		  * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Samples exitpoint
		 * @param module2 Pointer to module having Samples entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool ConnectModulesMultipleSamplesVectors(std::shared_ptr <T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {			
			if (!setupModeActivated) return false;
			module2->connectMultipleSamplesVectorsEntryTo(module1->GetMultipleSamplesVectorExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModulesMultipleSamplesVectors(std::shared_ptr <T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectMultipleSamplesVectorsEntryTo(module1->GetMultipleSamplesVectorExitPoint(exitPointID), entryPointID);
			return true;
		}

		/**
		 * @brief Disconnects the vector of multiples samples ExitPoint of one module with the vector of multiples samples EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Samples exitpoint
		 * @param module2 Pointer to module having Samples entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T, typename U>
		bool DisconnectModulesMultipleSamplesVectors(std::shared_ptr<T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectMultipleSamplesVectorsEntryTo(module1->GetMultipleSamplesVectorExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool DisconnectModulesMultipleSamplesVectors(std::shared_ptr<T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectMultipleSamplesVectorsEntryTo(module1->GetMultipleSamplesVectorExitPoint(exitPointID), entryPointID);
			return true;
		}

		/**
		 * @brief Connects the Command ExitPoint of one module to the Command EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Command exitpoint
		 * @param module2 Pointer to module having Command entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the connection. False in all other cases.
		*/
		template <typename T>
		bool ConnectModulesCommand(std::shared_ptr <T>& module1) {
			//if (!setupModeActivated) return false;
			module1->connectCommandEntryTo(commandsExitPoint);
			return true;
		}
		/**
		 * @brief Disconnects the Command ExitPoint of one module with the Command EntryPoint of another.
		 * @tparam T Type of module 1
		 * @tparam U Type of module 2
		 * @param module1 Pointer to module having Command exitpoint
		 * @param module2 Pointer to module having Command entrypoint
		 * @param entryPointID ID of entry point in module 2
		 * @return Returns true if it was possible to make the disconnection. False in all other cases.
		*/
		template <typename T>
		bool DisconnectModulesCommand(std::shared_ptr <T>& module1) {
			//if (!setupModeActivated) return false;
			module1->disconnectCommandEntryTo(commandsExitPoint);
			return true;
		}

		//////////////////////
		// PROCESS METHODs
		/////////////////////

		/**
		 * @brief Start audio processing
		*/
		void ProcessAll() {
			if (setupModeActivated) return;
			std::thread thread1 = std::thread(&BRTBase::CBRTManager::ProcessAllThread, this);
			thread1.join();
		}
		/**
		 * @brief Executes the received command. To do so, it distributes it to all the connected modules, which are responsible for executing the relevant actions.
		 * @param commandJson The command to execute following a json format.
		*/
		void ExecuteCommand(std::string commandJson) {			
			BRTBase::CCommand command(commandJson);
			commandsExitPoint->sendData(command);																		
		}

	private:
		std::shared_ptr<BRTBase::CExitPointCommand> commandsExitPoint;	// Exit point to emit control commands
		std::vector<std::shared_ptr<CSourceModelBase>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListenerModelBase>> listeners;		// List of audio sources 
		bool initialized;
		bool setupModeActivated;

		/////////////////
		// Methods
		/////////////////
		
		/**
		 * @brief Start processing on each of the sources.
		*/
		void ProcessAllThread() {
			for (auto it = audioSources.begin(); it != audioSources.end(); it++) (*it)->SetDataReady();
		}
	};
}
#endif