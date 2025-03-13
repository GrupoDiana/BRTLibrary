/**
* \class CAmbisonicBIR
*
* \brief Declaration of CAmbisonicBIR class interface.
* \date	October 2023
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

#ifndef _CAmbisonicBIR_HPP
#define _CAmbisonicBIR_HPP

#include <unordered_map>
#include <cmath>  
#include <Common/Buffer.hpp>
#include <Common/FFTCalculator.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ProcessingModules/AmbisonicEncoder.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/VirtualSpeakers.hpp>
#include <ServiceModules/HRBRIR.hpp>

namespace BRTServices
{		
	class CAmbisonicBIR : public CServicesBase
	{			
		/** \brief Type definition for the AmbisonicIR table	*/
		typedef std::unordered_map<int, BRTServices::THRIRStruct> TAmbisonicIRTable;

		/** \brief Type definition for the AmbisonicIR partitioned table */
		typedef std::unordered_map<int, THRIRPartitionedStruct> TAmbisonicIRPartitionedTable;
		
		/**
		* @brief Type definition for the AmbisonicIR table ordered by listener posision
		*/
		typedef std::unordered_map<TVector3, TAmbisonicIRPartitionedTable> TAmbisonicIRPartitionedTableByPosition;


	public:

		/** \brief Default constructor.
		*/
		CAmbisonicBIR() : ambisonicBIRLoaded{ false }, setupInProgress{ false }, impulseResponseLength { 0 }, IRNumberOfSubFilters{ 0 }, IRSubfilterLength{ 0 }
		{			
		}

		
		/** \brief Setup input and AIR lengths
		*	\param [in] _inputSourceLength length of input buffer
		*	\param [in] _irLength length of impulse response
		*   \eh On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int _ambisonicOrder, BRTProcessing::TAmbisonicNormalization _ambisonicNormalization)
		{									
			ASSERT(_ambisonicOrder > 0, RESULT_ERROR_BADSIZE, "Attempt to set an unllowed value for the ambisonic order.","");
			ASSERT(_ambisonicOrder < 4, RESULT_ERROR_BADSIZE, "Attempt to set an unllowed value for the ambisonic order.","");
			
			std::lock_guard<std::mutex> l(mutex);
			
			setupInProgress = true;
            
			Reset();											
			ambisonicEncoder.Setup(_ambisonicOrder, _ambisonicNormalization);			
			virtualSpeakers.Setup(_ambisonicOrder);			
		}
		
		/**
		 * @brief End the setup of the Ambisonic IR
		 * @return 
		 */
		bool EndSetup() override {
			if (setupInProgress) {
				setupInProgress = false;
				ambisonicBIRLoaded = true;				//	Indicate that the setup has been done
				return true;
			}			
			return false;			
		}

		/**
		 * @brief Get if the Ambisonic IR is ready to provide data
		 * @return true if the setup has been done
		 */
		bool IsReady() { return ambisonicBIRLoaded; }

		//////////////////////////////////////////////////////

		/** \brief Add impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] newData impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/				
		void AddImpulseResponse(int channel, THRIRPartitionedStruct && newPartitionedIR, const Common::CVector3 & _listenerPosition) {									
			std::lock_guard<std::mutex> l(mutex);			
			AddImpulseResponse_Private(channel, std::move(newPartitionedIR), _listenerPosition);												
		}
					

		/** \brief Get data from one BFormat_Partitioned channel
		*	\details In most cases, GetImpulseResponse_Partitioned should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained.
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method
		*   \eh On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse_Partitioned, AddImpulseResponse
		*/				
		const std::vector<CMonoBuffer<float>>& GetChannelPartitionedIR_OneEar(int channel, Common::T_ear _ear, Common::CTransform& _listenerLocation) {
			
			std::lock_guard<std::mutex> l(mutex);		
						
			if (!ambisonicBIRLoaded || setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Error trying to get Ambisonic IR data from a ambisonicIRPartitioned Table. The necessary setup of the class has not been carried out.");			
				return std::vector<CMonoBuffer<float>>(); 
			}			

			// Find Table to use
			Common::CVector3 nearestListenerPosition = FindNearestListenerPosition(_listenerLocation.GetPosition());			
			auto selectedTable = ambisonicIRPartitionedTable.find(TVector3(nearestListenerPosition));
			
			// Find channel into the selected table
			auto it = selectedTable->second.find(channel);
			if (it != selectedTable->second.end())
			{				
				if (_ear == Common::T_ear::LEFT) {					
					return it->second.leftHRIR_Partitioned;
				}
				else if (_ear == Common::T_ear::RIGHT) {					
					return it->second.rightHRIR_Partitioned;
				}
			}
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Error trying to get Ambisonic IR data from a ambisonicIRPartitioned Table. Either the channel is not found or the requested ear did not have a valid parameter.");			
			return std::vector<CMonoBuffer<float>>(); 
		}
								
		/** \brief Get data length of stored impulse responses
		*	\retval dataLength Impulse response buffer size
		*	\pre Impulse response length must be setup
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataLength() const
		{
			//ASSERT(impulseResponseLength > 0, RESULT_ERROR_NOTINITIALIZED, "Data length of AIR has not been defined; have you loaded any AIR data?", "AIR length returned succesfully");
			return impulseResponseLength;
		}
		

		/** \brief Get data length of stored impulse responses in frequency domain (the length of one partition, which is the same for every partition)
		*	\retval dataLength Impulse response FFT partition buffer size (frequency domain)
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetIRSubfilterLength() const {
			return IRSubfilterLength;
		}

		/** \brief Get number of sub-filters (blocks) fo the AIR partition
		*	\retval dataLength Number of sub filters
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetIRNumberOfSubfilters() const {
			return IRNumberOfSubFilters;
		}
						
		/** \brief Add impulse responses from HRTF or HRBRIR data
		*
		*/
		bool AddImpulseResponsesFromHRIR(std::shared_ptr<BRTServices::CServicesBase> _listenerHRTF) {
			
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Error trying to add IR to the Ambisonic IR Table. The necessary setup of the class has not been carried out.");
				return false;
			}

			std::lock_guard<std::mutex> l(mutex);

			impulseResponseLength = _listenerHRTF->GetHRIRLength();	// TODO Do we need this?
			IRSubfilterLength = _listenerHRTF->GetHRIRSubfilterLength();
			IRNumberOfSubFilters = _listenerHRTF->GetHRIRNumberOfSubfilters();

			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsLeft;
			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsRight;
			ambisonicChannelsLeft.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));
			ambisonicChannelsRight.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));

			std::vector<orientation> virtualSpeakerPositions = virtualSpeakers.GetVirtualSpeakersPositions();


			std::vector<Common::CVector3> listenerPositions = _listenerHRTF->GetListenerPositions();

			for (Common::CVector3 _listenerPosition : listenerPositions) {
				//1. Get BRIR values for each channel
				for (int i = 0; i < virtualSpeakerPositions.size(); i++) {
					THRIRPartitionedStruct oneVirtualSpeakersData;

					//oneVirtualSpeakersData = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);
					oneVirtualSpeakersData.leftHRIR_Partitioned = _listenerHRTF->GetHRIRPartitioned(Common::T_ear::LEFT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true, Common::CTransform(_listenerPosition));
					oneVirtualSpeakersData.rightHRIR_Partitioned = _listenerHRTF->GetHRIRPartitioned(Common::T_ear::RIGHT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true, Common::CTransform(_listenerPosition));

					if ((oneVirtualSpeakersData.leftHRIR_Partitioned.size() != _listenerHRTF->GetHRIRNumberOfSubfilters()) || (oneVirtualSpeakersData.rightHRIR_Partitioned.size() != _listenerHRTF->GetHRIRNumberOfSubfilters())) {
						SET_RESULT(RESULT_ERROR_BADSIZE, "The HRIR of a virtual speaker does not have an appropriate value.");
						return false;
					}

					ambisonicEncoder.EncodedPartitionedIR(oneVirtualSpeakersData.leftHRIR_Partitioned, ambisonicChannelsLeft, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation);
					ambisonicEncoder.EncodedPartitionedIR(oneVirtualSpeakersData.rightHRIR_Partitioned, ambisonicChannelsRight, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation);
				}

				for (int i = 0; i < ambisonicEncoder.GetTotalChannels(); i++) {
					THRIRPartitionedStruct oneAmbisonicChannel;
					oneAmbisonicChannel.leftHRIR_Partitioned = ambisonicChannelsLeft[i];
					oneAmbisonicChannel.rightHRIR_Partitioned = ambisonicChannelsRight[i];
					AddImpulseResponse_Private(i, std::move(oneAmbisonicChannel), _listenerPosition);
				}
			}
			return true;
		}


	private:
		

		////////////////////
		// PRIVATE METHODS
		////////////////////

		/** \brief Add impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] newData impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/
		void AddImpulseResponse_Private(int channel, THRIRPartitionedStruct && newPartitionedIR, const Common::CVector3 & _listenerPosition) {
			
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Error trying to ADD a IR to the Ambisonic IR data. The necessary setup of the class has not been carried out.");
				return;
			}
			
			ASSERT(channel < ambisonicEncoder.GetTotalChannels(), RESULT_ERROR_OUTOFRANGE, "Attempting to load Ambisonic IR data for a channel of a higher Ambisonic order than defined.", "");

			
			bool error = false;
			//Check if the listenerPosition is already in the table
			auto it = ambisonicIRPartitionedTable.find(TVector3(_listenerPosition));
			if (it != ambisonicIRPartitionedTable.end()) {
				auto returnValue = it->second.emplace(channel, std::move(newPartitionedIR));
				if (!returnValue.second) {
					error = true;
				}
			} else {
				TAmbisonicIRPartitionedTable irChannelTable;
				auto returnValue = irChannelTable.emplace(channel, std::forward<THRIRPartitionedStruct>(newPartitionedIR));
				if (returnValue.second) {
					auto returnValue2 = ambisonicIRPartitionedTable.emplace(TVector3(_listenerPosition), std::forward<TAmbisonicIRPartitionedTable>(irChannelTable));
					if (returnValue2.second) {
						AddToListenersPositions(_listenerPosition);
					} else {
						error = true;
					}
				} else {
					error = true;
				}
			}
			if (error) {
				SET_RESULT(RESULT_WARNING, "Error emplacing IR in ambisonicIRPartitioned Table for channel " + std::to_string(channel) + ", unknown reason.");
			}
		}


		/** 
		* \brief Add listener position to the list of listener positions
		**/
		void AddToListenersPositions(const Common::CVector3& _listenerPosition) {
			//Check if the listenerPosition is already in the table
			auto it = std::find(ambisonicIRPartitionedTable_ListenerPositions.begin(), ambisonicIRPartitionedTable_ListenerPositions.end(), _listenerPosition);
			if (it == ambisonicIRPartitionedTable_ListenerPositions.end()) {
				ambisonicIRPartitionedTable_ListenerPositions.push_back(_listenerPosition);
			}
		}

		/**
		 * @brief Find the nearest position of the listener stored in the data table.
		 * @param _listenerPosition listener position to compare
		 * @return Closest listener position
		 */
		Common::CVector3 FindNearestListenerPosition(const Common::CVector3& _listenerPosition) const {

			Common::CTransform _listenerLocation(_listenerPosition);
			Common::CTransform nearestListenerLocation(ambisonicIRPartitionedTable_ListenerPositions[0]);
			float minDistance = _listenerLocation.GetVectorTo(nearestListenerLocation).GetDistance();

			for (auto it = ambisonicIRPartitionedTable_ListenerPositions.begin() + 1; it != ambisonicIRPartitionedTable_ListenerPositions.end(); it++) {

				float distance = _listenerLocation.GetVectorTo(Common::CTransform(*it)).GetDistance();
				if (distance < minDistance) {
					minDistance = distance;
					nearestListenerLocation = *it;
				}
			}
			return nearestListenerLocation.GetPosition();
		}


		/** \brief Set to initial state
		*   \eh Nothing is reported to the error handler.*/
		void Reset() {
			ambisonicBIRLoaded = false;
			//setupInProgress = false;
			impulseResponseLength = 0;

			IRSubfilterLength = 0;
			IRNumberOfSubFilters = 0;

			ambisonicIRTable.clear();
			ambisonicIRPartitionedTable.clear();
			ambisonicIRPartitionedTable_ListenerPositions.clear();

			ambisonicEncoder.Reset();
			virtualSpeakers.Reset();
		}

		//////////////////////
		// PRIVATE ATRIBUTES
		//////////////////////

		mutable std::mutex mutex;
		CVirtualSpeakers virtualSpeakers;		// To Store virtual speakers data		
		bool setupInProgress;						// To know if the setup has started
		bool ambisonicBIRLoaded;							// To know if the setup has been done
		//int bufferSize;							// Input source length		
		int impulseResponseLength;				// Data length of one channel for one virtual speaker
		int IRSubfilterLength;					// Data length of one block of the FFT partitioned impulse response in time domain
		//int impulseResponseBlockLength_time;	// Data length of one block of the FFT partitioned impulse response in frequency domain		
		int IRNumberOfSubFilters;		// Number of blocks of the partitioned IR 
		
			
		TAmbisonicIRTable ambisonicIRTable;								// IR data (usally in time domain)
		//TAmbisonicIRPartitionedTable ambisonicIRPartitionedTable;		// IR data partitioned and transformed (FFT)
		TAmbisonicIRPartitionedTableByPosition ambisonicIRPartitionedTable;		// IR data partitioned and transformed (FFT) ordered by listener position

		std::vector<Common::CVector3>	ambisonicIRPartitionedTable_ListenerPositions;

		BRTProcessing::CAmbisonicEncoder ambisonicEncoder; // To do the ambisonic encoding

		Common::CGlobalParameters globalParameters;						// To store global parameters

		
	};
}

#endif

