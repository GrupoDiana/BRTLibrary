#ifndef _NEAR_FIELD_EFFECT_HPP
#define _NEAR_FIELD_EFFECT_HPP


#include <Common/Buffer.h>
#include <ServiceModules/ILD.hpp>
#define EPSILON 0.0001f

namespace BRTProcessing {

	class CNearFieldEffect {
	public:
		CNearFieldEffect() : enableNearFieldEffect{false} {
		
			nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation 
			nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation
			nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
			nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
		}


		///Enable near field effect for this source
		void EnableNearFieldEffect() { enableNearFieldEffect = true; };
		///Disable near field effect for this source
		void DisableNearFieldEffect() { enableNearFieldEffect = false; };
		///Get the flag for near field effect enabling
		bool IsNearFieldEffectEnabled() { return enableNearFieldEffect; };


		// Apply Near field effects (ILD)	
		
		void Process(CMonoBuffer<float>& _inLeftBuffer, CMonoBuffer<float>& _inRightBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CILD>& _listenerILDWeak)
		//void Process(CMonoBuffer<float>& leftBuffer, CMonoBuffer<float>& rightBuffer, float distance, float interauralAzimuth, std::weak_ptr<BRTServices::CILD>& _listenerHRTFWeak)
		{
			outLeftBuffer = _inLeftBuffer;
			outRightBuffer = _inRightBuffer;

			// Check process flag
			if (!IsNearFieldEffectEnabled()) { return;	}
			
			float distance = CalculateDistance(sourceTransform, listenerTransform);
			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) {	return; }
			
			//ASSERT(_inLeftBuffer.size() > 0 || _inRightBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Input buffer is empty when processing ILD", "");
			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize() || _inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");			
			
			// Check listener ILD
			std::shared_ptr<BRTServices::CILD> _listenerILD = _listenerILDWeak.lock();
			if (!_listenerILD) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "ILD listener pointer is null when trying to use in BRTProcessing::CNearFieldEffect");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			float interauralAzimuth = CalculateInterauralAzimuth(sourceTransform, listenerTransform);

			//Get coefficients from the ILD table
			std::vector<float> coefficientsLeft = _listenerILD->GetILDNearFieldEffectCoefficients(Common::T_ear::LEFT, distance, interauralAzimuth);
			std::vector<float> coefficientsRight = _listenerILD->GetILDNearFieldEffectCoefficients(Common::T_ear::RIGHT, distance, interauralAzimuth);
			
			if (coefficientsLeft.size() != 12 || coefficientsRight.size() != 12) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "Twelve coefficients were expected in order to be able to set up the filters in BRTProcessing::CNearFieldEffect");
				return;
			}						
			//Set LEFT coefficients into the filters and process the signal									
			SetCoefficients(nearFieldEffectFilters.left, coefficientsLeft);
			nearFieldEffectFilters.left.Process(outLeftBuffer);
			//Set RIGHT coefficients into the filters and process the signal						
			SetCoefficients(nearFieldEffectFilters.right, coefficientsRight);
			nearFieldEffectFilters.right.Process(outRightBuffer);									
		}
	
	private:
		///////////////////////
		// Private Methods
		///////////////////////
		float CalculateDistance(Common::CTransform _sourceTransform, Common::CTransform _listenerTransform) {

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();
			return _distanceToListener;
		}

		/// Calculates the parameters derived from the source and listener position
		float CalculateInterauralAzimuth(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform)
		{

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();

			//Check listener and source are in the same position
			if (_distanceToListener <= EPSILON) {
				SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CNearFieldEffect");
				return EPSILON;
			}						
			return _vectorToListener.GetInterauralAzimuthDegrees();			
		}

		void SetCoefficients(Common::CFiltersChain& _filter, std::vector<float>& cofficients) {
			std::vector<float> temp(cofficients.begin(), cofficients.begin() + 6);
			_filter.GetFilter(0)->SetCoefficients(temp);

			std::vector<float> temp2(cofficients.begin() + 6, cofficients.end());
			_filter.GetFilter(1)->SetCoefficients(temp2);
		}


		///////////////////////
		// Private Attributes
		///////////////////////
		
		Common::CGlobalParameters globalParameters;

		Common::CEarPair<Common::CFiltersChain> nearFieldEffectFilters;		// Computes the Near field effects

		bool enableNearFieldEffect;     // Enables/Disables the ILD (Interaural Level Difference) processing

	};
}
#endif