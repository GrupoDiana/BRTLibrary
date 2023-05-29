#ifndef _DIRECTIVITY_CONVOLVER_
#define _DIRECTIVITY_CONVOLVER_

#include <Common/UPCAnechoic.h>
#include <Common/Buffer.h>
#include <ServiceModules/SRTF.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON_GETSOURCECOORDINATES 0.0001f

namespace BRTProcessing {
	class CDirectivityConvolver  {
	public:
		CDirectivityConvolver() : enableInterpolation{ true }, enableSourceDirectionality{ true }, convolutionBuffersInitialized{false} { }


		///Enable spatialization process for this source	
		void EnableSourceDirectionality() { enableSourceDirectionality = true; }
		///Disable spatialization process for this source	
		void DisableSourceDirectionality() { enableSourceDirectionality = false; }
		///Get the flag for spatialization process enabling
		bool IsSourceDirectionalityEnabled() { return enableSourceDirectionality; }
		
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
		void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CSRTF>& _sourceSRTFWeak) {

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
						
			// Check process flag
			if (!enableSourceDirectionality)
			{
				outLeftBuffer = _inBuffer;
				outRightBuffer = _inBuffer;
				return;
			}

			// Check listener HRTF
			std::shared_ptr<BRTServices::CSRTF> _sourceSRTF = _sourceSRTFWeak.lock();
			if (!_sourceSRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "source SRTF pointer is null when trying to use in DirectivityConvolver");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// First time - Initialize convolution buffers
			//if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_sourceSRTF); }

			// Calculate Source coordinates taking into account Source and Listener transforms
			float listener_azimuth;
			float listener_elevation;


			CalculateListenerCoordinates(sourceTransform, listenerTransform, _sourceSRTF, listener_elevation, listener_azimuth);

			// GET HRTF
			CMonoBuffer<float>  dataDirectivityTF_real;
			CMonoBuffer<float>  dataDirectivityTF_imag;

			dataDirectivityTF_real = (_sourceSRTF->GetDirectivityTF(listener_azimuth, listener_elevation)).dataReal;
			dataDirectivityTF_imag = (_sourceSRTF->GetDirectivityTF(listener_azimuth, listener_elevation)).dataImag;


			// DO CONVOLUTION			
		//	outputLeftUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
		//	outputRightUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);

		}

		/// Reset convolvers and convolution buffers
		//void ResetSourceConvolutionBuffers() {
		//	convolutionBuffersInitialized = false;
		//	// Reset convolver classes
		//	outputLeftUPConvolution.Reset();
		//	outputRightUPConvolution.Reset();
		//	//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
		//	leftChannelDelayBuffer.clear();
		//	rightChannelDelayBuffer.clear();
		//}
	private:

		// Atributes
		Common::CGlobalParameters globalParameters;

		Common::CUPCAnechoic outputLeftUPConvolution;		// Object to make the inverse fft of the left channel with the UPC method
		Common::CUPCAnechoic outputRightUPConvolution;		// Object to make the inverse fft of the rigth channel with the UPC method

		//CMonoBuffer<float> leftChannelDelayBuffer;			// To store the delay of the left channel of the expansion method
		//CMonoBuffer<float> rightChannelDelayBuffer;			// To store the delay of the right channel of the expansion method

		bool enableSourceDirectionality;								// Flags for independent control of processes
		bool enableInterpolation;							// Enables/Disables the interpolation on run time
		bool convolutionBuffersInitialized;
		
		/// Initialize convolvers and convolition buffers		
		//void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CSRTF>& _sourceSRTF) {

		//	int numOfSubfilters = _sourceSRTF->GetHRIRNumberOfSubfilters();
		//	int subfilterLength = _sourceSRTF->GetHRIRSubfilterLength();

		//	//Common::CGlobalParameters globalParameters;
		//	outputLeftUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
		//	outputRightUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
		//	//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
		//	leftChannelDelayBuffer.clear();
		//	rightChannelDelayBuffer.clear();

		//	// Declare variable
		//	convolutionBuffersInitialized = true;
		//}


		/// Calculates the parameters derived from the source and listener position
		void CalculateListenerCoordinates(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, std::shared_ptr<BRTServices::CSRTF>& _sourceSRTF, float& _listenerElevation, float& _listenerAzimuth)
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
		}

	};
}
#endif