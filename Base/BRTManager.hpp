#pragma once

#include "SoundSource.h"
#include "Listener.hpp"
#include <thread>

namespace BRTBase {


	class CBRTManager {

	public:

		CBRTManager() : initialized{ false }, setupModeActivated{ false } {}

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


		std::shared_ptr<CSoundSource> CreateSoundSource() {
			// Create new source and add it to this core sources
			try
			{
				std::shared_ptr<CSoundSource> newSource(new CSoundSource());
				audioSources.push_back(newSource);
				//SET_RESULT(RESULT_OK, "Single source DSP created succesfully");
				return newSource;
			}
			catch (std::bad_alloc& ba)
			{
				//ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		std::shared_ptr<CListener> CreatListener() {
			// Create new source and add it to this core sources
			try
			{
				std::shared_ptr<CListener> newListener = std::make_shared<CListener>();
				listeners.push_back(newListener);
				//SET_RESULT(RESULT_OK, "Listener created succesfully");
				return newListener;
			}
			catch (std::bad_alloc& ba)
			{
				//ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		template <typename T>
		std::shared_ptr<T> CreateProcessor() {
			try
			{
				std::shared_ptr<T> newProcessor = std::make_shared<T>();
				
				//SET_RESULT(RESULT_OK, "Processor created succesfully");
				return newProcessor;
			}
			catch (std::bad_alloc& ba)
			{
				//ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}



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


		void ProcessAll() {
			std::thread thread1 = std::thread(&BRTBase::CBRTManager::ProcessAllThread, this);
			thread1.join();
		}



	private:

		void ProcessAllThread() {
			for (auto it = audioSources.begin(); it != audioSources.end(); it++) (*it)->SetDataReady();
		}

		std::vector<std::shared_ptr<CSoundSource>> audioSources;	// List of audio sources 
		std::vector<std::shared_ptr<CListener>> listeners;			// List of audio sources 
		bool initialized;
		bool setupModeActivated;
	};
}
