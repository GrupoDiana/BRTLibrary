#ifndef _BRT_MANAGER_
#define _BRT_MANAGER_

#include "SourceModelBase.hpp"
#include "ListenerModelBase.hpp"
//#include "ListenerModels/ListenerHRTFbasedModel.hpp"
#include <thread>
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

		/** \brief Create new source and add it to sources vector
		*   \eh Warnings may be reported to the error handler.
		*/
		template <typename T>
		std::shared_ptr<T> CreateSoundSource(std::string sourceID) {			
			try
			{
				std::shared_ptr<T> newSource(new T(sourceID));
				ConnectModulesCommand(newSource);
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
		

		/** \brief Create new listener and add it to listeners vector
		*   \eh Warnings may be reported to the error handler.
		*/
		template <typename T>
		std::shared_ptr<T> CreateListener(std::string _listenerID) {
			// Create new source and add it to this core sources
			try
			{
				std::shared_ptr<T> newListener = std::make_shared<T>(_listenerID);
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
		
		//template <typename T>
		//void RemoveSoundSource(std::string _sourceID) {			
		//	auto it = std::find_if(audioSources.begin(), audioSources.end(), [&_sourceID](std::shared_ptr<T>& sourceItem) { return sourceItem->GetSourceID() == _sourceID; });
		//	//if (it != audioSources.end()) { 
		//	//	audioSources.erase(it); 				
		//		//it->reset();
		//	//}				
		//	
		//}		
		void RemoveSoundSource(std::string _sourceID) {
			auto it = std::find_if(audioSources.begin(), audioSources.end(), [&_sourceID](std::shared_ptr<CSourceModelBase>& sourceItem) { return sourceItem->GetID() == _sourceID; });
			if (it != audioSources.end()) { 
				DisconnectModulesCommand(*it);
				audioSources.erase(it); 				
				it->reset();
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
		/*template <typename U>
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
		}*/

		/*template <typename U>
		bool ConnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSourceModelBase>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSourceModelBase>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool ConnectModuleToSoundSourceID(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectIDEntryTo(soundSourceModule->GetIDExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToSoundSourceID(std::shared_ptr<CSoundSource>& soundSourceModule, std::shared_ptr <U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectIDEntryTo(soundSourceModule->GetIDExitPoint(), entryPointID);
			return true;
		}*/

		///////////////////////////////
		// LISTENER CONNECTIONS
		//////////////////////////////
		/*template <typename U>
		bool ConnectListenerToModuleSamples(U& module1, std::string exitPointID, std::shared_ptr<CListenerModelBase>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->connectSamplesEntryTo(module1.GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		template <typename U>
		bool ConnectListenerToModuleSamples(std::shared_ptr <U>& module1, std::string exitPointID, std::shared_ptr<CListenerModelBase>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectListenerFromModuleSamples(std::shared_ptr <U>& module1, std::string exitPointID, std::shared_ptr<CListenerModelBase>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->disconnectSamplesEntryFrom(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;		
		}*/

		/*template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListenerModelBase>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerTransform(std::shared_ptr<CListenerModelBase>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.disconnectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}*/

		/*template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}*/
		
		/*template <typename U>
		bool DisconnectModuleToListenerTransform(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
			return true;
		}*/

		/*template <typename U>
		bool ConnectModuleToListenerHRTF(std::shared_ptr<CListenerModelBase>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}*/
		/*template <typename U>
		bool ConnectModuleToListenerHRTF(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerHRTF(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectHRTFEntryTo(listenerModule->GetHRTFPtrExitPoint(), entryPointID);
			return true;
		}*/
		
		/*template <typename U>
		bool ConnectModuleToListenerILD(std::shared_ptr<CListener>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectILDEntryTo(listenerModule->GetILDPtrExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerILD(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectILDEntryTo(listenerModule->GetILDPtrExitPoint(), entryPointID);
			return true;
		}*/

		/*template <typename U>
		bool ConnectModuleToListenerID(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectIDEntryTo(listenerModule->GetIDExitPoint(), entryPointID);
			return true;
		}

		template <typename U>
		bool DisconnectModuleToListenerID(std::shared_ptr<CListenerModelBase>& listenerModule, std::shared_ptr < U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectIDEntryTo(listenerModule->GetIDExitPoint(), entryPointID);
			return true;
		}*/

		///////////////////////////////////////////
		// GENERIC PROCESSOR MODULES CONNECTIONs
		///////////////////////////////////////////
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
		 * @tparam T 
		 * @tparam U 
		 * @param module1 Module having ILD exitpoint
		 * @param module2 Module having ILD entrypoint
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
		 * @brief 
		 * @tparam T 
		 * @tparam U 
		 * @param module1 
		 * @param exitPointID 
		 * @param module2 
		 * @param entryPointID 
		 * @return 
		*/
		template <typename T, typename U>
		bool ConnectModulesSamples(std::shared_ptr <T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename T, typename U>
		bool DisconnectModulesSamples(std::shared_ptr<T> module1, std::string exitPointID, std::shared_ptr <U> module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->disconnectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename T, typename U>
		bool ConnectModulesSamples(std::shared_ptr <T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectSamplesEntryTo(module1->GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename T, typename U>
		bool DisconnectModulesSamples(std::shared_ptr<T> module1, std::string exitPointID, U* module2, std::string entryPointID) {
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
		void ExecuteCommand(std::string commandJson) {
			//std::cout << "Command received by brt: " << commandJson << endl;
			BRTBase::CCommand command(commandJson);
			commandsExitPoint->sendData(command);																		
		}

	private:
		std::shared_ptr<BRTBase::CExitPointCommand> commandsExitPoint;		// Exit point to emit control commands

		std::vector<std::shared_ptr<CSourceModelBase>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListenerModelBase>> listeners;					// List of audio sources 
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