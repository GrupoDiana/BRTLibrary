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

namespace BRTServices {
	class CHRTF;
}

namespace BRTBase {
	
	class CListenerBase : public BRTConnectivity::CBRTConnectivity /*CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager*/ {
	public:
		
		CListenerBase(std::string _listenerID) : listenerID{ _listenerID }, 
			leftDataReady{ false },	rightDataReady{ false } {
												
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
			if (leftDataReady) {
				_leftBuffer = leftBuffer;
				leftDataReady = false;
			}
			else {
				_leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			
			if (rightDataReady) {
				_rightBuffer = rightBuffer;
				rightDataReady = false;
			}
			else {
				_rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}

		}

		/////////////////////		
		// Update Callbacks
		/////////////////////
		
		void UpdateEntryPointData(std::string id) override {
			if (id == "leftEar") {
				UpdateLeftBuffer();
			}
			else if (id == "rightEar") {
				UpdateRightBuffer();
			}
		}
		
		void UpdateCommand() override {
			//Do nothing		
		}
		//void UpdateFromCommandEntryPoint(std::string entryPointID) override {
		//	BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
		//	if (!_command.isNull()) {
		//		//UpdateCommand();
		//	}
		//}						

	private:
				
		//////////////////////////
		// Private Methods
		/////////////////////////
		
		/**
		 * @brief Mix the new buffer received for the left ear with the contents of the buffer.
		*/
		void UpdateLeftBuffer() {
			if (!leftDataReady) {
				leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}			
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("leftEar")->GetData();
			if (buffer.size() != 0) {
				leftBuffer += buffer;
				leftDataReady = true;
			}
		}
		/**
		 * @brief Mix the new buffer received for the right ear with the contents of the buffer.
		*/
		void UpdateRightBuffer() {
			if (!rightDataReady) {
				rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}			
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("rightEar")->GetData();
			if (buffer.size() != 0) {
				rightBuffer += buffer;
				rightDataReady = true;
			}
		}		


		//////////////////////////
		// Private Attributes
		/////////////////////////
		std::string listenerID;								// Store unique listener ID		
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  	

		Common::CGlobalParameters globalParameters;
		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;

		bool leftDataReady;
		bool rightDataReady;
	};
}
#endif