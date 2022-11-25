#ifndef _BRT_MANAGER_
#define _BRT_MANAGER_

#include "SoundSource.hpp"
#include "Listener.hpp"
#include <thread>

//#include <json.hpp>
#include "third_party_libraries/nlohmann/json.hpp"

namespace BRTBase {
	using json = nlohmann::json;

	class CBRTManager {

	public:

		CBRTManager() : initialized{ false }, setupModeActivated{ false } {
			commandsExitPoint = std::make_shared<BRTBase::CExitPointCommand>(static_cast<std::string>(Common::COMMAND_EXIT_POINT_ID));
		}

		void BeginSetup() {
			setupModeActivated = true;
		}

		bool EndSetup() {
			bool control = false;
			if (setupModeActivated) {
				//TODO Check the connections if they are OK return true
				control = true;
				initialized = true;
			}
			return control;
		}

		/** \brief Create new source and add it to this core sources
		*   \eh Warnings may be reported to the error handler.
		*/
		std::shared_ptr<CSoundSource> CreateSoundSource(std::string sourceID) {			
			try
			{
				std::shared_ptr<CSoundSource> newSource(new CSoundSource(sourceID));
				audioSources.push_back(newSource);
				SET_RESULT(RESULT_OK, "Single source DSP created succesfully");
				return newSource;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		std::shared_ptr<CListener> CreateListener(std::string listenerID) {
			// Create new source and add it to this core sources
			try
			{
				std::shared_ptr<CListener> newListener = std::make_shared<CListener>(listenerID);
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

		template <typename T>
		std::shared_ptr<T> CreateProcessor() {
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

		
		
		///////////////////////////////
		// SOUND SOURCE CONNECTIONS
		//////////////////////////////
		template <typename U>
		bool ConnectModuleToSoundSourceSamples(std::shared_ptr<CSoundSource>& soundSourceModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;												
			module2.connectSamplesEntryTo(soundSourceModule->GetSamplesVectorExitPoint(), entryPointID);			
			return true;
		}
		template <typename U>
		bool ConnectModuleToSoundSourceSamples(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr<U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(soundSourceModule->GetSamplesVectorExitPoint(), entryPointID);
			return true;
		}
		
		template <typename U>
		bool ConnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSource>& soundSourceModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
			return true;
		}
		template <typename U>
		bool ConnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		///////////////////////////////
		// LISTENER CONNECTIONS
		//////////////////////////////
		template <typename U>
		bool ConnectListenerToModuleSamples(U& module1, std::string exitPointID, std::shared_ptr<CListener>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->connectSamplesEntryTo(module1.GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename U>
		bool ConnectListenerToModuleSamples(std::shared_ptr <U>& module1, std::string exitPointID, std::shared_ptr<CListener>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}
		template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}
		
		//template <typename U>
		//bool ConnectModuleToListenerEarsTransform(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
		//	if (!setupModeActivated) return false;
		//	module2.connectEarsPositionEntryTo(listenerModule->GetEarsTransformExitPoint(), entryPointID);
		//}
		//template <typename U>
		//bool ConnectModuleToListenerEarsTransform(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
		//	if (!setupModeActivated) return false;
		//	module2->connectEarsPositionEntryTo(listenerModule->GetEarsTransformExitPoint(), entryPointID);
		//}

		template <typename U>
		bool ConnectModuleToListenerHRTF(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}
		template <typename U>
		bool ConnectModuleToListenerHRTF(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}


		///////////////////////////////////////////
		// GENERIC PROCESSOR MODULES CONNECTIONs
		///////////////////////////////////////////
		template <typename T, typename U>
		bool ConnectModulesSamples(T& module1, std::string exitPointID, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectSamplesEntryTo(module1.GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename T, typename U>
		bool ConnectModulesSamples(std::shared_ptr <T>& module1, std::string exitPointID, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename T>
		bool ConnectModulesCommand(std::shared_ptr <T>& module1) {
			//if (!setupModeActivated) return false;
			module1->connectCommandEntryTo(commandsExitPoint);
			return true;
		}


		//////////////////////
		// PROCESS METHODs
		/////////////////////
		void ProcessAll() {
			std::thread thread1 = std::thread(&BRTBase::CBRTManager::ProcessAllThread, this);
			thread1.join();
		}


		//////////////////////
		// GET SET
		/////////////////////
		void Do(std::string commandJson) {
			std::cout << "Command received by brt: " << commandJson << endl;
			BRTBase::CCommand command(commandJson);
			commandsExitPoint->sendData(command);
			

			//json j = json::parse(commandJson);
			//if (!j["command"].is_null()) { 
			//	std::string _command = j["command"].get<std::string>();
			//	//std::cout << _command <<std::endl;
			//	BRTBase::CCommand command (_command, commandJson);
			//	
			//	commandsExitPoint->sendData(command);


			//}
			//
			//			
			//double temp = 0.0;
			//std::vector<double> tempV;
			//if (!j["parameter"].is_null() && j["parameter"].is_number_float()) { 
			//	temp= j["parameter"]; std::cout << temp << std::endl;							
			//}
			//if (!j["parameter"].is_null() && j["parameter"].is_structured()) { 
		
			//	
			//	
			//	tempV = j["parameter"].get<std::vector<double>>(); 
			//	std::cout << tempV[0] << std::endl;
			//	std::cout << tempV[1] << std::endl;
			//	std::cout << tempV[2] << std::endl;			
			//}
			
			
			
			

		}



	private:
		std::shared_ptr<BRTBase::CExitPointCommand> commandsExitPoint;

		std::vector<std::shared_ptr<CSoundSource>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListener>> listeners;			// List of audio sources 
		bool initialized;
		bool setupModeActivated;

		/////////////////
		// Methods
		/////////////////

		void ProcessAllThread() {
			for (auto it = audioSources.begin(); it != audioSources.end(); it++) (*it)->SetDataReady();
		}
	};
}
#endif