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
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
            CreatePositionEntryPoint("listenerPosition");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");            
        }

        void Update() {            
            std::vector<float> buffer = GetSamplesEntryPoint("inputSamples")->getAttr();
            int sourcePosition = GetPositionEntryPoint("sourcePosition")->getAttr();
            int listenerPosition = GetPositionEntryPoint("listenerPosition")->getAttr();
            this->resetUpdatingStack();

            Process(buffer, sourcePosition, listenerPosition);
        }

        void setLeftGain(float _gain) { leftGain = _gain; }
        void setRighttGain(float _gain) { rightGain = _gain; }

    private:
        float leftGain;
        float rightGain;

        
        void Process(std::vector<float>& _inbuffer, int sourcePosition, int listenerPosition) {

            std::vector<float> _leftBuffer = _inbuffer;
            std::vector<float> _rightBuffer = _inbuffer;

            MultiplyVectorByValue(_leftBuffer, leftGain);
            GetSamplesExitPoint("leftEar")->sendData(_leftBuffer);

            MultiplyVectorByValue(_rightBuffer, rightGain);
            GetSamplesExitPoint("rightEar")->sendData(_rightBuffer);            
        }


        void MultiplyVectorByValue(std::vector<float>& v, float k) {
            std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
        }
    };
}