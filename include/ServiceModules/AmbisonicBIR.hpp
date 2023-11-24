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

#ifndef _CAmbisonicBIR_HPP
#define _CAmbisonicBIR_HPP

#include <unordered_map>
#include <cmath>  
#include <Common/Buffer.hpp>
#include <Common/Fprocessor.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/AmbisonicEncoder.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/VirtualSpeakers.hpp>

namespace BRTServices
{		
	class CAmbisonicBIR : public CServicesBase
	{	
		enum TDataOrigin { BRIR, HRTF };
		/** \brief Type definition for the AmbisonicIR table	*/
		typedef std::unordered_map<int, BRTServices::THRIRStruct> TAmbisonicIRTable;

		/** \brief Type definition for the AmbisonicIR partitioned table */
		typedef std::unordered_map<int, THRIRPartitionedStruct> TAmbisonicIRPartitionedTable;


	public:

		/** \brief Default constructor.
		*/
		CAmbisonicBIR() : dataOrigin{ TDataOrigin::BRIR }, AmbisonicBIRLoaded{ false }, setupInProgress{ false }, impulseResponseLength { 0 }, IRNumberOfSubFilters{ 0 }, IRSubfilterLength{ 0 }
		{			
		}


		/** \brief Setup input and AIR lengths
		*	\param [in] _inputSourceLength length of input buffer
		*	\param [in] _irLength length of impulse response
		*   \eh On error, an error code is reported to the error handler.
		*/
		//void BeginSetup(int _bufferSize, int _irLength)
		//{		
		//	// TODO this method has not yet been tested within BRT.

		//	//ASSERT(((_bufferSize > 0) & (_irLength > 0)), RESULT_ERROR_BADSIZE, "AIR and input source length must be greater than 0", "AIR setup successfull");

		//	//if (setupDone) { Reset(); }

		//	//bufferSize = _bufferSize;
		//	//impulseResponseLength = _irLength;
		//	//int impulseResponseBlockLength_time = 2 * _bufferSize;
		//	//IRSubfilterLength = 2 * impulseResponseBlockLength_time;
		//	//float temp_impulseResponseNumberOfBlocks = (float)impulseResponseLength / (float)bufferSize;
		//	//IRNumberOfSubFilters = static_cast<int>(std::ceil(temp_impulseResponseNumberOfBlocks));

		//	//ambisonicIRTable.clear();
		//	//ambisonicIRPartitionedTable.clear();

		//	//ambisonicEncoder.Setup(1, Common::TAmbisonicNormalization::N3D);

		//	//dataOrigin = TDataOrigin::BRIR;
		//	////setupDone = true;				//	Indicate that the setup has been done
		//}

		/** \brief Setup input and AIR lengths
		*	\param [in] _inputSourceLength length of input buffer
		*	\param [in] _irLength length of impulse response
		*   \eh On error, an error code is reported to the error handler.
		*/

		// TODO Do we need all these parameters? Check them when we integrate the BRIR file reader.
		void BeginSetup(/*int _irLength, int _numberOfSubfilters, int _partitionedSubfilterLength, */int _ambisonicOrder, Common::TAmbisonicNormalization _ambisonicNormalization)
		{						
			//ASSERT(nVirtualSpeakers > 0, RESULT_ERROR_BADSIZE, "Attempt to setup AIR for 0 virtual speakers", "");
			//ASSERT((_irLength > 0), RESULT_ERROR_BADSIZE, "IR and input source length must be greater than 0", "AIR setup successfull");
			
			std::lock_guard<std::mutex> l(mutex);

			Reset(),
					
			//impulseResponseLength = _irLength;
			
			//IRSubfilterLength = _partitionedSubfilterLength;			
			//IRNumberOfSubFilters = _numberOfSubfilters;
			
			//ambisonicIRTable.clear();
			//ambisonicIRPartitionedTable.clear();
			
			ambisonicEncoder.Setup(_ambisonicOrder, _ambisonicNormalization);			
			virtualSpeakers.Setup(_ambisonicOrder);

			dataOrigin = TDataOrigin::HRTF;		//TODO Do we need this?
			//setupDone = true;				//	Indicate that the setup has been done
			setupInProgress = true;
		}


		bool EndSetup() {
			if (setupInProgress) {
				setupInProgress = false;
				AmbisonicBIRLoaded = true;				//	Indicate that the setup has been done
				return true;
			}			
			return false;			
		}

		/** \brief Set to initial state
		*   \eh Nothing is reported to the error handler.*/
		void Reset() {
			AmbisonicBIRLoaded = false;
			setupInProgress = false;
			//bufferSize = 0;
			impulseResponseLength = 0;
			//impulseResponseBlockLength_time = 0;
			IRSubfilterLength = 0;
			IRNumberOfSubFilters = 0;
									
			ambisonicIRTable.clear();
			ambisonicIRPartitionedTable.clear();			

			ambisonicEncoder.Reset();
			virtualSpeakers.Reset();
		}

		bool IsReady() { return AmbisonicBIRLoaded; }

		//////////////////////////////////////////////////////

		/** \brief Add impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] newData impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/		
		void AddImpulseResponse(int channel, const THRIRStruct&& newIR)
		{			
			// TODO this method has not yet been tested within BRT.

			ASSERT(setupInProgress, RESULT_ERROR_NOTSET, "Error trying to ADD  a IR to the Ambisonic IR data. The necessary setup of the class has not been carried out.", "");
			ASSERT(channel < ambisonicEncoder.GetTotalChannels(), RESULT_ERROR_OUTOFRANGE, "Trying to load AIR data for a bFormat channel of a higher order Ambisonic", "");
			ASSERT(newIR.leftHRIR.size() == impulseResponseLength, RESULT_ERROR_BADSIZE, "Size of impulse response does not agree with the one specified in the AIR setup", "");
			ASSERT(newIR.rightHRIR.size() == impulseResponseLength, RESULT_ERROR_BADSIZE, "Size of impulse response does not agree with the one specified in the AIR setup", "");

			THRIRPartitionedStruct partitionedFFTIR;
			//partitionedFFTIR = Calculate_ARIRFFT_partitioned(newIR);		// TODO this method has not been implemented yet
			AddImpulseResponse(channel, std::move(partitionedFFTIR));
		}

		/** \brief Add partitioned impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] dataFFT_Partitioned partitioned impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/
		//void AddImpulseResponse(TBFormatChannel channel, TVirtualSpeakerID speaker, const TImpulseResponse_Partitioned && dataFFT_Partitioned)
		//{
		//	ASSERT(channel < NOMORECHANNELS, RESULT_ERROR_OUTOFRANGE, "Trying to load AIR data for a bFormat channel of a higher order Ambisonic", "");

		//	// Emplace new channel data in current bFormat_Partitioned data
		//	auto it = bFormat_Partitioned.find(channel);
		//	if (it != bFormat_Partitioned.end())
		//	{
		//		TBFormatChannelData_Partitioned<TVirtualSpeakerID> existingChannelData = it->second;
		//		existingChannelData.emplace(speaker, dataFFT_Partitioned);
		//		bFormat_Partitioned.erase(channel);
		//		bFormat_Partitioned.emplace(channel, std::move(existingChannelData));
		//		//SET_RESULT(RESULT_OK, "Adding AIR Partitioned to existing ambisonics channel succesfull");
		//	}
		//	else {
		//		TBFormatChannelData_Partitioned<TVirtualSpeakerID> channelData_Partitioned;
		//		channelData_Partitioned.emplace(speaker, dataFFT_Partitioned);
		//		bFormat_Partitioned.emplace(channel, std::move(channelData_Partitioned));
		//		//SET_RESULT(RESULT_OK, "Adding first AIR partitioned to a new ambisonics channel succesfull");
		//	}
		//}

		void AddImpulseResponse(int channel, THRIRPartitionedStruct&& newPartitionedIR)
		{			
			ASSERT(setupInProgress, RESULT_ERROR_NOTSET, "Error trying to ADD  a IR to the Ambisonic IR data. The necessary setup of the class has not been carried out.", "");
			ASSERT(channel < ambisonicEncoder.GetTotalChannels(), RESULT_ERROR_OUTOFRANGE, "Attempting to load Ambisonic IR data for a channel of a higher Ambisonic order than defined.", "");			

			//// Emplace new channel data in current bFormat_Partitioned data
			auto it = ambisonicIRPartitionedTable.find(channel);
			if (it != ambisonicIRPartitionedTable.end())
			{
				SET_RESULT(RESULT_WARNING, "Error emplacing IR in ambisonicIRPartitioned Table for channel " + std::to_string(channel) +", data already exists.");				
			}
			else {
				auto returnValue = ambisonicIRPartitionedTable.emplace(channel, std::move(newPartitionedIR));
				if (!returnValue.second) {
					SET_RESULT(RESULT_WARNING, "Error emplacing IR in ambisonicIRPartitioned Table for channel " + std::to_string(channel) + ", unknown reason.");
				}
			}
		}
		


		//////////////////////////////////////////////////////

		/** \brief Get data from one BFormat channel
		*	\details In most cases, GetImpulseResponse should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained 
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method 
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse, AddImpulseResponse
		*/
		//const THRIRStruct& GetChannelIR(int channel) const
		//{
		//	// TO DO: check if data was added for this channel?

		//	auto it = ambisonicIRTable.find(channel);
		//	if (it != ambisonicIRTable.end())
		//	{				
		//		return it->second; // return a const reference so the caller sees changes.		
		//	}
		//	else
		//	{				
		//		SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Error trying to get Ambisonic IR data from a ambisonicIR Table, channel not found.");
		//		return THRIRStruct(); // returning an empty channel.
		//	}
		//}

		//////////////////////////////////////////////////////

		/** \brief Get data from one BFormat_Partitioned channel
		*	\details In most cases, GetImpulseResponse_Partitioned should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained.
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method
		*   \eh On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse_Partitioned, AddImpulseResponse
		*/
		const THRIRPartitionedStruct& GetChannelPartitionedIR(int channel) const
		{

			// TO DO: check if data was added for this channel?

			auto it = ambisonicIRPartitionedTable.find(channel);
			if (it != ambisonicIRPartitionedTable.end())
			{
				//SET_RESULT(RESULT_OK, "AIR returned correct channel data");
				return it->second; // return a const reference so the caller sees changes.		
			}
			else
			{
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Error trying to get Ambisonic IR data from a ambisonicIRPartitioned Table, channel not found.");				
				return THRIRPartitionedStruct(); 
			}
		}

		const std::vector<CMonoBuffer<float>>& GetChannelPartitionedIR_OneEar(int channel, Common::T_ear _ear)
		{			
			std::lock_guard<std::mutex> l(mutex);
			if (!AmbisonicBIRLoaded) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "Error trying to get Ambisonic IR data from a ambisonicIRPartitioned Table. The necessary setup of the class has not been carried out.");
				return std::vector<CMonoBuffer<float>>();
			}
			
			auto it = ambisonicIRPartitionedTable.find(channel);
			if (it != ambisonicIRPartitionedTable.end())
			{
				//SET_RESULT(RESULT_OK, "ABIR returned correct channel data");
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
				
		//////////////////////////////////////////////////////


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

		/** \brief Get data length of a stored subfilter impulse response in time domain (the length of one partition, which is the same for every partition)
		*	\retval dataLength Impulse response partition buffer size (time domain)
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
	/*	const int GetDataBlockLength() const {
			return impulseResponseBlockLength_time;
		}*/

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



//		/** \brief Returns true when the data is loaded and ready to be used
//		*   \eh Nothing is reported to the error handler.*/
//		bool IsInitialized() {
//#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
//			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && bFormat.size() > 0;
//#else
//			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && bFormat_Partitioned.size() > 0;
//#endif
//		}

		//////////////////////////////////////////////////////
		

		// NEW 
		//bool GetHRIRofVirtualSpeakerPositions(std::shared_ptr<BRTServices::CHRTF> _listenerHRTF, std::vector<THRIRPartitionedStruct> & virtualSpeakersData) {
		//	
		//	std::vector<orientation> virtualSpeakerPositions = GetVirtualSpeakersPositions();			

		//	//1. Get BRIR values for each channel
		//	for (int i = 0; i < virtualSpeakerPositions.size(); i++) {
		//		THRIRPartitionedStruct oneVirtualSpeakersData;
		//		
		//		oneVirtualSpeakersData							= _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);
		//		oneVirtualSpeakersData.leftHRIR_Partitioned		= _listenerHRTF->GetHRIR_partitioned(Common::T_ear::LEFT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);
		//		oneVirtualSpeakersData.rightHRIR_Partitioned	= _listenerHRTF->GetHRIR_partitioned(Common::T_ear::RIGHT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);

		//		if ((oneVirtualSpeakersData.leftHRIR_Partitioned.size != _listenerHRTF->GetHRIRNumberOfSubfilters()) || (oneVirtualSpeakersData.rightHRIR_Partitioned.size != _listenerHRTF->GetHRIRNumberOfSubfilters())) {
		//			SET_RESULT(RESULT_ERROR_BADSIZE, "The HRIR of a virtual speaker does not have an appropriate value.");
		//			return false;
		//		}		

		//		virtualSpeakersData.push_back(oneVirtualSpeakersData);
		//		
		//	}
		//	return true;			
		//}

		bool AddImpulseResponsesFromHRTF(std::shared_ptr<BRTServices::CHRTF> _listenerHRTF) {

			std::lock_guard<std::mutex> l(mutex);

			impulseResponseLength = _listenerHRTF->GetHRIRLength();	// TODO Do we need this?
			IRSubfilterLength = _listenerHRTF->GetHRIRSubfilterLength();
			IRNumberOfSubFilters = _listenerHRTF->GetHRIRNumberOfSubfilters();
									
			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsLeft;
			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsRight;
			ambisonicChannelsLeft.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));
			ambisonicChannelsRight.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));

			std::vector<orientation> virtualSpeakerPositions = virtualSpeakers.GetVirtualSpeakersPositions();

			//1. Get BRIR values for each channel
			for (int i = 0; i < virtualSpeakerPositions.size(); i++) {
				THRIRPartitionedStruct oneVirtualSpeakersData;

				//oneVirtualSpeakersData = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);
				oneVirtualSpeakersData.leftHRIR_Partitioned = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::LEFT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);
				oneVirtualSpeakersData.rightHRIR_Partitioned = _listenerHRTF->GetHRIR_partitioned(Common::T_ear::RIGHT, virtualSpeakerPositions[i].azimuth, virtualSpeakerPositions[i].elevation, true);

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
				AddImpulseResponse(i, std::move(oneAmbisonicChannel));
			}
			return true;
		}
		

	private:
		
		mutable std::mutex mutex;
		CVirtualSpeakers virtualSpeakers;		// To Store virtual speakers data
		TDataOrigin dataOrigin;					// To know the origin of the data 
		bool setupInProgress;						// To know if the setup has started
		bool AmbisonicBIRLoaded;							// To know if the setup has been done
		//int bufferSize;							// Input source length		
		int impulseResponseLength;				// Data length of one channel for one virtual speaker
		int IRSubfilterLength;					// Data length of one block of the FFT partitioned impulse response in time domain
		//int impulseResponseBlockLength_time;	// Data length of one block of the FFT partitioned impulse response in frequency domain		
		int IRNumberOfSubFilters;		// Number of blocks of the partitioned IR 
		
			
		TAmbisonicIRTable ambisonicIRTable;								// IR data (usally in time domain)
		TAmbisonicIRPartitionedTable ambisonicIRPartitionedTable;		// IR data partitioned and transformed (FFT)
		Common::CAmbisonicEncoder ambisonicEncoder;						// To do the ambisonic encoding

		Common::CGlobalParameters globalParameters;
	};
}

#endif

