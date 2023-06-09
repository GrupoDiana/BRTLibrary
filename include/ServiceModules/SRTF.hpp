/**
* \class CSRTF
*
* \brief Declaration of CSRTF class interface to store directivity data
* \version 
* \date	May 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es 
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga 
*
* \b Licence: 
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CSRTF_H_
#define _CSRTF_H_

#include <unordered_map>
//#include <vector>
//#include <utility>
//#include <list>
//#include <cstdint>
//#include <Base/Listener.hpp>
//#include <Common/Buffer.h>
#include <Common/ErrorHandler.hpp>
//#include <Common/Fprocessor.h>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.h>
#include <ServiceModules/ServiceModuleInterfaces.hpp>

#ifndef DEFAULT_SRTF_RESAMPLING_STEP
#define DEFAULT_SRTF_RESAMPLING_STEP 5
#endif

/** \brief Type definition for the SRTF table
*/
typedef std::unordered_map<orientation, BRTServices::TDirectivityTFStruct> T_SRTFTable;

//namespace BRTBase { }

namespace BRTServices
{
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CSRTF : public CServicesBase
	{
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CSRTF()
			:resamplingStep{ DEFAULT_SRTF_RESAMPLING_STEP }, SRTFloaded{ false }, setupSRTFInProgress{ false }, aziMin{ DEFAULT_MIN_AZIMUTH }, aziMax{ DEFAULT_MAX_AZIMUTH },
			eleMin{ DEFAULT_MIN_ELEVATION }, eleMax{ DEFAULT_MAX_ELEVATION }, sphereBorder{ SPHERE_BORDER }, epsilon_sewing { EPSILON_SEWING	}
		{}

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

		/// <summary>
		/// 
		/// </summary>
		void BeginSetup(int32_t _directivityTFRealPartLength){
			//Update parameters			
			eleNorth = GetPoleElevation(TPole::north);
			eleSouth = GetPoleElevation(TPole::south);

			
			if (_directivityTFRealPartLength != 2.0 * globalParameters.GetBufferSize()) //
			{
				SET_RESULT(RESULT_WARNING, "Number of samples (N) in SOFA file is different from BUffer Size");
			}
			directivityTF_length = 2.0 * _directivityTFRealPartLength; //directivityTF will store the Real and Img parts interlaced
			directivityTF_numberOfSubfilters = 1;

			//Clear every table			
			t_SRTF_DataBase.clear();
			t_SRTF_Resampled.clear();
			 
			//Change class state
			setupSRTFInProgress = true;
			SRTFloaded = false;


			SET_RESULT(RESULT_OK, "HRTF Setup started");
		}

		/** \brief Stop the SRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupSRTFInProgress) {
				if (!t_SRTF_DataBase.empty())
				{
					//HRTF Resampling methdos
					//CalculateHRIR_InPoles(resamplingStep);
					//FillOutTableOfAzimuth360(resamplingStep);
					//FillSphericalCap_HRTF(gapThreshold, resamplingStep);
					CalculateResampled_SRTFTable(resamplingStep);
					auto stepVector = CalculateStep();


					//Setup values
					setupSRTFInProgress = false;
					SRTFloaded = true;

					SET_RESULT(RESULT_OK, "SRTF Table completed succesfully");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_SRTF_DataBase map has not been set");
				}
			}
			return false;
		}

		void SetResamplingStep(int _resamplingStep) {
			resamplingStep = _resamplingStep;
		}

		int GetResamplingStep() {
			return resamplingStep;
		}

		int GetDirectivityTFLength() { return directivityTF_length; }

		int GetDirectivityTFNumOfSubfilters() { return directivityTF_numberOfSubfilters; }

		/** \brief Add a new TF to the SRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newDirectivityTF DirectivityTF data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF)
		{
			if (setupSRTFInProgress) {
				auto returnValue = t_SRTF_DataBase.emplace(orientation(_azimuth, _elevation), std::forward<TDirectivityTFStruct>(DirectivityTF));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
		}

	
		/// <summary>
		/// Calculate the regular resampled-table using an interpolation method
		/// </summary>
		/// <param name="_resamplingStep" step for both azimuth and elevation></param>
		void CalculateResampled_SRTFTable(int _resamplingStep)
		{
			t_SRTF_Resampled = t_SRTF_DataBase;
			//int numOfInterpolatedHRIRs = 0;

			////Resample Interpolation Algorithm
			//for (int newAzimuth = aziMin; newAzimuth < aziMax; newAzimuth = newAzimuth + _resamplingStep)
			//{
			//	for (int newElevation = eleMin; newElevation <= eleNorth; newElevation = newElevation + _resamplingStep)
			//	{
			//		if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
			//	}

			//	for (int newElevation = eleSouth; newElevation < eleMax; newElevation = newElevation + _resamplingStep)
			//	{
			//		if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
			//	}
			//}
			//SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
		}

		/// <summary>
		/// Calculate the DirectivityTF using the an interpolation Method 
		/// </summary>
		/// <param name="newAzimuth"></param>
		/// <param name="newElevation"></param>
		/// <returns>interpolatedDTFs: true if the Directivity TF has been calculated using the interpolation</returns>
		bool CalculateAndEmplaceNewDirectivityTF(float _azimuth, float _elevation) {
			TDirectivityTFStruct interpolatedHRIR;
			bool bDirectivityTFInterpolated = false;
			auto it = t_SRTF_DataBase.find(orientation(_azimuth, _elevation));
			if (it != t_SRTF_DataBase.end()){
				//Fill out Directivity TF from the original Database
				auto returnValue =  t_SRTF_Resampled.emplace(orientation(_azimuth, _elevation), std::forward<TDirectivityTFStruct>(it->second));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing DirectivityTF into t_SRTF_Resampled map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
			else
			{
				SET_RESULT(RESULT_WARNING, "Resampling SRTF: cannot find Directivity TF in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "], in the database table");
			}
			return bDirectivityTFInterpolated;
		}

		/// <summary>
		/// Calculate the azimuth step for each orientation
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		std::unordered_map<orientation, float> CalculateStep()
		{
			std::unordered_map<orientation, float> stepVector;
			float elevation, aziStep, diff = 360, actual_ele = -1.0f;
			int secondTime = 0;
			std::vector<orientation> orientations;

			// Create a vector with all the orientations of the Database
			orientations.reserve(t_SRTF_DataBase.size());
			for (auto& itr : t_SRTF_DataBase)
			{
				orientations.push_back(itr.first);
			}
			//  Sort orientations of the DataBase with a lambda function in sort.
			std::sort(orientations.begin(), orientations.end(), [](orientation a, orientation b) {return (a.elevation < b.elevation); });


			for (int ori = 0; ori< orientations.size(); ori++)
			{
				// Maybe stop in each different elevation and make the difference between the start azimuth, 0, and the next azimuth in this elevation
				// with this form, we could save a vector like this [aziStep elevation]

				elevation = orientations[ori].elevation;

				if (actual_ele != elevation)
				{
					for (int ele = ori+1; ele < orientations.size(); ele++)
					{
						if (orientations[ele].elevation != elevation || orientations[ele] == orientations.back())
						{	
							if (diff == 360) { diff = 0; }
							stepVector.emplace(orientation(0, elevation), diff);
							diff = 360;
							break;
						}
						if (abs(orientations[ele].azimuth - orientations[ele -1].azimuth) < diff) { 
							diff = abs(orientations[ele].azimuth - orientations[ele - 1].azimuth); 
						}
					}
					actual_ele = elevation;

				}
			}
			return stepVector;
		}

		/// <summary>
		/// Get the Directivity TF of the nearest point to  the given one
		/// </summary>
		/// <param name="_azimuth"></param>
		/// <param name="_elevation"></param>
		/// <returns></returns>
		const TDirectivityTFStruct GetDirectivityTF(float _azimuth, float _elevation, std::unordered_map<orientation, float> _stepsMap) const {

			TDirectivityTFStruct newSRIR;
			auto it0 = t_SRTF_DataBase.find(orientation(_azimuth, _elevation));
			if (it0 != t_SRTF_DataBase.end()) {
				newSRIR.data = it0->second.data;
			}
			else {

				// HARCODE ELEVATION STEP TO 10
				int eleStep = 10;

				float aziCeilBack, aziCeilFront, aziFloorBack, aziFloorFront;
				int idxEle = ceil(_elevation / eleStep);
				float eleCeil = eleStep * idxEle;
				float eleFloor = eleStep * (idxEle - 1);

				eleCeil = CheckLimitsElevation_and_Transform(eleCeil);										//			   Back	  Front
				eleFloor = CheckLimitsElevation_and_Transform(eleFloor);									//	Ceil		A		B

				auto stepItr = _stepsMap.find(orientation(0, eleCeil));										//	Floor		D		C
				float aziStepCeil = stepItr->second;
																										
				CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
				// azimuth values passed by reference

				auto stepIt = _stepsMap.find(orientation(0, eleFloor));
				float aziStepFloor = stepIt->second;							

				CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth); 

				float pntMid_azimuth = (aziFloorBack + aziStepFloor * 0.5f);
				float pntMid_elevation = (eleFloor + eleStep * 0.5f);

				if (_azimuth >= pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//Second quadrant
						auto it = t_SRTF_DataBase.find(orientation(aziCeilFront, eleCeil));
						newSRIR.data = it->second.data;

					}
					else if (_elevation < pntMid_elevation)
					{
						//Forth quadrant
						auto it = t_SRTF_DataBase.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else if (_azimuth < pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//First quadrant
						auto it = t_SRTF_DataBase.find(orientation(aziCeilBack, eleCeil));
						newSRIR.data = it->second.data;
					}
					else if (_elevation < pntMid_elevation) {
						//Third quadrant
						auto it = t_SRTF_DataBase.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else { return newSRIR = {}; }
			}

			//SET_RESULT(RESULT_OK, "CalculateHRIR_InterpolationMethod completed succesfully");
			return newSRIR;
		}

		float CheckLimitsElevation_and_Transform(float elevation)const
		{
			if (elevation < 0) { elevation = elevation + 360; }
			if (elevation >= 360) { elevation = elevation - 360; }
			return elevation;

		}
		float CheckLimitsAzimuth_and_Transform(float azimuth)const
		{
			if (azimuth < 0) { azimuth = azimuth + 360; }
			else if (azimuth > 360) { azimuth = azimuth - 360; }
			return azimuth;
		}

		void CalculateAzimuth_BackandFront(float& aziBack, float& aziFront, float aziStep, float _azimuth)const
		{

			int idxAzi = ceil(_azimuth / aziStep);

			aziFront = idxAzi * aziStep;
			aziBack = (idxAzi - 1) * aziStep;

			aziFront = CheckLimitsAzimuth_and_Transform(aziFront);
			aziBack = CheckLimitsAzimuth_and_Transform(aziBack);
		}

	private:
		std::string title;
		std::string databaseName;
		std::string fileName;
		int resamplingStep;
		bool SRTFloaded;
		bool setupSRTFInProgress;
		int32_t directivityTF_length;	
		int32_t directivityTF_numberOfSubfilters;	
		
		T_SRTFTable	t_SRTF_DataBase;
		T_SRTFTable	t_SRTF_Resampled;

		Common::CGlobalParameters globalParameters;

		int aziMin, aziMax, eleMin, eleMax, eleNorth, eleSouth;	// Variables that define limits of work area
		float sphereBorder;
		float epsilon_sewing;
		enum class TPole { north, south };
		/** \brief Get Pole Elevation
		*	\param [in] Tpole var that indicates of which pole we need elevation
		*   \eh  On error, an error code is reported to the error handler.
		*/
		int GetPoleElevation(TPole _pole)const
		{
			if (_pole == TPole::north) { return ELEVATION_NORTH_POLE; }
			else if (_pole == TPole::south) { return ELEVATION_SOUTH_POLE; }
			else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get a non-existent pole");
				return 0;
			}
		}
	};
}
#endif
