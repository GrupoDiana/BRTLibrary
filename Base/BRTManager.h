#pragma once

#include "SoundSource.h"
#include "Listener.hpp"

namespace BRTBase {


	class CBRTManager {

	public:

		CBRTManager();

		std::shared_ptr<CSoundSource> CreateSoundSource();
		std::shared_ptr<CListener> CreatListener();
		

		//void ProcessOneListener(CListener _listener);
		void ProcessAllListener();
		
		
		void BeginSetup();
		bool EndSetup();

		/*template <typename T, typename U>
		bool ConnectModules(T& module1, U& module2) {
			return ConnectModules(module1, "1", module2, "1");
		}*/
		template <typename T, typename U>
		bool ConnectModules(T& module1, std::string exitPointID, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectSamplesEntryTo(module1.GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}
		/*
		template <typename T, typename U>
		bool ConnectModules(std::shared_ptr<T>& module1, std::string exitPointID, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectEntryTo(module1->GetExitPoint(exitPointID), entryPointID);
			return true;
		}
		
		template <typename T, typename U, typename W>
		bool ConnectModules(std::shared_ptr<T>& module1, std::string exitPointID, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectEntryTo<W>(module1->GetExitPoint<W>(exitPointID), entryPointID);
			return true;
		}

		template <typename T, typename U>
		bool ConnectModules(T& module1, std::string exitPointID, std::shared_ptr<U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectEntryTo(module1.GetExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename T, typename U>
		bool ConnectModules(std::shared_ptr<T>& module1, std::string exitPointID, std::shared_ptr<U>& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2->connectEntryTo(module1->GetExitPoint(exitPointID), entryPointID);
			return true;
		}*/

		template <typename U>
		bool ConnectModuleToSoundSourceSamples(std::shared_ptr<CSoundSource>& soundSourceModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;												
			module2.connectSamplesEntryTo(soundSourceModule->GetSamplesVectorExitPoint(), entryPointID);			
			return true;
		}

		template <typename U>
		bool ConnectModuleToSoundSourceTransform(std::shared_ptr<CSoundSource>& soundSourceModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(soundSourceModule->GetTransformExitPoint(), entryPointID);
		}

		template <typename U>
		bool ConnectListenerToModuleSamples(U& module1, std::string exitPointID, std::shared_ptr<CListener>& listenerModule, std::string entryPointID) {
			if (!setupModeActivated) return false;
			listenerModule->connectSamplesEntryTo(module1.GetSamplesExitPoint(exitPointID), entryPointID);
			return true;
		}

		template <typename U>
		bool ConnectModuleToListenerTransform(std::shared_ptr<CListener>& listenerModule, U& module2, std::string entryPointID) {
			if (!setupModeActivated) return false;
			module2.connectPositionEntryTo(listenerModule->GetTransformExitPoint(), entryPointID);
		}

	private:

		void ProcessAllListenerThread();

		std::vector<std::shared_ptr<CSoundSource>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListener>> listeners;			// List of audio sources 
		bool initialized;
		bool setupModeActivated;
	};
}
