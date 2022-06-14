#pragma once

#include "SoundSource.h"
#include "Listener.hpp"

namespace BRT_Base {


	class CBRTManager {

	public:

		CBRTManager();

		std::shared_ptr<CSoundSource> CreateSoundSource();
		std::shared_ptr<CListener> CreatListener();
		

		//void ProcessOneListener(CListener _listener);
		void ProcessAllListener();
		
		
		void BeginSetup();
		bool EndSetup();

		template <typename T, typename U>
		bool ConnectModules(T& module1, U& module2) {
			if (!setupModeActivated) return false;
			module2.connectEntryTo(module1.GetExitPoint());
			return true;
		}

		template <typename T, typename U>
		bool ConnectModules(std::shared_ptr<T>& module1, U& module2) {
			if (!setupModeActivated) return false;
			module2.connectEntryTo(module1->GetExitPoint());
			return true;
		}
		
		template <typename T, typename U>
		bool ConnectModules(T& module1, std::shared_ptr<U>& module2) {
			if (!setupModeActivated) return false;
			module2->connectEntryTo(module1.GetExitPoint());
			return true;
		}

		template <typename T, typename U>
		bool ConnectModules(std::shared_ptr<T>& module1, std::shared_ptr<U>& module2) {
			if (!setupModeActivated) return false;
			module2->connectEntryTo(module1->GetExitPoint());
			return true;
		}




	private:

		void ProcessAllListenerThread();

		std::vector<std::shared_ptr<CSoundSource>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListener>> listeners;			// List of audio sources 
		bool initialized;
		bool setupModeActivated;
	};
}
