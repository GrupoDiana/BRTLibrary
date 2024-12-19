/**
* \class CSOSFilters
*
* \brief Declaration of CSOSFilters class
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CSOS_FILTER_HPP_
#define _CSOS_FILTER_HPP_

#include <unordered_map>
#include <Common/FiltersChain.hpp>
#include <Common/Buffer.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServicesBase.hpp>


	/** \brief Class to be used as Key in the hash table used by CSOSFilters
	*/
	class CSOSFilter_Key
	{
	public:
		int distance;      ///< Distance to the center of the head, in millimeters 
		int azimuth;       ///< Azimuth angle of interaural coordinates, in degrees

		CSOSFilter_Key() :CSOSFilter_Key{ 0,0 } {}

		CSOSFilter_Key(int _distance, int _azimuth) :distance{ _distance }, azimuth{ _azimuth } {}

		bool operator==(const CSOSFilter_Key& key) const
		{
			return (this->azimuth == key.azimuth && this->distance == key.distance);
		}
	};

	namespace std
	{
		template<>
		struct hash<CSOSFilter_Key>
		{
			// adapted from http://en.cppreference.com/w/cpp/utility/hash
			size_t operator()(const CSOSFilter_Key & key) const
			{
				size_t h1 = std::hash<int>()(key.distance);
				size_t h2 = std::hash<int>()(key.azimuth);
				return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
			}
		};
	}
	

	/** \brief Hash table that contains a set of coefficients for two biquads filters that are indexed through a pair of distance
	 and azimuth values (interaural coordinates). */	
	typedef std::unordered_map<CSOSFilter_Key, BRTServices::TSOSFilterStruct> T_SOSFilter_HashTable;

namespace BRTServices {

	/** \details This class models the effect of frequency-dependent Interaural Level Differences when the sound source is close to the listener
	*/
	class CSOSFilters : public CServicesBase
	{

	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor.
		*	\details Leaves SOS Filter Table empty. Use SetSOSFilterTable to load.
		*   \eh Nothing is reported to the error handler.
		*/
		CSOSFilters() : setupInProgress{ false }, NFCFiltersLoaded{ false }, numberOfEars{ -1 },azimuthStep{-1}, distanceStep{-1}, fileTitle{""}, fileName{""}
		{					
		}


		bool BeginSetup() {
			setupInProgress = true;			
			NFCFiltersLoaded = false;			
			Clear();
			SET_RESULT(RESULT_OK, "SOS Filter Setup started");
			return true;
		}

		bool EndSetup()
		{
			if (setupInProgress) {
				setupInProgress = false;
				
				azimuthStep = CalculateTableAzimuthStep();
				distanceStep = CalculateTableDistanceStep();

				if (numberOfEars != -1 && azimuthStep != -1 && distanceStep != -1) {															
					NFCFiltersLoaded = true;
					SET_RESULT(RESULT_OK, "SOS Filter Setup finished");
					azimuthList.clear();
					distanceList.clear();
					return true;
				}								
			}
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Some parameter is missing in order to finish the data upload in BRTServices::CSOSFilters.");
			return false;
		}

		void Clear() {
			t_SOSFilter.clear();
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

		/** \brief Add the hash table for computing SOS Filter
		*	\param [in] newTable data for hash table
		*   \eh Nothing is reported to the error handler.
		*/					
		void AddSOSFilterTable(T_SOSFilter_HashTable && newTable)
		{
			t_SOSFilter = newTable;
		}

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddCoefficients(float azimuth, float distance, TSOSFilterStruct&& newCoefs)
		{
			if (setupInProgress) {
				int iAzimuth = static_cast<int> (round(azimuth));
				int iDistance = static_cast<int> (round(GetDistanceInMM(distance)));

				auto returnValue = t_SOSFilter.emplace(CSOSFilter_Key(iDistance, iAzimuth), std::forward<TSOSFilterStruct>(newCoefs));
				//Error handler
				if (returnValue.second) { 
					/*SET_RESULT(RESULT_OK, "ILD Coefficients emplaced into t_ILDNearFieldEffect succesfully"); */ 
					azimuthList.push_back(iAzimuth);
					distanceList.push_back(iDistance);
				}
				else { SET_RESULT(RESULT_WARNING, "Error emplacing SOS Filter Cofficients"); }
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

		/** \brief Get the internal hash table used for computing SOS Filter
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_SOSFilter_HashTable & GetSOSFilterTable() { return t_SOSFilter; }
		
		/** \brief Get the internal hash table used for computing ILD Spatialization
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		//const T_ILD_HashTable & GetILDSpatializationTable() { return t_ILDSpatialization; }
		
		/** \brief Get IIR filter coefficients for SOS Filter, for one ear
		*	\param [in] ear ear for which we want to get the coefficients
		*	\param [in] distance_m distance, in meters
		*	\param [in] azimuth azimuth angle, in degrees
		*	\retval std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		*   \eh On error, an error code is reported to the error handler.
		*/				
		std::vector<float> GetSOSFilterCoefficients(Common::T_ear ear, float distance_m, float azimuth)
		{
			if (!NFCFiltersLoaded) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "SOS Filter table was not initialized in BRTServices::CILD::GetSOSFilterCoefficients()");
				return std::vector<float>();
			}

			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get SOS Filter coefficients for a wrong ear (BOTH or NONE)");
				std::vector<float> temp;
				return temp;
			} 
			
			if  ((ear == Common::T_ear::RIGHT) && numberOfEars == 1) {
				return GetSOSFilterCoefficients(Common::LEFT, distance_m, -azimuth);
			}

			ASSERT(distance_m > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when processing ILD", "");
			ASSERT(azimuth >= -90.0 && azimuth <= 90, RESULT_ERROR_OUTOFRANGE, "Azimuth must be between -90 deg and 90 deg when processing ILD", "");
			//ASSERT(ILDNearFieldEffectTable_AzimuthStep > 0 && ILDNearFieldEffectTable_DistanceStep > 0, RESULT_ERROR_INVALID_PARAM, "Step values of ILD hash table are not valid", "");

			float distance_mm = GetDistanceInMM(distance_m);
								
			int q_distance_mm	= GetRoundUp(distance_mm, distanceStep);
			int q_azimuth		= GetRoundUp(azimuth, azimuthStep);

			auto itEar = t_SOSFilter.find(CSOSFilter_Key(q_distance_mm, q_azimuth));
			if (itEar != t_SOSFilter.end())
			{					
				if (ear == Common::T_ear::LEFT)			{	return itEar->second.leftCoefs;	} 
				else if (ear == Common::T_ear::RIGHT) {		return itEar->second.rightCoefs; }
				else { // Should never get here but keep compiler happy
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get SOS Filter coefficients for a wrong ear (BOTH or NONE)");
					return std::vector<float>();
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "{Distance-Azimuth} key value was not found in the SOS Filter look up table");					
				return std::vector<float>();
			}						
		}
		
		/// Rounds a value to the next valid integer according to a step value
		int GetRoundUp(float value, int roundStep) {			
			float sign = value > 0 ? 1 : -1;			
			return roundStep * (int)((value + sign * ((float)roundStep) / 2) / roundStep);
		}
		
	private:
		
		int CalculateTableAzimuthStep() {			
			// Order azimuth and remove duplicates
			std::sort(azimuthList.begin(), azimuthList.end());
			azimuthList.erase(unique(azimuthList.begin(), azimuthList.end()), azimuthList.end());

			// Calculate the minimum azimuth
			int azimuthStep = 999999;	//TODO Why this number?
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
			double distanceStep = 999999; //TODO Why this number?
			for (int i = 0; i < distanceList.size() - 1; i++) {
				if (distanceList[i + 1] - distanceList[i] < distanceStep) {
					distanceStep = distanceList[i + 1] - distanceList[i];
				}
			}
						
			return distanceStep;
		}

		/**
		 * @brief Transform distance in meters to distance in milimetres
		 * @param _distanceInMetres distance in meters
		 * @return 
		 */
		float GetDistanceInMM(float _distanceInMetres) {
			return _distanceInMetres * 1000.0f;
		}
		
		/**
		 * @brief Transform distance in milimetres to distance in meters
		 * @param _distanceInMilimetres distance in milimetres
		 * @return 
		 */
		float GetDistanceInMetres(float _distanceInMilimetres) {
			return _distanceInMilimetres * 0.001f;
		}


		///////////////
		// ATTRIBUTES
		///////////////	

		bool setupInProgress;						// Variable that indicates the SOS Filter load is in process
		bool NFCFiltersLoaded;						// Variable that indicates if the SOS Filter has been loaded correctly

		T_SOSFilter_HashTable t_SOSFilter;
		int azimuthStep;							// In degress
		int distanceStep;							// In milimeters
		std::vector<double> azimuthList;
		std::vector<double> distanceList;

		Common::CVector3 leftEarLocalPosition;		// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;		// Listener right ear relative position

		std::string fileName;
		std::string fileTitle;				
		std::string databaseName;
		std::string listenerShortName;
				
		int numberOfEars;
	};
}
#endif
