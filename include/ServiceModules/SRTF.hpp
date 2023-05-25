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


//#ifndef PI 
//#define PI 3.14159265
//#endif

#ifndef DEFAULT_SRTFRESAMPLING_STEP
#define DEFAULT_SRTF_RESAMPLING_STEP 5
#endif
//#define MAX_DISTANCE_BETWEEN_ELEVATIONS 5
//#define NUMBER_OF_PARTS 4 
//#define MARGIN 10
//#define ELEVATION_NORTH_POLE 90
//#define ELEVATION_SOUTH_POLE 270
//
//#define DEFAULT_GAP_THRESHOLD 10
//
//#define SPHERE_BORDER 360.0f
//
//#define DEFAULT_MIN_AZIMUTH 0
//#define DEFAULT_MAX_AZIMUTH 360
//#define DEFAULT_MIN_ELEVATION 0
//#define DEFAULT_MAX_ELEVATION 360
//
//
//#define ORIENTATION_RESOLUTION 0.01



/*! \file */

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
//struct orientation2
//{
//	float azimuth;		///< Azimuth angle in degrees
//	float elevation;	///< Elevation angle in degrees	
//	orientation2(float _azimuth, float _elevation) :azimuth{ _azimuth }, elevation{ _elevation } {}
//	orientation2() :orientation2{ 0,0 } {}
//	bool operator==(const orientation2& other) const
//	{
//		return ((Common::AreSame(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSame(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
//	}
//};
//namespace std
//{
//	template<>
//	struct hash<orientation2>
//	{
//		// adapted from http://en.cppreference.com/w/cpp/utility/hash
//		size_t operator()(const orientation2& key) const
//		{
//			int keyAzimuth_hundredth = static_cast<int> (round(key.azimuth / ORIENTATION_RESOLUTION));
//			int keyElevation_hundredth = static_cast<int> (round(key.elevation / ORIENTATION_RESOLUTION));
//
//			size_t h1 = std::hash<int32_t>()(keyAzimuth_hundredth);
//			size_t h2 = std::hash<int32_t>()(keyElevation_hundredth);
//			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
//		}
//	};
//}



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
			////Update parameters			
			//sphereBorder = SPHERE_BORDER;

			//aziMin = DEFAULT_MIN_AZIMUTH;
			//aziMax = DEFAULT_MAX_AZIMUTH;
			//eleMin = DEFAULT_MIN_ELEVATION;
			//eleMax = DEFAULT_MAX_ELEVATION;
			//eleNorth = GetPoleElevation(TPole::north);
			//eleSouth = GetPoleElevation(TPole::south);

			////Clear every table			
			//t_HRTF_DataBase.clear();
			//t_HRTF_Resampled_frequency.clear();
			//t_HRTF_Resampled_partitioned.clear();

			////Change class state
			setupSRTFInProgress = true;
			SRTFloaded = false;


			SET_RESULT(RESULT_OK, "HRTF Setup started");
		}

		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupSRTFInProgress) {
				if (!t_SRTF_DataBase.empty())
				{
					//Delete the common delay of every HRIR functions of the DataBase Table
					//RemoveCommonDelay_HRTFDataBaseTable();

					//HRTF Resampling methdos
					//CalculateHRIR_InPoles(resamplingStep);
					//FillOutTableOfAzimuth360(resamplingStep);
					//FillSphericalCap_HRTF(gapThreshold, resamplingStep);
					//CalculateResampled_HRTFTable(resamplingStep);


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

	private:
		std::string title;
		std::string databaseName;
		std::string fileName;
		int resamplingStep;
		bool SRTFloaded;
		bool setupSRTFInProgress;
		
		T_SRTFTable	t_SRTF_DataBase;

	};
}
#endif
