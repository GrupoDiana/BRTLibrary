/**
* \class CSOFAReader
*
* \brief Declaration of CSOFAReader class
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
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _SOFA_READER_
#define _SOFA_READER_

#include <ostream>
#include <string>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/HRTF.hpp>
#include <Common/ErrorHandler.hpp>
#include <Readers/LibMySofaLoader.hpp>
//#include "ofxlibMySofa.h"
#include <third_party_libraries/libmysofa/include/mysofa.h>

namespace BRTReaders {

	class CSOFAReader {				

	public:
		
		/** \brief Returns the sample rate in Hz in the sofa file
		*	\param [in] path of the sofa file
		*   \eh On error, an error code is reported to the error handler.
		*/
		int GetSampleRateFromSofa(const std::string& sofafile)
		{
			BRTReaders::CLibMySOFALoader loader(sofafile);
			int error = loader.getError();
			if (error != 0) return -1;

			return loader.GetSamplingRate();
		}

		/** \brief Loads an HRTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadHRTFFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CHRTF> listenerHRTF, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
						
			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR, _resamplingStep, _extrapolationMethod);
		}
		
		/*bool ReadHRTFFromSofaWithoutProcess(const std::string& sofafile, std::shared_ptr<BRTServices::CHRTF> listenerHRTF, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {

			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR, _resamplingStep, _extrapolationMethod, false);
		}*/

		/** \brief Loads an ILD from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadNFCFiltersFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CNearFieldCompensationFilters>& listenerILD)
		{
			std::shared_ptr<BRTServices::CServicesBase> data = listenerILD;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS, -1, BRTServices::TEXTRAPOLATION_METHOD::none);
		}

		/** \brief Loads an DirectivityTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] source affected by the directivityTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadDirectivityTFFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CDirectivityTF> sourceDirectivityTF, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {

			std::shared_ptr<BRTServices::CServicesBase> data = sourceDirectivityTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF, _resamplingStep, _extrapolationMethod);
		}
				

		/** \brief Loads an HRTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadBRIRFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CHRBRIR> listenerHRTF) {

			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SingleRoomMIMOSRIR, -1, BRTServices::TEXTRAPOLATION_METHOD::none);
		}
		
	private:
				
		// Methods
		bool ReadFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, CLibMySOFALoader::TSofaConvention _SOFAConvention, 
			int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod, bool process = true) {

			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) return false;
			
			// Check convention
			if (!loader.CheckSofaConvention(_SOFAConvention)) return false;			
			SET_RESULT(RESULT_OK, "Open a valid SOFA file");

			// Load convention data
			bool result;
			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR) { 
				return ReadFromSofa_SimpleFreeFieldHRIR(loader, sofafile, data, _resamplingStep, _extrapolationMethod);
			} else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) { 
				return ReadFromSofa_SimpleFreeFieldHRSOS(loader, sofafile, data);
			} else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF) { 
				return  ReadFromSofa_FreeFieldDirectivityTF(loader, sofafile, data, _resamplingStep, _extrapolationMethod);
			} else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SingleRoomMIMOSRIR) { 
				return ReadFromSofa_SingleRoomMIMOSRIR(loader, sofafile, data, _resamplingStep);
			}
			else { 
				SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "SOFA Convention loader not implemented"); 
				return false; 
			}			
		}
		
		/////////////////////////////////////////////////////////////////
		/////////////////	SimpleFreeFieldHRIR		/////////////////
		/////////////////////////////////////////////////////////////////
		bool ReadFromSofa_SimpleFreeFieldHRIR(BRTReaders::CLibMySOFALoader& loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR, sofafile, data);			
			CheckListenerOrientation(loader);					// Check listener view			
			GetAndSaveReceiverPosition(loader, data);			// Get and Save listener ear 
			bool result;
			result = GetHRIRs(loader, data, _extrapolationMethod);						
			if (!result) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "An error occurred creating the data structure from the SOFA file, please consider previous messages.");
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetResamplingStep(_resamplingStep); }
			data->EndSetup();
			return true;
		}

		bool GetHRIRs(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataHRTF, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			//Get source positions									
			std::vector< double > sourcePositionsVector(loader.getHRTF()->SourcePosition.values, loader.getHRTF()->SourcePosition.values + loader.getHRTF()->SourcePosition.elements);
			// GET delays and IRs
			std::vector< double > dataDelays(loader.getHRTF()->DataDelay.values, loader.getHRTF()->DataDelay.values + loader.getHRTF()->DataDelay.elements);
			std::vector< double > dataMeasurements(loader.getHRTF()->DataIR.values, loader.getHRTF()->DataIR.values + loader.getHRTF()->DataIR.elements);
			// Constants
			int numberOfMeasurements = loader.getHRTF()->M;
			int numberOfCoordinates = loader.getHRTF()->C;
			const unsigned int numberOfSamples = loader.getHRTF()->N;
			
			// Check number of delays			
			bool specifiedDelays = true;
			if (dataDelays.size() == dataMeasurements.size() / loader.getHRTF()->N) {
				specifiedDelays = true;
			}
			else {
				if (dataDelays.size() == 2)
				{
					specifiedDelays = false;
					SET_RESULT(RESULT_WARNING, "This HRTF file does not contain individual delays for each HRIR. Therefore, some comb filter effect can be perceived due to interpolations and custom head radius should not be used");
				}
				else {
					SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of HRIRs and delays");
					return false;
				}
			}

			// Get first distance to check if all HRIRs are measured at the same distance
			double measurementDistance = sourcePositionsVector[array2DIndex(0, 2, numberOfMeasurements, numberOfCoordinates)];		//We consider that every HRIR are meased at the same distance, so we get the firts one												
			if (measurementDistance <= 0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of HRIRs distance");
				return false;
			}				
			// Get and save HRIRs	
			dataHRTF->BeginSetup(numberOfSamples, _extrapolationMethod);			
			dataHRTF->SetSamplingRate(loader.GetSamplingRate());
			const int left_ear = 0;
			const int right_ear = 1;
						
			for (std::size_t measure = 0; measure < numberOfMeasurements; measure++)
			{
				BRTServices::THRIRStruct hrir_value;
								
				double azimuth, elevation, distance;
				GetSourcePosition(loader, sourcePositionsVector, measure, azimuth, elevation, distance);				
				if (distance <= 0 || distance != measurementDistance) {
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of HRIRs distance");
					return false;
				}

				double leftDelay = dataDelays[specifiedDelays ? array2DIndex(measure, left_ear, numberOfMeasurements, 2) : 0];
				double rightDelay = dataDelays[specifiedDelays ? array2DIndex(measure, right_ear, numberOfMeasurements, 2) : 1];
				if (leftDelay < 0 || rightDelay < 0) {
					SET_RESULT(RESULT_WARNING, "Error reading HRIR from SOFA file, one of the delay fields is negative. This data is rejected. [" + std::to_string(azimuth) + ", " + std::to_string(elevation) + "]");
					continue;
				}
				hrir_value.leftDelay = leftDelay;
				hrir_value.rightDelay = rightDelay;

				/*GetData(dataMeasurements, hrir_value.leftHRIR, numberOfMeasurements, 2,  numberOfSamples, left_ear, i);
				GetData(dataMeasurements, hrir_value.rightHRIR, numberOfMeasurements, 2, numberOfSamples, right_ear, i);*/
				Get3DMatrixData(dataMeasurements, hrir_value.leftHRIR, 2, numberOfSamples, left_ear, measure);
				Get3DMatrixData(dataMeasurements, hrir_value.rightHRIR, 2, numberOfSamples, right_ear, measure);
				
				dataHRTF->AddHRIR(azimuth, elevation, distance, std::move(hrir_value));
			}
			return true;

		}
		
		/////////////////////////////////////////////////////////////////
		/////////////////	SimpleFreeFieldHRSOS		/////////////////
		/////////////////////////////////////////////////////////////////
		bool ReadFromSofa_SimpleFreeFieldHRSOS(BRTReaders::CLibMySOFALoader& loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data) {
			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS, sofafile, data); // GET and Save Global Attributes 			
			CheckListenerOrientation(loader);									// Check listener view						
			GetAndSaveReceiverPosition(loader, data);							// Get and Save listener ear 
			
			bool result;			
			result = GetHRSOSCoefficients(loader, data);			
			if (!result) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "An error occurred creating the data structure from the SOFA file, please consider previous messages.");
				return false;
			}
			// Finish setup			
			data->EndSetup();
			return true;
		}

		bool GetHRSOSCoefficients(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& data) {
			//Get source positions									
			std::vector< double > sourcePositionsVector(loader.getHRTF()->SourcePosition.values, loader.getHRTF()->SourcePosition.values + loader.getHRTF()->SourcePosition.elements);

			// GET delays and IRs
			std::vector< double > dataDelays(loader.getHRTF()->DataDelay.values, loader.getHRTF()->DataDelay.values + loader.getHRTF()->DataDelay.elements);
			std::vector< double > dataMeasurements(loader.GetDataSOS()->values, loader.GetDataSOS()->values + loader.GetDataSOS()->elements);

			// Check number of receivers	
			int numberOfReceivers = loader.getHRTF()->R;
			if (numberOfReceivers == 1) {
				SET_RESULT(RESULT_WARNING, "This ILD SOFA file does not contain coefficients for each ear. Therefore, the same filters will be used for both ears.");
			}
			else if (numberOfReceivers == 2) {
				SET_RESULT(RESULT_OK, "This ILD SOFA file contains coefficients for both ears.");
			}
			else {
				SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of receivers and coefficients");
				return false;
			}

			// Get and save Coefficients
			int numberOfMeasurements = loader.getHRTF()->M;
			int numberOfCoordinates = loader.getHRTF()->C;
			const unsigned int numberOfSamples = loader.getHRTF()->N;		//number of coefficients

			data->BeginSetup();
			data->SetNumberOfEars(numberOfReceivers);
			const int left_ear = 0;
			const int right_ear = 1;
			// This outtermost loop iterates over HRIRs
			for (std::size_t measure = 0; measure < numberOfMeasurements; measure++)
			{
				BRTServices::TNFCFilterStruct coefficients;
				coefficients.leftCoefs.resize(numberOfSamples);
				
				double azimuth, elevation, distance;
				GetSourcePosition(loader, sourcePositionsVector, measure, azimuth, elevation, distance);
								
				Get3DMatrixData(dataMeasurements, coefficients.leftCoefs, numberOfReceivers, numberOfSamples, left_ear, measure);
				if (numberOfReceivers > 1) {
					coefficients.rightCoefs.resize(numberOfSamples);
					//GetData(dataMeasurements, coefficients.rightCoefs, numberOfMeasurements, numberOfReceivers, numberOfSamples, right_ear, i);
					Get3DMatrixData(dataMeasurements, coefficients.rightCoefs, numberOfReceivers, numberOfSamples, right_ear, measure);
				}

				data->AddCoefficients(azimuth, distance, std::move(coefficients));
			}
			return true;

		}
		/////////////////////////////////////////////////////////////////
		/////////////////	FreeFieldDirectivityTF		/////////////////
		/////////////////////////////////////////////////////////////////
		bool ReadFromSofa_FreeFieldDirectivityTF(BRTReaders::CLibMySOFALoader& loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			// Get and Save data			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF, sofafile, data); // GET and Save Global Attributes 
			//CheckCoordinateSystems(loader, _SOFAConvention);					// Check coordiante system for Source and Receiver positions
			CheckListenerOrientation(loader);									// Check listener view			
			
			bool result;			
			result = GetDirectivityTF(loader, data, _extrapolationMethod);						
			if (!result) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "An error occurred creating the data structure from the SOFA file, please consider previous messages.");
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetResamplingStep(_resamplingStep); }
			data->EndSetup();
			return true;
		}

		bool GetDirectivityTF(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataDirectivityTF, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			//Get source positions									
			std::vector< double > receiverPositionsVector(loader.getHRTF()->ReceiverPosition.values, loader.getHRTF()->ReceiverPosition.values + loader.getHRTF()->ReceiverPosition.elements);

			// GET Directivity Transfer Functions (Real dn Imag parts)
			std::vector< double > dataMeasurementsRealPart(loader.GetDataRealDirectivity()->values, loader.GetDataRealDirectivity()->values + loader.GetDataRealDirectivity()->elements);
			std::vector< double > dataMeasurementsImagPart(loader.GetDataImagDirectivity()->values, loader.GetDataImagDirectivity()->values + loader.GetDataImagDirectivity()->elements);

			// Constants
			int numberOfMeasurements = loader.getHRTF()->M;
			if (numberOfMeasurements != 1) {
				SET_RESULT(RESULT_WARNING, "Number of measurements (M) in SOFA file is different from 1 but only the first measurements are going to be taken into account.");
				numberOfMeasurements = 1;
			}
			
			const unsigned int numberOfFrequencySamples = loader.getHRTF()->N;
			int numberOfReceivers = loader.getHRTF()->R;

			// Get and save TFs			
			dataDirectivityTF->BeginSetup(numberOfFrequencySamples, _extrapolationMethod);

			// This outtermost loop iterates over TFs
			for (std::size_t receiver = 0; receiver < numberOfReceivers; receiver++)
			{
				BRTServices::TDirectivityTFStruct directivityTF_data;
				CMonoBuffer<float> dataRealPartPI;
				CMonoBuffer <float> dataImagPartPI;
				double azimuth, elevation, distance;
				GetReceiverPosition(loader, receiverPositionsVector, receiver, azimuth, elevation, distance);				
								
				GetDirectivityData(dataMeasurementsRealPart, dataRealPartPI, numberOfFrequencySamples, receiver);
				GetDirectivityData(dataMeasurementsImagPart, dataImagPartPI, numberOfFrequencySamples, receiver);

				directivityTF_data.realPart = dataRealPartPI;
				directivityTF_data.imagPart = dataImagPartPI;

				dataDirectivityTF->AddDirectivityTF(azimuth, elevation, std::move(directivityTF_data));
			}
			return true;
		}

		/////////////////	SingleRoomMIMOSRIR		/////////////////
		bool ReadFromSofa_SingleRoomMIMOSRIR(BRTReaders::CLibMySOFALoader &loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, int _resamplingStep) {
			
			// Get and Save data			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::SingleRoomMIMOSRIR ,sofafile, data);			// GET and Save Global Attributes			
			GetAndSaveReceiverPosition(loader, data); // Get and Save listener ear 
			
			bool result;
			result = GetBRIRs(loader, data);

			if (!result) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "An error occurred creating the data structure from the SOFA file, please consider previous messages.");
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetResamplingStep(_resamplingStep); }
			data->EndSetup();
			return true;

		}
		
		bool GetBRIRs(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataHRBRIR) {
			//Get source positions									
			std::vector< double > sourcePositionsVector(loader.getHRTF()->SourcePosition.values, loader.getHRTF()->SourcePosition.values + loader.getHRTF()->SourcePosition.elements);
			std::vector< double > emitterPositionsVector(loader.getHRTF()->EmitterPosition.values, loader.getHRTF()->EmitterPosition.values + loader.getHRTF()->EmitterPosition.elements);

			// GET delays and IRs
			std::vector< double > dataDelays(loader.getHRTF()->DataDelay.values, loader.getHRTF()->DataDelay.values + loader.getHRTF()->DataDelay.elements);
			std::vector< double > dataMeasurements(loader.getHRTF()->DataIR.values, loader.getHRTF()->DataIR.values + loader.getHRTF()->DataIR.elements);

			// Constants
			int numberOfMeasurements = loader.getHRTF()->M;
			int numberOfReceivers = loader.getHRTF()->R;
			const unsigned int numberOfSamples = loader.getHRTF()->N;
			int numberOfEmitters = loader.getHRTF()->E;

			int numberOfCoordinates = loader.getHRTF()->C;

			// Check number of delays			
			bool specifiedDelays = true;
			if (dataDelays.size() == dataMeasurements.size() / loader.getHRTF()->N) {
				specifiedDelays = true;
			}
			else {
				if (dataDelays.size() == 2)
				{
					specifiedDelays = false;
					SET_RESULT(RESULT_WARNING, "This HRTF file does not contain individual delays for each HRIR. Therefore, some comb filter effect can be perceived due to interpolations and custom head radius should not be used");
				}
				else {
					SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of HRIRs and delays");
					return false;
				}
			}

			// Get and save HRIRs
			double distance = sourcePositionsVector[array2DIndex(0, 2, numberOfMeasurements, numberOfCoordinates)];		//We consider that every HRIR are meased at the same distance, so we get the firts one									

			/*if (distance <= 0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of HRIRs distance");
				return false;
			}*/

			dataHRBRIR->BeginSetup(numberOfSamples);

			dataHRBRIR->SetSamplingRate(loader.GetSamplingRate());
			const int left_ear = 0;
			const int right_ear = 1;

			for (std::size_t emitter = 0; emitter < numberOfEmitters; emitter++)
			{
				// This outtermost loop iterates over HRIRs
				for (std::size_t measure = 0; measure < numberOfMeasurements; measure++)
				{
					BRTServices::THRIRStruct hrir_value;
					double azimuth = sourcePositionsVector[array2DIndex(measure, 0, numberOfMeasurements, numberOfCoordinates)];
					double elevation = sourcePositionsVector[array2DIndex(measure, 1, numberOfMeasurements, numberOfCoordinates)];

					double leftDelay = dataDelays[specifiedDelays ? array2DIndex(measure, left_ear, numberOfMeasurements, 2) : 0];
					double rightDelay = dataDelays[specifiedDelays ? array2DIndex(measure, right_ear, numberOfMeasurements, 2) : 1];
					if (leftDelay < 0 || rightDelay < 0) {
						SET_RESULT(RESULT_WARNING, "Error reading IR from SOFA file, one of the delay fields is negative. This data is rejected. [" + std::to_string(azimuth) + ", " + std::to_string(elevation) + "]");
						continue;
					}
					hrir_value.leftDelay = leftDelay;
					hrir_value.rightDelay = rightDelay;

					Get4DMatrixData(dataMeasurements, hrir_value.leftHRIR, numberOfReceivers, numberOfSamples, numberOfEmitters, measure, left_ear, emitter);
					Get4DMatrixData(dataMeasurements, hrir_value.rightHRIR, numberOfReceivers, numberOfSamples, numberOfEmitters, measure, right_ear, emitter);

					//dataHRBRIR->AddHRIR(azimuth, elevation, std::move(hrir_value));
				}
			}
			return true;

		}
				
		/////////////////////////
		// METHODS
		/////////////////////////
		
		/**
		 * @brief Extract source position from SOFA struct. If in Cartesian coordinates, convert to spherical.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param _sourcePositionsVector Vector with all positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure Measure to be recovered
		 * @param azimuth Output parameter containing the azimuth.
		 * @param elevation Output parameter containing the elevation.
		 * @param distance Output parameter containing the distance.
		 */
		void GetSourcePosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _sourcePositionsVector, std::size_t measure, double& azimuth, double& elevation, double& distance) {

			int numberOfMeasurements = loader.getHRTF()->M;
			int numberOfCoordinates = loader.getHRTF()->C;

			if (IsSourcePositionCoordinateSystemsSpherical(loader)) {
				azimuth = _sourcePositionsVector[array2DIndex(measure, 0, numberOfMeasurements, numberOfCoordinates)];
				elevation = _sourcePositionsVector[array2DIndex(measure, 1, numberOfMeasurements, numberOfCoordinates)];
				distance = _sourcePositionsVector[array2DIndex(measure, 2, numberOfMeasurements, numberOfCoordinates)];
			}
			else {
				double x = _sourcePositionsVector[array2DIndex(measure, 0, numberOfMeasurements, numberOfCoordinates)];
				double y = _sourcePositionsVector[array2DIndex(measure, 1, numberOfMeasurements, numberOfCoordinates)];
				double z = _sourcePositionsVector[array2DIndex(measure, 2, numberOfMeasurements, numberOfCoordinates)];
				ToSpherical(x, y, z, azimuth, elevation, distance);
			}
		}

		/**
		 * @brief Extract receiver position from SOFA struct. If in Cartesian coordinates, convert to spherical.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param receiverPositionsVector Vector with all positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure  Measure to be recovered
		 * @param azimuth  Output parameter containing the azimuth.
		 * @param elevation Output parameter containing the elevation.
		 * @param distance Output parameter containing the distance.		 
		 */
		void GetReceiverPosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& receiverPositionsVector, std::size_t measure, double& azimuth, double& elevation, double& distance) {			
			int numberOfCoordinates = loader.getHRTF()->C;

			if (IsReceiverPositionCoordinateSystemsSpherical(loader)) {				
				azimuth		= receiverPositionsVector[array2DIndex(measure, 0, 0, numberOfCoordinates)];
				elevation	= receiverPositionsVector[array2DIndex(measure, 1, 0, numberOfCoordinates)];
				distance	= receiverPositionsVector[array2DIndex(measure, 2, 0, numberOfCoordinates)];

			}
			else {
				double x = receiverPositionsVector[array2DIndex(measure, 0, 0, numberOfCoordinates)];
				double y = receiverPositionsVector[array2DIndex(measure, 1, 0, numberOfCoordinates)];
				double z = receiverPositionsVector[array2DIndex(measure, 2, 0, numberOfCoordinates)];
				ToSpherical(x, y, z, azimuth, elevation, distance);
			}
		}

		// Read GLOBAL data from sofa struct and save into HRTF class
		void GetAndSaveGlobalAttributes(BRTReaders::CLibMySOFALoader& loader, CLibMySOFALoader::TSofaConvention _SOFAConvention, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& dataHRTF) {
			// GET and Save Global Attributes 			
			std::string _title = mysofa_getAttribute(loader.getHRTF()->attributes, "Title");
			dataHRTF->SetTitle(_title);

			std::string _databaseName = mysofa_getAttribute(loader.getHRTF()->attributes, "DatabaseName");
			dataHRTF->SetDatabaseName(_databaseName);

			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR || 
				_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS ||
				_SOFAConvention == CLibMySOFALoader::TSofaConvention::SingleRoomMIMOSRIR) {
				std::string _listenerShortName = mysofa_getAttribute(loader.getHRTF()->attributes, "ListenerShortName");
				dataHRTF->SetListenerShortName(_listenerShortName);
			}

			std::filesystem::path p(sofafile);
			std::string fileName{ p.filename().u8string() };
			dataHRTF->SetFilename(fileName);

		}			

		/*void CheckCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, CLibMySOFALoader::TSofaConvention _SOFAConvention) {
			
			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR || _SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) {
				CheckSourcePositionCoordinateSystems(loader, "spherical");
				CheckReceiverPositionCoordinateSystems(loader, "cartesian");
			}
			else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF) {
				CheckSourcePositionCoordinateSystems(loader, "cartesian");
				CheckReceiverPositionCoordinateSystems(loader, "spherical");
			}
			else{ SET_RESULT(RESULT_WARNING, "CoordinateSystem could not be checked"); }	
		}*/

		bool IsSourcePositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourcePositionCoordinateSystemsNew(loader, "spherical");
		}

		bool IsSourcePositionCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourcePositionCoordinateSystemsNew(loader, "cartesian");
		}

		bool CheckSourcePositionCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {			
			if (mysofa_getAttribute(loader.getHRTF()->SourcePosition.attributes, "Type") == _coordinateSystem) { return true; }
			return false;
		}
		
		bool IsReceiverPositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckReceiverPositionCoordinateSystemsNew(loader, "spherical");
		}

		bool CheckReceiverPositionCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {
			if (mysofa_getAttribute(loader.getHRTF()->ReceiverPosition.attributes, "Type") == _coordinateSystem) { return true; }
			return false;
		}

		//void CheckSourcePositionCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {

		//	std::string sourcePostionsCoordianteSystem = mysofa_getAttribute(loader.getHRTF()->SourcePosition.attributes, "Type");
		//	if (sourcePostionsCoordianteSystem != _coordinateSystem) {
		//		//loader.Cartesian2Spherical();
		//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Source positions from SOFA file do not have the expected coordinate system");
		//	};
		//}

		//void CheckReceiverPositionCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {

		//	std::string receiverPostionsCoordianteSystem = mysofa_getAttribute(loader.getHRTF()->ReceiverPosition.attributes, "Type");
		//	if (receiverPostionsCoordianteSystem != _coordinateSystem) {
		//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "positions from SOFA file do not have the expected coordinate system");
		//	};
		//}

		// Check listener position in the SOFA file
		void CheckListenerOrientation(BRTReaders::CLibMySOFALoader& loader) {			
			//Check listener view			
			Common::CVector3 _listenerView = loader.GetListenerView();
			Common::CVector3 _forward(1, 0, 0);
			if (_listenerView != _forward) { SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener View in SOFA file different from [1,0,0]"); }

			//Check listener up			
			Common::CVector3 _listenerUp = loader.GetListenerUp();
			Common::CVector3 _up(0, 0, 1);
			if (_listenerUp != _up) { SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener Up in SOFA file different from [0,0,1]"); }			
		}
						
		/**
		 * @brief Get and save listener ear positions
		 * @param loader The complete data structure read from the SOFA file.
		 * @param data To save the data
		 */
		void GetAndSaveReceiverPosition(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& data) {									
			int numberOfReceivers = loader.getHRTF()->R;
			data->SetNumberOfEars(numberOfReceivers);			
			
			//Get and Save listener ear positions
			std::vector< double > receiverPosition = loader.GetReceiverPosition();
			
			// Check coordinate system
			std::string receiverPostionsCoordianteSystem = mysofa_getAttribute(loader.getHRTF()->ReceiverPosition.attributes, "Type");
			if (receiverPostionsCoordianteSystem == "spherical") {				
				std::vector< double > receiverPositionSpherical(receiverPosition);
				ToCartesian(receiverPositionSpherical[0], receiverPositionSpherical[1], receiverPositionSpherical[2], receiverPosition[0], receiverPosition[1], receiverPosition[2]);
			} else if (receiverPostionsCoordianteSystem != "cartesian") {
				SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Receiver positions from SOFA file do not have the expected coordinate system");
			}

			if (numberOfReceivers >= 1) {
				Common::CVector3 leftEarPos;
				leftEarPos.SetAxis(FORWARD_AXIS, receiverPosition[0]);
				leftEarPos.SetAxis(RIGHT_AXIS, receiverPosition[1]);
				leftEarPos.SetAxis(UP_AXIS, receiverPosition[2]);
				data->SetEarPosition(Common::T_ear::LEFT, leftEarPos);

				if (numberOfReceivers >= 2) {
					Common::CVector3 rightEarPos;
					rightEarPos.SetAxis(FORWARD_AXIS, receiverPosition[3]);
					rightEarPos.SetAxis(RIGHT_AXIS, receiverPosition[4]);
					rightEarPos.SetAxis(UP_AXIS, receiverPosition[5]);
					data->SetEarPosition(Common::T_ear::RIGHT, rightEarPos);
				}
			}
		}

		
		/////////////////////////
		// AUXILAR METHODS
		///////////////////////////


		

		/**
		 * @brief Obtain the requested data from a vector representing a 3D matrix.
		 * @param dataIR Input interlaced matrix
		 * @param outIR Output vector
		 * @param numberOfReceivers Total number of receivers (ears) (R)
		 * @param numberOfSamples Total number of samples per IR (N)
		 * @param measure Measure to be extracted
		 * @param receiver Ear to be extracted
		 * 
		 * @example  M=2, R=2, N= 6
		 * <-------------Measure 0 ---------------><---------------- Measure 1 ------------------->
		 *  <---Left ear--->   <--- Right ear --->  <----- Left ear ------> <----- Right ear ----->
		 * [1, 2, 3, 4, 5, 6 , 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24]
		 */
		void Get3DMatrixData(const std::vector<double>& dataIR, std::vector<float>& outIR, int numberOfReceivers, int numberOfSamples, int receiver, int measure) {
			std::vector<float> IR(numberOfSamples, 0);

			for (std::size_t sample = 0; sample < numberOfSamples; sample++)
			{
				const std::size_t index = array3DIndex(measure, receiver, sample, numberOfReceivers, numberOfSamples);
				IR[sample] = dataIR[index];
			}
			outIR = std::move(IR);
		}
		const std::size_t array3DIndex(const unsigned long measure, const unsigned long receiver, const unsigned long sample, const unsigned long numberOfReceivers, const unsigned long numberOfSamples)
		{
			return numberOfReceivers * numberOfSamples * measure + numberOfSamples * receiver + sample;
		}
		/*void GetData(const std::vector<double>& dataIR, std::vector<float>& outIR, int numberOfMeasurements,int numberOfReceivers, int numberOfSamples, int ear, int i) {
			std::vector<float> IR(numberOfSamples, 0);

			for (std::size_t k = 0; k < numberOfSamples; k++)
			{
				const std::size_t index = array3DIndex(i, ear, k, numberOfMeasurements, numberOfReceivers, numberOfSamples);
				IR[k] = dataIR[index];
			}
			outIR = std::move(IR);
		}*/			
		/*const std::size_t array3DIndex(const unsigned long i, const unsigned long j, const unsigned long k, const unsigned long dim1, const unsigned long dim2, const unsigned long dim3)
		{
			return dim2 * dim3 * i + dim3 * j + k;
		}	*/	 								

		/**
		 * @brief Obtain the requested data from a vector representing a 4D matrix.
		 * @param dataIR Input interlaced matrix
		 * @param outIR Output vector
		 * @param numberOfReceivers Total number of receivers (ears) (R)
		 * @param numberOfSamples Total number of samples per IR (N)
		 * @param numberOfEmmiters Total number of emiters (E)
		 * @param measure Measure to be extracted
		 * @param receiver Ear to be extracted
		 * @param emmiter Emittter to be extracted
		 * 
		 * @example M=2, R=2, N= 6, E=2
		 *  <-------------------------------------------Measure 0 --------------------------------------->
		 *  <---------------------Left ear--------------->  <-------------------- Right ear -------------> 
		 *  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1  E0  E1 
		 * [01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24]
		 */		 
		void Get4DMatrixData(const std::vector<double>& dataIR, std::vector<float>& outIR, int numberOfReceivers, int numberOfSamples, int numberOfEmmiters, int measure, int receiver, int emmiter) {
			std::vector<float> IR(numberOfSamples, 0);

			for (std::size_t sample = 0; sample < numberOfSamples; sample++)
			{
				const std::size_t index = array4DIndex(measure, receiver, sample, emmiter, numberOfReceivers, numberOfSamples, numberOfEmmiters);
				IR[sample] = dataIR[index];
			}
			outIR = std::move(IR);
		}
		
		const std::size_t array4DIndex(const unsigned long measure, const unsigned long receiver, const unsigned long sample, const unsigned long emitter, const unsigned long numberOfReceivers, const unsigned long numberOfSamples, const unsigned long numberOfEmitters)
		{									
			return numberOfReceivers * numberOfSamples * numberOfEmitters * measure + numberOfSamples * numberOfEmitters * receiver + numberOfEmitters * sample + emitter;
		}


		/**
		 * @brief Obtain the requested data from a vector representing a 2D matrix.
		 * @param dataTF 
		 * @param outTF 
		 * @param numberOfSamples 
		 * @param i 
		 */
		void GetDirectivityData(const std::vector<double>& dataTF, std::vector<float>& outTF, int numberOfSamples, int i) {
			std::vector<float> TF(numberOfSamples, 0);

			for (std::size_t k = 0; k < numberOfSamples; k++)
			{
				const std::size_t index = array2DIndex(i, k, 0, numberOfSamples);
				TF[k] = dataTF[index];
			}
			outTF = std::move(TF);
		}

		const std::size_t array2DIndex(const unsigned long i, const unsigned long j, const unsigned long dim1, const unsigned long dim2)
		{
			return dim2 * i + j;
		}

		/**
		 * @brief Convert from cartession to spherical coordinates
		 * @param _x 
		 * @param _y 
		 * @param _z 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _distance 
		 */
		void ToSpherical(double _x, double _y, double _z, double& _azimuth, double& _elevation, double& _distance) {

			Common::CVector3 _vector(_x, _y, _z);
			_azimuth = _vector.GetAzimuthDegrees();
			_elevation = _vector.GetElevationDegrees();
			_distance = _vector.GetDistance();

		}
		/**
		 * @brief Convert from spherical to cartesian coordinates
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _distance 
		 * @param _x 
		 * @param _y 
		 * @param _z 
		 */
		void ToCartesian(double _azimuth, double _elevation, double _distance, double& _x, double& _y, double& _z) {
			Common::CVector3 _vector;
			_vector.SetFromAED(_azimuth, _elevation, _distance);			
			_x = _vector.x;
			_y = _vector.y;
			_z = _vector.z;
		}
	};
};
#endif 
