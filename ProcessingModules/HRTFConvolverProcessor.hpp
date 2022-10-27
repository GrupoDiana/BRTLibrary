#ifndef _BINAURAL_CONVOLVER_PROCESSOR_
#define _BINAURAL_CONVOLVER_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.h>
#include <Common/Buffer.h>
//#include <Common/CommonDefinitions.h>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace BRTProcessing {
    class CHRTFConvolverProcessor : public BRTBase::CProcessorBase {
    public:
        CHRTFConvolverProcessor(): enableInterpolation{ true } {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");
           // CreateEarsPositionEntryPoint("listenerEarPosition");
			CreateHRTFPtrEntryPoint("listenerHRTF");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   						

			ResetSourceConvolutionBuffers();	// Initialize convolution buffers
        }

        void Update(std::string _entryPointId) {            

			if (_entryPointId == "inputSamples") {
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
				//Common::CEarsTransforms listenerEarPosition = GetEarsPositionEntryPoint("listenerEarPosition")->GetData();
				
				std::weak_ptr<BRTServices::CHRTF> _temp = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();
				std::shared_ptr<BRTServices::CHRTF> listenerHRTF= _temp.lock();
				if (listenerHRTF) {
					Process(buffer, sourcePosition, listenerPosition, /*listenerEarPosition,*/ listenerHRTF);
				}
				else {
					// error
					SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTFlistner pointer is null when trying to use in HRTFConvolverProcessor");
				}

				this->resetUpdatingStack();				
			}            
        }
      
    private:
       
        // Atributes
		Common::CUPCAnechoic outputLeftUPConvolution;		// Object to make the inverse fft of the left channel with the UPC method
		Common::CUPCAnechoic outputRightUPConvolution;	// Object to make the inverse fft of the rigth channel with the UPC method

		CMonoBuffer<float> leftChannelDelayBuffer;			// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;			// To store the delay of the right channel of the expansion method
		
		bool enableInterpolation;							// Enables/Disables the interpolation on run time

        /// Methods        
        void Process(CMonoBuffer<float>& _inBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerPosition, /*Common::CEarsTransforms& listenerEarPositio,*/ std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) {

            // Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth; 
			float centerElevation;
			float interauralAzimuth;	

			////Calculate listener ear transforms
			//Common::CEarsTransforms listenerEarsTransforms;					
			//listenerEarsTransforms.leftEarTransform = GetListenerEarTransform(Common::T_ear::LEFT, listenerPosition, _listenerHRTF);
			//listenerEarsTransforms.rightEarTransform = GetListenerEarTransform(Common::T_ear::RIGHT, listenerPosition, _listenerHRTF);
			//listenerEarsTransforms.leftEarLocalPosition = GetListenerEarLocalPosition(Common::T_ear::LEFT, _listenerHRTF);
			//listenerEarsTransforms.rightEarLocalPosition = GetListenerEarLocalPosition(Common::T_ear::RIGHT, _listenerHRTF);

			CalculateSourceCoordinates(sourceTransform, listenerPosition, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth, _listenerHRTF);
								
			// GET HRTF
            std::vector<CMonoBuffer<float>>  leftHRIR_partitioned;
            std::vector<CMonoBuffer<float>>  rightHRIR_partitioned;
            
			leftHRIR_partitioned  = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::LEFT, leftAzimuth, leftElevation, enableInterpolation);
			rightHRIR_partitioned = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::RIGHT, rightAzimuth, rightElevation, enableInterpolation);
						
            // GET DELAY
			uint64_t leftDelay; 				///< Delay, in number of samples
			uint64_t rightDelay;				///< Delay, in number of samples

			leftDelay  = _listenerHRTF->GetHRIRDelay(Common::T_ear::LEFT, centerAzimuth, centerElevation, enableInterpolation);
			rightDelay = _listenerHRTF->GetHRIRDelay(Common::T_ear::RIGHT, centerAzimuth, centerElevation, enableInterpolation);
             
            // DO CONVOLUTION			
			CMonoBuffer<float> leftChannel_withoutDelay;
			CMonoBuffer<float> rightChannel_withoutDelay;
            //UPC algorithm with memory
            outputLeftUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
            outputRightUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);
            
            // ADD Delay
			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

            ProcessAddDelay_ExpansionMethod(leftChannel_withoutDelay, outLeftBuffer, leftChannelDelayBuffer, leftDelay);
            ProcessAddDelay_ExpansionMethod(rightChannel_withoutDelay, outRightBuffer, rightChannelDelayBuffer, rightDelay);

			// Propagate Result            
            GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
            GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
        }


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

		void ResetSourceConvolutionBuffers() {
			int numOfSubfilters = 1;//  listener->GetHRTF()->GetHRIRNumberOfSubfilters();
			int subfilterLength = 2048;// listener->GetHRTF()->GetHRIRSubfilterLength();
			
			Common::CGlobalParameters globalParameters;
			outputLeftUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			outputRightUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}

		/// Calculates the parameters derived from the source and listener position
		void CalculateSourceCoordinates(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform, float& leftElevation, float& leftAzimuth, float& rightElevation, float& rightAzimuth, float& centerElevation, float& centerAzimuth, float& interauralAzimuth, std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF)
		{

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();

			Common::CTransform leftEarTransform = GetListenerEarTransform(Common::T_ear::LEFT, _listenerTransform, _listenerHRTF);
			Common::CTransform rightEarTransform = GetListenerEarTransform(Common::T_ear::RIGHT, _listenerTransform, _listenerHRTF);
			Common::CVector3 leftEarLocalPosition = GetListenerEarLocalPosition(Common::T_ear::LEFT, _listenerHRTF);
			Common::CVector3 rightEarLocalPosition = GetListenerEarLocalPosition(Common::T_ear::RIGHT, _listenerHRTF);

			//Check listener and source are in the same position
			if (_distanceToListener <= EPSILON) {
				return;
			}

			Common::CVector3 leftVectorTo = leftEarTransform.GetVectorTo(_sourceTransform);
			Common::CVector3 rightVectorTo = rightEarTransform.GetVectorTo(_sourceTransform);
			Common::CVector3 leftVectorTo_sphereProjection = GetSphereProjectionPosition(leftVectorTo, leftEarLocalPosition, 1.95f/*ownerCore->GetListener()->GetHRTF()->GetHRTFDistanceOfMeasurement()*/);
			Common::CVector3 rightVectorTo_sphereProjection = GetSphereProjectionPosition(rightVectorTo, rightEarLocalPosition, 1.95f /*ownerCore->GetListener()->GetHRTF()->GetHRTFDistanceOfMeasurement()*/);

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
		*	\retval transform current listener ear position and orientation
		*   \eh On error, an error code is reported to the error handler.
		*/
		Common::CTransform GetListenerEarTransform(Common::T_ear ear, Common::CTransform& listenerTransform, std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CTransform();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -_listenerHRTF->GetHeadRadius());
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, _listenerHRTF->GetHeadRadius());

			return listenerTransform.GetLocalTranslation(earLocalPosition);
		}

		/** \brief Get EarPosition local to the listenerr
		*   \param [in] ear indicates the ear which you want to knowthe position
		*	\retval ear local position
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CVector3 GetListenerEarLocalPosition(Common::T_ear ear, std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -_listenerHRTF->GetHeadRadius());
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, _listenerHRTF->GetHeadRadius());


			return earLocalPosition;
		}
    };
}
#endif