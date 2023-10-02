/**
* \class CLibMySOFALoader
*
* \brief Declaration of CLibMySOFALoader class
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

#ifndef _LIBMYSOFA_LOADER_
#define _LIBMYSOFA_LOADER_

#include <ostream>
#include <string>
#include <Common/ErrorHandler.hpp>
#include <third_party_libraries/libmysofa/include/mysofa.h>

namespace BRTReaders {

	class CLibMySOFALoader {

	public:

		enum class TSofaConvention { SimpleFreeFieldHRIR, SimpleFreeFieldHRSOS, FreeFieldDirectivityTF};
		const char* SofaConventioToString(TSofaConvention e) noexcept
		{
			switch (e)
			{
			case TSofaConvention::SimpleFreeFieldHRIR: return "SimpleFreeFieldHRIR";
			case TSofaConvention::SimpleFreeFieldHRSOS: return "SimpleFreeFieldHRSOS";
			case TSofaConvention::FreeFieldDirectivityTF: return "FreeFieldDirectivityTF";

			}
		}

		CLibMySOFALoader(const std::string& sofaFile) : error{ -1 } {
			int error = MySOFAInit(sofaFile);			
		}

		~CLibMySOFALoader() {
			if (hrtf && hrtf->hrtf) {
				mysofa_close(hrtf);				
			}
		}


		struct MYSOFA_EASY* get() const {
			return hrtf;
		}

		struct MYSOFA_HRTF* getHRTF() const {
			return hrtf->hrtf;
		}

		int getError() const {
			return error;
		}

		MYSOFA_ARRAY* GetDataSOS() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.SOS");
		}

		MYSOFA_ARRAY* GetDataRealDirectivity() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.Real");
		}

		MYSOFA_ARRAY* GetDataImagDirectivity() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.Imag");
		}



		bool CheckSofaConvention(TSofaConvention sofaConvention) {
			
			// To check SimpleFreeFieldHRIR we can use a method from the library 
			if (sofaConvention == TSofaConvention::SimpleFreeFieldHRIR) {
				return IsValidHRTFFile();
			}
			// To check other conventions we have to do it manually
			std::basic_string sofaConventions = mysofa_getAttribute(hrtf->hrtf->attributes, "SOFAConventions");
			if (sofaConventions != SofaConventioToString(sofaConvention))
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA convention file");
				return false;
			}
			return true;
		}

		int GetSamplingRate() {
			
			if (error == -1) return -1;
			
			try
			{				
				std::string samplingRatesUnits = mysofa_getAttribute(hrtf->hrtf->DataSamplingRate.attributes, "Units");
				std::transform(samplingRatesUnits.begin(), samplingRatesUnits.end(), samplingRatesUnits.begin(), ::tolower);
				if (samplingRatesUnits != "hertz")
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling rate units are not Herzs");
					return -1;
				}

				int samplingRateArraySize = sizeof(hrtf->hrtf->DataSamplingRate.values) / sizeof(double);
				if (samplingRateArraySize > 1) {

					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Multiple sampling rates in one SOFA is not a supported charasteristic.");
					return -1;
				}

				return hrtf->hrtf->DataSamplingRate.values[0];
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
				return -1;
			}
		}

		Common::CVector3 GetListenerView() {	
			if (error == -1) return Common::CVector3();
			Common::CVector3 _listenerView(hrtf->hrtf->ListenerView.values[0], hrtf->hrtf->ListenerView.values[1], hrtf->hrtf->ListenerView.values[2]);
			return _listenerView;
		}

		Common::CVector3 GetListenerUp() {			
			if (error == -1) return Common::CVector3();			
			Common::CVector3 _listenerUp(hrtf->hrtf->ListenerUp.values[0], hrtf->hrtf->ListenerUp.values[1], hrtf->hrtf->ListenerUp.values[2]);						
			return _listenerUp;
		}
		
		std::vector<double> GetReceiverPosition() {
			std::vector<double> _receiverPositions(hrtf->hrtf->ReceiverPosition.values, hrtf->hrtf->ReceiverPosition.values + hrtf->hrtf->ReceiverPosition.elements);
			return _receiverPositions;
		}


		void Cartesian2Spherical() {
			if (error == -1) return ;
			mysofa_tospherical(hrtf->hrtf);			
		}

		bool IsValidHRTFFile() {
			bool error = mysofa_check(hrtf->hrtf);
			if (error != MYSOFA_OK) {
				//mysofa_close(hrtf);
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid HRTF SOFA file");
				return false;
			}
			return true;
		}

	private:
		int error;
		//std::unique_ptr<struct MYSOFA_EASY, MYSOFA_HRTF_Deleter> hrtf_;		
		struct MYSOFA_EASY* hrtf;

		
		// Initialize MySOFA struct and open a sofafile
		bool MySOFAInit(const std::string& sofafile) {

			try
			{
				hrtf = (MYSOFA_EASY*)malloc(sizeof(struct MYSOFA_EASY));
				// Check file open and format				
				if (!hrtf)
				{
					SET_RESULT(RESULT_ERROR_BADALLOC, "Error trying to set the variable MYSOFA_EASY");
					return false;
				}

				hrtf->lookup = NULL;
				hrtf->neighborhood = NULL;
				hrtf->fir = NULL;
				
				hrtf->hrtf = mysofa_load(sofafile.data(), &error);
				if (!hrtf->hrtf) {
					//mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Error opening SOFA file");
					return false;
				}

				/*error = mysofa_check(hrtf->hrtf);
				if (error != MYSOFA_OK) {
					mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
					return -1;
				}*/

				//mysofa_tocartesian(hrtf->hrtf);

				/*hrtf->lookup = mysofa_lookup_init(hrtf->hrtf);
				if (hrtf->lookup == NULL) {
					error = MYSOFA_INTERNAL_ERROR;
					mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "MYSOFA INTERNAL ERROR");
					return -1;
				}
				hrtf->neighborhood = mysofa_neighborhood_init(hrtf->hrtf, hrtf->lookup);*/
				
				SET_RESULT(RESULT_OK, "SOFA file loaded");
				return true;
			}
			catch (int error)
			{
				// the description of the exception will be printed when raised
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
				return false;
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
				return false;
			}
		}

		MYSOFA_ARRAY* mysofa_getVariable(struct MYSOFA_VARIABLE* var, char* name) {
			while (var) {
				if (var->name && !strcmp(name, var->name)) {
					return var->value;
				}
				var = var->next;
			}
			return NULL;
		}
	};
};
#endif 
