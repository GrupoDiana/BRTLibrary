/**
* \class CListenerBase
*
* \brief Declaration of CListenerBase class
* \date	June 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
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

#ifndef _CLISTENER_BASE_H_
#define _CLISTENER_BASE_H_

#include <memory>
#include <Connectivity/BRTConnectivity.hpp>
#include <ListenerModels/ListenerModelBase.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/AudioMixer.hpp>
#include <Common/Buffer.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTBase {
	
	class CListenerBase : public BRTConnectivity::CBRTConnectivity /*CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager*/ {
	public:
		
		CListenerBase(std::string _listenerID) 
			: listenerID{ _listenerID }
		{												
			leftChannelMixer = Common::CAudioMixer(globalParameters.GetBufferSize());
			rightChannelMixer = Common::CAudioMixer(globalParameters.GetBufferSize());

			CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");									
			CreateTransformExitPoint();			
			CreateIDExitPoint();
			GetIDExitPoint()->sendData(listenerID);						
			CreateCommandEntryPoint();
		}
				

		/** \brief Set listener position and orientation
		*	\param [in] _listenerTransform new listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetListenerTransform(Common::CTransform _transform) {
			listenerTransform = _transform;
			GetTransformExitPoint()->sendData(listenerTransform);	// Send to subscribers			
		}

		/** \brief Get listener position and orientation
		*	\retval transform current listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CTransform GetListenerTransform() { return listenerTransform; }

		/**
		 * @brief Get listener ID
		 * @return Return listener identificator
		*/
		std::string GetID() { return listenerID; }		
			
		void SendMyID() { GetIDExitPoint()->sendData(listenerID); }
		/**
		 * @brief Get output sample buffers from the listener
		 * @param _leftBuffer Left ear sample buffer
		 * @param _rightBuffer Right ear sample buffer
		*/
		void GetBuffers(CMonoBuffer<float>& _leftBuffer, CMonoBuffer<float>& _rightBuffer) {									
			_leftBuffer = leftChannelMixer.GetMixedBuffer();
			_rightBuffer = rightChannelMixer.GetMixedBuffer();
		}

		/////////////////////		
		// Update Callbacks
		/////////////////////
		
		void UpdateEntryPointData(std::string id) override {
			if (id == "leftEar") {							
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("leftEar")->GetData();
				leftChannelMixer.AddBuffer(buffer);
			}
			else if (id == "rightEar") {		
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("rightEar")->GetData();
				rightChannelMixer.AddBuffer(buffer);
			}
		}
		
		void UpdateCommand() override {
			//Do nothing		
		}
		

	private:
				
		//////////////////////////
		// Private Methods
		/////////////////////////
		
		


		//////////////////////////
		// Private Attributes
		/////////////////////////
		std::string listenerID;								// Store unique listener ID		
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  	

		Common::CGlobalParameters globalParameters;

		Common::CAudioMixer leftChannelMixer;
		Common::CAudioMixer rightChannelMixer;		
	};
}
#endif