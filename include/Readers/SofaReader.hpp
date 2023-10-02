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
		bool ReadHRTFFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CHRTF> listenerHRTF, int _resamplingStep) {
						
			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR, _resamplingStep);			
		}
		
		bool ReadHRTFFromSofaWithoutProcess(const std::string& sofafile, std::shared_ptr<BRTServices::CHRTF> listenerHRTF, int _resamplingStep) {

			std::shared_ptr<BRTServices::CServicesBase> data = listenerHRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR, _resamplingStep, false);
		}

		/** \brief Loads an ILD from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadILDFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CILD>& listenerILD)
		{
			std::shared_ptr<BRTServices::CServicesBase> data = listenerILD;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS, -1);			
		}

		/** \brief Loads an SRTF from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] source affected by the srtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool ReadSRTFFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CSRTF> sourceSRTF, int _resamplingStep) {

			std::shared_ptr<BRTServices::CServicesBase> data = sourceSRTF;
			return ReadFromSofa(sofafile, data, CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF, _resamplingStep);
		}
				
	private:
				
		// Methods
		bool ReadFromSofa(const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& data, CLibMySOFALoader::TSofaConvention _SOFAConvention, int _resamplingStep, bool process = true) {

			// Open file
			BRTReaders::CLibMySOFALoader loader(sofafile);
			bool error = loader.getError();
			if (error) return false;
			
			// Check convention
			if (!loader.CheckSofaConvention(_SOFAConvention)) return false;			
			SET_RESULT(RESULT_OK, "Open a valid SOFA file");

			// Get and Save data			
			GetAndSaveGlobalAttributes(loader, _SOFAConvention, sofafile, data); // GET and Save Global Attributes 
			CheckCoordinateSystems(loader, _SOFAConvention);					// Check coordiante system for Source and Receiver positions
			CheckListenerOrientation(loader);									// Check listener view			
			
			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR || _SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) {
				GetAndSaveReceiverPosition(loader, data); // Get and Save listener ear 
			} 
			bool result;
			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR) {		result = GetHRIRs(loader, data); }
			else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) {	result = GetCoefficients(loader, data);	}
			else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF) { result = GetDirectivityTF(loader, data); }
						
			if (!result) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "An error occurred creating the data structure from the SOFA file, please consider previous messages.");
				return false;
			}

			// Finish setup
			if (_resamplingStep != -1) { data->SetResamplingStep(_resamplingStep); }			
			if (process) { data->EndSetup(); }
			return true;

		}

				
		/////////////////////////
		// METHODS
		/////////////////////////
		
		// Read GLOBAL data from sofa struct and save into HRTF class
		void GetAndSaveGlobalAttributes(BRTReaders::CLibMySOFALoader& loader, CLibMySOFALoader::TSofaConvention _SOFAConvention, const std::string& sofafile, std::shared_ptr<BRTServices::CServicesBase>& dataHRTF) {
			// GET and Save Global Attributes 			
			std::string _title = mysofa_getAttribute(loader.getHRTF()->attributes, "Title");
			dataHRTF->SetTitle(_title);

			std::string _databaseName = mysofa_getAttribute(loader.getHRTF()->attributes, "DatabaseName");
			dataHRTF->SetDatabaseName(_databaseName);

			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR || _SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) {
				std::string _listenerShortName = mysofa_getAttribute(loader.getHRTF()->attributes, "ListenerShortName");
				dataHRTF->SetListenerShortName(_listenerShortName);
			}

			std::filesystem::path p(sofafile);
			std::string fileName{ p.filename().u8string() };
			dataHRTF->SetFilename(fileName);

		}
		
		void CheckCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, CLibMySOFALoader::TSofaConvention _SOFAConvention) {

			if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRIR || _SOFAConvention == CLibMySOFALoader::TSofaConvention::SimpleFreeFieldHRSOS) {
				CheckSourcePositionCoordinateSystems(loader, "spherical");
				CheckReceiverPositionCoordinateSystems(loader, "cartesian");
			}
			else if (_SOFAConvention == CLibMySOFALoader::TSofaConvention::FreeFieldDirectivityTF) {
				CheckSourcePositionCoordinateSystems(loader, "cartesian");
				CheckReceiverPositionCoordinateSystems(loader, "spherical");
			}
			else{ SET_RESULT(RESULT_WARNING, "CoordinateSystem could not be checked"); }	
		}

		void CheckSourcePositionCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {

			std::string sourcePostionsCoordianteSystem = mysofa_getAttribute(loader.getHRTF()->SourcePosition.attributes, "Type");
			if (sourcePostionsCoordianteSystem != _coordinateSystem) {
				//loader.Cartesian2Spherical();
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Source positions from SOFA file do not have the expected coordinate system");
			};
		}

		void CheckReceiverPositionCoordinateSystems(BRTReaders::CLibMySOFALoader& loader, std::string _coordinateSystem) {

			std::string receiverPostionsCoordianteSystem = mysofa_getAttribute(loader.getHRTF()->ReceiverPosition.attributes, "Type");
			if (receiverPostionsCoordianteSystem != _coordinateSystem) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "positions from SOFA file do not have the expected coordinate system");
			};
		}

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

		// Get and save listener ear positions and save into HRTF class
		void GetAndSaveReceiverPosition(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& data) {									
			int numberOfReceivers = loader.getHRTF()->R;
			data->SetNumberOfEars(numberOfReceivers);
			
			//Get and Save listener ear positions
			std::vector< double > receiverPos = loader.GetReceiverPosition();

			if (numberOfReceivers >= 1) {
				Common::CVector3 leftEarPos;
				leftEarPos.SetAxis(FORWARD_AXIS, receiverPos[0]);
				leftEarPos.SetAxis(RIGHT_AXIS, receiverPos[1]);
				leftEarPos.SetAxis(UP_AXIS, receiverPos[2]);
				data->SetEarPosition(Common::T_ear::LEFT, leftEarPos);

				if (numberOfReceivers >= 2) {
					Common::CVector3 rightEarPos;
					rightEarPos.SetAxis(FORWARD_AXIS, receiverPos[3]);
					rightEarPos.SetAxis(RIGHT_AXIS, receiverPos[4]);
					rightEarPos.SetAxis(UP_AXIS, receiverPos[5]);
					data->SetEarPosition(Common::T_ear::RIGHT, rightEarPos);
				}
			}
		}


		bool GetHRIRs(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataHRTF) {			
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

			// Get and save HRIRs
			double distance = sourcePositionsVector[array2DIndex(0, 2, numberOfMeasurements, numberOfCoordinates)];		//We consider that every HRIR are meased at the same distance, so we get the firts one									
			dataHRTF->BeginSetup(numberOfSamples, distance);
			
			dataHRTF->SetSamplingRate(loader.GetSamplingRate());
			const int left_ear = 0;
			const int right_ear = 1;
			// This outtermost loop iterates over HRIRs
			for (std::size_t i = 0; i < numberOfMeasurements; i++)
			{
				BRTServices::THRIRStruct hrir_value;
				double azimuth = sourcePositionsVector[array2DIndex(i, 0, numberOfMeasurements, numberOfCoordinates)];
				double elevation = GetPositiveElevation(sourcePositionsVector[array2DIndex(i, 1, numberOfMeasurements, numberOfCoordinates)]);
				//double distance = sourcePositionsVector[array2DIndex(i, 2, numberOfMeasurements, numberOfCoordinates)];

				hrir_value.leftDelay = dataDelays[specifiedDelays ? array2DIndex(i, left_ear, numberOfMeasurements, 2) : 0];
				hrir_value.rightDelay = dataDelays[specifiedDelays ? array2DIndex(i, right_ear, numberOfMeasurements, 2) : 1];
				GetData(dataMeasurements, hrir_value.leftHRIR, numberOfMeasurements, 2,  numberOfSamples, left_ear, i);
				GetData(dataMeasurements, hrir_value.rightHRIR, numberOfMeasurements, 2, numberOfSamples, right_ear, i);

				dataHRTF->AddHRIR(azimuth, elevation, std::move(hrir_value));
			}
			return true;

		}	

		bool GetCoefficients(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& data) {
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
			for (std::size_t i = 0; i < numberOfMeasurements; i++)
			{
				BRTServices::TILDStruct coefficients;
				coefficients.leftCoefs.resize(numberOfSamples);
				double azimuth = sourcePositionsVector[array2DIndex(i, 0, numberOfMeasurements, numberOfCoordinates)];
				//double elevation = GetPositiveElevation(sourcePositionsVector[array2DIndex(i, 1, numberOfMeasurements, numberOfCoordinates)]);
				double distance = sourcePositionsVector[array2DIndex(i, 2, numberOfMeasurements, numberOfCoordinates)];
								
				GetData(dataMeasurements, coefficients.leftCoefs, numberOfMeasurements, numberOfReceivers, numberOfSamples, left_ear, i);
				if (numberOfReceivers > 1) {
					coefficients.rightCoefs.resize(numberOfSamples);
					GetData(dataMeasurements, coefficients.rightCoefs, numberOfMeasurements, numberOfReceivers, numberOfSamples, right_ear, i);
				}
				
				data->AddCoefficients(azimuth, distance, std::move(coefficients));
			}
			return true;

		}

		bool GetDirectivityTF(BRTReaders::CLibMySOFALoader& loader, std::shared_ptr<BRTServices::CServicesBase>& dataSRTF) {
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
			int numberOfCoordinates = loader.getHRTF()->C;
			const unsigned int numberOfFrequencySamples = loader.getHRTF()->N;
			int numberOfReceivers = loader.getHRTF()->R;

			// Get and save TFs
									
			dataSRTF->BeginSetup(numberOfFrequencySamples);
			dataSRTF->SetSamplingRate(loader.GetSamplingRate());

			// This outtermost loop iterates over TFs
			for (std::size_t i = 0; i < numberOfReceivers; i++)
			{
				BRTServices::TDirectivityTFStruct srtf_data;
				CMonoBuffer<float> dataRealPartPI;
				CMonoBuffer <float> dataImagPartPI;
				double azimuth = receiverPositionsVector[array2DIndex(i, 0, 0, numberOfCoordinates)];
				double elevation = GetPositiveElevation(receiverPositionsVector[array2DIndex(i, 1, 0, numberOfCoordinates)]);
				
				GetDirectivityData(dataMeasurementsRealPart, dataRealPartPI, numberOfFrequencySamples, i);
				GetDirectivityData(dataMeasurementsImagPart, dataImagPartPI, numberOfFrequencySamples, i);
					
				srtf_data.realPart = dataRealPartPI;
				srtf_data.imagPart = dataImagPartPI;				

				dataSRTF->AddDirectivityTF(azimuth, elevation, std::move(srtf_data));
			}
			return true;		
		}
		
		/////////////////////////
		// AUXILAR METHODS
		/////////////////////////
		double GetPositiveElevation(double _elevation) {

			while (_elevation < 0) _elevation += 360;
			return _elevation;
		}

		void GetData(const std::vector<double>& dataIR, std::vector<float>& outIR, int numberOfMeasurements,int numberOfReceivers, int numberOfSamples, int ear, int i) {
			std::vector<float> IR(numberOfSamples, 0);

			for (std::size_t k = 0; k < numberOfSamples; k++)
			{
				const std::size_t index = array3DIndex(i, ear, k, numberOfMeasurements, numberOfReceivers, numberOfSamples);
				IR[k] = dataIR[index];
			}
			outIR = std::move(IR);
		}

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

		 const std::size_t array3DIndex(const unsigned long i, const unsigned long j, const unsigned long k, const unsigned long dim1, const unsigned long dim2, const unsigned long dim3)
		{
			return dim2 * dim3 * i + dim3 * j + k;
		}		 
	};
};
#endif 
