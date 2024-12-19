/**
* \class CDirectivityConvolver
*
* \brief Declaration of CDirectivityTFConvolver class
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
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _DIRECTIVITYTF_CONVOLVER_
#define _DIRECTIVITYTF_CONVOLVER_

#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <ServiceModules/DirectivityTF.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON_GETSOURCECOORDINATES 0.0001f

namespace BRTProcessing {
	class CDirectivityTFConvolver  {
	public:
		CDirectivityTFConvolver() : enableSourceDirectivity{false}, convolutionBuffersInitialized{false} { }

		//Enable spatialization process for this source	
		void EnableSourceDirectionality() { enableSourceDirectivity = true; }
		//Disable spatialization process for this source	
		void DisableSourceDirectionality() { enableSourceDirectivity = false; }
		//Get the flag for spatialization process enabling
		bool IsSourceDirectionalityEnabled() { return enableSourceDirectivity; }
					
		
		/** \brief Process data from input buffer to generate spatialization by convolution
		* *	\param [in] inBuffer input buffer with anechoic audio
		* *	\param [in] sourceTransform transform of the source
		* *	\param [in] listenerTransform transform of the listener
		* *	\param [in] _sourceDirectivityTF shared pointer to the DirectivityTF class that will be applied to a sourc
		*	\param [out] outBuffer output mono buffer with spatialized audio
		*   \eh The error handler is informed if the size of the input buffer differs from that stored in the global
		*       parameters and if the DirectivityTF of the listener is null.		   
		*/
		void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::shared_ptr<BRTServices::CDirectivityTF>& _sourceDirectivityTF) {

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");	
			// Check process flag
			if (!enableSourceDirectivity)
			{
				outBuffer = _inBuffer;
				return;
			}
			// Check listener HRTF
			if (!_sourceDirectivityTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "source DirectivityTF pointer is null when trying to use in DirectivityConvolver");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}
			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { 
				InitializedSourceConvolutionBuffers(_sourceDirectivityTF); 
			}
			// Calculate Source coordinates taking into account Source and Listener transforms
			float listener_azimuth;
			float listener_elevation;
			CalculateListenerCoordinates(sourceTransform, listenerTransform, _sourceDirectivityTF, listener_elevation, listener_azimuth);

			// Get DirectivityTF
			CMonoBuffer<float>  dataDirectivityTF;
			std::vector<CMonoBuffer<float>>  dataDirectivityTF_vector;
			dataDirectivityTF = _sourceDirectivityTF->GetDirectivityTF(listener_azimuth, listener_elevation, true).front(); //IMPORTANT: We only have one partition in the Directivity
			dataDirectivityTF_vector.push_back(dataDirectivityTF);
			// Do convolution			
			outputUPConvolution.ProcessUPConvolution(_inBuffer, dataDirectivityTF_vector, outBuffer);
		}

		/**
		 * @brief Reset convolvers and convolution buffers
		*/
		void ResetSourceConvolutionBuffers() {
			convolutionBuffersInitialized = false;
			outputUPConvolution.Reset();
		}
	private:

		Common::CGlobalParameters globalParameters;
		BRTProcessing::CUniformPartitionedConvolution outputUPConvolution; // Object to make the inverse fft of the left channel with the UPC method

		bool enableSourceDirectivity;					// Flags for independent control of processes		
		bool convolutionBuffersInitialized;
		
		/** \brief Initialize convolvers and convolition buffers
		*	\param [in] _sourceDirectivityTF shared pointer to the DirectivityTF class that will be applied to a source
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CDirectivityTF>& _sourceDirectivityTF) {
			int directivityTFLength = _sourceDirectivityTF->GetDirectivityTFLength();
			int numOfSubfilters = _sourceDirectivityTF->GetDirectivityTFNumOfSubfilters();
			outputUPConvolution.Setup(globalParameters.GetBufferSize(), directivityTFLength, numOfSubfilters, false);			
			convolutionBuffersInitialized = true;
		}

		/** \brief Calculates the parameters derived from the source and listener position
		* *	\param [in] _sourceTransform transform of the source
		* *	\param [in] _listenerTransform transform of the listener
		* *	\param [in] _sourceDirectivityTF shared pointer to the DirectivityTF class that will be applied to a sourc
		*	\param [out] _listenerElevation output listener elevation (from the head center) float value
		* *	\param [out] _listenerAzimuth output listener azimuth (from the head center) float value
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		void CalculateListenerCoordinates(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, std::shared_ptr<BRTServices::CDirectivityTF>& _sourceDirectivityTF, float& _listenerElevation, float& _listenerAzimuth)
		{
			//Get azimuth and elevation between listener and source
			Common::CVector3 vectorToListener = _sourceTransform.GetVectorTo(_listenerTransform);
			float distance = vectorToListener.GetDistance();

			//Check listener and source are in the same position
			if (distance <= MINIMUM_DISTANCE_SOURCE_LISTENER) {
				SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CDirectivityConvolver");
				distance = MINIMUM_DISTANCE_SOURCE_LISTENER;
			}

			_listenerElevation = vectorToListener.GetElevationDegrees();		//Get elevation from the head center
			if (!Common::AreSame(ELEVATION_NORTH_POLE, _listenerElevation, EPSILON_GETSOURCECOORDINATES) && !Common::AreSame(ELEVATION_SOUTH_POLE, _listenerElevation, EPSILON_GETSOURCECOORDINATES))
			{
				_listenerAzimuth = vectorToListener.GetAzimuthDegrees();		//Get azimuth from the head center
			}
			else {
				_listenerAzimuth = 0.0f;
			}
		}
	};
}
#endif