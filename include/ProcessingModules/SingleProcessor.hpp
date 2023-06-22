/**
* \class CSingleProcessor
*
* \brief Declaration of CSingleProcessor class
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
* \b Copyright: University of Malaga 2023. Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#pragma once

#include <Base/ProcessorBase.hpp>
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
            CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
            Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
            this->resetUpdatingStack();

            Process(buffer, sourcePosition);
        }

    private:
        float gain;


        // Methods        
        void Process(CMonoBuffer<float>& inbuffer, Common::CTransform sourcePosition) {
            MultiplyVectorByValue(inbuffer, gain);
            GetSamplesExitPoint("outputSamples")->sendData(inbuffer);
        }

        void MultiplyVectorByValue(std::vector<float>& v, float k) {
            std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
        }
    };
}
