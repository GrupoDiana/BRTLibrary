/**
* \class CILD
*
* \brief Declaration of CILD class
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

#ifndef _CILD_H_
#define _CILD_H_

#include <unordered_map>
#include <Common/FiltersChain.hpp>
#include <Common/Buffer.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>

//#define NEAR_FIELD_TABLE_AZIMUTH_STEP 5
//#define NEAR_FIELD_TABLE_DISTANCE_STEP 10

	/** \brief Class to be used as Key in the hash table used by CILD
	*/
	class CILD_Key
	{
	public:
		int distance;      ///< Distance to the center of the head, in millimeters 
		int azimuth;       ///< Azimuth angle of interaural coordinates, in degrees

		CILD_Key() :CILD_Key{ 0,0 } {}

		CILD_Key(int _distance, int _azimuth) :distance{ _distance }, azimuth{ _azimuth } {}

		bool operator==(const CILD_Key& key) const
		{
			return (this->azimuth == key.azimuth && this->distance == key.distance);
		}
	};

	namespace std
	{
		template<>
		struct hash<CILD_Key>
		{
			// adapted from http://en.cppreference.com/w/cpp/utility/hash
			size_t operator()(const CILD_Key & key) const
			{
				size_t h1 = std::hash<int>()(key.distance);
				size_t h2 = std::hash<int>()(key.azimuth);
				return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
			}
		};
	}

	///** \brief Template class to hold the coefficients of a set of biquad filters.
	//*/
	//template <int NUMBER_OF_BIQUAD_FILTERS>
	//class CILD_BiquadFilterCoefs
	//{
	//public:
	//	float coefs[6 * NUMBER_OF_BIQUAD_FILTERS];   /**< Holds the coefficients of one or more biquad filters.
	//													Each biquad filter has 6 coefficients.
	//													Format: f1_b0, f1_b1, f1_b2, f1_a0, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a0, f2_a1, f2_a2
	//														where fx means filter xth     */
	//};

	///** \brief Type definition to work with a set of coefficients for two biquad filters
	//*/
	//typedef CILD_BiquadFilterCoefs<2> T_ILD_TwoBiquadFilterCoefs;


	//struct TILDStruct {		
	//	CMonoBuffer<float> leftCoefs;	///< Left filters coefs
	//	CMonoBuffer<float> rightCoefs;	///< Right filters coefs
	//};


	/** \brief Hash table that contains a set of coefficients for two biquads filters that are indexed through a pair of distance
	 and azimuth values (interaural coordinates). */
	//typedef std::unordered_map<CILD_Key, T_ILD_TwoBiquadFilterCoefs> T_ILD_HashTable;
	typedef std::unordered_map<CILD_Key, BRTServices::TILDStruct> T_ILD_HashTable;

namespace BRTServices {

	/** \details This class models the effect of frequency-dependent Interaural Level Differences when the sound source is close to the listener
	*/
	class CILD : public CServicesBase
	{

	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor.
		*	\details Leaves ILD Table empty. Use SetILDNearFieldEffectTable to load.
		*   \eh Nothing is reported to the error handler.
		*/
		CILD() : setupInProgress{ false }, ILDLoaded{ false }, numberOfEars{ -1 },azimuthStep{-1}, distanceStep{-1}, fileTitle{""}, fileName{""}
		{					
		}


		void BeginSetup() {
			setupInProgress = true;			
			ILDLoaded = false;			
			Clear();

			SET_RESULT(RESULT_OK, "ILD Setup started");
		}

		bool EndSetup()
		{
			if (setupInProgress) {
				setupInProgress = false;
				
				azimuthStep = CalculateTableAzimuthStep();
				distanceStep = CalculateTableDistanceStep();

				if (numberOfEars != -1 && azimuthStep != -1 && distanceStep != -1) {															
					ILDLoaded = true;
					SET_RESULT(RESULT_OK, "ILD Setup finished");
					azimuthList.clear();
					distanceList.clear();
					return true;
				}								
			}
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Some parameter is missing in order to finish the data upload in BRTServices::CILD.");
			return false;
		}

		void Clear() {
			t_ILDNearFieldEffect.clear();
			azimuthList.clear();
			distanceList.clear();			
			numberOfEars = -1;
			azimuthStep = -1;
			distanceStep = -1;
		}

		

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetTitle(std::string _title) {
			fileTitle = _title;
		}
		
		/** \brief Get the title of the SOFA file
		*   \return string contains title
		*/
		std::string GetTitle() {
			return fileTitle;
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

		/** \brief Set the samplingRate of the SOFA file
		*    \param [in]	samplingRate	int contains samplingRate
		*/
		//void SetFileSamplingRate(int _samplingRate) {
		//	samplingRate = _samplingRate;
		//}

		///** \brief Get the samplingRate of the SOFA file
		//*   \return int contains samplingRate
		//*/
		//int GetFileSamplingRate() {
		//	return samplingRate;
		//}

		/** \brief Set the samplingRate of the SOFA file
		*    \param [in]	samplingRate	int contains samplingRate
		*/
		void SetNumberOfEars(int _numberOfEars) {
			numberOfEars = _numberOfEars;
		}

		/** \brief Get the samplingRate of the SOFA file
		*   \return int contains samplingRate
		*/
		int GetNumberOfEars() {
			return numberOfEars;
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

		/** \brief Add the hash table for computing ILD Near Field Effect
		*	\param [in] newTable data for hash table
		*   \eh Nothing is reported to the error handler.
		*/					
		void AddILDNearFieldEffectTable(T_ILD_HashTable && newTable)
		{
			t_ILDNearFieldEffect = newTable;
		}

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddCoefficients(float azimuth, float distance, TILDStruct&& newCoefs)
		{
			if (setupInProgress) {
				int iAzimuth = static_cast<int> (round(azimuth));
				int iDistance = static_cast<int> (round(GetDistanceInMM(distance)));

				auto returnValue = t_ILDNearFieldEffect.emplace(CILD_Key(iDistance, iAzimuth), std::forward<TILDStruct>(newCoefs));
				//Error handler
				if (returnValue.second) { 
					/*SET_RESULT(RESULT_OK, "ILD Coefficients emplaced into t_ILDNearFieldEffect succesfully"); */ 
					azimuthList.push_back(iAzimuth);
					distanceList.push_back(iDistance);
				}
				else { SET_RESULT(RESULT_WARNING, "Error emplacing ILD Cofficients"); }
			}
		}


		/** \brief Add the hash table for computing ILD Spatialization
		*	\param [in] newTable data for hash table
		*   \eh Nothing is reported to the error handler.
		*/
		/*void AddILDSpatializationTable(T_ILD_HashTable && newTable)
		{
			t_ILDSpatialization = newTable;
		}*/

		/** \brief Get the internal hash table used for computing ILD Near Field Effect
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_ILD_HashTable & GetILDNearFieldEffectTable() { return t_ILDNearFieldEffect; }
		
		/** \brief Get the internal hash table used for computing ILD Spatialization
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		//const T_ILD_HashTable & GetILDSpatializationTable() { return t_ILDSpatialization; }
		
		/** \brief Get IIR filter coefficients for ILD Near Field Effect, for one ear
		*	\param [in] ear ear for which we want to get the coefficients
		*	\param [in] distance_m distance, in meters
		*	\param [in] azimuth azimuth angle, in degrees
		*	\retval std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		*   \eh On error, an error code is reported to the error handler.
		*/				
		std::vector<float> GetILDNearFieldEffectCoefficients(Common::T_ear ear, float distance_m, float azimuth)
		{
			if (!ILDLoaded) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "ILD table was not initialized in BRTServices::CILD::GetILDNearFieldEffectCoefficients()");
				return std::vector<float>();
			}

			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get Near Field ILD coefficients for a wrong ear (BOTH or NONE)");
				std::vector<float> temp;
				return temp;
			} 
			
			if  ((ear == Common::T_ear::RIGHT) && numberOfEars == 1) {
				return GetILDNearFieldEffectCoefficients(Common::LEFT, distance_m, -azimuth);
			}

			ASSERT(distance_m > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when processing ILD", "");
			ASSERT(azimuth >= -90.0 && azimuth <= 90, RESULT_ERROR_OUTOFRANGE, "Azimuth must be between -90 deg and 90 deg when processing ILD", "");
			//ASSERT(ILDNearFieldEffectTable_AzimuthStep > 0 && ILDNearFieldEffectTable_DistanceStep > 0, RESULT_ERROR_INVALID_PARAM, "Step values of ILD hash table are not valid", "");

			float distance_mm = GetDistanceInMM(distance_m);
								
			int q_distance_mm	= GetRoundUp(distance_mm, distanceStep);
			int q_azimuth		= GetRoundUp(azimuth, azimuthStep);

			auto itEar = t_ILDNearFieldEffect.find(CILD_Key(q_distance_mm, q_azimuth));
			if (itEar != t_ILDNearFieldEffect.end())
			{					
				if (ear == Common::T_ear::LEFT)			{	return itEar->second.leftCoefs;	} 
				else if (ear == Common::T_ear::RIGHT) {		return itEar->second.rightCoefs; }
				else { // Should never get here but keep compiler happy
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get Near Field ILD coefficients for a wrong ear (BOTH or NONE)");
					return std::vector<float>();
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "{Distance-Azimuth} key value was not found in the Near Field ILD look up table");					
				return std::vector<float>();
			}						
		}
		
		/// Rounds a value to the next valid integer according to a step value
		int GetRoundUp(float value, int roundStep) {			
			float sign = value > 0 ? 1 : -1;			
			return roundStep * (int)((value + sign * ((float)roundStep) / 2) / roundStep);
		}


		/** \brief Get IIR filter coefficients for ILD Spatialization, for one ear
		*	\param [in] ear ear for which we want to get the coefficients
		*	\param [in] distance_m distance, in meters
		*	\param [in] azimuth azimuth angle, in degrees
		*	\retval std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		*   \eh On error, an error code is reported to the error handler.
		*/
		//std::vector<float> GetILDSpatializationCoefficients(Common::T_ear ear, float distance_m, float azimuth)
		//{
		//	if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
		//	{
		//		SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get High Performance Spatialization ILD coefficients for a wrong ear (BOTH or NONE)");
		//		std::vector<float> temp;
		//		return temp;
		//	}

		//	ASSERT(distance_m > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when processing ILD", "");
		//	ASSERT(azimuth >= -90.0 && azimuth <= 90, RESULT_ERROR_OUTOFRANGE, "Azimuth must be between -90 deg and 90 deg when processing ILD", "");
		//	ASSERT(ILDSpatializationTable_AzimuthStep > 0 && ILDSpatializationTable_DistanceStep > 0, RESULT_ERROR_INVALID_PARAM, "Step values of ILD hash table are not valid", "");

		//	float distance_mm = distance_m * 1000.0f;

		//	float distSign = distance_mm > 0 ? 1 : -1;
		//	float azimSign = azimuth > 0 ? 1 : -1;

		//	int q_distance_mm = ILDSpatializationTable_DistanceStep * (int)((distance_mm + distSign * ((float)ILDSpatializationTable_DistanceStep) / 2) / ILDSpatializationTable_DistanceStep);
		//	int q_azimuth = ILDSpatializationTable_AzimuthStep * (int)((azimuth + azimSign * ((float)ILDSpatializationTable_AzimuthStep) / 2) / ILDSpatializationTable_AzimuthStep);
		//	if (ear == Common::T_ear::RIGHT)
		//		q_azimuth = -q_azimuth;

		//	auto itEar = t_ILDSpatialization.find(CILD_Key(q_distance_mm, q_azimuth));

		//	if (itEar != t_ILDSpatialization.end())
		//	{
		//		//std::vector<float> temp(itEar->second.coefs, itEar->second.coefs + 10);
		//		//return temp;
		//	}
		//	else {
		//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "{Distance-Azimuth} key value was not found in the High Performance Spatialization ILD look up table");
		//		std::vector<float> temp;
		//		return temp;
		//	}
		//}

	private:
		
		int CalculateTableAzimuthStep() {			
			// Order azimuth and remove duplicates
			std::sort(azimuthList.begin(), azimuthList.end());
			azimuthList.erase(unique(azimuthList.begin(), azimuthList.end()), azimuthList.end());

			// Calculate the minimum azimuth
			int azimuthStep = 999999;
			for (int i = 0; i < azimuthList.size() - 1; i++) {
				if (azimuthList[i + 1] - azimuthList[i] < azimuthStep) {
					azimuthStep = azimuthList[i + 1] - azimuthList[i];
				}
			}
			return azimuthStep;
		}

		/// Calculate the TABLE Distance STEP from the table
		float CalculateTableDistanceStep() {			
			// Order distances and remove duplicates
			std::sort(distanceList.begin(), distanceList.end());
			distanceList.erase(unique(distanceList.begin(), distanceList.end()), distanceList.end());

			// Calculate the minimum d
			double distanceStep = 999999;
			for (int i = 0; i < distanceList.size() - 1; i++) {
				if (distanceList[i + 1] - distanceList[i] < distanceStep) {
					distanceStep = distanceList[i + 1] - distanceList[i];
				}
			}
			// Rounding the result, to millimetres and back to metres
			//float distenceStepMM = distanceStep * 1000;
			//return std::round(distenceStepMM) * 0.001f;
			return distanceStep;
		}

		float GetDistanceInMM(float _distanceInMetres) {
			return _distanceInMetres * 1000.0f;
		}

		float GetDistanceInMetres(float _distanceInMilimetres) {
			return _distanceInMilimetres * 0.001f;
		}



		///////////////
		// ATTRIBUTES
		///////////////	

		bool setupInProgress;						// Variable that indicates the ILD load is in process
		bool ILDLoaded;								// Variable that indicates if the ILD has been loaded correctly

		T_ILD_HashTable t_ILDNearFieldEffect;
		int azimuthStep;							// In degress
		int distanceStep;							// In milimeters
		std::vector<double> azimuthList;
		std::vector<double> distanceList;

		//T_ILD_HashTable t_ILDSpatialization;	  
		//int ILDSpatializationTable_AzimuthStep;			// In degress
		//int ILDSpatializationTable_DistanceStep;		// In milimeters

		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position

		std::string fileName;
		std::string fileTitle;				
		std::string databaseName;
		std::string listenerShortName;
		
		//int samplingRate;
		int numberOfEars;
	};
}
#endif
