/**
* \class CHRTFConvolver
*
* \brief Declaration of CHRTFConvolver class interface.
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

#ifndef _HRTF_CONVOLVER_
#define _HRTF_CONVOLVER_

#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <ServiceModules/HRTF.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace BRTProcessing {
	class CHRTFConvolver  {
	public:
		CHRTFConvolver() : enableInterpolation{ true }, enableSpatialization{ true }, convolutionBuffersInitialized{false} { }


		///Enable spatialization process for this source	
		void EnableSpatialization() { enableSpatialization = true; }
		///Disable spatialization process for this source	
		void DisableSpatialization() { enableSpatialization = false; }
		///Get the flag for spatialization process enabling
		bool IsSpatializationEnabled() { return enableSpatialization; }
		
		///Enable HRTF interpolation method	
		void EnableInterpolation() { enableInterpolation = true; }
		///Disable HRTF interpolation method
		void DisableInterpolation() { enableInterpolation = false; }
		///Get the flag for HRTF interpolation method
		bool IsInterpolationEnabled() { return enableInterpolation; }

		
		
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
		void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CHRTF>& _listenerHRTFWeak) {

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
						
			// Check process flag
			if (!enableSpatialization)
			{
				outLeftBuffer = _inBuffer;
				outRightBuffer = _inBuffer;
				return;
			}

			// Check listener HRTF
			std::shared_ptr<BRTServices::CHRTF> _listenerHRTF = _listenerHRTFWeak.lock();
			if (!_listenerHRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTF listener pointer is null when trying to use in HRTFConvolver");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// TODO 
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization
			/*if (distanceToListener <= ownerCore->GetListener()->GetHeadRadius())
			{
				outLeftBuffer = inBuffer;
				outRightBuffer = inBuffer;
				return;
			}*/

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_listenerHRTF); }

			// Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth;
			float centerElevation;
			float interauralAzimuth;

			CalculateSourceCoordinates(sourceTransform, listenerTransform, _listenerHRTF, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);

			// GET HRTF
			std::vector<CMonoBuffer<float>>  leftHRIR_partitioned;
			std::vector<CMonoBuffer<float>>  rightHRIR_partitioned;

			leftHRIR_partitioned = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::LEFT, leftAzimuth, leftElevation, enableInterpolation);
			rightHRIR_partitioned = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::RIGHT, rightAzimuth, rightElevation, enableInterpolation);

			// GET DELAY
			uint64_t leftDelay; 				///< Delay, in number of samples
			uint64_t rightDelay;				///< Delay, in number of samples

			BRTServices::THRIRPartitionedStruct delays = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, centerAzimuth, centerElevation, enableInterpolation);
			leftDelay	= delays.leftDelay;
			rightDelay	= delays.rightDelay;
			
			// DO CONVOLUTION			
			CMonoBuffer<float> leftChannel_withoutDelay;
			CMonoBuffer<float> rightChannel_withoutDelay;
			//UPC algorithm with memory
			outputLeftUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
			outputRightUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);

			// ADD Delay
			//CMonoBuffer<float> outLeftBuffer;
			//CMonoBuffer<float> outRightBuffer;

			ProcessAddDelay_ExpansionMethod(leftChannel_withoutDelay, outLeftBuffer, leftChannelDelayBuffer, leftDelay);
			ProcessAddDelay_ExpansionMethod(rightChannel_withoutDelay, outRightBuffer, rightChannelDelayBuffer, rightDelay);

			// Propagate Result            
			//GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
			//GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
		}

		/// Reset convolvers and convolution buffers
		void ResetSourceConvolutionBuffers() {
			convolutionBuffersInitialized = false;
			// Reset convolver classes
			outputLeftUPConvolution.Reset();
			outputRightUPConvolution.Reset();
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}
	private:

		// Atributes
		Common::CGlobalParameters globalParameters;

		Common::CUPCAnechoic outputLeftUPConvolution;		// Object to make the inverse fft of the left channel with the UPC method
		Common::CUPCAnechoic outputRightUPConvolution;		// Object to make the inverse fft of the rigth channel with the UPC method

		CMonoBuffer<float> leftChannelDelayBuffer;			// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;			// To store the delay of the right channel of the expansion method

		bool enableSpatialization;								// Flags for independent control of processes
		bool enableInterpolation;							// Enables/Disables the interpolation on run time
		bool convolutionBuffersInitialized;
		
		/////////////////////
		/// PRIVATE Methods        
		/////////////////////

		// Apply doppler effect simulation
		void ProcessAddDelay_ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay)
		{
			//Prepare the outbuffer		
			if (output.size() != input.size()) { output.resize(input.size()); }

			//Prepare algorithm variables
			float position = 0;
			float numerator = input.size() - 1;
			float denominator = input.size() - 1 + newDelay - delayBuffer.size();
			float compressionFactor = numerator / denominator;

			//Add samples to the output from buffer
			for (int i = 0; i < delayBuffer.size(); i++)
			{
				output[i] = delayBuffer[i];
			}

			//Fill the others buffers
			//if the delay is the same one as the previous frame use a simplification of the algorithm
			if (newDelay == delayBuffer.size())
			{
				//Copy input to output
				int j = 0;
				for (int i = delayBuffer.size(); i < input.size(); i++)
				{
					output[i] = input[j++];
				}
				//Fill delay buffer
				for (int i = 0; i < newDelay; i++)
				{
					delayBuffer[i] = input[j++];
				}
			}
			//else, apply the expansion/compression algorihtm
			else
			{
				int j;
				float rest;
				int forLoop_end;
				//The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
				if (newDelay == 0) { forLoop_end = input.size() - 1; }
				else { forLoop_end = input.size(); }

				//Fill the output buffer with the new values 
				for (int i = delayBuffer.size(); i < forLoop_end; i++)
				{
					j = static_cast<int>(position);
					rest = position - j;
					output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
					position += compressionFactor;
				}

				//The last loop iteration must be addressed in a special way if newDelay = 0 (part 2)
				if (newDelay == 0)
				{
					output[input.size() - 1] = input[input.size() - 1];
					delayBuffer.clear();
				}

				//if newDelay!=0 fill out the delay buffer
				else
				{
					//Fill delay buffer 			
					CMonoBuffer<float> temp;
					temp.reserve(newDelay);
					for (int i = 0; i < newDelay - 1; i++)
					{
						int j = int(position);
						float rest = position - j;
						temp.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
						position += compressionFactor;
					}
					//Last element of the delay buffer that must be addressed in a special way
					temp.push_back(input[input.size() - 1]);
					//delayBuffer.swap(temp);				//To use in C++03
					delayBuffer = std::move(temp);			//To use in C++11
				}
			}
		}//End ProcessAddDelay_ExpansionMethod


		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) {

			int numOfSubfilters = _listenerHRTF->GetHRIRNumberOfSubfilters();
			int subfilterLength = _listenerHRTF->GetHRIRSubfilterLength();

			//Common::CGlobalParameters globalParameters;
			outputLeftUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			outputRightUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();

			// Declare variable
			convolutionBuffersInitialized = true;
		}


		/// Calculates the parameters derived from the source and listener position
		void CalculateSourceCoordinates(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF, float& leftElevation, float& leftAzimuth, float& rightElevation, float& rightAzimuth, float& centerElevation, float& centerAzimuth, float& interauralAzimuth)
		{

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();

			//Check listener and source are in the same position
			if (_distanceToListener <= MINIMUM_DISTANCE_SOURCE_LISTENER) {				
				SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CHRTFConvolver");
				_distanceToListener = MINIMUM_DISTANCE_SOURCE_LISTENER;
			}

			Common::CVector3 leftEarLocalPosition = _listenerHRTF->GetEarLocalPosition(Common::T_ear::LEFT);
			Common::CVector3 rightEarLocalPosition = _listenerHRTF->GetEarLocalPosition(Common::T_ear::RIGHT);
			Common::CTransform leftEarTransform = _listenerTransform.GetLocalTranslation(leftEarLocalPosition);
			Common::CTransform rightEarTransform = _listenerTransform.GetLocalTranslation(rightEarLocalPosition);

			Common::CVector3 leftVectorTo = leftEarTransform.GetVectorTo(_sourceTransform);
			Common::CVector3 rightVectorTo = rightEarTransform.GetVectorTo(_sourceTransform);
			Common::CVector3 leftVectorTo_sphereProjection = GetSphereProjectionPosition(leftVectorTo, leftEarLocalPosition, _listenerHRTF->GetHRTFDistanceOfMeasurement());
			Common::CVector3 rightVectorTo_sphereProjection = GetSphereProjectionPosition(rightVectorTo, rightEarLocalPosition, _listenerHRTF->GetHRTFDistanceOfMeasurement());

			leftElevation = leftVectorTo_sphereProjection.GetElevationDegrees();	//Get left elevation
			if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, leftElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, leftElevation, EPSILON))
			{
				leftAzimuth = leftVectorTo_sphereProjection.GetAzimuthDegrees();	//Get left azimuth
			}

			rightElevation = rightVectorTo_sphereProjection.GetElevationDegrees();	//Get right elevation	
			if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, rightElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, rightElevation, EPSILON))
			{
				rightAzimuth = rightVectorTo_sphereProjection.GetAzimuthDegrees();		//Get right azimuth
			}


			centerElevation = _vectorToListener.GetElevationDegrees();		//Get elevation from the head center
			if (!Common::AreSame(ELEVATION_SINGULAR_POINT_UP, centerElevation, EPSILON) && !Common::AreSame(ELEVATION_SINGULAR_POINT_DOWN, centerElevation, EPSILON))
			{
				centerAzimuth = _vectorToListener.GetAzimuthDegrees();		//Get azimuth from the head center
			}

			interauralAzimuth = _vectorToListener.GetInterauralAzimuthDegrees();	//Get Interaural Azimuth

		}


		// In orther to obtain the position where the HRIR is needed, this method calculate the projection of each ear in the sphere where the HRTF has been measured
		const Common::CVector3 GetSphereProjectionPosition(Common::CVector3 vectorToEar, Common::CVector3 earLocalPosition, float distance) const
		{
			//get axis according to the defined convention
			float rightAxis = vectorToEar.GetAxis(RIGHT_AXIS);
			float forwardAxis = vectorToEar.GetAxis(FORWARD_AXIS);
			float upAxis = vectorToEar.GetAxis(UP_AXIS);
			// Error handler:
			if ((rightAxis == 0.0f) && (forwardAxis == 0.0f) && (upAxis == 0.0f)) {
				ASSERT(false, RESULT_ERROR_DIVBYZERO, "Axes are not correctly set. Please, check axis conventions", "Azimuth computed from vector succesfully");
			}
			//get ear position in right axis
			float earRightAxis = earLocalPosition.GetAxis(RIGHT_AXIS);

			//Resolve a quadratic equation to get lambda, which is the parameter that define the line between the ear and the sphere, passing by the source
			// (x_sphere, y_sphere, z_sphere) = earLocalPosition + lambda * vectorToEar 
			// x_sphere^2 + y_sphere^2 + z_sphere^2 = distance^2


			float a = forwardAxis * forwardAxis + rightAxis * rightAxis + upAxis * upAxis;
			float b = 2.0f * earRightAxis * rightAxis;
			float c = earRightAxis * earRightAxis - distance * distance;
			float lambda = (-b + sqrt(b * b - 4.0f * a * c)) * 0.5f * (1 / a);

			Common::CVector3 cartesianposition;

			cartesianposition.SetAxis(FORWARD_AXIS, lambda * forwardAxis);
			cartesianposition.SetAxis(RIGHT_AXIS, (earRightAxis + lambda * rightAxis));
			cartesianposition.SetAxis(UP_AXIS, lambda * upAxis);

			return cartesianposition;
		}

		/** \brief Get position and orientation of one listener ear
		*	\param [in] ear listener ear for wich we want to get transform
		*   \param [in] listenerTransform position of the listener
		* 	\param [in] _listenerHRTF ptr to the HRTF of the listener
		*	\retval transform current listener ear position and orientation
		*   \eh On error, an error code is reported to the error handler.
		*/
		Common::CTransform GetListenerEarTransform(Common::T_ear ear, Common::CTransform& listenerTransform, std::shared_ptr<BRTServices::CHRTF>& listenerHRTF) const
		{
			return listenerTransform.GetLocalTranslation(listenerHRTF->GetEarLocalPosition(ear));
		}

	};
}
#endif