#ifndef _DISTANCE_ATTENUATION_PROCESSOR_
#define _DISTANCE_ATTENUATION_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>

#include <memory>
#include <vector>
#include <algorithm>


#define EPSILON_ATT 0.0001
#define EPSILON_DISTANCE  0.0001f			
#define FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB -6.0206f		///< std::log10(0.5f) * 20.0f	

namespace BRTProcessing {
    

    class CDistanceAttenuationProcessor : public BRTBase::CProcessorBase
    {
    public:
        CDistanceAttenuationProcessor() : previousAttenuation_Channel{ 0.0f }, referenceDistance{ DISTANCE_MODEL_THRESHOLD_NEAR }, enabled {true} {
            CreateSamplesEntryPoint("inputSamples");            
            CreateSamplesExitPoint("outputSamples");

            CreatePositionEntryPoint("sourcePosition");
            CreatePositionEntryPoint("listenerPosition");            
        }
        
        void Update(std::string _entryPointId) {
            if (_entryPointId == "inputSamples") {
                CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
                Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
                Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

                this->resetUpdatingStack();

                Process(buffer, sourcePosition, listenerPosition);
            }            
        }

        void Enabled() { enabled = true; }
        void Disable() { enabled = false; }
        bool IsEnabled() { return enabled; };


    private:        
        bool enabled;
        Common::CGlobalParameters globalParameters;
        float referenceDistance;                        // Distance at which the attenuation is 0 dB, in meters.
        float previousAttenuation_Channel;


        // Methods        
        void Process(CMonoBuffer<float>& inbuffer, Common::CTransform sourceTransform, Common::CTransform listenerTransform) {

            if (!enabled) {                
                GetSamplesExitPoint("outputSamples")->sendData(inbuffer);       // Send output buffer to next module
                return;
            }
                                                                                   
            // Calculate distance
            float distance = CalculateDistance(sourceTransform, listenerTransform);                        
            
            // Attenuation is computed
            float extraAttennuation_dB = 0;
            float attenuation = GetDistanceAttenuation(globalParameters.GetAnechoicDistanceAttenuation(), distance, extraAttennuation_dB);

            //Apply attenuation gradually using Exponential Moving Average method
            float unnecessary_fixme;
            if (inbuffer.size() != 0) inbuffer.ApplyGainExponentially(previousAttenuation_Channel, unnecessary_fixme, attenuation, globalParameters.GetBufferSize(), globalParameters.GetSampleRate());

            // Send output buffer to next module
            GetSamplesExitPoint("outputSamples")->sendData(inbuffer);
        }


        float GetDistanceAttenuation(float attenuationForDuplicateDistance, float distance, float extraAttennuation_dB) const
        {
            // Error handler:
            if (distance <= 0.0f)
            {
                SET_RESULT(RESULT_ERROR_DIVBYZERO, "Attempt to compute distance attenuation for a negative or zero distance");
                return 1.0f;
            }
            //else
            //	SET_RESULT(RESULT_OK, "Distance attenuation returned succesfully for single source");	

            // Compute gain
            if (distance > EPSILON_DISTANCE && std::fabs(attenuationForDuplicateDistance) > EPSILON_ATT)
            {
                // Compute attenuation factor
                float attenuationFactor = attenuationForDuplicateDistance / FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB;

                return std::pow(10.0f, extraAttennuation_dB + attenuationFactor * std::log10(referenceDistance / distance));
            }
            else
                return 1.0;
        }

        float CalculateDistance(Common::CTransform _sourceTransform, Common::CTransform _listenerTransform) {

            //Get azimuth and elevation between listener and source
            Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
            float _distanceToListener = _vectorToListener.GetDistance();
            return _distanceToListener;
        }
       
    };
}
#endif