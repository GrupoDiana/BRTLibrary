/**
*
* \brief Functions to handle HRTFs
*
* \date March 2016
*
* \authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, C.Garre, D.Gonzalez - Toledo, E.J.de la Rubia - Cuestas, L.Molina - Tanco ||
*Coordinated by, A.Reyes - Lecuona(University of Malaga) and L.Picinali(Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions : (additional authors / contributors can be added here)
*
* \b Project : 3DTI(3D - games for TUNing and lEarnINg about hearing aids) ||
*\b Website : http://3d-tune-in.eu/
*
* \b Copyright : University of Malaga and Imperial College London - 2018
*
* \b Licence : This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement : This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#ifndef _ILD_SOFA_READER_
#define _ILD_SOFA_READER_

#include <ostream>
#include <string>
#include <ServiceModules/ILD.hpp>
#include <Common/ErrorHandler.hpp>
#include <SOFA.h>
#include <SOFAExceptions.h>

namespace BRTReaders
{   
	class ILDSOFAReader {

	public:

		/** \brief Loads an ILD from a sofa file
		*	\param [in] path of the sofa file
		*	\param [out] listener affected by the hrtf
		*   \eh On error, an error code is reported to the error handler.
		*/
		bool CreateFromSofa(const std::string& sofafile, shared_ptr<BRTServices::CILD>& listenerILD)
		{
			if (LoadILDCoefficientsTableFromSOFA(sofafile, listenerILD))
			{
				if (listenerILD->EndSetup()) {
					return true;
				}
			}
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception creating ILD, please consider previous messages from the sofa library");
			return false;			
		}

		/** \brief Returns the sample rate in Hz in the sofa file
		*	\param [in] path of the sofa file
		*   \eh On error, an error code is reported to the error handler.
		*/
		int GetSampleRateFromSofa(const std::string& sofafile)
		{
			try
			{
				// Check file open and format
				const sofa::File theFile(sofafile);
				if (!theFile.IsValid())
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
					return -1;
				}

				// Check SOFA file type
				const sofa::SimpleFreeFieldSOS ildFile(sofafile);				
				if (!ildFile.IsValid())
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldSOS file");
					return -1;
				}
				
				// Check units of sampling rate. It must be Herzs
				sofa::Units::Type srUnits;
				ildFile.GetSamplingRateUnits(srUnits);
				if (srUnits != sofa::Units::Type::kHertz)
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling rate units are not Herzs");
					return -1;
				}

				// Get sample rate and cast to int (current type in 3DTi Toolkit core)
				double samplingRate;
				ildFile.GetSamplingRate(samplingRate);
				return (int)samplingRate;
			}
			catch (sofa::Exception& e)
			{
				// the description of the exception will be printed when raised
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
			}
			catch (std::exception& e)
			{
				std::string s("Error when reading samplerate from SOFA");
				s += e.what();
				SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
			}
			return -1;
		}

	private:

		/** \brief Loads an ILD coefficients from SOFA file and store them into CILD class
		*	\param [in] path of the sofa file
		*	\param [out] smart pointer to ILD class where the data is going to be stored
		*   \eh On error, an error code is reported to the error handler. */				
		bool LoadILDCoefficientsTableFromSOFA(const std::string& sofafile, shared_ptr<BRTServices::CILD>& listenerILD)
		{
			std::ostream& output = std::cout;
			try {

				const sofa::File theFile(sofafile);
				if (!theFile.IsValid())
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
					return false;
				}
				
				const sofa::SimpleFreeFieldSOS ildFile(sofafile);
				if (!ildFile.IsValid())
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldSOS file");
					return false;
				}
				
				SET_RESULT(RESULT_OK, "Valid SOFA file");
				listenerILD->BeginSetup();
				GetAndSaveGlobalAttributes(ildFile, listenerILD, sofafile);		// Save Global attributes
				GetAndSaveSamplingRate(ildFile, listenerILD);					// Save Sampling Rate
				GetAndSaveReceiverPosition(ildFile, listenerILD);				// Save ear positions				
				bool result = GetAndSaveData(ildFile, listenerILD);				// Save coefficients table
				
				return result;
			}
			catch (sofa::Exception& e)
			{
				// the description of the exception will be printed when raised
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
			}
			catch (std::exception& e)
			{
				std::string s("Error when creating HRTF representation");
				s += e.what();
				SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when creating HRTF representation");
			}
			return false;
		}


		// Read data from sofa struct and save into ILD class
		void GetAndSaveReceiverPosition(const sofa::SimpleFreeFieldSOS & ildFile, shared_ptr<BRTServices::CILD> listenerILD) {

			int numberOfReceivers;
			numberOfReceivers = ildFile.GetNumReceivers();
			listenerILD->SetNumberOfEars(numberOfReceivers);

			std::vector< double > receiverPos;
			ildFile.GetReceiverPosition(receiverPos);
			if (numberOfReceivers >= 1) {
				Common::CVector3 leftEarPos;
				leftEarPos.SetAxis(FORWARD_AXIS, receiverPos[0]);
				leftEarPos.SetAxis(RIGHT_AXIS, receiverPos[1]);
				leftEarPos.SetAxis(UP_AXIS, receiverPos[2]);
				listenerILD->SetEarPosition(Common::T_ear::LEFT, leftEarPos);

				if (numberOfReceivers >= 2) {
					Common::CVector3 rightEarPos;
					rightEarPos.SetAxis(FORWARD_AXIS, receiverPos[3]);
					rightEarPos.SetAxis(RIGHT_AXIS, receiverPos[4]);
					rightEarPos.SetAxis(UP_AXIS, receiverPos[5]);
					listenerILD->SetEarPosition(Common::T_ear::RIGHT, rightEarPos);
				}
			}
		}

		// Read data from sofa struct and save into ILD class
		void GetAndSaveSamplingRate(const sofa::SimpleFreeFieldSOS& ildFile, shared_ptr<BRTServices::CILD> listenerILD) {
			double samplingRate;
			ildFile.GetSamplingRate(samplingRate);
			listenerILD->SetFileSamplingRate((int)samplingRate);
		}

		// Read data from sofa struct and save into ILD class
		void GetAndSaveGlobalAttributes(const sofa::SimpleFreeFieldSOS& ildFile, shared_ptr<BRTServices::CILD> listenerILD, std::string _sofafile) {
			sofa::Attributes _attributes;
			ildFile.GetGlobalAttributes(_attributes);
			std::string _title = _attributes.Get(sofa::Attributes::Type::kTitle);
			std::string _description = _attributes.Get(sofa::Attributes::Type::kComment);
			listenerILD->SetFileName(_sofafile);
			listenerILD->SetFileTitle(_title);
			listenerILD->SetFileDescription(_description);
		}

		// Read data from sofa struct and save into ILD class
		bool GetAndSaveData(const sofa::SimpleFreeFieldSOS& ildFile, shared_ptr<BRTServices::CILD> listenerILD) {
			
			std::vector< std::size_t > dims;
			ildFile.GetVariableDimensions(dims, "SourcePosition");
			if (dims.size() != 2)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA File gives invalid number of dimensions for Source Positions"); 				
				return false;
			}

			std::vector< double > pos;
			pos.resize(dims[0] * dims[1]);
			ildFile.GetSourcePosition(&pos[0], dims[0], dims[1]); // dims[0] is the number of positions (187, say), dims[1] is the dimensions of the positions (3).
			const unsigned int nMeasurements = (unsigned int)ildFile.GetNumMeasurements();  // The number of HRIRs (187, say)
			if (dims[0] != nMeasurements)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of source positions and measurements");
				return false;
			}

			std::vector< double > data;
			ildFile.GetDataSOS(data);			
			const unsigned int nCoefficients = (unsigned int)ildFile.GetNumDataSamples();   // For example: 12 coefs
			const unsigned int numberOfReceivers = (unsigned int)ildFile.GetNumReceivers();

			if (numberOfReceivers == 0) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of receivers and coefficients");
				return false;
			}
			else if (numberOfReceivers == 1) {
				SET_RESULT(RESULT_WARNING, "This ILD SOFA file does not contain coefficients for each ear. Therefore, the same filters will be used for both ears.");

			}
			
			// This outtermost loop iterates over Cofficients
			for (std::size_t i = 0; i < nMeasurements; i++) // or for( std::size_t i = 0; i < dims[0]; i++ ), should be the same.
			{
				TILDStruct coefficients;
				coefficients.leftCoefs.resize(nCoefficients);

				double azimuth = pos[array2DIndex(i, 0, nMeasurements, dims[1])];
				//double elevation = pos[array2DIndex(i, 1, nMeasurements, dims[1])];
				double distance = pos[array2DIndex(i, 2, nMeasurements, dims[1])];

				//while (elevation < 0) elevation += 360; // TODO: check who should do this
				


				const int left_ear = 0;
				//hrir_value.leftDelay = delays[specifiedDelays ? array2DIndex(i, left_ear, nMeasurements, 2) : 0];
				for (std::size_t k = 0; k < nCoefficients; k++)
				{
					const std::size_t index = array3DIndex(i, left_ear, k, nMeasurements, numberOfReceivers, nCoefficients);
					//hrir_value.leftHRIR[k] = data[index];
					coefficients.leftCoefs[k] = data[index];
				}
				if (numberOfReceivers > 1) {
					coefficients.rightCoefs.resize(nCoefficients);
					const int right_ear = 1;
					//hrir_value.rightDelay = delays[specifiedDelays ? array2DIndex(i, right_ear, nMeasurements, 2) : 1];
					for (std::size_t k = 0; k < nCoefficients; k++)
					{
						const std::size_t index = array3DIndex(i, right_ear, k, nMeasurements, numberOfReceivers, nCoefficients);
						coefficients.rightCoefs[k] = data[index];
					}
				}

				listenerILD->AddCoefficients(azimuth, distance, std::move(coefficients));
			}			
			return true;
		}

		// Calculate sofa 2D matrix index
		const std::size_t array2DIndex(const unsigned long i, const unsigned long j, const unsigned long dim1, const unsigned long dim2) {
			return dim2 * i + j;
		}

		// Calculate sofa 3D matrix index
		const std::size_t array3DIndex(const unsigned long i, const unsigned long j, const unsigned long k, const unsigned long dim1, const unsigned long dim2,	const unsigned long dim3) {
			return dim2 * dim3 * i + dim3 * j + k;
		}
	};
};

#endif 
