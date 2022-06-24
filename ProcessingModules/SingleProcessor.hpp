#pragma once

#include "../Base/ProcessorBase.hpp"
#include <memory>
#include <vector>
#include <algorithm>
 
namespace BRTProcessing {
    class CSingleProcessor : public BRTBase::CProcessorBase<CSingleProcessor>
    {
    public:
        CSingleProcessor() : gain{ 1.0f } {
            CreateEntryPoint(*this, "inputSamples");
            CreateExitPoint("outputSamples");

        }

        void setGain(float _gain) { gain = _gain; }

        void Update() {
            std::vector<float> temp = GetEntryPoint("inputSamples")->GetData();
            Process(temp);
        }

    private:
        float gain;


        // Methods
        void Process(std::vector<float>& inbuffer) {
            MultiplyVectorByValue(inbuffer, gain);
            GetExitPoint("outputSamples")->sendData(inbuffer);
        }

        void MultiplyVectorByValue(std::vector<float>& v, float k) {
            std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
        }
    };
}
