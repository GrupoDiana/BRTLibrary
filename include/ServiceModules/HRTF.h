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
#include <Common/ErrorHandler.hpp>
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
			:enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, HRIRLength{ 0 }, fileName {""},
			HRTFLoaded{ false }, setupInProgress{ false }, distanceOfMeasurement{ DEFAULT_HRTF_MEASURED_DISTANCE }, headRadius{ DEFAULT_LISTENER_HEAD_RADIOUS }, leftEarLocalPosition {Common::CVector3()}, rightEarLocalPosition{ Common::CVector3() }
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
		void BeginSetup(int32_t _HRIRLength, float _distance)		
		{
			//if ((ownerListener != nullptr) && ownerListener->ownerCore!=nullptr)
			{
				//Update parameters			
				HRIRLength = _HRIRLength;
				distanceOfMeasurement = _distance;
				bufferSize = globalParameters.GetBufferSize();
				//resamplingStep = ownerListener->GetHRTFResamplingStep();		// TODO Do we need this? 

				float partitions = (float)HRIRLength / (float)bufferSize;
				HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				//Clear every table			
				t_HRTF_DataBase.clear();
				t_HRTF_Resampled_frequency.clear();
				t_HRTF_Resampled_partitioned.clear();

				//Change class state
				setupInProgress = true;
				HRTFLoaded = false;

				sphereBorder = 360.0f;

				SET_RESULT(RESULT_OK, "HRTF Setup started");
			}
			/*else
			{
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "Error in HRTF Begin Setup: OwnerCore or OwnerListener are nullPtr");
			}	*/
		}

		/** \brief Set the full HRIR matrix.
		*	\param [in] newTable full table with all HRIR data
		*   \eh Nothing is reported to the error handler.
		*/		
		void AddHRTFTable(T_HRTFTable&& newTable)
		{
			if (setupInProgress) {
				t_HRTF_DataBase = newTable;
			}
		}

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddHRIR(float azimuth, float elevation, THRIRStruct && newHRIR)		
		{
			if (setupInProgress) {
				int iAzimuth = static_cast<int> (round(azimuth));
				int iElevation = static_cast<int> (round(elevation));

				auto returnValue = t_HRTF_DataBase.emplace(orientation(iAzimuth, iElevation), std::forward<THRIRStruct>(newHRIR));
				//Error handler
				if (returnValue.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_DataBase succesfully"); */ }
				else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map"); }
			}
		}

		/** \brief Stop the HRTF configuration		
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/		
		void EndSetup()
		{
			if (setupInProgress) {
				if (!t_HRTF_DataBase.empty())
				{
					//Delete the common delay of every HRIR functions of the DataBase Table
					RemoveCommonDelay_HRTFDataBaseTable();

					//HRTF Resampling methdos
					CalculateHRIR_InPoles();	//Specific method for LISTEN DataBase

					CalculateResampled_HRTFTable(resamplingStep);

					//Setup values
#ifndef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
					auto it = t_HRTF_Resampled_partitioned.begin();
					HRIR_partitioned_SubfilterLength = it->second.leftHRIR_Partitioned[0].size();
#endif // !USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC

					setupInProgress = false;
					HRTFLoaded = true;

					//if (ownerListener != nullptr)
					//{
					//	ownerListener->SetHRTFLoaded();		//Report to the listener that the HRTF has been a loaded.
					//}
					SET_RESULT(RESULT_OK, "HRTF Matrix resample completed succesfully");
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_HRTF_DataBase map has not been set");
				}
			}
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

		/** \brief Get interpolated HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR 
		*	\param [in] _azimuth azimuth angle in degrees
		*	\param [in] _elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/		
		const oneEarHRIR_struct GetHRIR_frequency(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)");
				return emptyOneEarHRIR;
			}

			oneEarHRIR_struct s_HRIR;

			if (!setupInProgress)
			{
				if (runTimeInterpolation)
				{
					// TODO: Modify the orientation one degree resolution as in the partitioned get method
					int iazimuth = static_cast<int>(round(_azimuth));
					int ielevation = static_cast<int>(round(_elevation));
					// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
					if (iazimuth == 360) { iazimuth = 0; }
					if (ielevation == 360) { ielevation = 0; }
					// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
					if ((ielevation == 90) || (ielevation == 270)) { iazimuth = 0; }

					auto it = t_HRTF_Resampled_frequency.find(orientation(iazimuth, ielevation));
					if (it != t_HRTF_Resampled_frequency.end())
					{
						if (ear == Common::T_ear::LEFT)
						{
							s_HRIR.delay = it->second.leftDelay;
							s_HRIR.HRIR = it->second.leftHRIR;
						}
						else
						{
							s_HRIR.delay = it->second.rightDelay;
							s_HRIR.HRIR = it->second.rightHRIR;
						}
					}
					else
					{
						//Run time interpolation ON
						s_HRIR = GetHRIR_InterpolationMethod(ear, iazimuth, ielevation);
					}

					//Modify delay if customized delay is activate
					if (enableCustomizedITD) {
						s_HRIR.delay = GetCustomizedDelay(iazimuth, ielevation, ear);
					}

					return s_HRIR;
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

					auto it = t_HRTF_Resampled_frequency.find(orientation(nearestAzimuth, nearestElevation));
					if (it != t_HRTF_Resampled_frequency.end())
					{
						if (ear == Common::T_ear::LEFT)
						{
							s_HRIR.delay = it->second.leftDelay;
							s_HRIR.HRIR = it->second.leftHRIR;
						}
						else
						{
							s_HRIR.delay = it->second.rightDelay;
							s_HRIR.HRIR = it->second.rightHRIR;
						}
						//Modify delay if customized delay is activate
						if (enableCustomizedITD) {
							s_HRIR.delay = GetCustomizedDelay(nearestAzimuth, nearestElevation, ear);
						}
						return s_HRIR;
					}
					else
					{
						SET_RESULT(RESULT_ERROR_NOTSET, "GetInterpolated_HRIR_frequency: HRIR not found");
						return emptyOneEarHRIR;
					}
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetInterpolated_HRIR_frequency: HRTF Set up in progress");
				return emptyOneEarHRIR;
			}
			SET_RESULT(RESULT_WARNING, "GetInterpolated_HRIR_frequency return empty");
			return emptyOneEarHRIR;
		}

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
		const int32_t GetHRIRNumberOfSubfilters() const {
			return HRIR_partitioned_NumberOfSubfilters;
		}

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetHRIRSubfilterLength() const {
			return HRIR_partitioned_SubfilterLength;
		}

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/		
		bool IsHRTFLoaded()
		{
			return HRTFLoaded;
		}

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/		
		const T_HRTFTable& GetRawHRTFTable() const
		{
			return t_HRTF_DataBase;
		}

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

			float ITD = CalculateITDFromHeadRadius(headRadius /*ownerListener->GetHeadRadius()*/, interauralAzimuth);

			if ((ITD > 0 && ear == Common::T_ear::RIGHT) || (ITD < 0 && ear == Common::T_ear::LEFT)) {
				customizedDelay = static_cast <unsigned long> (round(std::abs(globalParameters.GetSampleRate() * ITD)));
			}
			return customizedDelay;
		}


		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/		
		float GetHRTFDistanceOfMeasurement() {
			return distanceOfMeasurement;
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

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() {
			return headRadius;
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition( Common::T_ear _ear, Common::CVector3 _earPosition) {
			if (_ear == Common::T_ear::LEFT)		{ leftEarLocalPosition = _earPosition; }
			else if (_ear == Common::T_ear::RIGHT)	{ rightEarLocalPosition = _earPosition; }
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
				return CalculateEarLocalPositionFromHeadRadius(_ear);
			}
			else {
				if (_ear == Common::T_ear::LEFT)		{ return leftEarLocalPosition; }
				else if (_ear == Common::T_ear::RIGHT)	{ return rightEarLocalPosition; }
				else // if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE)
				{
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
					return Common::CVector3();
				}
			}
		}

		/** \brief	Calculate the relative position of one ear taking into account the listener head radius
		*	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 CalculateEarLocalPositionFromHeadRadius(Common::T_ear ear) {
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -headRadius);
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, headRadius);

			return earLocalPosition;
		}



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
		float headRadius;								// Head radius of listener 
		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position


		float sphereBorder;						// Define spheere "sewing"
		float epsilon_sewing = 0.001f;

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableCustomizedITD;					// Indicate the use of a customized delay
		std::string fileName;

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
		void CalculateHRIR_InPoles()
		{
			THRIRStruct precalculatedHRIR_270, precalculatedHRIR_90;
			int azimuthStep = AZIMUTH_STEP;
			bool found = false;

			//Clasify every HRIR of the HRTF into the two hemispheres by their orientations
			std::vector<orientation> keys_southernHemisphere, keys_northenHemisphere;

			//	NORTHERN HEMOSPHERE POLES (90 degrees elevation ) ____________________________________________________________________________

			//If HRIR with orientation (0,90) exist in t_HRTF_DataBase
			auto it90 = t_HRTF_DataBase.find(orientation(0, 90));
			if (it90 != t_HRTF_DataBase.end())
			{
				precalculatedHRIR_90 = it90->second;
			}
			else
			{
				keys_northenHemisphere.reserve(t_HRTF_DataBase.size());
				for (auto& it : t_HRTF_DataBase)
				{
					if (it.first.elevation < 90) { keys_northenHemisphere.push_back(it.first); }
				}
				// sort using a custom function object
				struct {
					bool operator()(orientation a, orientation b) const
					{
						return a.elevation > b.elevation;
					}
				} customLess;
				std::sort(keys_northenHemisphere.begin(), keys_northenHemisphere.end(), customLess);

				//C++14
				/*std::sort(keys_northenHemisphere.begin(), keys_northenHemisphere.end(), [](auto &left, auto &right) {
				return left.elevation > right.elevation;
				});*/

				precalculatedHRIR_90 = CalculateHRIR_InOneHemispherePole(keys_northenHemisphere);

			}

			//	SOURTHERN HEMOSPHERE POLES (270 degrees elevation) ____________________________________________________________________________

			//If HRIR with orientation (0,270) exist in t_HRTF_DataBase
			if (t_HRTF_DataBase.find(orientation(0, 270)) != t_HRTF_DataBase.end())
			{
				auto it270 = t_HRTF_DataBase.find(orientation(0, 270));
				precalculatedHRIR_270 = it270->second;
			}
			else
			{
				keys_southernHemisphere.reserve(t_HRTF_DataBase.size());
				for (auto& it : t_HRTF_DataBase)
				{
					if (it.first.elevation > 270) { keys_southernHemisphere.push_back(it.first); }
				}
				//Get a vector of iterators ordered from highest to lowest elevation.		
				struct {
					bool operator()(orientation a, orientation b) const
					{
						return a.elevation < b.elevation;
					}
				} customLess;
				std::sort(keys_southernHemisphere.begin(), keys_southernHemisphere.end(), customLess);

				//C++14
				/*std::sort(keys_southernHemisphere.begin(), keys_southernHemisphere.end(), [](auto &left, auto &right) {
				return left.elevation < right.elevation;
				});*/

				precalculatedHRIR_270 = CalculateHRIR_InOneHemispherePole(keys_southernHemisphere);
			}


			// Fill out the table ____________________________________________________________________________

			for (int i = 0; i < 360; i = i + AZIMUTH_STEP)
			{
				//Elevation 270 degrees
				t_HRTF_DataBase.emplace(orientation(i, 270), precalculatedHRIR_270);
				//Elevation 90 degrees
				t_HRTF_DataBase.emplace(orientation(i, 90), precalculatedHRIR_90);
				//Azimuth 360 degrees
				auto it0 = t_HRTF_DataBase.find(orientation(0, i));
				if (it0 != t_HRTF_DataBase.end()) {
					t_HRTF_DataBase.emplace(orientation(360, i), it0->second);
				}
			}
		}

		//	Calculate the HRIR in the pole of one of the hemispheres
		//param hemisphereParts	vector of the HRTF orientations of the hemisphere		
		THRIRStruct CalculateHRIR_InOneHemispherePole(std::vector<orientation> keys_hemisphere)
		{
			THRIRStruct calculatedHRIR;
			std::vector < std::vector <orientation>> hemisphereParts;
			hemisphereParts.resize(NUMBER_OF_PARTS);
			int border = std::ceil(360.0f / NUMBER_OF_PARTS);

			auto currentElevation = keys_hemisphere.begin()->elevation;
			for (auto& it : keys_hemisphere)
			{
				if (it.elevation == currentElevation)
				{
					//Clasify each orientation in its corresponding hemisphere part group (by the azimuth value). The number of the parts and the size depend on the NUMBER_OF_PARTS value
					int32_t currentAzimuth = it.azimuth;
					for (int j = 0; j < NUMBER_OF_PARTS; j++)
					{
						if (currentAzimuth >= (border * j) && currentAzimuth < (border * (j + 1))) {
							hemisphereParts[j].push_back(it);
							break;
						}
					}
				}
				else
				{
					//Check if there are values in every hemisphere part
					bool everyPartWithValues = true;
					for (int j = 0; j < NUMBER_OF_PARTS; j++)
					{
						everyPartWithValues = everyPartWithValues & (hemisphereParts[j].size() > 0);
					}

					//If hemisphere every part has at list a value, the algorithm of select the interpolated HRIR finish
					if (everyPartWithValues) { break; }
					//else, the algorithm continue, unless the elevation is MAX_DISTANCE_BETWEEN_ELEVATIONS bigger than the first selected elevation
					else
					{
						currentElevation = it.elevation;
						if (currentElevation > (keys_hemisphere.begin()->elevation + MAX_DISTANCE_BETWEEN_ELEVATIONS))
						{
							break;
						}
						else
						{
							int32_t currentAzimuth = it.azimuth;
							for (int j = 0; j < NUMBER_OF_PARTS; j++)
							{
								//The iterator (it) has moved fordward, so this orientation must be clasified a hemispehere part group
								if (currentAzimuth >= (border * j) && currentAzimuth < (border * (j + 1))) {
									hemisphereParts[j].push_back(it);
									break;
								}
							}
						}
					} //END else 'if(everyPartWithValues)'
				}// END else of 'if(it.elevation == currentElevation)'
			}//END loop keys_hemisphere

			 //Calculate the delay and the HRIR of each hemisphere part
			float totalDelay_left = 0.0f;
			float totalDelay_right = 0.0f;

			std::vector< THRIRStruct> newHRIR;
			newHRIR.resize(hemisphereParts.size());

			for (int q = 0; q < hemisphereParts.size(); q++)
			{
				newHRIR[q].leftHRIR.resize(HRIRLength, 0.0f);
				newHRIR[q].rightHRIR.resize(HRIRLength, 0.0f);

				float scaleFactor;
				if (hemisphereParts[q].size())
				{
					scaleFactor = 1.0f / hemisphereParts[q].size();
				}
				else
				{
					scaleFactor = 0.0f;
				}

				for (auto it = hemisphereParts[q].begin(); it != hemisphereParts[q].end(); it++)
				{
					auto itHRIR = t_HRTF_DataBase.find(orientation(it->azimuth, it->elevation));

					//Get the delay
					newHRIR[q].leftDelay = (newHRIR[q].leftDelay + itHRIR->second.leftDelay);
					newHRIR[q].rightDelay = (newHRIR[q].rightDelay + itHRIR->second.rightDelay);

					//Get the HRIR
					for (int i = 0; i < HRIRLength; i++) {
						newHRIR[q].leftHRIR[i] = (newHRIR[q].leftHRIR[i] + itHRIR->second.leftHRIR[i]);
						newHRIR[q].rightHRIR[i] = (newHRIR[q].rightHRIR[i] + itHRIR->second.rightHRIR[i]);
					}


				}//END loop hemisphere part

				 //Multiply by the factor (weighted sum)
				 //Delay 
				totalDelay_left = totalDelay_left + (scaleFactor * newHRIR[q].leftDelay);
				totalDelay_right = totalDelay_right + (scaleFactor * newHRIR[q].rightDelay);
				//HRIR
				for (int i = 0; i < HRIRLength; i++)
				{
					newHRIR[q].leftHRIR[i] = newHRIR[q].leftHRIR[i] * scaleFactor;
					newHRIR[q].rightHRIR[i] = newHRIR[q].rightHRIR[i] * scaleFactor;
				}
			}

			//Get the FINAL values
			float scaleFactor_final = 1.0f / hemisphereParts.size();

			//Calculate Final delay
			calculatedHRIR.leftDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_left));
			calculatedHRIR.rightDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_right));

			//calculate Final HRIR
			calculatedHRIR.leftHRIR.resize(HRIRLength, 0.0f);
			calculatedHRIR.rightHRIR.resize(HRIRLength, 0.0f);

			for (int i = 0; i < HRIRLength; i++)
			{
				for (int q = 0; q < hemisphereParts.size(); q++)
				{
					calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] + newHRIR[q].leftHRIR[i];
					calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] + newHRIR[q].rightHRIR[i];
				}
			}
			for (int i = 0; i < HRIRLength; i++)
			{
				calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] * scaleFactor_final;
				calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] * scaleFactor_final;
			}

			return calculatedHRIR;
		}

		//	Calculate the resample matrix using the Barycentric interpolation Method (copy the HRIR function of the nearest orientation)
		//param resamplingStep	HRTF resample matrix step for both azimuth and elevation		
		void CalculateResampled_HRTFTable(int resamplingStep)
		{
			THRIRStruct interpolatedHRIR;

			//Resample Interpolation Algorithm
			for (int newAzimuth = 0; newAzimuth < 360; newAzimuth = newAzimuth + resamplingStep)
			{
				for (int newElevation = 0; newElevation <= 90; newElevation = newElevation + resamplingStep)
				{
					auto it = t_HRTF_DataBase.find(orientation(newAzimuth, newElevation));
					if (it != t_HRTF_DataBase.end())
					{

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
						//Fill out interpolated frequency table. IR in frequency domain
						THRIRStruct temp;
						Common::CFprocessor::GetFFT(it->second.leftHRIR, temp.leftHRIR, bufferSize);
						Common::CFprocessor::GetFFT(it->second.rightHRIR, temp.rightHRIR, bufferSize);
						temp.leftDelay = it->second.leftDelay;
						temp.rightDelay = it->second.rightDelay;
						auto returnValue2 = t_HRTF_Resampled_frequency.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRStruct>(temp));
						//Error handler
						if (returnValue2.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_frequency successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_frequency table"); }
#else
						//Fill out HRTF partitioned table.IR in frequency domain
						THRIRPartitionedStruct newHRIR_partitioned;
						newHRIR_partitioned = SplitAndGetFFT_HRTFData(it->second);
						auto returnValue3 = t_HRTF_Resampled_partitioned.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRPartitionedStruct>(newHRIR_partitioned));
						//Error handler
						if (returnValue3.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_partitioned successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_partitioned table"); }
#endif
					}

					else
					{

						//Get the interpolated HRIR 
						interpolatedHRIR = CalculateHRIR_offlineMethod(newAzimuth, newElevation);

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
						//Fill out interpolated frequency table. IR in frequency domain
						THRIRStruct newHRIR;
						Common::CFprocessor::GetFFT(interpolatedHRIR.leftHRIR, newHRIR.leftHRIR, bufferSize);
						Common::CFprocessor::GetFFT(interpolatedHRIR.rightHRIR, newHRIR.rightHRIR, bufferSize);
						newHRIR.leftDelay = interpolatedHRIR.leftDelay;
						newHRIR.rightDelay = interpolatedHRIR.rightDelay;

						auto returnValue2 = t_HRTF_Resampled_frequency.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRStruct>(newHRIR));
						//Error handler
						if (returnValue2.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_frequency successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_frequency table"); }

#else
						//Fill out HRTF partitioned table.IR in frequency domain
						THRIRPartitionedStruct newHRIR_partitioned;
						newHRIR_partitioned = SplitAndGetFFT_HRTFData(interpolatedHRIR);
						auto returnValue1 = t_HRTF_Resampled_partitioned.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRPartitionedStruct>(newHRIR_partitioned));
						//Error handler
						if (returnValue1.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_partitioned successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_partitioned table"); }
#endif												
					}
				}

				for (int newElevation = 270; newElevation < 360; newElevation = newElevation + resamplingStep)
				{
					auto it2 = t_HRTF_DataBase.find(orientation(newAzimuth, newElevation));
					if (it2 != t_HRTF_DataBase.end())
					{
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC

						//Fill out interpolated frequency table. IR in frequency domain
						THRIRStruct temp;
						Common::CFprocessor::GetFFT(it2->second.leftHRIR, temp.leftHRIR, bufferSize);
						Common::CFprocessor::GetFFT(it2->second.rightHRIR, temp.rightHRIR, bufferSize);
						temp.leftDelay = it2->second.leftDelay;
						temp.rightDelay = it2->second.rightDelay;
						auto returnValue = t_HRTF_Resampled_frequency.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRStruct>(temp));
						//Error handler
						if (returnValue.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_frequency successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_frequency table"); }
#else
						//Fill out HRTF partitioned table.IR in frequency domain
						THRIRPartitionedStruct newHRIR_partitioned;
						newHRIR_partitioned = SplitAndGetFFT_HRTFData(it2->second);
						auto returnValue3 = t_HRTF_Resampled_partitioned.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRPartitionedStruct>(newHRIR_partitioned));
						//Error handler
						if (returnValue3.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_partitioned successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_partitioned table"); }
#endif
					}
					else
					{
						//Get the interpolated HRIR 
						interpolatedHRIR = CalculateHRIR_offlineMethod(newAzimuth, newElevation);
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
						//Fill out interpolated frequency table. IR in frequency domain
						THRIRStruct newHRIR;
						Common::CFprocessor::GetFFT(interpolatedHRIR.leftHRIR, newHRIR.leftHRIR, bufferSize);
						Common::CFprocessor::GetFFT(interpolatedHRIR.rightHRIR, newHRIR.rightHRIR, bufferSize);
						newHRIR.leftDelay = interpolatedHRIR.leftDelay;
						newHRIR.rightDelay = interpolatedHRIR.rightDelay;
						auto returnValue = t_HRTF_Resampled_frequency.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRStruct>(newHRIR));
						//Error handler
						if (returnValue.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_frequency successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_frequency table"); }
#else
						//Fill out HRTF partitioned table.IR in frequency domain
						THRIRPartitionedStruct newHRIR_partitioned;
						newHRIR_partitioned = SplitAndGetFFT_HRTFData(interpolatedHRIR);
						auto returnValue1 = t_HRTF_Resampled_partitioned.emplace(orientation(newAzimuth, newElevation), std::forward<THRIRPartitionedStruct>(newHRIR_partitioned));
						//Error handler
						if (returnValue1.second) { /*SET_RESULT(RESULT_OK, "HRIR emplaced into t_HRTF_Resampled_partitioned successfully");*/ }
						else { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR into t_HRTF_Resampled_partitioned table"); }
#endif // 															
					}
				}
			}
			//SET_RESULT(RESULT_OK, "CalculateResampled_HRTFTable has finished succesfully");
		}

		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain		
		THRIRPartitionedStruct SplitAndGetFFT_HRTFData(const THRIRStruct& newData_time)
		{
			int blockSize = bufferSize;
			int numberOfBlocks = HRIR_partitioned_NumberOfSubfilters;
			int data_time_size = newData_time.leftHRIR.size();

			THRIRPartitionedStruct new_DataFFT_Partitioned;
			new_DataFFT_Partitioned.leftHRIR_Partitioned.reserve(numberOfBlocks);
			new_DataFFT_Partitioned.rightHRIR_Partitioned.reserve(numberOfBlocks);
			//Index to go throught the AIR values in time domain
			int index;
			for (int i = 0; i < data_time_size; i = i + blockSize)
			{
				CMonoBuffer<float> left_data_FFT_doubleSize, right_data_FFT_doubleSize;
				//Resize with double size and zeros to make the zero-padded demanded by the algorithm
				left_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
				right_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
				//Fill each AIR block (question about this comment: AIR???)
				for (int j = 0; j < blockSize; j++) {
					index = i + j;
					if (index < data_time_size) {
						left_data_FFT_doubleSize[j] = newData_time.leftHRIR[index];
						right_data_FFT_doubleSize[j] = newData_time.rightHRIR[index];
					}
				}
				//FFT
				CMonoBuffer<float> left_data_FFT, right_data_FFT;
				Common::CFprocessor::CalculateFFT(left_data_FFT_doubleSize, left_data_FFT);
				Common::CFprocessor::CalculateFFT(right_data_FFT_doubleSize, right_data_FFT);
				//Prepare struct to return the value
				new_DataFFT_Partitioned.leftHRIR_Partitioned.push_back(left_data_FFT);
				new_DataFFT_Partitioned.rightHRIR_Partitioned.push_back(right_data_FFT);
			}
			//Store the delays
			new_DataFFT_Partitioned.leftDelay = newData_time.leftDelay;
			new_DataFFT_Partitioned.rightDelay = newData_time.rightDelay;

			return new_DataFFT_Partitioned;
		}

		//		Calculate the distance between two points [(azimuth1, elevation1) and (azimuth2, elevation2)] using the Haversine formula
		//return	float	the distance value		
		float CalculateDistance_HaversineFormula(float azimuth1, float elevation1, float azimuth2, float elevation2)
		{
			float incremento_azimuth = azimuth1 - azimuth2;
			float incremento_elevacion = elevation1 - elevation2;
			float term1 = sin(incremento_elevacion / 2 * PI / 180.0);
			term1 = term1 * term1;
			float term2 = std::cos(elevation1 * PI / 180.0);
			float term3 = std::cos(elevation2 * PI / 180.0);
			float term4 = std::sin(incremento_azimuth / 2 * PI / 180.0);
			term4 = term4 * term4;
			float raiz = term1 + term2 * term3 * term4;

			ASSERT(raiz > 0, RESULT_ERROR_OUTOFRANGE, "Attempt to compute square root of a negative value using Haversine Formula to compute distance", "");
			float sqrtDistance = std::sqrt(raiz);

			ASSERT(sqrtDistance >= -1.0f && sqrtDistance <= 1.0f, RESULT_ERROR_OUTOFRANGE,
				"Attempt to compute arcsin of a value outside [-1..1] using Harvesine Formula to compute distnace",
				"");

			float distance = std::asin(std::sqrt(raiz));

			return distance;
		}

		//	Calculate the HRIR of a specific orientation (newazimuth, newelevation) using the Barycentric interpolation Method
		//param newAzimuth		azimuth of the orientation of interest (the one whose HRIR will be calculated)
		//param newElevation	elevation of the orientation of interest (the one whose HRIR will be calculated)		
		THRIRStruct CalculateHRIR_offlineMethod(int newAzimuth, int newElevation)
		{
			THRIRStruct newHRIR;
			// Get a list sorted by distances to the orientation of interest
			std::list<T_PairDistanceOrientation> sortedList = GetSortedDistancesList(newAzimuth, newElevation);

			if (sortedList.size() != 0) {
				// Obtain  the valid Barycentric coordinates:
				TBarycentricCoordinatesStruct barycentricCoordinates;
				std::vector<orientation> mygroup(sortedList.size());
				auto it = sortedList.begin();
				for (int copy = 0; copy < sortedList.size(); copy++) {
					mygroup[copy] = it->second;
					it++;
				}
				//Algorithm to get a triangle around the orientation of interest
				for (int groupSize = 3; groupSize < sortedList.size(); groupSize++)
				{
					for (int i = 0; i < groupSize - 2; i++)
					{
						for (int j = i + 1; j < groupSize - 1; j++)
						{
							for (int k = j + 1; k < groupSize; k++)
							{
								//Azimuth and elevation transformation in order to get the barientric coordinates (due to we are working with a spehere not a plane)
								float newAzimuthTransformed = TransformAzimuth(newAzimuth, newAzimuth);
								float iAzimuthTransformed = TransformAzimuth(newAzimuth, mygroup[i].azimuth);
								float jAzimuthTransformed = TransformAzimuth(newAzimuth, mygroup[j].azimuth);
								float kAzimuthTransformed = TransformAzimuth(newAzimuth, mygroup[k].azimuth);
								float newElevationTransformed = TransformElevation(newElevation, newElevation);
								float iElevationTransformed = TransformElevation(newElevation, mygroup[i].elevation);
								float jElevationTransformed = TransformElevation(newElevation, mygroup[j].elevation);
								float kElevationTransformed = TransformElevation(newElevation, mygroup[k].elevation);

								barycentricCoordinates = GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, iAzimuthTransformed, iElevationTransformed, jAzimuthTransformed, jElevationTransformed, kAzimuthTransformed, kElevationTransformed);

								if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f) {
									// Calculate the new HRIR with the barycentric coorfinates
									auto it0 = t_HRTF_DataBase.find(orientation(mygroup[i].azimuth, mygroup[i].elevation));
									auto it1 = t_HRTF_DataBase.find(orientation(mygroup[j].azimuth, mygroup[j].elevation));
									auto it2 = t_HRTF_DataBase.find(orientation(mygroup[k].azimuth, mygroup[k].elevation));

									if (it0 != t_HRTF_DataBase.end() && it1 != t_HRTF_DataBase.end() && it2 != t_HRTF_DataBase.end()) {

										//FIXME!!! another way to initialize?
										newHRIR = it0->second;
										//END FIXME

										for (int i = 0; i < HRIRLength; i++) {
											newHRIR.leftHRIR[i] = barycentricCoordinates.alpha * it0->second.leftHRIR[i] + barycentricCoordinates.beta * it1->second.leftHRIR[i] + barycentricCoordinates.gamma * it2->second.leftHRIR[i];
											newHRIR.rightHRIR[i] = barycentricCoordinates.alpha * it0->second.rightHRIR[i] + barycentricCoordinates.beta * it1->second.rightHRIR[i] + barycentricCoordinates.gamma * it2->second.rightHRIR[i];
										}

										// Calculate delay
										newHRIR.leftDelay = barycentricCoordinates.alpha * it0->second.leftDelay + barycentricCoordinates.beta * it1->second.leftDelay + barycentricCoordinates.gamma * it2->second.leftDelay;
										newHRIR.rightDelay = barycentricCoordinates.alpha * it0->second.rightDelay + barycentricCoordinates.beta * it1->second.rightDelay + barycentricCoordinates.gamma * it2->second.rightDelay;
										//SET_RESULT(RESULT_OK, "HRIR calculated with interpolation method succesfully");
										return newHRIR;
									}
									else {
										SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod return empty because HRIR with a specific orientation was not found");
										return emptyHRIR;
									}
								}
							}
						}
					}
				}
				//SET_RESULT(RESULT_OK, "");
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Orientation List sorted by distances in GetHRIR_InterpolationMethod is empty");
			}

			SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod returns empty");
			return emptyHRIR;

		}

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
		float TransformAzimuth(float azimuthOrientationOfInterest, float originalAzimuth)
		{
			float azimuth;
			azimuth = originalAzimuth + 180 - azimuthOrientationOfInterest;

			// Check limits (always return 0 instead of 360)
			if (azimuth >= 360)
				azimuth = std::fmod(azimuth, (float)360);

			if (azimuth < 0)
				azimuth = azimuth + 360;

			return azimuth;
		}

		//		Transform the orientation in order to express the elevation in the interval [-90,90]
		//returnval float transformed elevation		
		float TransformElevation(float elevationOrientationOfInterest, float originalElevation)
		{
			if (originalElevation >= 270) {
				originalElevation = originalElevation - 360;
			}
			return originalElevation;
		}

		//	Calculate the distance between the given orientation (newAzimuth, newElevation) and all other values of the databsde HRTF table. And store these values in a sorted list
		//param	newAzimuth		azimuth of the orientation of interest in degrees
		//param	newElevation	elevation of the orientation of interest in degrees
		//return the distances sorted list
		std::list<T_PairDistanceOrientation> GetSortedDistancesList(int newAzimuth, int newElevation)
		{
			T_PairDistanceOrientation temp;
			float distance;
			std::list<T_PairDistanceOrientation> sortedList;

			// Algorithm to calculate the three shortest distances between the point (newAzimuth, newelevation) and all the points in the HRTF table (t)
			for (auto it = t_HRTF_DataBase.begin(); it != t_HRTF_DataBase.end(); ++it)
			{
				distance = CalculateDistance_HaversineFormula(newAzimuth, newElevation, it->first.azimuth, it->first.elevation);

				temp.first = distance;
				temp.second = it->first;

				sortedList.push_back(temp);
			}

			if (sortedList.size() != 0) {
				sortedList.sort([](const T_PairDistanceOrientation& a, const T_PairDistanceOrientation& b) { return a.first < b.first; });
				//SET_RESULT(RESULT_OK, "Sorted distances list obtained succesfully");
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted by distances is empty");
			}

			return sortedList;
		}

		//	Get HRIR from resample table using a barycentric interpolation of the three nearest orientation.		
		const oneEarHRIR_struct GetHRIR_InterpolationMethod(Common::T_ear ear, int azimuth, int elevation) const
		{
			oneEarHRIR_struct newHRIR;
			TBarycentricCoordinatesStruct barycentricCoordinates;
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = trunc(azimuth / resamplingStep) * resamplingStep;
			orientation_ptoC.elevation = trunc(elevation / resamplingStep) * resamplingStep;
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
			if (azimuth >= azimuth_ptoP)
			{
				if (elevation >= elevation_ptoP)
				{
					//Second quadrant
					barycentricCoordinates = GetBarycentricCoordinates(azimuth, elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIRFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoD);
				}
				else if (elevation < elevation_ptoP)
				{
					//Forth quadrant
					barycentricCoordinates = GetBarycentricCoordinates(azimuth, elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIRFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoB, orientation_ptoC, orientation_ptoD);
				}
			}
			else if (azimuth < azimuth_ptoP)
			{
				if (elevation >= elevation_ptoP)
				{
					//First quadrant
					barycentricCoordinates = GetBarycentricCoordinates(azimuth, elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoB.azimuth, orientation_ptoB.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation);
					newHRIR = CalculateHRIRFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoB, orientation_ptoC);
				}
				else if (elevation < elevation_ptoP) {
					//Third quadrant
					barycentricCoordinates = GetBarycentricCoordinates(azimuth, elevation, orientation_ptoA.azimuth, orientation_ptoA.elevation, orientation_ptoC.azimuth, orientation_ptoC.elevation, orientation_ptoD.azimuth, orientation_ptoD.elevation);
					newHRIR = CalculateHRIRFromBarycentricCoordinates(ear, barycentricCoordinates, orientation_ptoA, orientation_ptoC, orientation_ptoD);
				}
			}

			//SET_RESULT(RESULT_OK, "CalculateHRIR_InterpolationMethod completed succesfully");
			return newHRIR;
		}

		//	Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		const std::vector<CMonoBuffer<float>> GetHRIR_partitioned_InterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const		
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
		void RemoveCommonDelay_HRTFDataBaseTable()
		{
			//1. Init the minumun value with the fist value of the table
			auto it0 = t_HRTF_DataBase.begin();
			unsigned long minimumDelayLeft = it0->second.leftDelay;		//Vrbl to store the minumun delay value for left ear
			unsigned long minimumDelayRight = it0->second.rightDelay;	//Vrbl to store the minumun delay value for right ear

			//2. Find the common delay
			//Scan the whole table looking for the minimum delay for left and right ears
			for (auto it = t_HRTF_DataBase.begin(); it != t_HRTF_DataBase.end(); it++) {
				//Left ear
				if (it->second.leftDelay < minimumDelayLeft) {
					minimumDelayLeft = it->second.leftDelay;
				}
				//Right ear
				if (it->second.rightDelay < minimumDelayRight) {
					minimumDelayRight = it->second.rightDelay;
				}
			}

			//3. Delete the common delay
			//Scan the whole table substracting the common delay to every delays for both ears separately
			//The common delay of each canal have been calculated and subtracted separately in order to correct the asymmetry of the measurement
			if (minimumDelayRight != 0 || minimumDelayLeft != 0)
			{
				for (auto it = t_HRTF_DataBase.begin(); it != t_HRTF_DataBase.end(); it++)
				{
					it->second.leftDelay = it->second.leftDelay - minimumDelayLeft;		//Left ear
					it->second.rightDelay = it->second.rightDelay - minimumDelayRight;	//Right ear
				}
			}

			//SET_RESULT(RESULT_OK, "Common delay deleted from HRTF table succesfully");
		}

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
		void CalculateNewHRTFTable() {
			if (!t_HRTF_DataBase.empty())
			{
				//BeginSetup
				//Update parameters								
				bufferSize = globalParameters.GetBufferSize();
				//resamplingStep = ownerListener->GetHRTFResamplingStep();		// TODO Do we need this? 
				float partitions = (float)HRIRLength / (float)bufferSize;
				HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				//Clear every table		
				t_HRTF_Resampled_frequency.clear();
				t_HRTF_Resampled_partitioned.clear();

				//Change class state
				setupInProgress = true;
				HRTFLoaded = false;

				//Calculate Tables
				EndSetup();
			}
		}

		// Reset HRTF		
		void Reset() {

			//Change class state
			setupInProgress = false;
			HRTFLoaded = false;

			//Clear every table			
			t_HRTF_DataBase.clear();
			t_HRTF_Resampled_frequency.clear();
			t_HRTF_Resampled_partitioned.clear();

			//Update parameters			
			HRIRLength = 0;
			bufferSize = 0;
			resamplingStep = 0;
		}		
	};
}
#endif
