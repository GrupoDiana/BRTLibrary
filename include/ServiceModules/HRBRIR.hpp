/**
* \class CHRBRIR
*
* \brief Declaration of CHRTF class interface
* \date	March 2024
*
*\authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, D.Gonzalez - Toledo, L.Molina - Tanco, F.Morales - Benitez ||
*Coordinated by, A.Reyes - Lecuona(University of Malaga) ||
*\b Contact : areyes@uma.es
*
*\b Contributions : (additional authors / contributors can be added here)
*
*\b Project : SONICOM ||
*\b Website : https://www.sonicom.eu/
*
*\b Copyright : University of Malaga 2024. Code based in the 3DTI Toolkit library(https ://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
	*
*\b Licence : This program is free software, you can redistribute it and /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*\b Acknowledgement : This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CHRBRIR_H_
#define _CHRBRIR_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/Fprocessor.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>

namespace BRTServices
{
	class CHRBRIR : public CServicesBase
	{
	public:

		CHRBRIR() : setupInProgress{ false }, samplingRate{ 0 }, IRLength{-1}, bufferSize{-1}, samplingStep{-1}, title{""}, headRadius {-1},
			databaseName{""}, listenerShortName{""}, fileName{""} {}



		/** \brief Start a new HRBRIR configuration
		*	\param [in] _HRIRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _IRLength)
		{
			//if ((ownerListener != nullptr) && ownerListener->ownerCore!=nullptr)
			{
				////Update parameters			
				IRLength = _IRLength;
				//distanceOfMeasurement = _distance;
				//bufferSize = globalParameters.GetBufferSize();
				//SetExtrapolationMethod(_extrapolationMethod);

				//float partitions = (float)HRIRLength / (float)bufferSize;
				//HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				//elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
				//elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

				////Clear every table			
				//t_HRTF_DataBase.clear();
				//t_HRTF_Resampled_frequency.clear();
				//t_HRTF_Resampled_partitioned.clear();

				//Change class state
				setupInProgress = true;
				//HRTFLoaded = false;


				SET_RESULT(RESULT_OK, "HRBRIR Setup started");
			}			
		}

		void AddHRBRIR(Common::CVector3 sourcePosition, Common::CVector3 sourceView, Common::CVector3 sourceUp, THRIRStruct&& newHRBRIR)
		{
			if (setupInProgress) {
				
				/*Common::CVector3 ofApp::GetListenerOrientationView(const std::string & listenerID) {
				if (!ListenerExist(listenerID)) return Common::CVector3();
				Common::CTransform _listenerTransform = GetListener(listenerID)->GetListenerTransform();
				Common::CVector3 front(1, 0, 0);
				Common::CVector3 listenerView = _listenerTransform.GetOrientation().RotateVector(front);
				return listenerView;
			}

			Common::CVector3 ofApp::GetListenerOrientationUp(const std::string & listenerID) {
				if (!ListenerExist(listenerID)) return Common::CVector3();
				Common::CTransform _listenerTransform = GetListener(listenerID)->GetListenerTransform();
				Common::CVector3 up(0, 0, 1);
				Common::CVector3 listenerUp = _listenerTransform.GetOrientation().RotateVector(up);
				return listenerUp;
			}*/
				
				
				//_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				//_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
				
				
				//auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation), std::forward<THRIRStruct>(newHRIR));
				//Error handler
				/*if (!returnValue.second) {
					SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
				}*/
			}
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {
			if (_ear == Common::T_ear::LEFT) { leftEarLocalPosition = _earPosition; }
			else if (_ear == Common::T_ear::RIGHT) { rightEarLocalPosition = _earPosition; }
			else if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
			}
		}

		/** \brief	Get the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) {
			if (enableCustomizedITD) {
				return CHRTFAuxiliarMethods::CalculateEarLocalPositionFromHeadRadius(_ear, headRadius);
			}
			else {
				if (_ear == Common::T_ear::LEFT) { return leftEarLocalPosition; }
				else if (_ear == Common::T_ear::RIGHT) { return rightEarLocalPosition; }
				else // either _ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE
				{
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
					return Common::CVector3();
				}
			}
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetTitle(std::string _title) {
			title = _title;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetDatabaseName(std::string _databaseName) {
			databaseName = _databaseName;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetListenerShortName(std::string _listenerShortName) {
			listenerShortName = _listenerShortName;
		}


		/** \brief Set the name of the SOFA file
		*    \param [in]	_fileName		string contains filename
		*/
		void SetFilename(std::string _fileName) {
			fileName = _fileName;
		}

		/** \brief Get the name of the SOFA file
		*   \return string contains filename
		*/
		std::string GetFilename() {
			return fileName;
		}

		/** \brief Set the sampling rate for the HRTF
		*	\param [in] sampling rate
		*/
		void SetSamplingRate(int _samplingRate) {
			samplingRate = _samplingRate;
		}

		/** \brief Ask for the sampling rate
		*	\retval sampling step
		*/
		int GetSamplingRate() {
			return samplingRate;
		}
		
		/**
		 * @brief Set the sampling step for the IR
		 * The intended angular separation between two adjacent points at the equator or two adjacent points in the median plane (azimuth=0)
		 * @param _samplingStep 
		 */
		void SetSamplingStep(int _resamplingStep) {
			samplingStep = _resamplingStep;
		}

		/** \brief Ask for the sampling step
		*	\retval sampling step
		*/
		int GetSamplingStep() {
			return samplingStep;
		}

		/** \brief	Set the radius of the listener head		
		*   \eh Nothing is reported to the error handler.
		*/
		float SetHeadRadius(float _headRadius) {
			headRadius = _headRadius;
		}

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() {
			return headRadius;
		}

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHRTFCustomizedITD() {
			enableCustomizedITD = true;
		}

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHRTFCustomizedITD() {
			enableCustomizedITD = false;
		}

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFCustomizedITDEnabled()
		{
			return enableCustomizedITD;
		}

	private:

		///////////////
		// ATTRIBUTES
		///////////////	
		std::string title;
		std::string databaseName;
		std::string listenerShortName;
		std::string fileName;
		
		int samplingRate;

		int32_t IRLength;								// HRIR vector length
		int32_t bufferSize;								// Input signal buffer size		
		//int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		//int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		//float distanceOfMeasurement;					//Distance where the HRIR have been measurement
		float headRadius;								// Head radius of listener 
		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position
		//TEXTRAPOLATION_METHOD extrapolationMethod;	// Methods that is going to be used to extrapolate
		
		int samplingStep; 								// The intended angular separation between two adjacent points at the equator or two adjacent points in the median plane (azimuth=0)
		bool enableCustomizedITD;						// Indicate the use of a customized delay

		bool setupInProgress;						// Variable that indicates the HRBRIR add and resample algorithm are in process
	};
}
#endif