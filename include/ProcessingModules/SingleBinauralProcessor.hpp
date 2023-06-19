/**
* \class CSingleBinauralProcessor
*
* \brief Declaration of CSingleBinauralProcessor class
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

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