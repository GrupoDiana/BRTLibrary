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
#include <ServiceModules/ServiceModuleInterfaces.hpp>
//#include <ServiceModules/HRTFDefinitions.hpp>		//TODO remove this line

/*! \file */

// Type definitions

/** \brief Type definition for Impulse response 
*/
//typedef CMonoBuffer<float> TImpulseResponse;

/** \brief Type definition for partitioned Impulse response 
*/
//typedef CMonoBuffer<TImpulseResponse> TImpulseResponse_Partitioned;

/** \brief Type definition for bFormat channel specification
*/


///** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
//*/
//template<class TVirtualSpeakerID>
//using TBFormatChannelData = std::unordered_map<TVirtualSpeakerID, TImpulseResponse>;
//
///** \brief Type definition for the Ambisonics bFormat. Contains the channel data of all ambisonics channels
//*/
//template<class TVirtualSpeakerID>
//using TBFormat = std::unordered_map<TBFormatChannel, TBFormatChannelData<TVirtualSpeakerID>>;
//
///** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
//*/
//template<class TVirtualSpeakerID>
//using TBFormatChannelData_Partitioned = std::unordered_map<TVirtualSpeakerID, TImpulseResponse_Partitioned>;
//
///** \brief Type definition for the Ambisonics bFormat. Contains the channel data of all ambisonics channels
//*/
//template<class TVirtualSpeakerID>
//using TBFormat_Partitioned = std::unordered_map<TBFormatChannel, TBFormatChannelData_Partitioned<TVirtualSpeakerID>>;


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

namespace BRTServices
{

	//enum TBFormatChannel {
	//	W = 0,				///<	Channel W
	//	X,					///<	Channel X
	//	Y,					///<	Channel Y
	//	Z,					///<	Channel Z
	//	NOMORECHANNELS
	//};

	//// Hash function to access one bformat channel
	//namespace std
	//{
	//	template<>
	//	struct hash<TBFormatChannel>
	//	{
	//		size_t operator()(const TBFormatChannel& key) const
	//		{
	//			return std::hash<int>()(static_cast<int>(key));
	//		}
	//	};
	//}
		
	/** \brief Type definition for the AmbisonicIR table
	*/
	typedef std::unordered_map<int, BRTServices::THRIRStruct> TAmbisonicIRTable;

	/** \brief Type definition for the AmbisonicIR partitioned table
	*/
	typedef std::unordered_map<int, THRIRPartitionedStruct> TAmbisonicIRPartitionedTable;


	/**
	 * @brief This is a template class for storing the impulse responses needed for processing using Virtual Ambisonics.
	 * @tparam TVirtualSpeakerID 
	 * @tparam nVirtualSpeakers 
	*/
	//template <unsigned int nVirtualSpeakers, typename TVirtualSpeakerID>		
	class CAmbisonicBIR : public CServicesBase
	{	
	public:

		/** \brief Default constructor.
		*/
		CAmbisonicBIR() : setupDone{ false }, bufferSize{ 0 }, impulseResponseLength{ 0 }, impulseResponseBlockLength_time{ 0 }, impulseResponseNumberOfBlocks{ 0 }, impulseResponseBlockLength_freq {0}
		{			
		}

		/** \brief Setup input and AIR lengths
		*	\param [in] _inputSourceLength length of input buffer
		*	\param [in] _irLength length of impulse response
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int _ambisonicOrder, Common::TAmbisonicNormalization _ambisonicNormalization, int _bufferSize, int _irLength)
		{
			//ASSERT(nVirtualSpeakers > 0, RESULT_ERROR_BADSIZE, "Attempt to setup AIR for 0 virtual speakers", "");
			ASSERT(((_bufferSize > 0) & (_irLength > 0)), RESULT_ERROR_BADSIZE, "AIR and input source length must be greater than 0", "AIR setup successfull");

			if (setupDone) { Reset(); }
			
			bufferSize = _bufferSize;
			impulseResponseLength = _irLength;
			impulseResponseBlockLength_time = 2 * _bufferSize;
			impulseResponseBlockLength_freq = 2 * impulseResponseBlockLength_time;
			float temp_impulseResponseNumberOfBlocks = (float)impulseResponseLength / (float)bufferSize;
			impulseResponseNumberOfBlocks = static_cast<int>(std::ceil(temp_impulseResponseNumberOfBlocks));
			
			ambisonicIRTable.clear();
			ambisonicIRPartitionedTable.clear();
			
			ambisonicEncoder.Setup(_ambisonicOrder, _ambisonicNormalization, _bufferSize);

			setupDone = true;				//	Indicate that the setup has been done
		}

		/** \brief Set to initial state
		*   \eh Nothing is reported to the error handler.*/
		void Reset() {
			setupDone = false;
			bufferSize = 0;
			impulseResponseLength = 0;
			impulseResponseBlockLength_time = 0;
			impulseResponseBlockLength_freq = 0;
			impulseResponseNumberOfBlocks = 0;
									
			ambisonicIRTable.clear();
			ambisonicIRPartitionedTable.clear();			

			ambisonicEncoder.Reset();
		}

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
			ASSERT(setupDone, RESULT_ERROR_NOTSET, "The necessary parameters have not been set; you must call Setup before", "");
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

		void AddImpulseResponse(int channel, const THRIRPartitionedStruct&& newPartitionedIR)
		{
			ASSERT(setupDone, RESULT_ERROR_NOTSET, "The necessary parameters have not been set; you must call Setup before", "");
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
		const THRIRStruct& GetChannelIR(int channel) const
		{
			// TO DO: check if data was added for this channel?

			auto it = ambisonicIRTable.find(channel);
			if (it != ambisonicIRTable.end())
			{				
				return it->second; // return a const reference so the caller sees changes.		
			}
			else
			{				
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Error trying to get Ambisonic IR data from a ambisonicIR Table, channel not found.");
				return THRIRStruct(); // returning an empty channel.
			}
		}

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
				return THRIRPartitionedStruct(); // returning an empty data
			}
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
		const int GetDataBlockLength() const {
			return impulseResponseBlockLength_time;
		}

		/** \brief Get data length of stored impulse responses in frequency domain (the length of one partition, which is the same for every partition)
		*	\retval dataLength Impulse response FFT partition buffer size (frequency domain)
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataBlockLength_freq() const {
			return impulseResponseBlockLength_freq;
		}

		/** \brief Get number of sub-filters (blocks) fo the AIR partition
		*	\retval dataLength Number of sub filters
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataNumberOfBlocks() const {
			return impulseResponseNumberOfBlocks;
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

		bool CalculateImpulseResponsesFromHRTF(std::shared_ptr<BRTServices::CHRTF> _listenerHRTF) {

			//std::vector<THRIRPartitionedStruct> virtualSpeakersAmbisonicChannels;
									
			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsLeft;
			std::vector<std::vector< CMonoBuffer<float>>> ambisonicChannelsRight;
			ambisonicChannelsLeft.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));
			ambisonicChannelsRight.resize(ambisonicEncoder.GetTotalChannels(), std::vector<CMonoBuffer<float>>(_listenerHRTF->GetHRIRNumberOfSubfilters(), CMonoBuffer<float>(_listenerHRTF->GetHRIRSubfilterLength(), 0)));

			//virtualSpeakersAmbisonicChannels.resize(ambisonicEncoder.GetTotalChannels(), THRIRPartitionedStruct());
												
			std::vector<orientation> virtualSpeakerPositions = GetVirtualSpeakersPositions(ambisonicEncoder.GetOrder());

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

		
		std::vector< orientation> GetVirtualSpeakersPositions(int ambisonicOrder) {
			
			std::vector< orientation> virtualSpeakerOrientationList;

			if (ambisonicOrder == 1) { 
				for (int i = 0; i < GetTotalVirtualSpeakers(ambisonicOrder); i++) {
					virtualSpeakerOrientationList.push_back(orientation(ambisonicAzimut_order1[i], ambisonicElevation_order1[i]));
				}							
			}
			else if (ambisonicOrder == 2) { 
				for (int i = 0; i < GetTotalVirtualSpeakers(ambisonicOrder); i++) {
					virtualSpeakerOrientationList.push_back(orientation(ambisonicAzimut_order2[i], ambisonicElevation_order2[i]));
				}
			}
			else { 
				for (int i = 0; i < GetTotalVirtualSpeakers(ambisonicOrder); i++) {
					virtualSpeakerOrientationList.push_back(orientation(ambisonicAzimut_order3[i], ambisonicElevation_order3[i]));
				}
			}
			return virtualSpeakerOrientationList;
		}

		std::vector<float> GetAmbisonicAzimutsList(int ambisonicOrder) {
			if (ambisonicOrder == 1) { return ambisonicAzimut_order1; }
			else if (ambisonicOrder == 2) { return ambisonicAzimut_order2; }
			else { return ambisonicAzimut_order3; }
		}

		std::vector<float> GetAmbisonicElevationsList(int ambisonicOrder)
		{
			if (ambisonicOrder == 1) { return ambisonicElevation_order1; }
			else if (ambisonicOrder == 2) { return ambisonicElevation_order2; }
			else { return ambisonicElevation_order3; }
		}

		int GetTotalVirtualSpeakers(int ambisonicOrder)
		{
			if (ambisonicOrder == 1) { return 6; }
			else if (ambisonicOrder == 2) { return 12; }
			else { return 20; }
		}

	private:
		bool setupDone;							//To know if the setup has been done
		int impulseResponseLength;				// Data length of one channel for one virtual speaker
		int impulseResponseBlockLength_freq;	// Data length of one block of the FFT partitioned impulse response in time domain
		int impulseResponseBlockLength_time;	// Data length of one block of the FFT partitioned impulse response in frequency domain
		int bufferSize;							// Input source length		
		int impulseResponseNumberOfBlocks;		// Number of blocks of the partitioned IR 

			
		TAmbisonicIRTable ambisonicIRTable;								// IR data (usally in time domain)
		TAmbisonicIRPartitionedTable ambisonicIRPartitionedTable;		// IR data partitioned and transformed (FFT)

		//TBFormat<TVirtualSpeakerID> bFormat;							// The actual data
		//TBFormat_Partitioned<TVirtualSpeakerID> bFormat_Partitioned;	// The actual data partitioned and transformed (FFT)

		//TBFormatChannelData<TVirtualSpeakerID> emptyChannelData;
		//TImpulseResponse emptyImpulseResponse;
		//TBFormatChannelData_Partitioned<TVirtualSpeakerID> emptyChannelData_Partitioned;
		//TImpulseResponse_Partitioned emptyImpulseResponse_Partitioned;


		Common::CAmbisonicEncoder ambisonicEncoder;		

		// TODO move to extern struct
		/** \brief Type definition for position of virtual speakers in HRIR */
		const std::vector<float> ambisonicAzimut_order1 = { 90, 270,  0,  0,  0, 180 };
		const std::vector<float> ambisonicElevation_order1 = { 0,   0,  90, 270, 0,  0 };

		const std::vector<float> ambisonicAzimut_order2 = { 328.28, 31.72, 148.28, 211.72,      270,       90,     270,      90,      180,       0,      180,       0 };
		const std::vector<float> ambisonicElevation_order2 = { 0,       0,        0,        0, 328.28, 328.28, 31.72, 31.72, 301.72, 301.72, 58.28, 58.28 };

		const std::vector<float> ambisonicAzimut_order3 = { 290.91, 69.1, 249.1, 110.91,     315,      45,     225,      135,      315,       45,      225,      135,        0,      180,       0,     180,     270,      90,     270,        90 };
		const std::vector<float> ambisonicElevation_order3 = { 0,       0,        0,        0, 35.26, 35.26, 35.26,  35.26, 324.74, 324.74, 324.74, 324.74, 339.1, 339.1, 20.91, 20.91, 69.1, 69.1, 290.91, 290.91 };

		//METHODS

		//THRIRPartitionedStruct Calculate_ARIRFFT_partitioned(const THRIRStruct& newData_time)
		//{
		//	int blockSize = bufferSize;
		//	int numberOfBlocks = GetDataNumberOfBlocks();

		//	THRIRPartitionedStruct new_DataFFT_Partitioned;
		//	new_DataFFT_Partitioned.reserve(numberOfBlocks);
		//	//Index to go throught the AIR values in time domain
		//	int index;
		//	for (int i = 0; i < newData_time.size(); i = i + blockSize)
		//	{
		//		CMonoBuffer<float> data_FFT_doubleSize;
		//		//Resize with double size and zeros to make the zero-padded demanded by the algorithm
		//		data_FFT_doubleSize.resize(blockSize * 2);
		//		//Fill each AIR block
		//		for (int j = 0; j < blockSize; j++) {
		//			index = i + j;
		//			if (index < newData_time.size()) {
		//				data_FFT_doubleSize[j] = newData_time[index];
		//			}
		//		}
		//		//FFT
		//		CMonoBuffer<float> data_FFT;
		//		Common::CFprocessor::CalculateFFT(data_FFT_doubleSize, data_FFT);
		//		//Prepare struct to return the value
		//		new_DataFFT_Partitioned.push_back(data_FFT);
		//	}
		//	return new_DataFFT_Partitioned;
		//}




	};
}

//////////////////////////////////////////////////////
// INSTANCES: 
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// ABIR //////////////////////////////////////////////

//namespace std
//{
//	template<>
//	struct hash<Common::T_ear>
//	{
//		size_t operator()(const Common::T_ear & key) const
//		{
//			return std::hash<int>()(static_cast<int>(key));
//		}
//	};
//}

//namespace BRTServices
//{
//	/**
//	 * @brief Class storing Ambisonics Binaural Impulse Response data, for Binaural spatialization
//	 * @details This is an instance of CAmbisonicIR template
//	*/
//	//using CAmbisonicBIR = BRTServices::CAmbisonicIR<2, Common::T_ear>;
//	using CAmbisonicBIR = BRTServices::CAmbisonicIR<2>;
//
//	/**
//	 * @brief Instance of TBFormatChannelData for use with ABIR
//	*/
//	//using TAmbisonicBIRChannelData = TBFormatChannelData<Common::T_ear>;
//}



//////////////////////////////////////////////////////
// ARIR //////////////////////////////////////////////

///** \brief ID of binaural virtual speakers, for use with ARIR
//*/
//struct TLoudspeakersSpeakerID
//{
//	int azimuth;		///< Azimuth position of virtual speaker
//	int elevation;		///< Elevation position of virtual speaker
//	bool operator==(const TLoudspeakersSpeakerID& rhs) const
//	{
//		return ((azimuth == rhs.azimuth) && (elevation == rhs.elevation));
//	}	
//};
//
//namespace std
//{
//	template<>
//	struct hash<TLoudspeakersSpeakerID>
//	{
//		size_t operator()(const TLoudspeakersSpeakerID & key) const
//		{
//			//return std::hash<int>()(static_cast<int>(key.azimuth*1000 + key.elevation));
//			return std::hash<int>()(static_cast<int>(key.azimuth)) ^ std::hash<int>()(static_cast<int>(key.elevation));
//		}
//	};
//}

///** \brief
//* Class storing Ambisonics Room Impulse Response data, for Loudspeakers spatialization reverb
//*	\details This is an instance of CAIR template
//*/
//using CAmbisonicRIR = BRTServices::CAmbisonicIR<8, TLoudspeakersSpeakerID>;
//
///** \brief
//* Instance of TBFormatChannelData for use with ARIR
//*/
//using TAmbisonicRIRChannelData = TBFormatChannelData<TLoudspeakersSpeakerID>;

#endif

