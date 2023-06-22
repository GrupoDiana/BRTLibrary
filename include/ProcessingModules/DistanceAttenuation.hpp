/**
* \class CDistanceAttenuation
*
* \brief Declaration of CDistanceAttenuation class
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
#ifndef _DISTANCE_ATTENUATION_
#define _DISTANCE_ATTENUATION_

#include <Common/CommonDefinitions.hpp>
#include <Common/Buffer.hpp>
#include <memory>
#include <vector>
#include <algorithm>


#define EPSILON_ATT 0.0001
#define EPSILON_DISTANCE  0.0001f			
#define FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB -6.0206f		///< std::log10(0.5f) * 20.0f	

namespace BRTProcessing {
    

    class CDistanceAttenuation
    {
    public:
        CDistanceAttenuation() : previousAttenuation_Channel{ 0.0f }, referenceDistance{ REFERENCE_DISTANCE_ATTENUATION }, enabledDistanceAttenuation {true} {
        }
                        
        ///Enable distance attenuation 	
        void EnabledDistanceAttenuation() { enabledDistanceAttenuation = true; }
        ///Disable distance attenuation 	
        void DisableDistanceAttenuation() { enabledDistanceAttenuation = false; }
        ////Get the flag for distance attenuation process enabling 	
        bool IsEnabledDistanceAttenuation() { return enabledDistanceAttenuation; };


        // Methods        
        void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outBuffer, Common::CTransform sourceTransform, Common::CTransform listenerTransform) {
            
            ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

            outBuffer = _inBuffer;

            if (!enabledDistanceAttenuation) {                                
                return;
            }

            // Calculate distance
            float distance = CalculateDistance(sourceTransform, listenerTransform);

            // Attenuation is computed
            float extraAttennuation_dB = 0;
            float attenuation = GetDistanceAttenuation(globalParameters.GetAnechoicDistanceAttenuation(), distance, extraAttennuation_dB);

            //Apply attenuation gradually using Exponential Moving Average method
            float unnecessary_fixme;
            if (outBuffer.size() != 0)
                outBuffer.ApplyGainExponentially(previousAttenuation_Channel, unnecessary_fixme, attenuation, globalParameters.GetBufferSize(), globalParameters.GetSampleRate());

        }

    private:        
        bool enabledDistanceAttenuation;
        Common::CGlobalParameters globalParameters;
        float referenceDistance;                        // Distance at which the attenuation is 0 dB, in meters.
        float previousAttenuation_Channel;


       
        /////////////////////
        /// PRIVATE Methods        
        /////////////////////

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