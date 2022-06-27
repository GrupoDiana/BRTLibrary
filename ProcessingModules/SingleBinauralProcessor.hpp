#pragma once
#include "../brt/Base/ObserverBase.hpp"
#include "../Base/EntryPoint.hpp"

#include <memory>
#include <vector>
#include <algorithm>
    
namespace BRTProcessing {
    class CSingleBinauralProcessor : public BRTBase::CProcessorBase {
    public:
        CSingleBinauralProcessor() : leftGain{ 1.0f }, rightGain{ 1.0f } {
            CreateEntryPoint("inputSamples");
            CreateExitPoint("leftEar");
            CreateExitPoint("rightEar");
        }

        void Update() {
            std::vector<float> temp = GetEntryPoint("inputSamples")->GetData();
            Process(temp);
        }

        void setLeftGain(float _gain) { leftGain = _gain; }
        void setRighttGain(float _gain) { rightGain = _gain; }

    private:
        float leftGain;
        float rightGain;



        void Process(std::vector<float>& _inbuffer) {

            std::vector<float> _leftBuffer = _inbuffer;
            std::vector<float> _rightBuffer = _inbuffer;

            MultiplyVectorByValue(_leftBuffer, leftGain);
            GetExitPoint("leftEar")->sendData(_leftBuffer);

            MultiplyVectorByValue(_rightBuffer, rightGain);
            GetExitPoint("rightEar")->sendData(_rightBuffer);
        }


        void MultiplyVectorByValue(std::vector<float>& v, float k) {
            std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
        }
    };
}