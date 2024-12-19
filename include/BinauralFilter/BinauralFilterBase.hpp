/**
* \class CBinauralFilterBase
*
* \brief This class implements the binaural filter.
* \date	Nov 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
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

#ifndef _C_BINAURAL_FILTER_BASE_HPP_
#define _C_BINAURAL_FILTER_BASE_HPP_

#include <Base/ModelBase.hpp>

namespace BRTBinauralFilter {
	class CBinauralFilterBase : public BRTBase::CModelBase {
	public:

		virtual bool SetSOSFilter(std::shared_ptr<BRTServices::CSOSFilters> _listenerILD) { return false; };
		virtual std::shared_ptr<BRTServices::CSOSFilters> GetSOSFilter() const { return nullptr; }
		virtual void RemoveSOSFilter() {};
		
		virtual bool ConnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) { return false; };
		virtual bool DisconnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) { return false; };


		CBinauralFilterBase(const std::string & _binauraFilterID)
			: CModelBase(_binauraFilterID)
			, leftDataReady { false }
			, rightDataReady { false } {
			

			CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");
			CreateIDEntryPoint("listenerID");			
			
			CreateIDExitPoint();
			CreateSamplesExitPoint("leftEar");
			CreateSamplesExitPoint("rightEar");
		}

		/**
		 * @brief Check if this binaural filter is connected to a listener
		 * @return true if connected, false otherwise
		 */
		bool IsConnectedToListener() {
			std::string _listenerID = GetIDEntryPoint("listenerID")->GetData();
			if (_listenerID != "") {
				return true;
			}
			return false;
		}

	private:
		
		
		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		 * @param _entryPointId entryPoint ID
		*/

		void OneEntryPointOneDataReceived(std::string _entryPointId) override {

			if (_entryPointId == "leftEar") {
				if (!leftDataReady) {
					InitBuffer(leftBuffer);
				}
				CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("leftEar")->GetData();
				leftDataReady = MixEarBuffers(leftBuffer, newBuffer);
			} else if (_entryPointId == "rightEar") {
				if (!rightDataReady) {
					InitBuffer(rightBuffer);
				}
				CMonoBuffer<float> newBuffer = GetSamplesEntryPoint("rightEar")->GetData();
				rightDataReady = MixEarBuffers(rightBuffer, newBuffer);
			} else {
				//nothing
			}
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {

			GetSamplesExitPoint("leftEar")->sendData(leftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(rightBuffer);
			leftDataReady = false;
			rightDataReady = false;
		}

		void UpdateCommand() override {
			// TODO

			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") {
				return;
			}
			std::string a = command.GetStringParameter("listenerID");
		}


		///**
		// * @brief Mix the new buffer received with the contents of the buffer.
		//*/
		bool MixEarBuffers(CMonoBuffer<float> & buffer, const CMonoBuffer<float> & newBuffer) {
			if (newBuffer.size() != 0) {
				buffer += newBuffer;
				return true;
			}
			return false;
		}

		void InitBuffer(CMonoBuffer<float> & buffer) {
			buffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		}


		// Attributes
		Common::CGlobalParameters globalParameters;	
	
	protected:

		void SendMyID() { GetIDExitPoint()->sendData(modelID); }

		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;		
		bool leftDataReady;
		bool rightDataReady;
	};
}
#endif