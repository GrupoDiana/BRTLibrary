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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SOFA_READER_
#define _SOFA_READER_

#define EPSILON 0.0001f

#include <ostream>
#include <string>
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/HRTF.hpp>
#include <Common/ErrorHandler.hpp>
#include <Readers/LibMySofaLoader.hpp>
//#include "ofxlibMySofa.h"
#include <third_party_libraries/libmysofa/include/mysofa.h>

namespace BRTReaders {

	class CSOFAReader {				

	public:
		
		CSOFAReader()
			: errorDescription { "No error." } {
		}

		/**
		 * @brief Get the last action error description, if any
		 * @return error description
		 */
		std::string GetLastError() {
			std::string _errorDescription = errorDescription;
			ResetError();
			return _errorDescription;
		}

		/** \brief Returns the sample rate in Hz in the sofa file
		*	\param [in] path of the sofa file
		*   \eh On error, an error code is reported to the error handler.
		*/
		int GetSampleRateFromSofa(const std::string& sofafile)
		{
			BRTReaders::CLibMySOFALoader loader(sofafile);
			int error = loader.getError();
			if (error) {
				errorDescription = "Error reading SOFA file - " +  loader.GetErrorName(error);
				return -1;
			}

			std::string dataType = loader.GetDataType();
			if (dataType == "FIR" || dataType == "FIR-E" || dataType == "SOS") { 
				ResetError();
				return loader.GetSamplingRate();
			} else {
				errorDescription = "The data type contained in the sofa file is not valid - " + dataType;
				return -1;
			}			
		}

		/** \brief Loads an HRTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadHRTFFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CHRTF> listenerHRTF, int _spatialResolution, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
						
			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			
			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) {
				errorDescription = "Error reading SOFA file - " + loader.GetErrorName(error);
				return false;
			}

			// Discover the file type
			std::string dataType = loader.GetDataType();
			//std::string sofaConvention = loader.GetSofaConvention();

			// Load Data
			if (dataType == "FIR" || dataType == "FIR-E") {	
				ResetError();
				return ReadFromSofaFIRDataType(loader, sofafile, data, _spatialResolution, _extrapolationMethod, 0.0f, 0.0f, 0.0f, 0.0f);		
			} else {
				errorDescription = "The data type contained in the sofa file is not valid for loading HRTFs - " + dataType;
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, errorDescription);
				return false;
			}				
		}
		
		
		/** \brief Loads an ILD from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadSOSFiltersFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CSOSFilters>& listenerNFCFilters)
		{
			std::shared_ptr<BRTServices::CServicesBase> data = listenerNFCFilters;
			
			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) {
				errorDescription = "Error reading SOFA file - " + loader.GetErrorName(error);
				return false;
			}

			// Discover the file type
			std::string dataType = loader.GetDataType();
			//std::string sofaConvention = loader.GetSofaConvention();

			// Load Data			
			if (dataType == "SOS") {
				ResetError();
				return ReadFromSofaSOSDataType(loader, sofafile, data);			
			} else {			
				errorDescription = "The data type contained in the sofa file is not valid for loading NFC Filters - " + dataType;
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, errorDescription);
				return false;
			}															
		}

		/** \brief Loads an DirectivityTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] source affected by the directivityTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadDirectivityTFFromSofa(const std::string & sofafile, std::shared_ptr<BRTServices::CDirectivityTF> sourceDirectivityTF, int _spatialResolution, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {

			std::shared_ptr<BRTServices::CServicesBase> data = sourceDirectivityTF;

			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) {
				errorDescription = "Error reading SOFA file - " + loader.GetErrorName(error);
				return false;
			}

			// Discover the file type
			std::string dataType = loader.GetDataType();
			//std::string sofaConvention = loader.GetSofaConvention();
			// Load Data						
			if (dataType == "TF") {
				ResetError();
				return ReadFromSofaTFDataType(loader, sofafile, data, _spatialResolution, _extrapolationMethod);
			} else {		
				errorDescription = "The data type contained in the sofa file is not valid for loading Directivity TF - " + dataType;
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, errorDescription);
				return false;
			}							
		}
				
				
		/** \brief Loads an HRTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadBRIRFromSofa(const std::string & sofafile, std::shared_ptr<BRTServices::CHRBRIR> listenerHRTF, int _spatialResolution, 
			BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod, 
			float _fadeInWindowThreshold, float _fadeInWindowRiseTime, float _fadeOutWindowThreshold, float _fadeOutWindowRiseTime) {

			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) {
				errorDescription = "Error reading SOFA file - " + loader.GetErrorName(error);
				return false;
			}

			// Discover the file type
			std::string dataType = loader.GetDataType();
			std::string sofaConvention = loader.GetSofaConvention();

			// Load Data
			if (dataType == "FIR" || dataType == "FIR-E") {
				ResetError();
				return ReadFromSofaFIRDataType(loader, sofafile, data, _spatialResolution, _extrapolationMethod, _fadeInWindowThreshold, _fadeInWindowRiseTime, _fadeOutWindowThreshold, _fadeOutWindowRiseTime);			
			} else {
				errorDescription = "The data type contained in the sofa file is not valid for loading BRIRs - " + dataType;
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, errorDescription);
				return false;
			}													
		}
		
	private:
				
		// Methods
				
		/////////////////////////////////////////////////////////////////
		/////////////////	SimpleFreeFieldHRSOS		/////////////////
		/////////////////////////////////////////////////////////////////
		bool ReadFromSofaSOSDataType(BRTReaders::CLibMySOFALoader& loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data) {
			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS, sofafile, data); // GET and Save Global Attributes 			
			CheckListenerOrientation(loader);									// Check listener view						
			GetAndSaveReceiverPosition(loader, data);							// Get and Save listener ear 
			
			bool result;			
			std::string _error;
			result = GetHRSOSCoefficients(loader, data, _error);
			if (!result) {
				errorDescription = "An error occurred creating the data structure from the SOFA file - " + _error;
				SET_RESULT(RESULT_ERROR_UNKNOWN, errorDescription);
				return false;
			}
			// Finish setup			
			data->EndSetup();
			return true;
		}

		bool GetHRSOSCoefficients(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& data, std::string& _error) {
			//Get source positions												
			std::vector< double > sourcePositionsVector = std::move(loader.GetSourcePositionVector());
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
				_error = "SOFA gives incoherent number of receivers and coefficients";
				SET_RESULT(RESULT_ERROR_BADSIZE, _error);
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
				BRTServices::TSOSFilterStruct coefficients;
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
		bool ReadFromSofaTFDataType(BRTReaders::CLibMySOFALoader& loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			// Get and Save data			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF, sofafile, data); // GET and Save Global Attributes 
			//CheckCoordinateSystems(loader, _SOFAConvention);					// Check coordiante system for Source and Receiver positions
			CheckListenerOrientation(loader);									// Check listener view			
			
			bool result;			
			std::string _error;
			result = GetDirectivityTF(loader, data, _extrapolationMethod, _error);						
			if (!result) {		
				errorDescription = _error;				
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetGridSamplingStep(_resamplingStep); }			
			bool success = data->EndSetup();			
			
			return success;
		}

		bool GetDirectivityTF(BRTReaders::CLibMySOFALoader & loader, std::shared_ptr<BRTServices::CServicesBase> & dataDirectivityTF, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod, std::string& error) {
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
			bool success = dataDirectivityTF->BeginSetup(numberOfFrequencySamples, _extrapolationMethod);
			if (!success) {
				error = dataDirectivityTF->GetLastError();				
				return false;
			}

			// This outtermost loop iterates over TFs
			for (std::size_t receiver = 0; receiver < numberOfReceivers; receiver++)
			{
				BRTServices::TDirectivityTFStruct directivityTF_data;
				CMonoBuffer<float> dataRealPartPI;
				CMonoBuffer <float> dataImagPartPI;
				double azimuth, elevation, distance;
				GetReceiverPosition(loader, receiverPositionsVector, receiver, azimuth, elevation, distance);				
								
				Get2DMatrixData(dataMeasurementsRealPart, dataRealPartPI, numberOfFrequencySamples, receiver);
				Get2DMatrixData(dataMeasurementsImagPart, dataImagPartPI, numberOfFrequencySamples, receiver);

				directivityTF_data.realPart = dataRealPartPI;
				directivityTF_data.imagPart = dataImagPartPI;

				dataDirectivityTF->AddDirectivityTF(azimuth, elevation, std::move(directivityTF_data));
			}
			return true;
		}

		/////////////////////////////////////////////////////////////////
		//////////////////	 SingleRoomMIMOSRIR		 ////////////////////
		/////////////////////////////////////////////////////////////////
		bool ReadFromSofaFIRDataType(BRTReaders::CLibMySOFALoader &loader, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data,
			int _resamplingStep, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod, 
			float _fadeInWindowThreshold, float _fadeInWindowRiseTime, float _fadeOutWindowThreshold, float _fadeOutWindowRiseTime) {
			
			// Get and Save data			
			GetAndSaveGlobalAttributes(loader, CLibMySOFALoader::TSofaConvention::SingleRoomMIMOSRIR ,sofafile, data);			// GET and Save Global Attributes			
			GetAndSaveReceiverPosition(loader, data); // Get and Save listener ear
			
			data->SetWindowingParameters(_fadeInWindowThreshold, _fadeInWindowRiseTime, _fadeOutWindowThreshold, _fadeOutWindowRiseTime);
			bool result;			
			result = GetBRIRs(loader, data, _extrapolationMethod);			
			if (!result) {
				errorDescription = "An error occurred creating the data structure from the SOFA file.";
				SET_RESULT(RESULT_ERROR_UNKNOWN, errorDescription);
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetGridSamplingStep(_resamplingStep); }
			data->EndSetup();
			return true;

		}
		
		bool GetBRIRs(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataHRBRIR, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) {
			//Get source positions												
			std::vector<double> sourcePositionsVector	= std::move(loader.GetSourcePositionVector());
			
			std::vector<double> sourceViewVector		= std::move(loader.GetSourceViewVector());
			if (sourceViewVector.size() == 0) {
				sourceViewVector = std::vector<double>({1,0,0});
			}
			std::vector<double> sourceUpVector			= std::move(loader.GetSourceUpVector());			
			if (sourceUpVector.size() == 0) {
				sourceUpVector = std::vector<double>({ 0,0,1 });
			}
			
			//Emitter
			std::vector<double> emitterPositionsVector = std::move(loader.GetEmitterPositionVector());
			//Listener
			std::vector<double> listenerPositionsVector = std::move(loader.GetListenerPositionVector());
			std::vector<double> listenerViewVector = std::move(loader.GetListenerViewVector());
			std::vector<double> listenerUpVector = std::move(loader.GetListenerUpVector());
			//Delays
			std::vector< double > dataDelayVector(loader.getHRTF()->DataDelay.values, loader.getHRTF()->DataDelay.values + loader.getHRTF()->DataDelay.elements);
			//IRs
			std::vector< double > dataMeasurements(loader.getHRTF()->DataIR.values, loader.getHRTF()->DataIR.values + loader.getHRTF()->DataIR.elements);

			// Constants
			const int numberOfMeasurements = loader.getHRTF()->M;
			const int numberOfReceivers = loader.getHRTF()->R;
			const unsigned int numberOfSamples = loader.getHRTF()->N;
			const int numberOfEmitters = loader.getHRTF()->E;
			const int numberOfCoordinates = loader.getHRTF()->C;

			
			bool success = dataHRBRIR->BeginSetup(numberOfSamples, _extrapolationMethod);
			if (!success) {
				errorDescription = dataHRBRIR->GetLastError();
				return false;
			}
			dataHRBRIR->SetSamplingRate(loader.GetSamplingRate());
			const int left_ear = 0;
			const int right_ear = 1;

			//for (std::size_t emitter = 0; emitter < numberOfEmitters; emitter++)
			std::size_t emitter = 0;
			{
				// This outtermost loop iterates over HRIRs
				for (std::size_t measure = 0; measure < numberOfMeasurements; measure++)
				{
					BRTServices::THRIRStruct hrir_value;
					
					// Get Source position
					Common::CVector3 sourcePosition = GetSourcePositionCartesian(loader, sourcePositionsVector, measure);
					// Get source orientation components	
					Common::CVector3 sourceView = GetSourceView(loader, sourceViewVector, numberOfMeasurements, measure);
					Common::CVector3 sourceUp = GetSourceUp(loader, sourceUpVector, numberOfMeasurements, measure);
					
					//Get Emmiter location
					Common::CVector3 emitterPosition = GetEmitterPosition(loader, emitterPositionsVector, measure, emitter); 
					Common::CTransform globalEmitterLocation = CalculateTransformFromSeparateComponents(emitterPosition, sourceView, sourceUp);					
					globalEmitterLocation.SetPosition(globalEmitterLocation.GetPosition() + sourcePosition);
					
					//Get Listener location			
					Common::CVector3 listenerPosition = GetListenerPosition(loader, listenerPositionsVector, numberOfMeasurements, measure);
					Common::CVector3 listenerView = GetListenerView(loader, listenerViewVector, numberOfMeasurements, measure);
					Common::CVector3 listenerUp = GetListenerUp(loader, listenerUpVector, numberOfMeasurements, measure);					
					Common::CTransform listenerLocation = CalculateTransformFromSeparateComponents(listenerPosition, listenerView, listenerUp);
					
					// Make projection of source position to listener coordinate system
					Common::CVector3 vectorListenerToEmitter = listenerLocation.GetVectorTo(globalEmitterLocation);
					float _relativeAzimuthListenerEmitter = vectorListenerToEmitter.GetAzimuthDegrees();
					float _relativeElevationListenerEmitter = vectorListenerToEmitter.GetElevationDegrees();
					float _relativeDistanceListenerEmitter = vectorListenerToEmitter.GetDistance();

					//Get Delay					
					double leftDelay, rightDelay;;
					GetDelays(loader, dataDelayVector, leftDelay, rightDelay, numberOfMeasurements, numberOfReceivers, measure);
					hrir_value.leftDelay = std::round(leftDelay);
					hrir_value.rightDelay = std::round(rightDelay);

					//Get IR
					Get4DMatrixData(dataMeasurements, hrir_value.leftHRIR, numberOfReceivers, numberOfSamples, numberOfEmitters, measure, left_ear, emitter);
					Get4DMatrixData(dataMeasurements, hrir_value.rightHRIR, numberOfReceivers, numberOfSamples, numberOfEmitters, measure, right_ear, emitter);
					
					//TODO Listener position in our Convention
					Common::CVector3 listenerPositionBRTConvention;					
					listenerPositionBRTConvention.SetAxis(FORWARD_AXIS, listenerPosition.x);
					listenerPositionBRTConvention.SetAxis(RIGHT_AXIS, listenerPosition.y);
					listenerPositionBRTConvention.SetAxis(UP_AXIS, listenerPosition.z);
					// Set data to HRBIR struct
					dataHRBRIR->AddHRIR(_relativeAzimuthListenerEmitter, _relativeElevationListenerEmitter, _relativeDistanceListenerEmitter, listenerPosition, std::move(hrir_value));					
				}
			}
			return true;

		}
				
		/////////////////////////
		// METHODS
		/////////////////////////
		
		/**
		 * @brief Extract source position from LibMySofa struct. If in Cartesian coordinates, convert to spherical.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param _sourcePositionsVector Vector with all source positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure Measure to be recovered
		 * @param azimuth Output parameter containing the azimuth.
		 * @param elevation Output parameter containing the elevation.
		 * @param distance Output parameter containing the distance.
		 */			
		void GetSourcePosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _sourcePositionsVector, std::size_t measure, double& azimuth, double& elevation, double& distance) {

			int numberOfCoordinates = loader.getHRTF()->C;			
			std::vector<double> _sourcePosition;

			GetOneVector3From2DMatrix(_sourcePositionsVector, _sourcePosition, measure, numberOfCoordinates);			
			if (IsSourcePositionCoordinateSystemsCartesian(loader)) {
				double tempX, tempY, tempZ;
				ToSpherical(_sourcePosition[0], _sourcePosition[1], _sourcePosition[2], tempX, tempY, tempZ);
				_sourcePosition = std::vector<double>({ tempX, tempY, tempZ });
			}
			_sourcePosition = RoundToZeroIfClose(_sourcePosition);
			azimuth = _sourcePosition[0];
			elevation = _sourcePosition[1];
			distance = _sourcePosition[2];			
		}


		Common::CVector3 GetSourcePositionCartesian(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _sourcePositionsVector, std::size_t measure) {
			int numberOfCoordinates = loader.getHRTF()->C;
			std::vector<double> _sourcePosition;

			GetOneVector3From2DMatrix(_sourcePositionsVector, _sourcePosition, measure, numberOfCoordinates);
			if (IsSourcePositionCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_sourcePosition[0], _sourcePosition[1], _sourcePosition[2], tempX, tempY, tempZ);
				_sourcePosition = std::vector<double>({ tempX, tempY, tempZ });
			}			

			return RoundToZeroIfClose(Common::CVector3(_sourcePosition[0], _sourcePosition[1], _sourcePosition[2]));
		}

		/**
		 * @brief Extract source position from LibMySofa struct. If in spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param emitterPositionsVector 
		 * @param measure 
		 * @param _emitter 
		 * @return 
		 */
		Common::CVector3 GetEmitterPosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& emitterPositionsVector, std::size_t measure, std::size_t _emitter) {
			int numberOfCoordinates = loader.getHRTF()->C;
			//Get emmiter position	
			std::vector<double> _emitterPosition;
			GetOneVector3From2DMatrix(emitterPositionsVector, _emitterPosition, _emitter, numberOfCoordinates);
			if (IsEmitterPositionCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_emitterPosition[0], _emitterPosition[1], _emitterPosition[2], tempX, tempY, tempZ);
				_emitterPosition = std::vector<double>({ tempX, tempY, tempZ });
			}
			return RoundToZeroIfClose(Common::CVector3(_emitterPosition[0], _emitterPosition[1], _emitterPosition[2]));
		}		

		/**
		 * @brief Extract source view from LibMySofa struct. If in spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param sourceViewVector Vector with all source VIEW. We pass it on so that we don't have to map it every time and save time.
		 * @param numberOfMeasurements Total number of measurements
		 * @param measure Measure to be recovered
		 * @return View of the source, 3-dimensional vector, Cartesian.
		 */
		Common::CVector3 GetSourceView(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& sourceViewVector, std::size_t numberOfMeasurements, std::size_t measure) {
			int numberOfCoordinates = loader.getHRTF()->C;
			std::vector<double> _sourceView;			
			
			if (sourceViewVector.size() != numberOfMeasurements * numberOfCoordinates) {	measure = 0;}
			
			GetOneVector3From2DMatrix(sourceViewVector, _sourceView, measure, numberOfCoordinates);

			if (IsSourceViewCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_sourceView[0], _sourceView[1], _sourceView[2], tempX, tempY, tempZ);
				_sourceView = std::vector<double>({ tempX, tempY, tempZ });
			}
			return RoundToZeroIfClose(Common::CVector3(_sourceView));
		}
		
		/**
		 * @brief Extract source up from LibMySofa struct. If in spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param sourceUpVector Vector with all source UP. We pass it on so that we don't have to map it every time and save time.
		 * @param numberOfMeasurements Total number of measurements
		 * @param measure Measure to be recovered
		 * @return UP of the source, 3-dimensional vector, Cartesian.
		 */
		Common::CVector3 GetSourceUp(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& sourceUpVector, std::size_t numberOfMeasurements, std::size_t measure) {
			int numberOfCoordinates = loader.getHRTF()->C;			
			std::vector<double> _sourceUp;
			
			if (sourceUpVector.size() != numberOfMeasurements * numberOfCoordinates) { measure = 0; }
			GetOneVector3From2DMatrix(sourceUpVector, _sourceUp, measure, numberOfCoordinates);
					
			if (IsSourceViewCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_sourceUp[0], _sourceUp[1], _sourceUp[2], tempX, tempY, tempZ);
				_sourceUp = std::vector<double>({ tempX, tempY, tempZ });
			}
			return RoundToZeroIfClose(Common::CVector3(_sourceUp));
		}

		/**
		 * @brief Extract receiver position from SOFA struct. If in Cartesian coordinates, convert to spherical.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param receiverPositionsVector Vector with all the positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure  Measure to be recovered
		 * @param azimuth  Output parameter containing the azimuth.
		 * @param elevation Output parameter containing the elevation.
		 * @param distance Output parameter containing the distance.		 
		 */
		void GetReceiverPosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& receiverPositionsVector, std::size_t measure, double& azimuth, double& elevation, double& distance) {			
			int numberOfCoordinates = loader.getHRTF()->C;

			if (IsReceiverPositionCoordinateSystemsSpherical(loader)) {				
				azimuth		= receiverPositionsVector[array2DIndex(measure, 0, numberOfCoordinates)];
				elevation	= receiverPositionsVector[array2DIndex(measure, 1, numberOfCoordinates)];
				distance	= receiverPositionsVector[array2DIndex(measure, 2, numberOfCoordinates)];
			}
			else {
				double x = receiverPositionsVector[array2DIndex(measure, 0, numberOfCoordinates)];
				double y = receiverPositionsVector[array2DIndex(measure, 1, numberOfCoordinates)];
				double z = receiverPositionsVector[array2DIndex(measure, 2, numberOfCoordinates)];
				ToSpherical(x, y, z, azimuth, elevation, distance);
			}
		}


		/**
		 * @brief Extract listener position from SOFA struct. If in Spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param _listenerPositionsVector Vector with all the positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure  Measure to be recovered
		 * @return positions of the listener, 3-dimensional vector, Cartesian.
		 */
		std::vector<double> GetListenerPosition(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _listenerPositionsVector, std::size_t numberOfMeasurements, std::size_t measure) {
			std::vector<double> _listenerPosition;
			int numberOfCoordinates = loader.getHRTF()->C;
			// Get Listener position						
			if (_listenerPositionsVector.size() != numberOfMeasurements * numberOfCoordinates) { measure = 0; }
			GetOneVector3From2DMatrix(_listenerPositionsVector, _listenerPosition, measure, numberOfCoordinates);
						
			if (IsListenerPositionCoordinateSystemsSpherical(loader)) {				
				double tempX, tempY, tempZ;
				ToCartesian(_listenerPosition[0], _listenerPosition[1], _listenerPosition[2], tempX, tempY, tempZ);
				_listenerPosition = std::vector<double>({ tempX, tempY, tempZ });
			};						
			return _listenerPosition;
		}


		/**
		 * @brief Extract listener view from SOFA struct. If in Spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param _listenerPositionsVector Vector with all the positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure  Measure to be recovered
		 * @return VIEW of the listener, 3-dimensional vector, Cartesian.
		 */
		std::vector<double> GetListenerView(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _listenerViewVector, std::size_t numberOfMeasurements, std::size_t measure) {
			std::vector<double> _listenerView;
			int numberOfCoordinates = loader.getHRTF()->C;
									
			if (_listenerViewVector.size() != numberOfMeasurements * numberOfCoordinates) { measure = 0; }
			GetOneVector3From2DMatrix(_listenerViewVector, _listenerView, measure, numberOfCoordinates);

			if (IsListenerViewCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_listenerView[0], _listenerView[1], _listenerView[2], tempX, tempY, tempZ);
				_listenerView = std::vector<double>({ tempX, tempY, tempZ });
			};
			return RoundToZeroIfClose(_listenerView);
		}

		/**
		 * @brief Extract listener UP from SOFA struct. If in Spherical  coordinates, convert to Cartesian.
		 * @param loader The complete data structure read from the SOFA file.
		 * @param _listenerPositionsVector Vector with all the positions.We pass it on so that we don't have to map it every time and save time.
		 * @param measure  Measure to be recovered
		 * @return UP of the listener, 3-dimensional vector, Cartesian.
		 */
		std::vector<double> GetListenerUp(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _listenerUpVector, std::size_t numberOfMeasurements, std::size_t measure) {
			std::vector<double> _listenerUp;
			int numberOfCoordinates = loader.getHRTF()->C;
			
			if (_listenerUpVector.size() != numberOfMeasurements * numberOfCoordinates) { measure = 0; }
			GetOneVector3From2DMatrix(_listenerUpVector, _listenerUp, measure, numberOfCoordinates);

			if (IsListenerViewCoordinateSystemsSpherical(loader)) {
				double tempX, tempY, tempZ;
				ToCartesian(_listenerUp[0], _listenerUp[1], _listenerUp[2], tempX, tempY, tempZ);
				_listenerUp = std::vector<double>({ tempX, tempY, tempZ });
			};
			return RoundToZeroIfClose(_listenerUp);
		}


		void GetDelays(BRTReaders::CLibMySOFALoader& loader, const std::vector<double>& _dataDelaysVector, double& leftDelay, double& rightDelay, std::size_t numberOfMeasurements, std::size_t numberOfReceivers, std::size_t measure) {
			leftDelay = -1;
			rightDelay = -1;
			if (_dataDelaysVector.size() == numberOfMeasurements * numberOfReceivers) {
				leftDelay = _dataDelaysVector[array2DIndex(measure, 0, numberOfReceivers)];
				rightDelay = _dataDelaysVector[array2DIndex(measure, 1, numberOfReceivers)];
			} else if (_dataDelaysVector.size() == 2) {
				SET_RESULT(RESULT_WARNING, "This HRTF file does not contain individual delays for each HRIR. Therefore, some comb filter effect can be perceived due to interpolations and custom head radius should not be used.");
				leftDelay = _dataDelaysVector[0];
				rightDelay = _dataDelaysVector[1];										
			} else {
				SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of HRIRs and delays");					
			}		
			if (leftDelay < 0 || rightDelay < 0) {
				SET_RESULT(RESULT_WARNING, "Error reading IR from SOFA file, one of the delay fields is negative. This data is rejected.");		
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

		
		// Check Source Position Coordinate Systems
		bool IsSourcePositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourcePositionCoordinateSystems(loader, "spherical");
		}

		bool IsSourcePositionCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourcePositionCoordinateSystems(loader, "cartesian");
		}

		bool CheckSourcePositionCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {						
			if (loader.GetSourcePositionType() == _coordinateSystem) { return true; }			
			return false;
		}
		
		//Check Source View Coordinate Systems
		bool IsSourceViewCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourceViewCoordinateSystems(loader, "spherical");
		}
		bool IsSourceViewCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckSourceViewCoordinateSystems(loader, "cartesian");
		}
		bool CheckSourceViewCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {
			if (loader.GetSourceViewType() == _coordinateSystem) { return true; }
			return false;
		}

		// Receiver Coordinate Systems Checks
		bool IsReceiverPositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckReceiverPositionCoordinateSystemsNew(loader, "spherical");
		}
		bool IsReceiverPositionCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckReceiverPositionCoordinateSystemsNew(loader, "cartesian");
		}
		bool CheckReceiverPositionCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {			
			if (loader.GetReceiverPositionType() == _coordinateSystem) { return true; }
			return false;
		}

		// Emitter Coordinate Systems Checks
		bool IsEmitterPositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckEmitterPositionCoordinateSystemsNew(loader, "spherical");
		}
		bool IsEmitterPositionCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckEmitterPositionCoordinateSystemsNew(loader, "cartesian");
		}
		bool CheckEmitterPositionCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {
			if (mysofa_getAttribute(loader.getHRTF()->EmitterPosition.attributes, "Type") == _coordinateSystem) { return true; }
			return false;
		}

		// Listener Position Coordinate Systems Checks
		bool IsListenerPositionCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckListenerPositionCoordinateSystemsNew(loader, "spherical");
		}
		bool IsListenerPositionCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckListenerPositionCoordinateSystemsNew(loader, "cartesian");
		}
		bool CheckListenerPositionCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {
			if (mysofa_getAttribute(loader.getHRTF()->EmitterPosition.attributes, "Type") == _coordinateSystem) { return true; }
			return false;
		}

		// Listener View Coordinate Systems Checks
		bool IsListenerViewCoordinateSystemsSpherical(BRTReaders::CLibMySOFALoader& loader) {
			return CheckListenerViewCoordinateSystemsNew(loader, "spherical");
		}
		bool IsListenerViewCoordinateSystemsCartesian(BRTReaders::CLibMySOFALoader& loader) {
			return CheckListenerViewCoordinateSystemsNew(loader, "cartesian");
		}
		bool CheckListenerViewCoordinateSystemsNew(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {
			if (mysofa_getAttribute(loader.getHRTF()->ListenerView.attributes, "Type") == _coordinateSystem) { return true; }
			return false;
		}


		// Check listener position in the SOFA file
		void CheckListenerOrientation(BRTReaders::CLibMySOFALoader& loader) {			
			
			if (!IsListenerViewCoordinateSystemsCartesian(loader)) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Listener View/UP in SOFA file does not have the casterian coordinates system");				
			}

			//Check listener view			
			std::vector<double> _listenerViewVector = loader.GetListenerView();
			if (_listenerViewVector.size() != 3) { 
				SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener View in SOFA file does not have the expected size"); 
			}
			else {
				Common::CVector3 _listenerView(_listenerViewVector[0], _listenerViewVector[1], _listenerViewVector[2]);
				Common::CVector3 _forward(1, 0, 0);
				if (_listenerView != _forward) { SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener View in SOFA file different from [1,0,0]"); }
			}
			//Check listener up	
			std::vector<double> _listenerUpVector = loader.GetListenerUp();
			if (_listenerUpVector.size() != 3) { 
				SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener Up in SOFA file does not have the expected size"); 
			}
			else {
				Common::CVector3 _listenerUp(_listenerUpVector[0], _listenerUpVector[1], _listenerUpVector[2]);
				Common::CVector3 _up(0, 0, 1);
				if (_listenerUp != _up) { SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Listener Up in SOFA file different from [0,0,1]"); }
			}										
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
		Common::CTransform CalculateTransformFromSeparateComponents(Common::CVector3 _position, Common::CVector3 _view, Common::CVector3 _up) {
			//Yaw and Pitch are calculated from the view vector
			Common::CVector3 _viewNormalized = _view.Normalize();
			float azimuth = _viewNormalized.GetAzimuthRadians();
			float elevation = _viewNormalized.GetElevationRadians();

			Common::CQuaternion _orientation = Common::CQuaternion::FromYawPitchRoll(-azimuth, elevation, 0.0f);

			// Roll is calculated from the up vector
			Common::CVector3 _upNormalized = _up.Normalize();
			Common::CVector3 _rotatedUpVector = _orientation.RotateVector(Common::CVector3(0, 0, 1));
			Common::CVector3 rollAxis = (_rotatedUpVector.CrossProduct(_upNormalized)).Normalize();							
			//float roll = std::acos(_rotatedUpVector.Normalize().DotProduct(_upNormalized));
			float roll = Common::SafeAcos(_rotatedUpVector.Normalize().DotProduct(_upNormalized));

			Common::CQuaternion rollQuaternion = Common::CQuaternion::FromAxisAngle(rollAxis, roll);

			// Get complete orientation quaternion by multiplying yaw-pitch-roll quaternions
			_orientation.Rotate(rollQuaternion);
		
			// Create the transform
			Common::CTransform _transform;
			_transform.SetPosition(_position);
			_transform.SetOrientation(_orientation);
			return _transform;
		}

		Common::CQuaternion GetObjectOrientation(Common::CVector3 _view, Common::CVector3 _up) {
			//1 Obten el yaw como Yaw = View.getAzimutzRadians()
			float  yaw = _view.GetAzimuthRadians();			 
			//2 Obten Pitch como Pitch = View.getElevationRadians()
			float pitch = _view.GetElevationRadians();
			//3 Rota la matriz identidad con I.rotate(Z, Yaw) y I.rotate(Y, Pitch)
			//Common::CTransform I;
			Common::CQuaternion sourceOrientation; 
			sourceOrientation = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, 0);

			//Common::CVector3 z;
			//z.GetAxis(Common::TAxis::FORWARD_AXIS);)			
			//I.Rotate(Common::CVector3(0, 0, 1), yaw);
			//I.Rotate(Common::CVector3(0, 1, 0), pitch);
			
			//4️ Obten el up rotado Yaw y Pitch aplicnado la pmatriz anterior a Z.Lo llamaremos ZZ
			//Common::CVector3 z(0, 0, 1);
			
			//Common::CVector3 ZZ = I * _up;
			//5️ Calcula Roll como Roll = arccos(arccos(ZZ·Up)
			//6️ Rota de nuevo ahora para el roll : I.rotate(X, Roll)	
			// 
			Common::CQuaternion sourceOrientation2;
			//sourceOrientation2 = Common::CQuaternion::LookAt(_view, _up);
			return sourceOrientation;
		}

				
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
		 * @param numberOfSamples Total number of samples per measure(IR) (N)
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
		 * @param dataTF Input interlaced matrix
		 * @param outTF Output vector
		 * @param numberOfSamples Total number of samples per measure (N)
		 * @param measure Measure to be extracted
		 * * @example  N= 8
		 * <----Measure 0 -------->  <-------- Measure 1 -------->		 
		 * [1, 2, 3, 4, 5, 6 , 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
		 */
		void Get2DMatrixData(const std::vector<double>& dataTF, std::vector<float>& outTF, int numberOfSamples, int measure) {
			std::vector<float> TF(numberOfSamples, 0);

			for (std::size_t sample = 0; sample < numberOfSamples; sample++)
			{
				const std::size_t index = array2DIndex(measure, sample, numberOfSamples);
				TF[sample] = dataTF[index];
			}
			outTF = std::move(TF);
		}

		/**
		 * @brief Obtain the requested data from a vector representing a 2D matrix.
		 * @param _inVector Input interlaced matrix
		 * @param _outVector Output vector
		 * @param measure Measure to be extracted
		 * @param numberOfCoordinates 
		 * @return 
		 */
		void GetOneVector3From2DMatrix(const std::vector<double>& _inVector, std::vector<double>& _outVector, std::size_t measure, int numberOfCoordinates) {						
			if (numberOfCoordinates < 3) { _outVector = std::vector<double>({0, 0, 0}); }
			double a = _inVector[array2DIndex(measure, 0, numberOfCoordinates)];
			double b = _inVector[array2DIndex(measure, 1, numberOfCoordinates)];
			double c = _inVector[array2DIndex(measure, 2, numberOfCoordinates)];
			_outVector = std::vector<double>({ a, b, c });
		}

		const std::size_t array2DIndex(const unsigned long measure, const unsigned long sample,  const unsigned long numberOfSamples)
		{
			return numberOfSamples * measure + sample;
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

	 
		/**
		 * @brief Round any coordinate to zero if it is close to zero
		 * @param _inPoint 
		 * @return 
		 */
		Common::CVector3 RoundToZeroIfClose(Common::CVector3 _inPoint) {
			Common::CVector3 _outPoint;
			_outPoint.x =  Common::AreSameDouble(_inPoint.x, 0, EPSILON) ? 0: _inPoint.x;
			_outPoint.y = Common::AreSameDouble(_inPoint.y, 0, EPSILON) ? 0 : _inPoint.y;
			_outPoint.z = Common::AreSameDouble(_inPoint.z, 0, EPSILON) ? 0 : _inPoint.z;
			return _outPoint;
		}

		/**
		 * @brief Round any coordinate to zero if it is close to zero
		 * @param _inPoint
		 * @return
		 */
		std::vector<double> RoundToZeroIfClose(std::vector<double> _inPoint) {
			std::vector<double> _outPoint;
			for (std::size_t i = 0; i < _inPoint.size(); i++) {
				_outPoint.push_back(Common::AreSameDouble(_inPoint[i], 0, EPSILON) ? 0 : _inPoint[i]);
			}									
			return _outPoint;
		}
						
		void ResetError() {
			errorDescription = "No error.";
		}

		////////////////
		// Attributes
		////////////////
		std::string errorDescription;

	};	
};
#endif 
