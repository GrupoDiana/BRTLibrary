#pragma once

#include "../Base/ProcessorBase.hpp"
#include <memory>
#include <vector>
#include <algorithm>
 
namespace BRTProcessing {
    class CSingleProcessor : public BRTBase::CProcessorBase
    {
    public:
        CSingleProcessor() : gain{ 1.0f } {
            CreateSamplesEntryPoint("inputSamples");            
            CreatePositionEntryPoint("sourcePosition");

            CreateSamplesExitPoint("outputSamples");
        }

        void setGain(float _gain) { gain = _gain; }

        void Update() {
            std::vector<float> buffer = GetSamplesEntryPoint("inputSamples")->getAttr();            
            int position = GetPositionEntryPoint("sourcePosition")->getAttr();
            this->resetUpdatingStack();

            Process(buffer, position);
        }

    private:
        float gain;


        // Methods        
        void Process(std::vector<float>& inbuffer, int sourcePosition) {
            std::cout << "Source position: " << sourcePosition << std::endl;
            MultiplyVectorByValue(inbuffer, gain);
            GetSamplesExitPoint("outputSamples")->sendData(inbuffer);
        }

        void MultiplyVectorByValue(std::vector<float>& v, float k) {
            std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
        }
    };
}
