#include "BRTManager.h"
#include <thread>

namespace BRTBase {


	
	CBRTManager::CBRTManager() : initialized{ false }, setupModeActivated{false} {}


	std::shared_ptr<CSoundSource> CBRTManager::CreateSoundSource() {
	
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



	std::shared_ptr<CListener> CBRTManager::CreatListener() {
	
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


	void CBRTManager::BeginSetup() {
		setupModeActivated = true;
	}

	bool CBRTManager::EndSetup() {
		bool control = false;
		if (setupModeActivated) {
			//TODO Check the connections if they are OK return true
			control = true;
		}
		return control;
	}

	//void CBRTManager::ProcessOneListener(CListener _listener) {
	//	
	//}

	void CBRTManager::ProcessAllListener() {


		std::thread thread1 = std::thread(&BRTBase::CBRTManager::ProcessAllListenerThread, this);


		thread1.join();		
	}

	void CBRTManager::ProcessAllListenerThread() {
		for (auto it = audioSources.begin(); it != audioSources.end(); it++) (*it)->SetDataReady();
	}
}