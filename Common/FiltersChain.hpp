/**
* \class CFiltersChain
*
* \brief Declaration of FiltersChain class interface.
* \date	July 2016
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#ifndef _CFILTERS_CHAIN_H_
#define _CFILTERS_CHAIN_H_

#include <Common/BiquadFilter.hpp>
#include <Common/ErrorHandler.hpp>
#include <vector>
#include <memory>

//using namespace std;  //TODO: Try to avoid this

namespace Common {

	/** \brief Type definition for a set of coefficients of a filters chain
	*/
	typedef std::vector<TBiquadCoefficients> TFiltersChainCoefficients;

	/** \details Class to handle a set of cascade digital filters that are arranged so the samples are processed along a pipeline
	*/
	class CFiltersChain
	{
	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor
		*   \eh On error, an error code is reported to the error handler.
		*/
		CFiltersChain() {};


		/** \brief Create and add a new CBiquadFilter object to the chain
		*	\retval filter shared pointer to filter created and added to the bank
		*   \throws May throw exceptions and errors to debugger
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		shared_ptr <CBiquadFilter> AddFilter()
		{
			try
			{
				shared_ptr<CBiquadFilter> newFilter(new CBiquadFilter());
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
		shared_ptr <CBiquadFilter> GetFilter(int index)
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
			//SET_RESULT(RESULT_OK, "");
			for (std::size_t c = 0; c < filters.size(); c++)
			{
				shared_ptr<CBiquadFilter> f = filters[c];
				if (f != NULL)
					f->Process(buffer);
			}
		}

		/** \brief Setup a filters chain from a vector of (ordered) coefficients for any number of biquads	
		*	\details If the number of coefficients in the vector fits the current number of filters in the chain, the existing filter coefficients are set, 
		*	instead of creating a new filters chain from scratch
		*	\param [in] coefficients vector of ordered coefficients for all biquads in the chain				
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromCoefficientsVector(TFiltersChainCoefficients& coefficients)
		{
			if (coefficients.size() == filters.size())
			{
				// Set existing filters
				for (int i = 0; i < coefficients.size(); i++)
				{
					filters[i]->SetCoefficients(coefficients[i]);
				}
			}
			else
			{
				// Create chain from scratch
				RemoveFilters();
				for (int i = 0; i < coefficients.size(); i++)
				{
					shared_ptr<Common::CBiquadFilter> newBiquad = AddFilter();
					newBiquad->SetCoefficients(coefficients[i]);
				}
			}
		}

	private:
		////////////////////////
		// PRIVATE ATTRIBUTES
		////////////////////////
		vector<shared_ptr<CBiquadFilter>> filters;                      // Hold the filters in the chain. 
																		// Indexes indicate the order within the chain.
	};
}//end namespace Common
#endif