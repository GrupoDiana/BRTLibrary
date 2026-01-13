/**
* \class CascadeGraphicEq9OctaveBands
*
* \brief This is a chain of Low Shelf, Peak Notch and High Shelf filters that are used to implement a 9 octave band graphic equalizer.
* \date	Nov 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CASCADE_GRAPHIC_EQ_9_OCTAVE_BANDS_H_
#define _CASCADE_GRAPHIC_EQ_9_OCTAVE_BANDS_H_

#include <vector>
#include <Filters/FilterBase.hpp>
#include <ProcessingModules/BiquadFilter.hpp>
#include <ProcessingModules/BiquadFilterChain.hpp>


namespace BRTFilters {

    class CCascadeGraphicEq9OctaveBands : public CFilterBase, private BRTProcessing::CBiquadFilterChain {
        constexpr static int NUM_BANDS = 9;
        constexpr static float Q = 1.414213562373095; // sqrt(2)
        constexpr static float BANDS_CENTERS[NUM_BANDS] = {62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
        // Inverse matrix in 
        // Abel, J.S.; Berners, D.P. Filter design using second-order peaking and shelving sections. 
        // In Proceedings of the International Computer Music Conference, Coral Gables, FL, USA, 1–6 November 2004.

        constexpr static float inverseBmatrix[NUM_BANDS][NUM_BANDS] = 
        { { 1.3617, -0.3280,   0.0403,  0.0043,  0.0007,  0.0001,  0.0000,  0.0000,  0.0000},
        {-0.2750,  1.1128,  -0.2298, -0.0009, -0.0014, -0.0001, -0.0000, -0.0000, -0.0000 },
	    {-0.0023, -0.2138,   1.0915, -0.2207,  0.0001, -0.0012, -0.0001, -0.0000, -0.0000 },
	    {-0.0016, -0.0007,  -0.2172,  1.0919, -0.2196,  0.0000, -0.0012, -0.0001, -0.0000 },
	    {-0.0002, -0.0012,  -0.0006, -0.2184,  1.0922, -0.2187,  0.0004, -0.0010, -0.0000 },
	    {-0.0000, -0.0001,  -0.0012, -0.0005, -0.2209,  1.0932, -0.2158,  0.0020, -0.0005 },
	    {-0.0000, -0.0000,  -0.0001, -0.0013, -0.0008, -0.2277,  1.0969, -0.2028,  0.0064 },
	    {-0.0000, -0.0000,  -0.0000, -0.0002, -0.0018, -0.0036, -0.2632,  1.0738, -0.1416 },
	    { 0.0000,  0.0000,   0.0000,  0.0001,  0.0004,  0.0024,  0.0356, -0.1909,  1.1250 } }; 
        // FIXME: Check if this matrix might be valid only for 48kHz sampling rate

    public:

        /** 
         * \brief Default constructor which creates a chain of 9 octave bands graphic equalizer filters.
         */
		CCascadeGraphicEq9OctaveBands()
			: CFilterBase { BRTFilters::TFilterType::IIR_CASCADE_GRAPHIC_EQ}
            //, enable { false }
            //, generalGain { 0 }
			, commandGains { std::vector<float> (NUM_BANDS, 1.0f) }
        {            
            
			enable = false;
			generalGain = 0;			
			SetCommandGains(commandGains);
        }
        
        /** 
         * \brief Constructor which creates a chain of 9 octave bands graphic equalizer filters.
         * \param commandGains Vector of command gains at each band (note that these are not the peak gains of each inidividual filter)
        */
		CCascadeGraphicEq9OctaveBands(const std::vector<float> & _commandGains)
			: CFilterBase { BRTFilters::TFilterType::IIR_CASCADE_GRAPHIC_EQ }
            //,enable { false }
            //, generalGain { 0 } 
            , commandGains { std::vector<float>(NUM_BANDS, 1.0f) }
        {                        
            
			enable = false;
			generalGain = 0;
			SetCommandGains(_commandGains);
        }

        /** 
         * \brief Process an buffer through the whole set of filters
         * \param buffer Buffer to be processed
        */
        void Process(CMonoBuffer<float> &buffer) override {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (!enable) return;
            return CBiquadFilterChain::Process(buffer);
       }

       /** 
        * \brief Process an buffer through the whole set of filters
        */
       void Process(const CMonoBuffer<float> &buffer, CMonoBuffer <float> &output) override {
		   std::lock_guard<std::mutex> l(mutex); // Lock the mutex
           if (!enable) {
               output = buffer;
			   return;
           }
           return CBiquadFilterChain::Process(buffer, output);
        }

        /** 
         * \brief Remove all previously created filters
         */
        void RemoveFilters() {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			enable = false;
            return CBiquadFilterChain::RemoveFilters();
        }

        /** 
         * \brief Set the command gains of the filter at each band
         * \param samplingRate Sampling rate of the audio signal
         * \param gains Vector of command gains to be obtained at each band. Note that these are not the peak gains of each inidividual filter. 
         * \return True if the gains were set correctly, false otherwise
        */
		bool SetCommandGains(const std::vector<float> & _gains) override {        
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
            if (_gains.size() != NUM_BANDS) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "CascadeGraphicEq9OctaveBands: gains vector must have " + std::to_string(NUM_BANDS) + " elements");
                return false;
            }
            else {
                commandGains = _gains;
                ResetFiltersChain(CalculatePeakGains());
                enable = true;
                return true;
            }
        }

        /** 
         * \brief Get the effective gains of the filters in the chain
        */
        std::vector<float> GetCommandGains() {
            return commandGains;
        }

        /** 
         * \brief Enable the processing of the filter chain
        */
        //void Enable() {	enable = true; }

        /** 
         * \brief Disable the processing of the filter chain
        */
        //void Disable() { enable = false; }

        /** 
         * \brief Check if the processing of the filter chain is enabled
         * \retval true if the processing is enabled, false otherwise
        */
        //bool IsEnabled() const { return enable; }


#ifndef NDEBUG
        /**
         * @brief Get the filters in the chain 
         * 
         */
		std::shared_ptr<BRTProcessing::CBiquadFilter> GetFilter(int index) {
            return CBiquadFilterChain::GetFilter(index);
        }
#endif
    private:


        /** 
         * \brief Calculate the command gains for the filters in the chain
         * \param gains Vector of gains to be set
        */
        std::vector<float> CalculatePeakGains() {

            // Convert effective gains to dB
            std::vector<float> commandGainsDB(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                commandGainsDB[i] = 20.0 * std::log10(commandGains[i]);
            }
            float meanCommandGainDB = 0.0;
            
            // Calculate mean command gain
            for (int i = 0; i < NUM_BANDS; i++) {
                meanCommandGainDB += commandGainsDB[i];
            }
            meanCommandGainDB /= NUM_BANDS;

            // Substract mean command gain
            for (int i = 0; i < NUM_BANDS; i++) {
                commandGainsDB[i] -= meanCommandGainDB;
            }
            
            std::vector<float> peakGainsDB(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                    peakGainsDB[i] = 0.0;
                for (int j = 0; j < NUM_BANDS; j++) {
                    peakGainsDB[i] += inverseBmatrix[i][j] * commandGainsDB[j];
                }
            }
            
            // store the mean command gain in generalGain
            generalGain = std::pow(10.0, meanCommandGainDB / 20.0);
            
            // Convert command gains to linear scale
            std::vector<float> peakGains(NUM_BANDS, 0.0);
            for (int i = 0; i < NUM_BANDS; i++) {
                peakGains[i] = std::pow(10.0, peakGainsDB[i] / 20.0);
            }

            return peakGains;
        } 

        /** 
         * \brief Reset the filters chain with the new peak gains
         * \param gains Vector of gains to be set
        */
       void ResetFiltersChain(const std::vector<float> & peakGains) {            
		    CBiquadFilterChain::RemoveFilters();
            // Create first low shelf filter
			auto propagationFilter = CBiquadFilterChain::AddFilter();
            constexpr float dummyQ = 1.0; // Not used inside the low shelf filter nor the high shelf filter
			propagationFilter->Setup(62.5 * Q, dummyQ, BRTProcessing::TBiquadType::LOWSHELF, peakGains[0]);

            // Create 7 peak notch filters
            for (int i = 1; i < 8; i++) {
				propagationFilter = CBiquadFilterChain::AddFilter();
				propagationFilter->Setup(BANDS_CENTERS[i], Q, BRTProcessing::TBiquadType::PEAKNOTCH, peakGains[i]);
            }

            // Create last high shelf filter
			propagationFilter = CBiquadFilterChain::AddFilter();
			propagationFilter->Setup(16000 / Q, dummyQ, BRTProcessing::TBiquadType::HIGHSHELF, peakGains[8]); 

            // Set general gain of last filter
            propagationFilter->SetGeneralGain(generalGain);                        
       }
       
       //////////////
       // Attributes
       //////////////
	   mutable std::mutex mutex; // To avoid access collisions
	   //bool enable; 

       /** 
        * \brief Latest vector of commnand gains of the filters in the chain
        * set via SetCommandGains
       */
	   std::vector<float> commandGains;

	   /**
        * \brief general gain applied to last filter, updated in CalculatePeakGains
        */
	   //float generalGain;

       
    };
}
#endif
