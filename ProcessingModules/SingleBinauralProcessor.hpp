#pragma once
#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>

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
            CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
            Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
            Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
            this->resetUpdatingStack();

            Process(buffer, sourcePosition, listenerPosition);
        }

        void setLeftGain(float _gain) { leftGain = _gain; }
        void setRighttGain(float _gain) { rightGain = _gain; }

    private:
        float leftGain;
        float rightGain;

        
        void Process(CMonoBuffer<float>& _inbuffer, Common::CTransform sourcePosition, Common::CTransform listenerPosition) {

            CMonoBuffer<float> _leftBuffer = _inbuffer;
            CMonoBuffer<float> _rightBuffer = _inbuffer;

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