/**
* \class CILD
*
* \brief Declaration of CILD class interface.
* \date	July 2016
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/
#ifndef _CILD_H_
#define _CILD_H_

#include <unordered_map>
#include <Common/FiltersChain.hpp>
#include <Common/Buffer.h>
#include <Common/CommonDefinitions.h>

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


	struct TILDStruct {		
		CMonoBuffer<float> leftCoefs;	///< Left filters coefs
		CMonoBuffer<float> rightCoefs;	///< Right filters coefs
	};


	/** \brief Hash table that contains a set of coefficients for two biquads filters that are indexed through a pair of distance
	 and azimuth values (interaural coordinates). */
	//typedef std::unordered_map<CILD_Key, T_ILD_TwoBiquadFilterCoefs> T_ILD_HashTable;
	typedef std::unordered_map<CILD_Key, TILDStruct> T_ILD_HashTable;

namespace BRTServices {

	/** \details This class models the effect of frequency-dependent Interaural Level Differences when the sound source is close to the listener
	*/
	class CILD
	{

	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor.
		*	\details Leaves ILD Table empty. Use SetILDNearFieldEffectTable to load.
		*   \eh Nothing is reported to the error handler.
		*/
		CILD() : setupInProgress{ false }, ILDLoaded{ false }, samplingRate{ -1 }, numberOfEars{ -1 },azimuthStep{-1}, distanceStep{-1}, fileTitle{""}, fileName{""}, fileDescription{""}
		{
			//ILDNearFieldEffectTable_AzimuthStep = 5;	// In degress
			//ILDNearFieldEffectTable_DistanceStep = 10;	// In milimeters
			//ILDSpatializationTable_AzimuthStep = 5;		// In degress
			//ILDSpatializationTable_DistanceStep = 10;	// In milimeters				
		}


		void BeginSetup() {
			setupInProgress = true;
			ILDLoaded = false;

			SET_RESULT(RESULT_OK, "ILD Setup started");
		}

		bool EndSetup()
		{
			if (setupInProgress) {
				
				if (samplingRate != -1 && numberOfEars != -1 && azimuthStep != -1 && distanceStep != -1) {
					setupInProgress = false;
					ILDLoaded = true;
					SET_RESULT(RESULT_OK, "ILD Setup finished");
					return true;
				}								
			}
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Some parameter is missing in order to finish the data upload in BRTServices::CILD.");
			return false;
		}

		/** \brief Set the name of the SOFA file
		*    \param [in]	_fileName		string contains filename
		*/
		void SetFileName(std::string _fileName) {
			fileName = _fileName;
		}

		/** \brief Get the name of the SOFA file
		*   \return string contains filename
		*/
		std::string GetFileName() {
			return fileName;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetFileTitle(std::string _title) {
			fileTitle = _title;
		}

		/** \brief Get the title of the SOFA file
		*   \return string contains title
		*/
		std::string GetFileTitle() {
			return fileTitle;
		}

		/** \brief Set the Description of the SOFA file
		*    \param [in]	_title		string contains Description
		*/
		void SetFileDescription(std::string _description) {
			fileDescription = _description;
		}

		/** \brief Get the Description of the SOFA file
		*   \return string contains Description
		*/
		std::string GetFileDescription() {
			return fileDescription;
		}

		/** \brief Set the samplingRate of the SOFA file
		*    \param [in]	samplingRate	int contains samplingRate
		*/
		void SetFileSamplingRate(int _samplingRate) {
			samplingRate = _samplingRate;
		}

		/** \brief Get the samplingRate of the SOFA file
		*   \return int contains samplingRate
		*/
		int GetFileSamplingRate() {
			return samplingRate;
		}

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

		/** \brief Set the azimuth step in degrees in the ILD table of the SOFA file.
		*    \param [in]	azimuthStep	int contains table azimuth step
		*/
		void SetAzimuthTableStep(int _azimuthStep) {
			azimuthStep = _azimuthStep;
		}

		/** \brief Get the azimuth step 
		*   \return int contains azimuth step in degrees
		*/
		int GetAzimuthTableStep() {
			return azimuthStep;
		}

		/** \brief Set the distance step in meter in the ILD table of the SOFA file
		*    \param [in]	distanceStep	int contains table distance Step in metres
		*/
		void SetDistanceTableStep(float _distanceStep) {
			
			distanceStep = static_cast<int> (round(GetDistanceInMM(_distanceStep)));
		}

		/** \brief Get the distance step
		*   \return int contains distance step in metres
		*/
		float GetDistanceTableStep() {
			return GetDistanceInMetres(distanceStep);
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
				if (returnValue.second) { /*SET_RESULT(RESULT_OK, "ILD Coefficients emplaced into t_ILDNearFieldEffect succesfully"); */ }
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
		int azimuthStep;		// In degress
		int distanceStep;		// In milimeters
												  
		//T_ILD_HashTable t_ILDSpatialization;	  
		//int ILDSpatializationTable_AzimuthStep;			// In degress
		//int ILDSpatializationTable_DistanceStep;		// In milimeters

		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position

		std::string fileName;
		std::string fileTitle;		
		std::string fileDescription;
		
		int samplingRate;
		int numberOfEars;
	};
}
#endif
