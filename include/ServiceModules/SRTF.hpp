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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
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
			:resamplingStep{ DEFAULT_SRTF_RESAMPLING_STEP }, SRTFloaded{ false }, setupSRTFInProgress{ false }
			
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
		void BeginSetup(){
			//Update parameters			
			sphereBorder = SPHERE_BORDER;

			aziMin = DEFAULT_MIN_AZIMUTH;
			aziMax = DEFAULT_MAX_AZIMUTH;
			eleMin = DEFAULT_MIN_ELEVATION;
			eleMax = DEFAULT_MAX_ELEVATION;
			eleNorth = GetPoleElevation(TPole::north);
			eleSouth = GetPoleElevation(TPole::south);

			//Clear every table			
			t_SRTF_DataBase.clear();
			t_SRTF_Resampled.clear();

			////Change class state
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
			int numOfInterpolatedHRIRs = 0;

			//Resample Interpolation Algorithm
			for (int newAzimuth = aziMin; newAzimuth < aziMax; newAzimuth = newAzimuth + _resamplingStep)
			{
				for (int newElevation = eleMin; newElevation <= eleNorth; newElevation = newElevation + _resamplingStep)
				{
					if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
				}

				for (int newElevation = eleSouth; newElevation < eleMax; newElevation = newElevation + _resamplingStep)
				{
					if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
				}
			}
			SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
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

	private:
		std::string title;
		std::string databaseName;
		std::string fileName;
		int resamplingStep;
		bool SRTFloaded;
		bool setupSRTFInProgress;
		
		T_SRTFTable	t_SRTF_DataBase;
		T_SRTFTable	t_SRTF_Resampled;

		int aziMin, aziMax, eleMin, eleMax, eleNorth, eleSouth;	// Variables that define limits of work area
		float sphereBorder;

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
