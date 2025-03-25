#ifndef _DISTANCE_ATTENUATOR_PROCESSOR_HPP_
	#define _DISTANCE_ATTENUATOR_PROCESSOR_HPP_

#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Base/BRTManager.hpp>
#include <Connectivity/BRTConnectivity.hpp>
#include <ProcessingModules/DistanceAttenuator.hpp>

namespace BRTProcessing {
class CDistanceAttenuatorProcessor : public BRTConnectivity::CBRTConnectivity, public CDistanceAttenuator {
	public:
		CDistanceAttenuatorProcessor(){

			CreateSamplesEntryPoint("inputSamples");
			CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");
			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

			CreateSamplesExitPoint("outputSamples");
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			/*if (!initialized) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "The Free field environment processor is not initialized");
				return;
			}*/

			// Get data from entry points
			CMonoBuffer<float> inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
			Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

			if (inBuffer.size() == 0) {				
				SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is 0");
				return;
			}

			CMonoBuffer<float> outBuffer;			
			Process(inBuffer, outBuffer, sourcePosition, listenerPosition);		
			GetSamplesExitPoint("outputSamples")->sendData(outBuffer);
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void UpdateCommand() override {

			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") {
				return;
			}

			//std::string sourceID = GetIDEntryPoint("sourceID")->GetData();
			//if (sourceID == command.GetStringParameter("sourceID")) {
			//	// Propagete the command to the virtual sources
			//	nlohmann::json j;
			//	j["command"] = command.GetCommand();
			//	j["sourceID"] = virtualSource->GetID();
			//	brtManager->ExecuteCommand(j.dump());
			//}
		}

		private:
		/////////////////
		// Attributes
		/////////////////

		mutable std::mutex mutex; // To avoid access collisions		
};
}
#endif