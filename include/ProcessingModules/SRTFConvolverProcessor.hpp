#ifndef SRTF_CONVOLVER_PROCESSOR_
#define SRTF_CONVOLVER_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.h>
#include <Common/Buffer.h>
#include <ProcessingModules/SRTFConvolver.hpp>

#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CSRTFConvolverProcessor : public BRTBase::CProcessorBase, CSRTFConvolver {
		
    public:
		CSRTFConvolverProcessor() {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           
			CreateSRTFPtrEntryPoint("sourceSRTF");

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

            CreateSamplesExitPoint("outSamples");  									
        }

        void Update(std::string _entryPointId) {            
			
			std::lock_guard<std::mutex> l(mutex);

			CMonoBuffer<float> outBuffer;

			if (_entryPointId == "inputSamples") {
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
				std::weak_ptr<BRTServices::CSRTF> sourceSRTF = GetSRTFPtrEntryPoint("sourceSRTF")->GetData();
				if (buffer.size() != 0) {
					//Process(buffer, outBuffer, sourcePosition, listenerPosition, sourceSRTF);
					GetSamplesExitPoint("outSamples")->sendData(outBuffer);
				}				
				this->resetUpdatingStack();				
			}            
        }

		void UpdateCommand() {					
			
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
															
			//if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
			//	if (command.GetCommand() == "/listener/enableSpatialization") {					
			//		if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
			//		else { DisableSpatialization(); }
			//	}
			//	else if (command.GetCommand() == "/listener/enableInterpolation") {					
			//		if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
			//		else { DisableInterpolation(); }
			//	}
			////}

			//if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
			//	if (command.GetCommand() == "/source/HRTFConvolver/resetBuffers") {
			//		ResetSourceConvolutionBuffers();
			//	}
			//}
		} 
      
    private:
       
		mutable std::mutex mutex;

		bool IsToMySoundSource(std::string _sourceID) {
			std::string mySourceID = GetIDEntryPoint("sourceID")->GetData();
			return mySourceID == _sourceID;
		}
		
		bool IsToMyListener(std::string _listenerID) {
			std::string myListenerID = GetIDEntryPoint("listenerID")->GetData();
			return myListenerID == _listenerID;
		}
    };
}
#endif