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

		void RemoveSoundSource(std::string _sourceID) {			
			auto it = std::find_if(audioSources.begin(), audioSources.end(), [&_sourceID](std::shared_ptr<CSoundSource>& sourceItem) { return sourceItem->GetSourceID() == _sourceID; });
			if (it != audioSources.end()) { 
				audioSources.erase(it); 				
				//it->reset();
			}				
		}
		
		template <typename T>
		void RemoveProcessor(std::shared_ptr<T> _processor) {
			DisconnectModulesCommand(_processor);
			_processor.reset();
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
		bool DisconnectModuleToSoundSourceSamples(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr<U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectSamplesEntryTo(soundSourceModule->GetSamplesVectorExitPoint(), entryPointID);
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

		template <typename U>
		bool DisconnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
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
		bool DisconnectListenerFromModuleSamples(std::shared_ptr <U>& module1, std::string exitPointID, std::shared_ptr<CListener>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->disconnectSamplesEntryFrom(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;		
		}

		template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.disconnectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}
		
		template <typename U>
		bool DisconnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

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

		template <typename U>
		bool DisconnectModuleToListenerHRTF(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}

		//
		template <typename U>
		bool ConnectModuleToListenerILD(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectILDEntryTo(listenerModule->GetILDPtrExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerILD(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectILDEntryTo(listenerModule->GetILDPtrExitPoint(), entryPointID);
			return true;
		}

		//

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

		template <typename T, typename U>
		bool DisconnectModulesSamples(std::shared_ptr <T>& module1, std::string exitPointID, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename T>
		bool ConnectModulesCommand(std::shared_ptr <T>& module1) {
			//if (!setupModeActivated) return false;
			module1->connectCommandEntryTo(commandsExitPoint);
			return true;
		}
		template <typename T>
		bool DisconnectModulesCommand(std::shared_ptr <T>& module1) {
			//if (!setupModeActivated) return false;
			module1->disconnectCommandEntryTo(commandsExitPoint);
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
		//TODO: delete?
		void Do(std::string commandJson) {
			std::cout << "Command received by brt: " << commandJson << endl;
			BRTBase::CCommand command(commandJson);
			commandsExitPoint->sendData(command);																		
		}

		void SendCommand2BRT(BRTBase::CCommand _command) {
			//std::cout << "Command received by brt: " << commandJson << endl;
			//BRTBase::CCommand command(commandJson);
			commandsExitPoint->sendData(_command);
		}



	private:
		std::shared_ptr<BRTBase::CExitPointCommand> commandsExitPoint;		// Exit point to emit control commands

		std::vector<std::shared_ptr<CSoundSource>> audioSources;			// List of audio sources 
		std::vector<std::shared_ptr<CListener>> listeners;					// List of audio sources 
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