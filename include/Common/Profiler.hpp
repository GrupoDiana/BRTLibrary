/**
* \class CProfiler
*
* \brief Declaration of CProfiler class interface.
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
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/ 

#ifndef _CPROFILER_H_
#define _CPROFILER_H_

/*! \file */

// TO DO: Move this to a global .h with platform definitions
#ifndef PLATFORM_DEFINED
#if defined(_WIN32) 
#define PLATFORM_WIN32
#elif defined(_WIN64)
#define PLATFORM_WIN64
#elif defined(__ANDROID_API__)
#define PLATFORM_ANDROID
#endif
#define PLATFORM_DEFINED
#endif

#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
#include "Windows.h"
#endif

#ifdef PLATFORM_ANDROID
#define CLOCK_SOURCE CLOCK_PROCESS_CPUTIME_ID
//#define CLOCK_SOURCE CLOCK_MONOTONIC_RAW
// CLOCK_THREAD_CPUTIME_ID could be used, but it is not correctly supported by all Linux kernels (from 2.6.12?)
#ifndef DEBUG_ANDROID
#define DEBUG_ANDROID(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "AndroidProject1.NativeActivity", __VA_ARGS__))
#endif
#endif


#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <Common/ErrorHandler.hpp>

/** \brief Macro used for easy access to profiler singleton
*/
#define BRT_PROFILER CProfiler::Instance()

//
// Defines that you may want to use
//

// Time Units presets
#define UNITS_TICKS			0	///< Time units as processor ticks
#define UNITS_MICROSECONDS	1	///< Time units as microseconds
#define UNITS_NANOSECONDS	2	///< Time units as nanoseconds

// Keep only one of these two defines. Don't change if you don't know the meaning!!
#define PROFILER_USE_VECTOR
//#define PROFILER_USE_ARRAY		// TO DO: Array implementation is NOT tested


//
// Defines you will never need to change
//

// Some values used by resolution presets
#define MICROSECONDS_IN_ONE_SECOND	1000000
#define NANOSECONDS_IN_ONE_SECOND	1000000000

// This ugly c-like use of arrays is recommended for minimum overhead of the profiler:
#ifdef PROFILER_USE_ARRAY
#define MAX_PROFILER_SAMPLES 100000
#else
#define DEFAULT_PROFILER_SAMPLES 1000	///< Maximum number of samples stored in profiler. This ugly c-like use of arrays is recommended for minimum overhead of the profiler (avoids dynamic memory allocation)
#endif
// Instead, we use c++ vectors at the cost of a small overhead
// This overhead can be significant for array sizes <100 or >100000
// Please, see: http://assoc.tumblr.com/post/411601680/performance-of-stl-vector-vs-plain-c-arrays


/// Type redefinition for 64 bit integers
typedef long long TInt64;

///////////////////////////////////////////////////////////////////////////////

namespace Common {

	/** \brief Class for storing time measures, with units
	*/
	class CTimeMeasure
	{
		// PUBLIC METHODS:
	public:

		/** \brief Set time units
		*	\details If this method is not called, default units are \link UNITS_TICKS \endlink
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		void SetUnits(unsigned int unitsPreset)
		{
			units = unitsPreset;
		}

		/** \brief Set only the value of time measure
		*	\param [in] _value value of time measure
		*   \eh Nothing is reported to the error handler.
		*/
		void SetValue(TInt64 _value)
		{
			value = _value;
		}

		/** \brief Set value of time measure, specifying time units
		*	\param [in] _value value of time measure
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		void SetValue(TInt64 _value, unsigned int unitsPreset)
		{
			SetUnits(unitsPreset);
			SetValue(_value);
		}

		/** \brief Get time measure value
		*	\retval value time measure value
		*   \eh Nothing is reported to the error handler.
		*/
		TInt64 GetValue() const
		{
			return value;
		}

		/** \brief Get time units
		*	\retval units one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		unsigned int GetUnits() const
		{
			return units;
		}

		/** \brief Get time measure value and units in std::string format
		*/
		std::string to_string() {
			std::string unitShortName;

			// Not sure of the cost of always creating strings for each measure... for this reason, I prefer hardcoding these names here
			switch (GetUnits())
			{
			case UNITS_MICROSECONDS:
				//unitShortName = "us";
				unitShortName = "\xe6s";
				break;
			case UNITS_NANOSECONDS:
				unitShortName = "ns";
				break;
			case UNITS_TICKS:
			default:
				unitShortName = " ticks";
				break;
			}

			std::string out;
			out = std::to_string(GetValue()) + unitShortName;
			return out;
		}

		/** \brief Convert the value of a measure from ticks to the units set, depending on tick frequency
		*	\param [in] tickFrequency tick frequency of the processor
		*	\retval value value of measure converted from ticks to other units
		*	\pre Time units other than UNITS_TICKS were previously set for this time measure (See \link SetUnits \endlink)
		*   \eh Nothing is reported to the error handler.
		*/
		CTimeMeasure FromTicksToUnits(TInt64 tickFrequency) const
		{
			CTimeMeasure result;
			result.units = units;

			TInt64 unitsPerSecond;
			switch (units)
			{
			case UNITS_MICROSECONDS:
				unitsPerSecond = MICROSECONDS_IN_ONE_SECOND;
				break;
			case UNITS_NANOSECONDS:
				unitsPerSecond = NANOSECONDS_IN_ONE_SECOND;
				break;
			case UNITS_TICKS:
			default:
				SET_RESULT(RESULT_WARNING, "Conversion from ticks to ticks in time measure; set units first");
				result.SetInvalid();
				return result;
				break;
			}

			SET_RESULT(RESULT_OK, "Conversion from ticks to units was succesfull");
			result.value = value * unitsPerSecond;
			result.value /= tickFrequency;

			return result;
		}

		/** \brief Substraction of two time measures
		*/
		CTimeMeasure operator-(const CTimeMeasure _rightHand) const
		{
			CTimeMeasure result;
			result.value = value - _rightHand.value;
			result.units = units;
			return result;
		}

		/** \brief Addition of two time measures
		*/
		CTimeMeasure operator+(const CTimeMeasure _rightHand) const
		{
			CTimeMeasure result;
			result.value = value + _rightHand.value;
			result.units = units;
			return result;
		}

		/** \brief Mark this time measure as invalid (it does not contains an actual time measure)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetInvalid()
		{
			value = -1;
		}

		/** \brief Returns false if this was marked as invalid or if this has a negative value
		*	\retval isvalid false if this was marked as invalid or if this has a negative value
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsValid() const
		{
			return (value >= 0);
		}

		/// ATTRIBUTES:
	private:
		TInt64 value;			// Measure as a 64 bit integer
		unsigned int units;		// Units according to presets
	};

	/** \brief Stream output of profiler time measures
	*/
	inline std::ostream & operator<<(std::ostream & out, const CTimeMeasure & t)
	{
		std::string unitShortName;

		// Not sure of the cost of always creating strings for each measure... for this reason, I prefer hardcoding these names here
		switch (t.GetUnits())
		{
		case UNITS_MICROSECONDS:
			//unitShortName = "us";
			unitShortName = "\xe6s";
			break;
		case UNITS_NANOSECONDS:
			unitShortName = "ns";
			break;
		case UNITS_TICKS:
		default:
			unitShortName = " ticks";
			break;
		}

		out << t.GetValue() << unitShortName;
		return out;
	}


	/******************************************************************************/
	/******************************************************************************/

	/** \brief Type definition for profiler sample type
	*/
	enum TSampleType {
		ST_RELATIVE,	///< Relative sample (time elapsed from previous sample)
		ST_ABSOLUTE		///< Absolute sample (as read from SO)
	};

	/** \brief Class with data sets for multi-sample (periodical) profiling
	*/
	class CProfilerDataSet
	{
		// PUBLIC METHODS:
	public:

		/** \brief Default constructor
		*	\details By default, sets sample type to \link ST_RELATIVE \endlink and dataset size to \link DEFAULT_PROFILER_SAMPLES \endlink
		*/
		CProfilerDataSet()
		{
			dataSize = 0;
			sampleType = TSampleType::ST_RELATIVE;
			sampling = false;

			relativeStart.SetInvalid();
			isAutomatic = false;

#ifdef PROFILER_USE_ARRAY
			maxDataSize = DEFAULT_PROFILER_SAMPLES;
#else
			SetMaximumSize(DEFAULT_PROFILER_SAMPLES);
#endif
		}

#ifdef PROFILER_USE_VECTOR
		/** \brief Set maximum size of dataset
		*	\details Set maximum number of samples before sampling starts. Static memory must be used to guarantee deterministic time sampling
		*/
		void SetMaximumSize(long _maxDataSize)
		{
			maxDataSize = _maxDataSize;

			try
			{
				samples.reserve(maxDataSize);
				SET_RESULT(RESULT_OK, "Maximum size for profiler data set succesfully set");
			}
			catch (std::bad_alloc const&)
			{
				SET_RESULT(RESULT_ERROR_BADALLOC, "Trying to allocate too much memory for profiler data set");
			}
		}
#endif

		/** \brief Set sample type for data set
		*	\param [in] _sampleType sample type for data set
		*/
		void SetSampleType(TSampleType _sampleType)
		{
			//SET_RESULT(RESULT_OK, "Sample type for profiler data set succesfully set");
			sampleType = _sampleType;
		}

		/** \brief Write dataset into a stream with data converted to proper time units
		*	\param [in] out output stream to write in
		*	\param [in] tickFrequency tick frequency of processor
		*	\pre This may cause an important overhead. Use only AFTER sampling ends!
		*/
		void WriteToStream(std::ostream & out, TInt64 tickFrequency) const
		{
			// Generic implementation for both types of data sets (array and vector)

			// Check errors
			if (dataSize <= 0)
			{
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to read a profiler data set which was not previously sampled");
				return;
			}
			else if (sampling)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Writing a profiler data set to file breaks determinism while sampling. Please, end sampling first");
				return;
			}
			else
				SET_RESULT(RESULT_OK, "Profiler data set written to stream succesfully");

			// Write samples
			if (sampleType == TSampleType::ST_ABSOLUTE)
			{
				for (int i = 1; i < dataSize; i++)
				{
					out << samples[i].FromTicksToUnits(tickFrequency).GetValue() << std::endl;
				}
			}
			else	// RELATIVE SAMPLES
			{
				for (int i = 1; i < dataSize; i++)
				{
					out << samples[i].GetValue() << std::endl;
				}
			}
		}

		/** \brief Write dataset into a file with data converted to proper time units
		*	\param [in] fileName name of file to write in
		*	\param [in] tickFrequency tick frequency of processor
		*	\pre This may cause an important overhead. Use only AFTER sampling ends!
		*/
		void WriteToFile(std::string fileName, TInt64 tickFrequency) const
		{
			// TO DO: error handler
			std::ofstream dataSetFile;
			dataSetFile.open(fileName);
			WriteToStream(dataSetFile, tickFrequency);
			dataSetFile.close();
			SET_RESULT(RESULT_WARNING, "Profiler wrote dataset to file" + fileName);
		}

		/** \brief Return if sampling process is active
		*	\retval issampling true if sampling process has started but not finished
		*/
		bool IsSampling() const
		{
			// We avoid using error handler while succesfully profiling, to reduce overhead
			//SET_RESULT(RESULT_OK, "");
			return sampling;
		}

		/** \brief Get number of samples already stored
		*	\retval current number of stored samples
		*/
		long GetCurrentSize() const
		{
			return dataSize;
		}

		/** \brief Switch on/off automatic write to file
		*	\details Will write all data to file when enough samples are acquired. By default, this feature is switched off, unless this method is explictly called
		*	\param [in] filename name of the file to write in
		*	\param [in] nSamples number of samples to acquire before starting write to file
		*	\param [in] tickFrequency tick frequency of processor
		*	\param [in] setAutomatic switch on/off automatic write (default, true)
		*/
		void SetAutomaticWrite(std::string filename, long nSamples, TInt64 tickFrequency, bool setAutomatic = true)
		{
			SET_RESULT(RESULT_OK, "Automatic write to file for profiler data set succesfully set");
			automaticFileName = filename;
			nAutomaticSamples = nSamples;
			automaticTickFrequency = tickFrequency;
			isAutomatic = setAutomatic;
		}

		// Currently not in use
		void ComputeStatistics()
		{
			// TO DO: IF WE WANT TO USE THIS METHOD, WE SHOULD GIVE ANY OUTPUT OF THESE LOCAL VARS!!!!!
			CTimeMeasure worst;
			CTimeMeasure average;
			CTimeMeasure deviation;
			//

			// Init statistics
			worst.SetValue(0);
			average.SetValue(0);
			deviation.SetValue(0);

			// Average and worst case
			for (int i = 0; i < dataSize; i++)
			{
				// Check for worst
				if (samples[i].GetValue() > worst.GetValue())
					worst = samples[i];

				// Add to average
				average = average + samples[i];
			}
			// Divide average by number of samples
			average.SetValue(average.GetValue() / dataSize);

			// Standard deviation				
			for (int i = 0; i < dataSize; i++)
			{
				// Add one sample to deviation
				TInt64 difference = samples[i].GetValue() - average.GetValue();
				deviation.SetValue(deviation.GetValue() + difference * difference);
			}
			// Square root of summation
			deviation.SetValue(std::sqrt(deviation.GetValue() / dataSize));
		}



		// METHODS USED INTERNALLY BY THE PROFILER FOR BUILDING DATASET:		
		void Start()								// Used internally by profiler
		{
			SET_RESULT(RESULT_OK, "Profiler data set started succesfully");
			dataSize = 0;
			sampling = true;

			if (sampleType == TSampleType::ST_RELATIVE)
				relativeStart.SetInvalid();

			// vector.clear is supposed not to affect capacity (only size), but in my own tests capacity is reset!!!!!
#ifdef PROFILER_USE_VECTOR
			long maxSizeBackup = maxDataSize;
			samples.clear();
			SetMaximumSize(maxSizeBackup);
#endif
		}


		void AddSample(CTimeMeasure sample)		// Used internally by profiler
		{
			if (sampling)
			{
				if (dataSize < maxDataSize)
				{
#if defined(PROFILER_USE_ARRAY)
					samples[dataSize] = sample;
#elif defined(PROFILER_USE_VECTOR)
					samples.push_back(sample);
#endif				
					dataSize++;

					if (sampleType == TSampleType::ST_RELATIVE)
						relativeStart.SetInvalid();

					if (isAutomatic)
					{
						if (dataSize >= nAutomaticSamples)
							End();
					}
					// We avoid using error handler while succesfully profiling, to reduce overhead
					//SET_RESULT(RESULT_OK, "Sample added succesfully to profiler data set.");
				}
				else
					SET_RESULT(RESULT_WARNING, "Profiler data set is full. New samples are being ignored");
			}
			else
				SET_RESULT(RESULT_WARNING, "Adding samples to a profiler data set which has not started sampling");
		}

		void End()									// Used internally by profiler
		{
			// error handler: trust in WriteToFile for setting result 
			sampling = false;
			if (isAutomatic)
				WriteToFile(automaticFileName, automaticTickFrequency);
		}

		CTimeMeasure GetRelativeStart() const		// Used internally by profiler
		{
			return relativeStart;
		}
		void SetRelativeStart(CTimeMeasure _time)	// Used internally by profiler
		{
			relativeStart = _time;
		}


	// ATTRIBUTES:
	private:
		TSampleType sampleType;							// Type of samples in the data set (absolute, relative) 
		CTimeMeasure relativeStart;						// Start time for relative samples
		bool sampling;									// True if dataset is being sampled
		long maxDataSize;								// Maximum number of samples that can be stored in data set
		long dataSize;									// Number of samples actually stored in data set		

		// Automatic write to file
		std::string automaticFileName;		// File name for automatic writing data set to file when full or after filling nSamples
		long nAutomaticSamples;				// Number of samples to store in data set before automatic write to file
		TInt64 automaticTickFrequency;		// Remember tick frequency
		bool isAutomatic;

#if defined(PROFILER_USE_VECTOR)
		std::vector<CTimeMeasure> samples;				// Data set using C++ vectors. Read the comments above
#elif defined(PROFILER_USE_ARRAY)
		CTimeMeasure samples[MAX_PROFILER_SAMPLES];		// Data set using C arrays. Read the comments above 	
#endif

	};


	/******************************************************************************/
	/******************************************************************************/

	// Profiler class
	// TO DO: create one parent and child classes for oneshot and multi-sample profilers?

	/** \details Class with profiling tools for measuring performance of algorithms.
	*	\n This class is a handler of multiple data sets, with centralized processor-wise time units
	*/
	class CProfiler
	{
		// PUBLIC METHODS
	public:

		// General methods:

			/** \brief Access to singleton instance with lazy initialization
			*	\details Use CProfiler::Instance().Method or the mabro BRT_PROFILER.Method() to call any profiler method
			*	\sa BRT_PROFILER
			*/
		static CProfiler& Instance()
		{
			static CProfiler singletonInstance;
			return singletonInstance;
		}

		
#ifdef PLATFORM_ANDROID
		/** \brief Initialize profiler
		*	\details Sets default resolution and clear all samples, and Setup other platform-dependent stuff.
		*	\n In Android, set external data path and set initial number of seconds
		*/		
		void CProfiler::InitProfiler(std::string externalDataPath)
		{
			//SetResolution(UNITS_MICROSECONDS);				
			SetResolution(UNITS_NANOSECONDS);
			//isAutomatic = false;

			dataPath = externalDataPath;

			struct timespec now;
			if (clock_gettime(CLOCK_SOURCE, &now) != 0)
			{
				DEBUG_ANDROID("Error in call to clock_gettime!");
				SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Could not setup high-performance system timer for profiling (ANDROID platform)");
				startingSeconds = 0;
			}
			else
			{
				SET_RESULT(RESULT_OK, "Profiler was initalized succesfully");
				startingSeconds = (TInt64)now.tv_sec;
				isInitialized = true;
			}
		}

#else
		/** \brief Initialize profiler
		*	\details Sets default resolution and clear all samples, and Setup other platform-dependent stuff.
		*	\n In windows: get QPF (from Vista and newer, this can be read only once)
		*/		
		void InitProfiler()
		{
			//SetResolution(UNITS_MICROSECONDS);				
			SetResolution(UNITS_NANOSECONDS);
			//isAutomatic = false;

#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
			LARGE_INTEGER qfc;
			if (!QueryPerformanceFrequency(&qfc))
				SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Error in QueryPerformanceFrequency (WINDOWS platform)");	// This should never happen
			else
			{
				SET_RESULT(RESULT_OK, "Profiler was initalized succesfully");
				isInitialized = true;
			}

			TSCFrequency = qfc.QuadPart;	// This cast seems to work ok
#endif
		}
#endif





		/** \brief Set resolution (time units) for all time measures
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*/
		void SetResolution(unsigned int unitsPreset)
		{
			if ((unitsPreset == UNITS_MICROSECONDS) || (unitsPreset == UNITS_NANOSECONDS))
				SET_RESULT(RESULT_OK, "Resolution for profiler succesfully set");
			else
				SET_RESULT(RESULT_WARNING, "Profiler should use only microseconds or nanoseconds resolution, not ticks");
			resolution = unitsPreset;
		}

		/** \brief Get TSC frequency
		*	\details Used while debugging the profiler itself
		*	\retval freq TSC frequency
		*/
		TInt64 GetTSCFrequency() const
		{
			if (isInitialized)
				return TSCFrequency;
			else
			{
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
				return 0;
			}
		}

		/** \brief Get resolution (time units) of profiler time measures
		*	\retval unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*/
		unsigned int GetResolution() const
		{
			if (isInitialized)
				return resolution;
			else
			{
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
				return 0;
			}
		}

		/** \brief Write data to file
		*	\param [in] dataSet data set containing the data
		*	\param [in] fileName name of the file to write in
		*/
		void WriteToFile(CProfilerDataSet dataSet, std::string fileName) const
		{
			if (isInitialized)
			{
				// We trust in dataset WriteToFile for setting error handler
				dataSet.WriteToFile(fileName, TSCFrequency);
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		/** \brief Switch on/off automatic write to file
		*	\details Will write all data from a data set to file when enough samples are acquired. By default, this feature is switched off, unless this method is explictly called
		*	\param [in,out] dataSet data set containing the data
		*	\param [in] filename name of the file to write in
		*	\param [in] nSamples number of samples to acquire before starting write to file (default, \link DEFAULT_PROFILER_SAMPLES \endlink)
		*	\param [in] setAutomatic switch on/off automatic write (default, true)
		*/
		void SetAutomaticWrite(CProfilerDataSet& dataSet, std::string filename, long nSamples = DEFAULT_PROFILER_SAMPLES, bool setAutomatic = true)
		{
			if (isInitialized)
			{
				// We trust in dataset SetAutomaticWrite for setting error handler
				//isAutomatic = setAutomatic;
				//nAutomaticSamples = nSamples;
				//automaticFileName = filename;
#ifdef PLATFORM_ANDROID
				dataSet.SetAutomaticWrite(dataPath + filename, nSamples, TSCFrequency, setAutomatic);
#else
				dataSet.SetAutomaticWrite(filename, nSamples, TSCFrequency, setAutomatic);
#endif
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		// One shot profiler:

		/** \brief Get current time measure
		*	\details Absolute one shot profiler
		*	\retval time current time measure
		*/
		CTimeMeasure GetTimeMeasure() const
		{
			CTimeMeasure currentTime;
			currentTime.SetInvalid();

			if (isInitialized)
			{
#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
				// NOTE: SO must be Windows Vista or newer for performant and reliable (between CPUs) time measures using QPC

				LARGE_INTEGER now;
				if (!QueryPerformanceCounter(&now))
				{
					SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Error in QueryPerformanceCounter (WINDOWS platform)");	// This should never happen in XP or above
					currentTime.SetInvalid();
					return currentTime;
				}
				else
				{
					// We avoid using error handler while succesfully profiling, to reduce overhead
					//SET_RESULT(RESULT_OK, "");
				}

				currentTime.SetValue(now.QuadPart, resolution);	// This cast seems to work ok	

#elif defined(PLATFORM_ANDROID)	
				struct timespec now;
				if (clock_gettime(CLOCK_SOURCE, &now) != 0)
				{
					DEBUG_ANDROID("Error in call to clock_gettime!");
					//SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Could not setup high-performance system timer for profiling (ANDROID platform)");
					currentTime.SetInvalid();
					return currentTime;
				}

				TInt64 newValue = ((TInt64)now.tv_sec - startingSeconds) * NANOSECONDS_IN_ONE_SECOND + (TInt64)now.tv_nsec;
				currentTime.SetValue(newValue, UNITS_NANOSECONDS); // NOTE: currently, it is forced to be nanoseconds			
#endif
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");

			return currentTime;
		}

		/** Get elapsed time
		*	\details Relative one shot profiler
		*	\param [in] _fromTime reference time measure
		*	\retval time time elapsed from reference time measure to current time
		*/
		CTimeMeasure GetTimeFrom(CTimeMeasure& _fromTime) const
		{
			CTimeMeasure elapsedTime;
			elapsedTime.SetInvalid();

			if (isInitialized)
			{
#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
				CTimeMeasure currentTime = GetTimeMeasure();
				elapsedTime = currentTime - _fromTime;

				elapsedTime = elapsedTime.FromTicksToUnits(TSCFrequency);	// error handler: we trust in this call for setting result
#else			
				CTimeMeasure currentTime = GetTimeMeasure();
				elapsedTime = currentTime - _fromTime;
#endif
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");

			return elapsedTime;
		}

		// Multi-sample profiler (absolute samples):

		/** \brief Start sampling one data set with absolute time samples
		*	\param [in,out] dataSet data set on which to start sampling
		*/
		void StartAbsoluteSampling(CProfilerDataSet &dataSet) const
		{
			if (isInitialized)
			{
				if (dataSet.IsSampling())
				{
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Profiling was already started on this dataset");
					return;
				}
				else
					SET_RESULT(RESULT_OK, "Absolute sampling in profiler started succesfully");

				dataSet.SetSampleType(TSampleType::ST_ABSOLUTE);
				dataSet.Start();
				//dataSet.AddSample(GetTimeMeasure());	// Give first sample for post-processing relative samples
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		/** \brief Take one absolute time measure and store it in data set
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Absolute sampling was started in data set (See \link StartAbsoluteSampling \endlink)
		*/
		void TakeAbsoluteSample(CProfilerDataSet &dataSet) const
		{
			// TO DO: Check dataset sample type

			if (isInitialized)
			{
				if (!dataSet.IsSampling())
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a sample to a dataset without starting sampling first");
				else
				{
					// We avoid using error handler while succesfully profiling, to reduce overhead
					//SET_RESULT(RESULT_OK, "");
				}

				dataSet.AddSample(GetTimeMeasure());
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		/** \brief Ends sampling one data set, regardless the type (absolute or relative)
		*	\param [in,out] dataSet data set on which to stop sampling
		*/
		void EndSampling(CProfilerDataSet &dataSet) const
		{
			if (isInitialized)
			{
				if (!dataSet.IsSampling())
					SET_RESULT(RESULT_WARNING, "Ending sampling for a dataset which was not sampling");
				else
					SET_RESULT(RESULT_OK, "Sampling data set in profiler ended succesfully");

				dataSet.End();
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		// Multi-sample profiler (relative samples):

		/** \brief Start sampling one data set with relative time samples
		*	\param [in,out] dataSet data set on which to start sampling
		*/
		void StartRelativeSampling(CProfilerDataSet &dataSet)
		{
			if (isInitialized)
			{
				if (dataSet.IsSampling())
				{
					//SET_RESULT(RESULT_ERROR_NOTALLOWED, "Profiling was already started on this dataset");
					return;
				}
				else
					SET_RESULT(RESULT_OK, "Sampling data set in profiler started succesfully");

				//relativeSampleStart.SetInvalid();
				dataSet.SetSampleType(TSampleType::ST_RELATIVE);
				dataSet.Start();
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		/** \brief Start one relative time measure in data set
		*	\details The current time measure will be obtained and use as reference for this sample
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Relative sampling was started in data set
		*/
		void RelativeSampleStart(CProfilerDataSet &dataSet)
		{
			if (isInitialized)
			{
				//if (relativeSampleStart.IsValid())
				//	SET_RESULT(RESULT_WARNING, "This relative sample was already started. Previous start point will be ignored.");
				//else
				//	SET_RESULT(RESULT_OK, "");

				//relativeSampleStart = GetTimeMeasure();
				dataSet.SetRelativeStart(GetTimeMeasure());
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		/** \brief Ends one relative time measure in data set
		*	\details The time measure obtained internally when calling to \link StartRelativeSampling \endlink is used as reference for this sample
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Relative sampling was started in data set (See \link StartRelativeSampling \endlink)
		*	\pre This sample was started (See \link RelativeSampleStart \endlink)
		*/
		void RelativeSampleEnd(CProfilerDataSet &dataSet)
		{
			if (isInitialized)
			{
				if (!dataSet.IsSampling())
				{
					//SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a sample to a dataset without starting sampling first.");
					return;
				}
				else if (!dataSet.GetRelativeStart().IsValid())
				{
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a relative sample without establishing reference first (please, use RelativeSampleStart)");
					return;
				}
				else
				{
					// We avoid using error handler while succesfully profiling, to reduce overhead
					//SET_RESULT(RESULT_OK, "");
				}

				CTimeMeasure relativeStart = dataSet.GetRelativeStart();
				dataSet.AddSample(GetTimeFrom(relativeStart));

				//relativeSampleStart.SetInvalid();

				// Automatic write to file
				//if (isAutomatic)
				//{
				//	if (dataSet.GetCurrentSize() >= nAutomaticSamples)
				//	{
				//		dataSet.End();
				//		dataSet.WriteToFile(automaticFileName, TSCFrequency);
				//	}
				//}
			}
			else
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
		}

		// HIDDEN METHODS
	protected:

		/** \brief Default constructor
		*	\details Sets the profiler as Not Initialized.
		*/
		CProfiler()
		{
			isInitialized = false;
		}

		///** \brief Destructor
		//*	\details Closes all open files 
		//*/
		//~CProfiler()
		//{
		//}

	// ATTRIBUTES:
	private:
		bool isInitialized;			// Have you called InitProfiler?
		unsigned int resolution;	// Resolution preset	
		TInt64 TSCFrequency;		// Windows specific: tick frequency of your machine, for using QPC
		TInt64 startingSeconds;		// Android/Linux specific: number of seconds in initial call to clock_gettime, to avoid overflow
		std::string dataPath;		// Android specific: path to add for external data storage (to allow file write)
	};
}//end namespace Common
#endif