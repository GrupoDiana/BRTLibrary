/**
* \class CBiquadFilterChain
*
* \brief Declaration of CBiquadFilterChain class interface.
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _BIQUAD_FILTER_CHAIN_HPP_
#define _BIQUAD_FILTER_CHAIN_HPP_

#include <vector>
#include <memory>
#include <Common/ErrorHandler.hpp>
#include <ProcessingModules/BiquadFilter.hpp>


namespace BRTProcessing {

	/** \brief Type definition for a set of coefficients of a filters chain
	*/
	typedef std::vector<TBiquadCoefficients> TFiltersChainCoefficients;

	/** \details Class to handle a set of cascade digital filters that are arranged so the samples are processed along a pipeline
	*/
	class CBiquadFilterChain
	{
	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor
		*   \eh On error, an error code is reported to the error handler.
		*/
		CBiquadFilterChain() {};

			
		/**
		 * @brief Add multiple biquad filters to the chain.
		 * @param nFilters number of filters to add
		 */
		void AddNFilters(int nFilters) {
			if (nFilters < 0) return;
			for (int i = 0; i < nFilters; i++) {
				AddFilter();
			}
		}
		/** \brief Create and add a new CBiquadFilter object to the chain
		*	\retval filter shared pointer to filter created and added to the bank
		*   \throws May throw exceptions and errors to debugger
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <CBiquadFilter> AddFilter()
		{
			try
			{
				std::shared_ptr<CBiquadFilter> newFilter(new CBiquadFilter());
				filters.push_back(newFilter);

				SET_RESULT(RESULT_OK, "Filter added to filter chain succesfully");
				return newFilter;
			}
			catch (std::bad_alloc& ba)
			{
				//SET_RESULT(RESULT_ERROR_BADALLOC, ba.what());
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}

		/** \brief Get one filter from the chain
		*	\param [in] index ID of the filter within the chain
		*	\retval filter shared pointer to filter from the chain
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <CBiquadFilter> GetFilter(int index)
		{
			if (index < 0 || filters.size() <= index)
			{
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to get a filter from filter chain outside chain size");
				return NULL;
			}
			else
			{
				//SET_RESULT(RESULT_OK, "Succesfully got filter from filter chain");
				return filters[index];
			}
		}


		/** \brief Remove all previously created filters.
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void RemoveFilters()
		{
			filters.clear();

			SET_RESULT(RESULT_OK, "All filters succesfully removed from filter chain");
		}

		/** \brief Get the current number of filters in the chain
		*	\retval n Current number of filters in the chain
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumFilters()
		{
			return filters.size();
		}

		/** \brief Process an buffer through the whole set of filters
		*	\details The buffer is processed through each filter in the bank in chain.
		*	\param [in,out] buffer input and output buffer		
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer <float> & buffer)
		{			
			for (std::shared_ptr<CBiquadFilter> itFilter : filters) {
				if (itFilter != nullptr)
				{
					itFilter->Process(buffer);
				}
			}
		}

		/**
		 * @brief Processes a mono audio buffer through a sequence of biquad filters.
		 * @param buffer Reference to the input mono buffer to be processed.
		 * @param outBuffer Reference to the output mono buffer where processed data is stored.
		 */
		void Process(const CMonoBuffer<float> & buffer, CMonoBuffer<float> & outBuffer) {
			
			for (std::size_t c = 0; c < filters.size(); c++) {
				/*shared_ptr<CBiquadFilter> f = filters[c];
				if (f != NULL)
					f->Process(buffer, outBuffer);
				buffer = outBuffer;*/
				if (filters[c]!= nullptr)
				{
					filters[c]->Process(buffer, outBuffer);
					//buffer = outBuffer;
				}
			}
		}

		/** \brief Setup a filters chain from a vector of (ordered) coefficients for any number of biquads	
		*	\details If the number of coefficients in the vector fits the current number of filters in the chain, the existing filter coefficients are set, 
		*	instead of creating a new filters chain from scratch
		*	\param [in] coefficients vector of ordered coefficients for all biquads in the chain				
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromCoefficientsVector(TFiltersChainCoefficients& coefficients, bool _crossfadingEnabled = true)
		{
			if (coefficients.size() == filters.size())
			{
				// Set existing filters
				for (int i = 0; i < coefficients.size(); i++)
				{
					filters[i]->Setup(coefficients[i], _crossfadingEnabled);
				}
			}
			else
			{
				// Create chain from scratch
				RemoveFilters();
				for (int i = 0; i < coefficients.size(); i++)
				{
					std::shared_ptr<BRTProcessing::CBiquadFilter> newBiquad = AddFilter();
					newBiquad->Setup(coefficients[i]);
				}
			}
		}
		

		void ResetBuffers() {			
			for (int i = 0; i < filters.size(); i++)
			{
				filters[i]->ResetBuffers();
			}
		}

	private:
		////////////////////////
		// PRIVATE ATTRIBUTES
		////////////////////////
		std::vector<std::shared_ptr<CBiquadFilter>> filters;                    // Hold the filters in the chain. 
																				// Indexes indicate the order within the chain.
	};
}//end namespace Common
#endif