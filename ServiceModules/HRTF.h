/**
* \class CHRTF
*
* \brief Declaration of CHRTF class interface
* \version 
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


#ifndef _CHRTF_H_
#define _CHRTF_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Base/Listener.hpp>
#include <Common/Buffer.h>
#include <Common/ErrorHandler.h>
#include <Common/Fprocessor.h>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.h>


#ifndef PI 
#define PI 3.14159265
#endif
#ifndef DEFAULT_RESAMPLING_STEP
#define DEFAULT_RESAMPLING_STEP 5
#endif

#ifndef DEFAULT_HRTF_MEASURED_DISTANCE
#define DEFAULT_HRTF_MEASURED_DISTANCE 1.95f
#endif

#define MAX_DISTANCE_BETWEEN_ELEVATIONS 5
#define NUMBER_OF_PARTS 4 
#define AZIMUTH_STEP  15
//#define EPSILON 0.01f;

/*! \file */

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
struct orientation
{
	int32_t azimuth;	///< Azimuth angle in degrees
	int32_t elevation;	///< Elevation angle in degrees
    orientation(int32_t _azimuth, int32_t _elevation):azimuth{_azimuth}, elevation{_elevation}{}
    orientation():orientation{0,0}{}
    bool operator==(const orientation& oth) const
    {
        return ((this->azimuth == oth.azimuth) && (this->elevation == oth.elevation));
    }
};

/** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
*/
struct THRIRStruct {
	uint64_t leftDelay;				///< Left delay, in number of samples
	uint64_t rightDelay;			///< Right delay, in number of samples
	CMonoBuffer<float> leftHRIR;	///< Left impulse response data
	CMonoBuffer<float> rightHRIR;	///< Right impulse response data
};

/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
*/
struct THRIRPartitionedStruct {
	uint64_t leftDelay;				///< Left delay, in number of samples
	uint64_t rightDelay;			///< Right delay, in number of samples
	std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
	std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data
};

/** \brief Type definition for an impulse response with the ITD removed and stored in a specific struct field
*/
struct oneEarHRIR_struct {
	uint64_t delay;				///< Delay, in number of samples
	CMonoBuffer<float> HRIR;	///< Impulse response data
};

/** \brief Type definition for an impulse response subfilter set with the ITD removed and stored in a specific struct field
*/
struct TOneEarHRIRPartitionedStruct {
	std::vector<CMonoBuffer<float>> HRIR_Partitioned;	///< Partitioned impulse response data
	uint64_t delay;				///< Delay, in number of samples
};

/**	\brief Type definition for barycentric coordinates
*/
struct TBarycentricCoordinatesStruct {
	float alpha;	///< Coordinate alpha
	float beta;		///< Coordinate beta
	float gamma;	///< Coordinate gamma
};

namespace std
{
	//[TBC]
    template<>
    struct hash<orientation>
    {
        // adapted from http://en.cppreference.com/w/cpp/utility/hash
        size_t operator()(const orientation & key) const
        {
            size_t h1 = std::hash<int32_t>()(key.azimuth);
            size_t h2 = std::hash<int32_t>()(key.elevation);
            return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
        }
    };
}


/** \brief Type definition for the HRTF table
*/
typedef std::unordered_map<orientation, THRIRStruct> T_HRTFTable;

/** \brief Type definition for the HRTF partitioned table used when UPConvolution is activated
*/
typedef std::unordered_map<orientation, THRIRPartitionedStruct> T_HRTFPartitionedTable;

/** \brief Type definition for a distance-orientation pair
*/
typedef std::pair <float, orientation> T_PairDistanceOrientation;

namespace BRTBase { class CListener; }

namespace BRTServices
{
	//class CCore;
	//class CListener;

	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CHRTF
	{
	public:
		
		/** \brief Constructor with parameters
		*	\param [in] _ownerListener pointer to already created listener object
		*	\details By default, customized ITD is switched off and resampling step is set to 5 degrees
		*   \eh Nothing is reported to the error handler.
		*/
		/*CHRTF(BRTBase::CListener* _ownerListener)
			:ownerListener{ _ownerListener }, enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, HRIRLength{ 0 }, HRTFLoaded{ false }, setupInProgress{ false }, distanceOfMeasurement { DEFAULT_HRTF_MEASURED_DISTANCE }
		{}*/

		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CHRTF()
			:enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, HRIRLength{ 0 }, 
			HRTFLoaded{ false }, setupInProgress{ false }, distanceOfMeasurement{ DEFAULT_HRTF_MEASURED_DISTANCE }, listenerHeadRadius{ DEFAULT_LISTENER_HEAD_RADIOUS }
		{}

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		int32_t GetHRIRLength() const
		{
			return HRIRLength;
		}

		/** \brief Start a new HRTF configuration
		*	\param [in] _HRIRLength buffer size of the HRIR to be added		
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _HRIRLength, float _distance);
		
		/** \brief Set the full HRIR matrix.
		*	\param [in] newTable full table with all HRIR data
		*   \eh Nothing is reported to the error handler.
		*/
		void AddHRTFTable(T_HRTFTable && newTable);

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddHRIR(float azimuth, float elevation, THRIRStruct && newHRIR);

		/** \brief Stop the HRTF configuration		
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void EndSetup();

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHRTFCustomizedITD();

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHRTFCustomizedITD();

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFCustomizedITDEnabled();

		/** \brief Get interpolated HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR 
		*	\param [in] _azimuth azimuth angle in degrees
		*	\param [in] _elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const oneEarHRIR_struct GetHRIR_frequency(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation) const;

		/** \brief Get interpolated and partitioned HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] _azimuth azimuth angle in degrees
		*	\param [in] _elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const std::vector<CMonoBuffer<float>> GetHRIR_partitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation) const
		//const std::vector<CMonoBuffer<float>> CHRTF::GetHRIR_partitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)");
			}

			std::vector<CMonoBuffer<float>> newHRIR;

			if (!setupInProgress)
			{
				if (runTimeInterpolation)
				{
					if (Common::AreSame(_azimuth, sphereBorder, epsilon_sewing)) { _azimuth = 0.0f; }
					if (Common::AreSame(_elevation, sphereBorder, epsilon_sewing)) { _elevation = 0.0f; }

					//If we are in the sphere poles, do not perform the interpolation (the HRIR value for this orientations have been calculated with a different method in the resampled methods, because our barycentric interpolation method doesn't work in the poles)
					int iazimuth = static_cast<int>(round(_azimuth));
					int ielevation = static_cast<int>(round(_elevation));
					if ((ielevation == 90) || (ielevation == 270))
					{
						//In the sphere poles the azimuth is always 0 degrees
						iazimuth = 0.0f;
						auto it = t_HRTF_Resampled_partitioned.find(orientation(iazimuth, ielevation));
						if (it != t_HRTF_Resampled_partitioned.end())
						{
							if (ear == Common::T_ear::LEFT)
							{
								newHRIR = it->second.leftHRIR_Partitioned;
							}
							else
							{
								newHRIR = it->second.rightHRIR_Partitioned;
							}
						}
						else
						{
							SET_RESULT(RESULT_WARNING, "Orientations in GetHRIR_partitioned() not found");
						}
					}

					else
					{
						//Run time interpolation ON
						newHRIR = GetHRIR_partitioned_InterpolationMethod(ear, _azimuth, _elevation);
					}

					return newHRIR;
				}
				else
				{
					//Run time interpolation OFF
					int nearestAzimuth = static_cast<int>(round(_azimuth / resamplingStep) * resamplingStep);
					int nearestElevation = static_cast<int>(round(_elevation / resamplingStep) * resamplingStep);
					// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
					if (nearestAzimuth == 360) { nearestAzimuth = 0; }
					if (nearestElevation == 360) { nearestElevation = 0; }
					// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
					if ((nearestElevation == 90) || (nearestElevation == 270)) { nearestAzimuth = 0; }

					auto it = t_HRTF_Resampled_partitioned.find(orientation(nearestAzimuth, nearestElevation));
					if (it != t_HRTF_Resampled_partitioned.end())
					{
						if (ear == Common::T_ear::LEFT)
						{
							newHRIR = it->second.leftHRIR_Partitioned;
						}
						else
						{
							newHRIR = it->second.rightHRIR_Partitioned;
						}
					}
					else
					{
						SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRIR not found");
					}
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRTF Setup in progress return empty");
			}
			SET_RESULT(RESULT_WARNING, "GetHRIR_partitioned return empty");
			return *new std::vector<CMonoBuffer<float>>();
		}


		/** \brief Get the HRIR delay, in number of samples, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] _azimuthCenter azimuth angle from the source and the listener head center in degrees
		*	\param [in] _elevationCenter elevation angle from the source and the listener head center in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		float GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation)
		//float CHRTF::GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation)
		{
			float HRIR_delay = 0.0f;

			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIRDelay: Attempt to get the delay of the HRIR for a wrong ear (BOTH or NONE)");
			}

			if (!setupInProgress)
			{
				//Modify delay if customized delay is activate
				if (enableCustomizedITD)
				{
					HRIR_delay = GetCustomizedDelay(_azimuthCenter, _elevationCenter, ear);
				}
				else
				{
					if (runTimeInterpolation)
					{
						if (Common::AreSame(_azimuthCenter, sphereBorder, epsilon_sewing)) { _azimuthCenter = 0.0f; }
						if (Common::AreSame(_elevationCenter, sphereBorder, epsilon_sewing)) { _elevationCenter = 0.0f; }

						//If we are in the sphere poles, do not perform the interpolation (the HRIR value for this orientations have been calculated with a different method in the resampled methods, because our barycentric interpolation method doesn't work in the poles)
						int iazimuth = static_cast<int>(round(_azimuthCenter));
						int ielevation = static_cast<int>(round(_elevationCenter));
						if ((ielevation == 90) || (ielevation == 270))
						{
							//In the sphere poles the azimuth is always 0 degrees
							iazimuth = 0.0f;
							auto it = t_HRTF_Resampled_partitioned.find(orientation(iazimuth, ielevation));
							if (it != t_HRTF_Resampled_partitioned.end())
							{
								if (ear == Common::T_ear::LEFT)
								{
									HRIR_delay = it->second.leftDelay;
								}
								else
								{
									HRIR_delay = it->second.rightDelay;
								}
							}
							else
							{
								SET_RESULT(RESULT_WARNING, "Orientations in GetHRIRDelay() not found");
							}
						}

						else
						{
							//Run time interpolation ON
							HRIR_delay = GetHRIRDelayInterpolationMethod(ear, _azimuthCenter, _elevationCenter);
						}

						return HRIR_delay;

					}
					else
					{
						//Run time interpolation OFF
						int nearestAzimuth = static_cast<int>(round(_azimuthCenter / resamplingStep) * resamplingStep);
						int nearestElevation = static_cast<int>(round(_elevationCenter / resamplingStep) * resamplingStep);
						// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
						if (nearestAzimuth == 360) { nearestAzimuth = 0; }
						if (nearestElevation == 360) { nearestElevation = 0; }
						// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
						if ((nearestElevation == 90) || (nearestElevation == 270)) { nearestAzimuth = 0; }

						auto it = t_HRTF_Resampled_partitioned.find(orientation(nearestAzimuth, nearestElevation));
						if (it != t_HRTF_Resampled_partitioned.end())
						{
							if (ear == Common::T_ear::LEFT)
							{
								HRIR_delay = it->second.leftDelay;
							}
							else
							{
								HRIR_delay = it->second.rightDelay;
							}

							return HRIR_delay;
						}
						else
						{
							SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRIR not found");
						}
					}
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRTF Setup in progress return empty");
			}

			SET_RESULT(RESULT_WARNING, "GetHRIRDelay return delay=0");
			return HRIR_delay;
		}

		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetHRIRNumberOfSubfilters() const;

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetHRIRSubfilterLength() const;

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFLoaded();

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_HRTFTable & GetRawHRTFTable() const;

		/** \brief	Calculate the ITD value for a specific source
		*   \param [in]	_azimuth		source azimuth in degrees
		*   \param [in]	_elevation		source elevation in degrees
		*   \param [in]	ear				ear where the ITD is calculated (RIGHT, LEFT)
		*   \return ITD ITD calculated with the current listener head circunference
		*   \eh Nothing is reported to the error handler.
		*/
		const unsigned long GetCustomizedDelay(float _azimuth, float _elevation, Common::T_ear ear)const
		//const unsigned long CHRTF::GetCustomizedDelay(float _azimuth, float _elevation, Common::T_ear ear)  const
		{

			float rAzimuth = _azimuth * PI / 180;
			float rElevation = _elevation * PI / 180;

			//Calculate the customized delay
			unsigned long customizedDelay = 0;
			float interauralAzimuth = std::asin(std::sin(rAzimuth) * std::cos(rElevation));

			float ITD = CalculateITDFromHeadRadius(listenerHeadRadius /*ownerListener->GetHeadRadius()*/, interauralAzimuth);

			if ((ITD > 0 && ear == Common::T_ear::RIGHT) || (ITD < 0 && ear == Common::T_ear::LEFT)) {
				customizedDelay = static_cast <unsigned long> (round(std::abs(globalParameters.GetSampleRate() * ITD)));
			}
			return customizedDelay;
		}


		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHRTFDistanceOfMeasurement();
				

	private:
		///////////////
		// ATTRIBUTES
		///////////////
		//BRTBase::CListener* ownerListener;						// owner Listener
		int32_t HRIRLength;								// HRIR vector length
		int32_t bufferSize;								// Input signal buffer size
		//int32_t sampleRate;							// Sample Rate		
		int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		float distanceOfMeasurement;					//Distance where the HRIR have been measurement
		float listenerHeadRadius;							// Head radius of listener 

		float sphereBorder;						// Define spheere "sewing"
		float epsilon_sewing = 0.001f;

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableCustomizedITD;					// Indicate the use of a customized delay
		

		// HRTF tables			
		T_HRTFTable				t_HRTF_DataBase;
		T_HRTFTable				t_HRTF_Resampled_frequency;
		T_HRTFPartitionedTable	t_HRTF_Resampled_partitioned;

		// Empty object to return in some methods
		THRIRStruct						emptyHRIR;
		THRIRPartitionedStruct			emptyHRIR_partitioned;
		CMonoBuffer<float>				emptyMonoBuffer;
		oneEarHRIR_struct				emptyOneEarHRIR;
		TOneEarHRIRPartitionedStruct	emptyOneEarHRIR_partitioned;

		Common::CGlobalParameters globalParameters;

		/////////////
		// METHODS
		/////////////

		//	Fill out the HRTF for every azimuth and two specific elevations: 90 and 270 degrees
		void CalculateHRIR_InPoles();

		//	Calculate the HRIR in the pole of one of the hemispheres
		//param hemisphereParts	vector of the HRTF orientations of the hemisphere
		THRIRStruct CalculateHRIR_InOneHemispherePole(vector<orientation> hemisphereParts);

		//	Calculate the resample matrix using the Barycentric interpolation Method (copy the HRIR function of the nearest orientation)
		//param resamplingStep	HRTF resample matrix step for both azimuth and elevation
		void CalculateResampled_HRTFTable(int resamplingStep);

		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain
		THRIRPartitionedStruct SplitAndGetFFT_HRTFData(const THRIRStruct & newData_time);

		//		Calculate the distance between two points [(azimuth1, elevation1) and (azimuth2, elevation2)] using the Haversine formula
		//return	float	the distance value
		float CalculateDistance_HaversineFormula(float azimuth1, float elevation1, float azimuth2, float elevation2);

		//	Calculate the HRIR of a specific orientation (newazimuth, newelevation) using the Barycentric interpolation Method
		//param newAzimuth		azimuth of the orientation of interest (the one whose HRIR will be calculated)
		//param newElevation	elevation of the orientation of interest (the one whose HRIR will be calculated)
		THRIRStruct CalculateHRIR_offlineMethod(int newAzimuth, int newElevation);

		//		Calculate the barycentric coordinates of three vertex [(x1,y1), (x2,y2), (x3,y3)] and the orientation of interest (x,y)
		//const TBarycentricCoordinatesStruct GetBarycentricCoordinates(float newAzimuth, float newElevation, float x1, float y1, float x2, float y2, float x3, float y3) const;
		const TBarycentricCoordinatesStruct GetBarycentricCoordinates(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3)const
		{
			// Obtain Barycentric coordinates:
			TBarycentricCoordinatesStruct barycentricCoordinates;

			float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);

			if (round(denominator) == 0) {	//if denominator=0 -> no triangle -> barycentric coordinates NO VALID 
				//SET_RESULT(RESULT_WARNING, "Barycentric coordinates can be computed only on triangles");
				barycentricCoordinates.alpha = -1;
				barycentricCoordinates.beta = -1;
				barycentricCoordinates.gamma = -1;
			}
			else {
				barycentricCoordinates.alpha = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
				barycentricCoordinates.alpha = trunc(1000 * barycentricCoordinates.alpha) / 1000;
				barycentricCoordinates.beta = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
				barycentricCoordinates.beta = trunc(1000 * barycentricCoordinates.beta) / 1000;
				barycentricCoordinates.gamma = 1.0f - barycentricCoordinates.alpha - barycentricCoordinates.beta;
				barycentricCoordinates.gamma = trunc(1000 * barycentricCoordinates.gamma) / 1000;
				//SET_RESULT(RESULT_OK, "Barycentric coordinates computed succesfully");
			}
			return barycentricCoordinates;
		}

		//		Transform the orientation in order to move the orientation of interest to 180 degrees
		//returnval	float	transformed azimuth
		float TransformAzimuth(float azimuthOrientationOfInterest, float originalAzimuth);

		//		Transform the orientation in order to express the elevation in the interval [-90,90]
		//returnval float transformed elevation
		float TransformElevation(float elevationOrientationOfInterest, float originalElevation);

		//	Calculate the distance between the given orientation (newAzimuth, newElevation) and all other values of the databsde HRTF table. And store these values in a sorted list
		//param	newAzimuth		azimuth of the orientation of interest in degrees
		//param	newElevation	elevation of the orientation of interest in degrees
		//return the distances sorted list
		std::list<T_PairDistanceOrientation> GetSortedDistancesList(int newAzimuth, int newElevation);

		//	Get HRIR from resample table using a barycentric interpolation of the three nearest orientation.
		const oneEarHRIR_struct GetHRIR_InterpolationMethod(Common::T_ear ear, int azimuth, int elevation) const;

		//	Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		const std::vector<CMonoBuffer<float>> GetHRIR_partitioned_InterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const
		//const std::vector<CMonoBuffer<float>> CHRTF::GetHRIR_partitioned_InterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const
		{
			std::vector<CMonoBuffer<float>> newHRIR;
			TBarycentricCoordinatesStruct barycentricCoordinates;
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = trunc(_azimuth / resamplingStep) * resamplingStep;
			orientation_ptoC.elevation = trunc(_elevation / resamplingStep) * resamplingStep;
			orientation_ptoA.azimuth = orientation_ptoC.azimuth;
			orientation_ptoA.elevation = orientation_ptoC.elevation + resamplingStep;
			orientation_ptoB.azimuth = orientation_ptoC.azimuth + resamplingStep;
			orientation_ptoB.elevation = orientation_ptoC.elevation + resamplingStep;
			orientation_ptoD.azimuth = orientation_ptoC.azimuth + resamplingStep;
			orientation_ptoD.elevation = orientation_ptoC.elevation;
			orientation_ptoP.azimuth = orientation_ptoC.azimuth + (resamplingStep * 0.5f);
			float azimuth_ptoP = orientation_ptoC.azimuth + (resamplingStep * 0.5f);
			float elevation_ptoP = orientation_ptoC.elevation + (resamplingStep * 0.5f);

			//Depend on the quadrant where the point of interest is situated obtain the Barycentric coordinates and the HRIR of the orientation of interest (azimuth, elevation)
			if (_azimuth >= azimuth_ptoP)
			{
				if (_elevation >= elevation_ptoP)
				{
					//Second quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIR_partitioned_FromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoD);
				}
				else if (_elevation < elevation_ptoP)
				{
					//Forth quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIR_partitioned_FromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoB, orientation_ptoC, orientation_ptoD);
				}
			}
			else if (_azimuth < azimuth_ptoP)
			{
				if (_elevation >= elevation_ptoP)
				{
					//First quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation);
					newHRIR = CalculateHRIR_partitioned_FromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoC);
				}
				else if (_elevation < elevation_ptoP) {
					//Third quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIR_partitioned_FromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoC, orientation_ptoD);
				}
			}
			//SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return newHRIR;
		}


		//	Calculate HRIR using a barycentric coordinates of the three nearest orientation.
		const oneEarHRIR_struct CalculateHRIRFromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3) const
		//const oneEarHRIR_struct CHRTF::CalculateHRIRFromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const
		{
			oneEarHRIR_struct newHRIR;
			//HRIR size will be differente for HRTF in time domain and in frequency domain
			int size;

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (orientation_pto1.azimuth == 360) { orientation_pto1.azimuth = 0; }
				if (orientation_pto2.azimuth == 360) { orientation_pto2.azimuth = 0; }
				if (orientation_pto3.azimuth == 360) { orientation_pto3.azimuth = 0; }
				if (orientation_pto1.elevation == 360) { orientation_pto1.elevation = 0; }
				if (orientation_pto2.elevation == 360) { orientation_pto2.elevation = 0; }
				if (orientation_pto3.elevation == 360) { orientation_pto3.elevation = 0; }
				// Find the HRIR for the specific orientations
				auto it1 = t_HRTF_Resampled_frequency.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_frequency.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_frequency.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_frequency.end() && it2 != t_HRTF_Resampled_frequency.end() && it3 != t_HRTF_Resampled_frequency.end())
				{
					size = it1->second.leftHRIR.size();
					newHRIR.HRIR.resize(size, 0.0f);

					if (ear == Common::T_ear::LEFT) {
						for (int i = 0; i < size; i++) {
							newHRIR.HRIR[i] = barycentricCoordinates.alpha * it1->second.leftHRIR[i] + barycentricCoordinates.beta * it2->second.leftHRIR[i] + barycentricCoordinates.gamma * it3->second.leftHRIR[i];
						}
						newHRIR.delay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.leftDelay + barycentricCoordinates.beta * it2->second.leftDelay + barycentricCoordinates.gamma * it3->second.leftDelay));
					}

					else if (ear == Common::T_ear::RIGHT) {
						for (int i = 0; i < size; i++) {
							newHRIR.HRIR[i] = barycentricCoordinates.alpha * it1->second.rightHRIR[i] + barycentricCoordinates.beta * it2->second.rightHRIR[i] + barycentricCoordinates.gamma * it3->second.rightHRIR[i];
						}
						newHRIR.delay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.rightDelay + barycentricCoordinates.beta * it2->second.rightDelay + barycentricCoordinates.gamma * it3->second.rightDelay));
					}

					else {
						SET_RESULT(RESULT_WARNING, "Ear Type for calculating HRIR from Barycentric Coordinates is not valid");
					}

					//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
				}
				else
				{
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateHRIRFromBarycentricCoordinates not found");
				}
				//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateHRIRFromBarycentricCoordinates");
			}

			return newHRIR;
		}


		//	Calculate HRIR subfilters using a barycentric coordinates of the three nearest orientation.
		const std::vector<CMonoBuffer<float>> CalculateHRIR_partitioned_FromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const
		//const std::vector<CMonoBuffer<float>> CHRTF::CalculateHRIR_partitioned_FromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const
		{
			std::vector<CMonoBuffer<float>> newHRIR;

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (orientation_pto1.azimuth == 360) { orientation_pto1.azimuth = 0; }
				if (orientation_pto2.azimuth == 360) { orientation_pto2.azimuth = 0; }
				if (orientation_pto3.azimuth == 360) { orientation_pto3.azimuth = 0; }
				if (orientation_pto1.elevation == 360) { orientation_pto1.elevation = 0; }
				if (orientation_pto2.elevation == 360) { orientation_pto2.elevation = 0; }
				if (orientation_pto3.elevation == 360) { orientation_pto3.elevation = 0; }

				// Find the HRIR for the given orientations
				auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{
					int subfilterLength = HRIR_partitioned_SubfilterLength;
					newHRIR.resize(HRIR_partitioned_NumberOfSubfilters);

					if (ear == Common::T_ear::LEFT)
					{
						for (int subfilterID = 0; subfilterID < HRIR_partitioned_NumberOfSubfilters; subfilterID++)
						{
							newHRIR[subfilterID].resize(subfilterLength);
							for (int i = 0; i < subfilterLength; i++)
							{
								newHRIR[subfilterID][i] = barycentricCoordinates.alpha * it1->second.leftHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.beta * it2->second.leftHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.gamma * it3->second.leftHRIR_Partitioned[subfilterID][i];
							}
						}
					}

					else if (ear == Common::T_ear::RIGHT)
					{
						for (int subfilterID = 0; subfilterID < HRIR_partitioned_NumberOfSubfilters; subfilterID++)
						{
							newHRIR[subfilterID].resize(subfilterLength, 0.0f);
							for (int i = 0; i < subfilterLength; i++)
							{
								newHRIR[subfilterID][i] = barycentricCoordinates.alpha * it1->second.rightHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.beta * it2->second.rightHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.gamma * it3->second.rightHRIR_Partitioned[subfilterID][i];
							}
						}
					}

					else {
						SET_RESULT(RESULT_WARNING, "Ear Type for calculating HRIR from Barycentric Coordinates is not valid");
					}

					//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
				}
				else {
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateHRIR_partitioned_FromBarycentricCoordinates not found");
				}
			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateHRIR_partitioned_FromBarycentricCoordinates");
			}

			return newHRIR;
		}


		//	Calculate HRIR DELAY using intepolation of the three nearest orientation, in number of samples
		const float GetHRIRDelayInterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const
		//const float CHRTF::GetHRIRDelayInterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const
		{
			float newHRIRDelay;
			TBarycentricCoordinatesStruct barycentricCoordinates;
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = trunc(_azimuth / resamplingStep) * resamplingStep;
			orientation_ptoC.elevation = trunc(_elevation / resamplingStep) * resamplingStep;
			orientation_ptoA.azimuth = orientation_ptoC.azimuth;
			orientation_ptoA.elevation = orientation_ptoC.elevation + resamplingStep;
			orientation_ptoB.azimuth = orientation_ptoC.azimuth + resamplingStep;
			orientation_ptoB.elevation = orientation_ptoC.elevation + resamplingStep;
			orientation_ptoD.azimuth = orientation_ptoC.azimuth + resamplingStep;
			orientation_ptoD.elevation = orientation_ptoC.elevation;
			orientation_ptoP.azimuth = orientation_ptoC.azimuth + (resamplingStep * 0.5f);
			float azimuth_ptoP = orientation_ptoC.azimuth + (resamplingStep * 0.5f);
			float elevation_ptoP = orientation_ptoC.elevation + (resamplingStep * 0.5f);

			//Depend on the quadrant where the point of interest is situated obtain the Barycentric coordinates and the HRIR of the orientation of interest (azimuth, elevation)
			if (_azimuth >= azimuth_ptoP)
			{
				if (_elevation >= elevation_ptoP)
				{
					//Second quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIRDelay = CalculateHRIRDelayFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoD);
				}
				else if (_elevation < elevation_ptoP)
				{
					//Forth quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIRDelay = CalculateHRIRDelayFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoB, orientation_ptoC, orientation_ptoD);
				}
			}
			else if (_azimuth < azimuth_ptoP)
			{
				if (_elevation >= elevation_ptoP)
				{
					//First quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation);
					newHRIRDelay = CalculateHRIRDelayFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoC);
				}
				else if (_elevation < elevation_ptoP) {
					//Third quadrant
					barycentricCoordinates = GetBarycentricCoordinates(_azimuth, _elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIRDelay = CalculateHRIRDelayFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoC, orientation_ptoD);
				}
			}
			//SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return newHRIRDelay;
		}
		
		//	Calculate HRIR DELAY using a barycentric coordinates of the three nearest orientation, in number of samples
		const float CalculateHRIRDelayFromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const
		//const float CHRTF::CalculateHRIRDelayFromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const
		{
			float newHRIRDelay = 0.0f;

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (orientation_pto1.azimuth == 360) { orientation_pto1.azimuth = 0; }
				if (orientation_pto2.azimuth == 360) { orientation_pto2.azimuth = 0; }
				if (orientation_pto3.azimuth == 360) { orientation_pto3.azimuth = 0; }
				if (orientation_pto1.elevation == 360) { orientation_pto1.elevation = 0; }
				if (orientation_pto2.elevation == 360) { orientation_pto2.elevation = 0; }
				if (orientation_pto3.elevation == 360) { orientation_pto3.elevation = 0; }

				// Find the HRIR for the given orientations
				auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{

					if (ear == Common::T_ear::LEFT)
					{
						newHRIRDelay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.leftDelay + barycentricCoordinates.beta * it2->second.leftDelay + barycentricCoordinates.gamma * it3->second.leftDelay));
					}

					else if (ear == Common::T_ear::RIGHT)
					{
						newHRIRDelay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.rightDelay + barycentricCoordinates.beta * it2->second.rightDelay + barycentricCoordinates.gamma * it3->second.rightDelay));
					}

					else {
						SET_RESULT(RESULT_WARNING, "Ear Type for calculating HRIR Delay from Barycentric Coordinates is not valid");
					}

					//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
				}
				else {
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateHRIRDelayFromBarycentricCoordinates not found");
				}
			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateHRIRDelayFromBarycentricCoordinates");
			}

			return newHRIRDelay;
		}

		//		Calculate and remove the common delay of every HRIR functions of the DataBase Table. Off line Method, called from EndSetUp()
		void RemoveCommonDelay_HRTFDataBaseTable();

		//		Calculate the ITD using the Lord Rayleight formula which depend on the interaural azimuth and the listener head radious
		//param		_headRadious		listener head radius, set by the App
		//param		_interauralAzimuth	source interaural azimuth
		//return	float				customizated ITD
		const  float CalculateITDFromHeadRadius(float _headRadius, float _interauralAzimuth)const
		//const float CHRTF::CalculateITDFromHeadRadius(float _headRadius, float _interauralAzimuth) const
		{
			//Calculate the ITD (from https://www.lpi.tel.uva.es/~nacho/docencia/ing_ond_1/trabajos_05_06/io5/public_html/ & http://interface.cipic.ucdavis.edu/sound/tutorial/psych.html)
			float ITD = _headRadius * (_interauralAzimuth + std::sin(_interauralAzimuth)) / globalParameters.GetSoundSpeed(); //_azimuth in radians!
			return 0;// ITD;
		}
				
		// Recalculate the HRTF FFT table partitioned or not with a new bufferSize or resampling step
		void CalculateNewHRTFTable();

		// Reset HRTF
		void Reset();


		//friend class CListener;
	};
}
#endif
