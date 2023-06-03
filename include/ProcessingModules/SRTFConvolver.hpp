#ifndef _SRTF_CONVOLVER_
#define _SRTF_CONVOLVER_

#include <Common/UPCAnechoic.h>
#include <Common/Buffer.h>
#include <ServiceModules/SRTF.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON_GETSOURCECOORDINATES 0.0001f

namespace BRTProcessing {
	class CSRTFConvolver  {
	public:
		CSRTFConvolver() : enableSourceDirectivity{ false }, convolutionBuffersInitialized{false} { }


		///Enable spatialization process for this source	
		void EnableSourceDirectionality() { enableSourceDirectivity = true; }
		///Disable spatialization process for this source	
		void DisableSourceDirectionality() { enableSourceDirectivity = false; }
		///Get the flag for spatialization process enabling
		bool IsSourceDirectionalityEnabled() { return enableSourceDirectivity; }
		
		/////Enable HRTF interpolation method	
		//void EnableInterpolation() { enableInterpolation = true; }
		/////Disable HRTF interpolation method
		//void DisableInterpolation() { enableInterpolation = false; } 
		/////Get the flag for HRTF interpolation method
		//bool IsInterpolationEnabled() { return enableInterpolation; }

		
		
		/** \brief Process data from input buffer to generate spatialization by convolution
		*	\param [in] inBuffer input buffer with anechoic audio
		* *	\param [in] sourceTransform transform of the source
		* *	\param [in] listenerPosition transform of the listener
		* *	\param [in] listenerHRTFWeak weak smart pointer to the listener HRTF
		*	\param [out] outLeftBuffer output mono buffer with spatialized audio for the left channel
		*	\param [out] outRightBuffer output mono buffer with spatialized audio for the right channel
		*   \eh The error handler is informed if the size of the input buffer differs from that stored in the global
		*       parameters and if the HRTF of the listener is null.		   
		*/
		void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::shared_ptr<BRTServices::CSRTF>& _sourceSRTF) {

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
						
			// Check process flag
			if (!enableSourceDirectivity)
			{
				outBuffer = _inBuffer;
				return;
			}

			// Check listener HRTF
			if (!_sourceSRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "source SRTF pointer is null when trying to use in DirectivityConvolver");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { 
				InitializedSourceConvolutionBuffers(_sourceSRTF); 
			}

			// Calculate Source coordinates taking into account Source and Listener transforms
			float listener_azimuth;
			float listener_elevation;

			CalculateListenerCoordinates(sourceTransform, listenerTransform, _sourceSRTF, listener_elevation, listener_azimuth);

			//std::cout << "azimuth: " << listener_azimuth << ", elevation; " << listener_elevation << endl;

			//To do TESTING:
			//listener_azimuth = 5;
			//listener_elevation = 10;

			// GET SRTF
			CMonoBuffer<float>  dataDirectivityTF;
			std::vector<CMonoBuffer<float>>  dataDirectivityTF_vector;

			std::unordered_map<orientation, float> stepVector = _sourceSRTF->CalculateStep();
			
			dataDirectivityTF = _sourceSRTF->GetDirectivityTF(listener_azimuth, listener_elevation, stepVector).data;
			dataDirectivityTF_vector.push_back(dataDirectivityTF);


			// DO CONVOLUTION			
			outputUPConvolution.ProcessUPConvolution(_inBuffer, dataDirectivityTF_vector, outBuffer);
			//outputUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, dataDirectivityTF_vector, outBuffer);
		}

		/// Reset convolvers and convolution buffers
		void ResetSourceConvolutionBuffers() {
			convolutionBuffersInitialized = false;
			// Reset convolver classes
			outputUPConvolution.Reset();
		}
	private:

		// Atributes
		Common::CGlobalParameters globalParameters;
		Common::CUPCAnechoic outputUPConvolution;		// Object to make the inverse fft of the left channel with the UPC method

		bool enableSourceDirectivity;					// Flags for independent control of processes		
		bool convolutionBuffersInitialized;
		


		// METHODS
		// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CSRTF>& _sourceSRTF) {
			int directivityTFLength = _sourceSRTF->GetDirectivityTFLength();
			int numOfSubfilters = _sourceSRTF->GetDirectivityTFNumOfSubfilters();
			outputUPConvolution.Setup(globalParameters.GetBufferSize(), directivityTFLength, numOfSubfilters, false);			
			convolutionBuffersInitialized = true;
		}


		/// Calculates the parameters derived from the source and listener position
		void CalculateListenerCoordinates(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, std::shared_ptr<BRTServices::CSRTF>& _sourceSRTF, float& _listenerElevation, float& _listenerAzimuth)
		{
			//Get azimuth and elevation between listener and source
			Common::CVector3 vectorToListener = _sourceTransform.GetVectorTo(_listenerTransform);
			//std::cout << "Vector to Listener: " << vectorToListener << std::endl;
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