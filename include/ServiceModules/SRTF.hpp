/**
* \class CSRTF
*
* \brief Declaration of CSRTF class interface to store directivity data
* \version 
* \date	May 2023
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
* \b Licence: 
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CSRTF_H_
#define _CSRTF_H_

#include <unordered_map>
#include <Common/ErrorHandler.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>

#ifndef DEFAULT_SRTF_RESAMPLING_STEP
#define DEFAULT_SRTF_RESAMPLING_STEP 5
#endif


namespace BRTServices
{
	
	struct TDirectivityInterlacedTFStruct {
		CMonoBuffer<float> data;
	};

	/** \brief Type definition for the SRTF table */
	typedef std::unordered_map<orientation, BRTServices::TDirectivityTFStruct> T_SRTFTable;
	typedef std::unordered_map<orientation, BRTServices::TDirectivityInterlacedTFStruct> T_SRTFInterlacedDataTable;


	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CSRTF : public CServicesBase
	{
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CSRTF()
			:resamplingStep{ DEFAULT_SRTF_RESAMPLING_STEP }, SRTFloaded{ false }, setupSRTFInProgress{ false }, aziMin{ DEFAULT_MIN_AZIMUTH }, aziMax{ DEFAULT_MAX_AZIMUTH },
			eleMin{ DEFAULT_MIN_ELEVATION }, eleMax{ DEFAULT_MAX_ELEVATION }, sphereBorder{ SPHERE_BORDER }, epsilon_sewing{ EPSILON_SEWING }, samplingRate{ -1 }
		{}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetTitle(std::string _title) {
			title = _title;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetDatabaseName(std::string _databaseName) {
			databaseName = _databaseName;
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
		
		/** \Start a new SRTF configuration
		*	\param [in] directivityTFPartLength number of samples in the frequency domain (size of the real part or the imaginary part)
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t directivityTFPartLength){
			//Update parameters			
			eleNorth = GetPoleElevation(TPole::north);
			eleSouth = GetPoleElevation(TPole::south);

			if (directivityTFPartLength != globalParameters.GetBufferSize()) //
			{
				SET_RESULT(RESULT_ERROR_BADSIZE, "Number of frequency samples (N) in SOFA file is different from Buffer Size");
			}

			directivityTF_length = 4.0 * directivityTFPartLength; //directivityTF will store the Real and Img parts interlaced
			directivityTF_numberOfSubfilters = 1;

			//Clear every table			
			t_SRTF_DataBase.clear();
			t_SRTF_Resampled.clear();
			 
			//Change class state
			setupSRTFInProgress = true;
			SRTFloaded = false;

			SET_RESULT(RESULT_OK, "HRTF Setup started");
		}

		/** \brief Stop the SRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupSRTFInProgress) {
				if (!t_SRTF_DataBase.empty())
				{
					//SRTF Resampling methdos
					//CalculateHRIR_InPoles(resamplingStep);
					//FillOutTableOfAzimuth360(resamplingStep);
					//FillSphericalCap_HRTF(gapThreshold, resamplingStep);
					CalculateResampled_SRTFTable(resamplingStep);
					auto stepVector = CalculateStep();
					//CalculateExtendUpTo2PI();

					//Setup values
					setupSRTFInProgress = false;
					SRTFloaded = true;

					SET_RESULT(RESULT_OK, "SRTF Table completed succesfully");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_SRTF_DataBase map has not been set");
				}
			}
			return false;
		}


		/** \brief Set the step for the SRTF resampled table
		*	\param [in] Step to create the resampled table
		*/
		void SetResamplingStep(int _resamplingStep) {
			resamplingStep = _resamplingStep;
		}
		
		/** \brief Ask for the step used to create the resampled table
		*	\retval Step used to create the resampling step
		*/
		int GetResamplingStep() {
			return resamplingStep;
		}

		/** \brief Set the sampling rate for the SRTF
		*	\param [in] sampling rate
		*/
		void SetSamplingRate(int _samplingRate) {
			samplingRate = _samplingRate;
		}

		/** \brief Ask for the sampling rate 
		*	\retval sampling step
		*/
		int GetSamplingRate() {
			return samplingRate;
		}

		/** \brief Get the number of samples of the Directivity TF
		*	\retval Length of the TF with the Real and Img parts interlaced
		*/
		int GetDirectivityTFLength() { return directivityTF_length; }

		/** \brief Get the number of subfilters if the TF is partitioned
		*	\retval Number of partitions or 1 if there is no partition
		*/
		int GetDirectivityTFNumOfSubfilters() { return directivityTF_numberOfSubfilters; }

		/** \brief Add a new TF to the SRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newDirectivityTF DirectivityTF data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF)
		{
			if (setupSRTFInProgress) {
				auto returnValue = t_SRTF_DataBase.emplace(orientation(_azimuth, _elevation), std::forward<TDirectivityTFStruct>(DirectivityTF));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
		}

		/** \brief Calculate the regular resampled-table using an interpolation method
		*	\param [in] _resamplingStep step for both azimuth and elevation
		*   \eh Warnings may be reported to the error handler.
		*/
		void CalculateResampled_SRTFTable(int _resamplingStep)
		{
			// COPY the loaded table:
			for (auto& it : t_SRTF_DataBase){
				TDirectivityInterlacedTFStruct interlacedData;
				// Extend to 2PI real part
				CMonoBuffer<float> dataRealPart2PI;				
				CalculateTFRealPartTo2PI(it.second.realPart , dataRealPart2PI);								
				// Extend to 2PI imag part
				CMonoBuffer<float> dataImagPart2PI;				
				CalculateTFImagPartTo2PI(it.second.imagPart, dataImagPart2PI);				 				
				// Invert sign of imag part
				CalculateTFImagPartToBeCompatibleWithOouraFFTLibrary(dataImagPart2PI);
				// Interlaced real and imag part
				interlacedData.data.Interlace(dataRealPart2PI, dataImagPart2PI);
				//Add data to the Resampled Table
				t_SRTF_Resampled.emplace(it.first, interlacedData);
			}
			//
			//int numOfInterpolatedHRIRs = 0;

			////Resample Interpolation Algorithm
			//for (int newAzimuth = azimuthMin; newAzimuth < azimuthMax; newAzimuth = newAzimuth + _resamplingStep)
			//{
			//	for (int newElevation = elevationMin; newElevation <= elevationNorth; newElevation = newElevation + _resamplingStep)
			//	{
			//		if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
			//	}

			//	for (int newElevation = elevationSouth; newElevation < elevationMax; newElevation = newElevation + _resamplingStep)
			//	{
			//		if (CalculateAndEmplaceNewDirectivityTF(newAzimuth, newElevation)) { numOfInterpolatedHRIRs++; }
			//	}
			//}
			//SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
		}


		/// <returns></returns>
		
		
		/** \brief  Calculate the DirectivityTF using the an interpolation Method 
		*	\param [in] _azimuth orientation of the sound source (relative to the listener)
		*	\param [in] _elevation orientation of the sound source (relative to the listener)
		*	\retval interpolatedDTFs: true if the Directivity TF has been calculated using the interpolation
		*   \eh Warnings may be reported to the error handler.
		*/
		bool CalculateAndEmplaceNewDirectivityTF(float _azimuth, float _elevation) {
			TDirectivityInterlacedTFStruct interpolatedHRIR;
			bool bDirectivityTFInterpolated = false;
			auto it = t_SRTF_DataBase.find(orientation(_azimuth, _elevation));
			if (it != t_SRTF_DataBase.end()){				
				interpolatedHRIR.data.Interlace(it->second.realPart, it->second.imagPart);	// Interlaced
				//Fill out Directivity TF from the original Database	
				auto returnValue =  t_SRTF_Resampled.emplace(orientation(_azimuth, _elevation), std::forward<TDirectivityInterlacedTFStruct>(interpolatedHRIR));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing DirectivityTF into t_SRTF_Resampled map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
			else
			{
				SET_RESULT(RESULT_WARNING, "Resampling SRTF: cannot find Directivity TF in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "], in the database table");
			}
			return bDirectivityTFInterpolated;
		}

		
		/** \brief  Calculate the azimuth step for each orientation
		*	\retval unordered map with all the orientations of the resampled table 
		*   \eh Warnings may be reported to the error handler.
		*/
		std::unordered_map<orientation, float> CalculateStep()
		{
			std::unordered_map<orientation, float> stepVector;
			float elevation, aziStep, diff = 360, actual_ele = -1.0f;
			int secondTime = 0;
			std::vector<orientation> orientations;

			// Create a vector with all the orientations of the Database
			orientations.reserve(t_SRTF_DataBase.size());
			for (auto& itr : t_SRTF_DataBase)
			{
				orientations.push_back(itr.first);
			}
			//  Sort orientations of the DataBase with a lambda function in sort.
			std::sort(orientations.begin(), orientations.end(), [](orientation a, orientation b) {return (a.elevation < b.elevation); });


			for (int ori = 0; ori< orientations.size(); ori++)
			{
				// Maybe stop in each different elevation and make the difference between the start azimuth, 0, and the next azimuth in this elevation
				// with this form, we could save a vector like this [aziStep elevation]

				elevation = orientations[ori].elevation;

				if (actual_ele != elevation)
				{
					for (int ele = ori+1; ele < orientations.size(); ele++)
					{
						if (orientations[ele].elevation != elevation || orientations[ele] == orientations.back())
						{	
							if (diff == 360) { diff = 0; }
							stepVector.emplace(orientation(0, elevation), diff);
							diff = 360;
							break;
						}
						if (abs(orientations[ele].azimuth - orientations[ele -1].azimuth) < diff) { 
							diff = abs(orientations[ele].azimuth - orientations[ele - 1].azimuth); 
						}
					}
					actual_ele = elevation;

				}
			}
			return stepVector;
		}

	
		/** \brief  Get the Directivity TF of the nearest point to  the given one
		*	\param [in] _azimuth orientation of the sound source (relative to the listener)
		*	\param [in] _elevation orientation of the sound source (relative to the listener)
		* *	\param [in] _stepsMap map that contains orientations of the grid
		*	\retval  Directivity TF with the Real and Imag part interlaced
		*/
		const TDirectivityInterlacedTFStruct GetDirectivityTF(float _azimuth, float _elevation, std::unordered_map<orientation, float> _stepsMap) const {

			TDirectivityInterlacedTFStruct newSRIR;
			auto it0 = t_SRTF_Resampled.find(orientation(_azimuth, _elevation));
			if (it0 != t_SRTF_Resampled.end()) {
				newSRIR.data = it0->second.data;
			}
			else {

				// HARCODE ELEVATION STEP TO 10
				int eleStep = 10;

				float aziCeilBack, aziCeilFront, aziFloorBack, aziFloorFront;
				int idxEle = ceil(_elevation / eleStep);
				float eleCeil = eleStep * idxEle;
				float eleFloor = eleStep * (idxEle - 1);

				eleCeil = CheckLimitsElevation_and_Transform(eleCeil);										//			   Back	  Front
				eleFloor = CheckLimitsElevation_and_Transform(eleFloor);									//	Ceil		A		B

				auto stepItr = _stepsMap.find(orientation(0, eleCeil));										//	Floor		D		C
				float aziStepCeil = stepItr->second;
																										
				CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
				// azimuth values passed by reference

				auto stepIt = _stepsMap.find(orientation(0, eleFloor));
				float aziStepFloor = stepIt->second;							

				CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth); 

				// Mid Point of a trapezoid can be compute by averaging all azimuths
				float pntMid_azimuth = (aziCeilBack + aziCeilFront + aziFloorBack + aziFloorFront) / 4;
				float pntMid_elevation = (eleCeil - eleStep * 0.5f);

				// compute eleCeil being 360 to find triangles at border
				//eleCeil = eleStep * idxEle;

				if (_azimuth >= pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//Second quadrant
						auto it = t_SRTF_Resampled.find(orientation(aziCeilFront, eleCeil));
						newSRIR.data = it->second.data;

					}
					else if (_elevation < pntMid_elevation)
					{
						//Forth quadrant
						auto it = t_SRTF_Resampled.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else if (_azimuth < pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//First quadrant
						auto it = t_SRTF_Resampled.find(orientation(aziCeilBack, eleCeil));
						newSRIR.data = it->second.data;
					}
					else if (_elevation < pntMid_elevation) {
						//Third quadrant
						auto it = t_SRTF_Resampled.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else { return newSRIR = {}; }
			}

			//SET_RESULT(RESULT_OK, "CalculateHRIR_InterpolationMethod completed succesfully");
			return newSRIR;
		}

		/** \brief  Check limit values for elevation and transform to the desired intervals
		*	\retval elevation value within the desired intervals
		*/
		float CheckLimitsElevation_and_Transform(float elevation)const
		{
			if (elevation < 0) { elevation = elevation + 360; }
			if (elevation >= 360) { elevation = elevation - 360; }
			return elevation;
		}

		/** \brief  Check limit values for elevation and transform to the desired intervals
		*	\retval azimuth value within the desired intervals
		*/
		float CheckLimitsAzimuth_and_Transform(float azimuth)const
		{
			if (azimuth < 0) { azimuth = azimuth + 360; }
			else if (azimuth > 360) { azimuth = azimuth - 360; }
			return azimuth;
		}

		void CalculateAzimuth_BackandFront(float& aziBack, float& aziFront, float aziStep, float _azimuth)const
		{

			int idxAzi = ceil(_azimuth / aziStep);

			aziFront = idxAzi * aziStep;
			aziBack = (idxAzi - 1) * aziStep;

			aziFront = CheckLimitsAzimuth_and_Transform(aziFront);
			aziBack = CheckLimitsAzimuth_and_Transform(aziBack);
		}

	private:
		std::string title;
		std::string databaseName;
		std::string fileName;
		int samplingRate;
		int resamplingStep;
		bool SRTFloaded;
		bool setupSRTFInProgress;
		int32_t directivityTF_length;	
		int32_t directivityTF_numberOfSubfilters;	
		
		T_SRTFTable					t_SRTF_DataBase;
		T_SRTFInterlacedDataTable	t_SRTF_Resampled;

		Common::CGlobalParameters globalParameters;

		int aziMin, aziMax, eleMin, eleMax, eleNorth, eleSouth;	// Variables that define limits of work area
		float sphereBorder;
		float epsilon_sewing;
		enum class TPole { north, south };
		
		///////////////////
		///// METHODS
		///////////////////

		/** \brief Get Pole Elevation
		*	\param [in] Tpole var that indicates of which pole we need elevation
		*   \eh  On error, an error code is reported to the error handler.
		*/
		int GetPoleElevation(TPole _pole)const
		{
			if (_pole == TPole::north) { return ELEVATION_NORTH_POLE; }
			else if (_pole == TPole::south) { return ELEVATION_SOUTH_POLE; }
			else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get a non-existent pole");
				return 0;
			}
		}

		/** \brief Transform Real part of the Directivity TF to 2PI
		*	\param [in] inBuffer Samples with the original Real part
		* *	\param [in] outBuffer Samples transformed to 2PI
		*/
		void CalculateTFRealPartTo2PI(const CMonoBuffer<float>& inBuffer, CMonoBuffer<float>& outBuffer) {
			outBuffer.reserve(inBuffer.size() * 2);
			outBuffer.insert(outBuffer.begin(), inBuffer.begin(), inBuffer.end());
			outBuffer.insert(outBuffer.end(), 0);
			outBuffer.insert(outBuffer.end(), inBuffer.rbegin(), inBuffer.rend() - 1);
		}

		/** \brief Transform Imag part of the Directivity TF to 2PI
		*	\param [in] inBuffer Samples with the original IMAG part
		* *	\param [in] outBuffer Samples transformed to 2PI
		*/		
		void CalculateTFImagPartTo2PI(const CMonoBuffer<float>& inBuffer, CMonoBuffer<float>& outBuffer) {
			outBuffer.reserve(inBuffer.size() * 2);
			outBuffer.insert(outBuffer.begin(), inBuffer.begin(), inBuffer.end());
			outBuffer.insert(outBuffer.end(), 0);

			CMonoBuffer<float> temp;
			temp.reserve(inBuffer.size() - 1);
			temp.insert(temp.begin(), inBuffer.begin() + 1, inBuffer.end());
			std::transform(temp.begin(), temp.end(), temp.begin(), [](float v) -> float { return -v; });

			outBuffer.insert(outBuffer.end(), temp.rbegin(), temp.rend());
		}

		/** \brief Invert the Imag part to prepare the data in the way that the Ooura Library is expecting to do the complex multiplication
		*	\param [in] buffer with the samples to be inverted (multiplied by -1)
		*/
		void CalculateTFImagPartToBeCompatibleWithOouraFFTLibrary(CMonoBuffer<float>& buffer) {
			for (int i = 0; i < buffer.size(); i++) {
				buffer[i] = buffer[i] * -1;
			}
		}

	};
}
#endif
