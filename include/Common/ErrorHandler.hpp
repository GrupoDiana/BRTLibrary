/**
* \class CErrorHandler
*
* \brief Declaration of CErrorHandler class interface.
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


#ifndef _ERROR_HANDLER_HPP_
#define _ERROR_HANDLER_HPP_

#include <string>
#include <mutex>
#include <fstream>

/*! \file */

//using namespace std;

/** \brief If SWITCH_ON_BRT_ERRORHANDLER is undefined, the error handler is completely disabled, causing 0 overhead
*/

#define SWITCH_ON_BRT_ERRORHANDLER

#ifdef _BRT_ANDROID_ERRORHANDLER

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "3DTI_CORE", __VA_ARGS__))
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "3DTI_CORE", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "3DTI_CORE", __VA_ARGS__))

#define BRT_ERRORHANDLER Common::CErrorHandler::Instance()

#define SET_RESULT(errorID, suggestion) Common::CErrorHandler::Instance().AndroidSetResult(errorID, suggestion, __FILE__, __LINE__)

#define ASSERT(condition, errorID, suggestionError, suggestionOK) Common::CErrorHandler::Instance().AndroidAssertTest(condition, errorID, suggestionError, suggestionOK, __FILE__, __LINE__)

#define WATCH(whichVar, varValue, className) ((void)0)

#define GET_LAST_RESULT() Common::CErrorHandler::Instance().GetLastResult()

#define GET_LAST_RESULT_STRUCT() Common::CErrorHandler::Instance().GetLastResultStruct()

#define GET_FIRST_ERROR_STRUCT() Common::CErrorHandler::Instance().GetFirstErrorStruct()

#define RESET_ERRRORS() Common::CErrorHandler::Instance().ResetErrors()

#endif

#if !defined (SWITCH_ON_BRT_ERRORHANDLER) && !defined(_BRT_ANDROID_ERRORHANDLER)

///////////////////////////////////////////////////
/// Dummy Macro definitions 

#define BRT_ERRORHANDLER ((void)0)

#define SET_RESULT(errorID, suggestion) ((void)0)

#define ASSERT(condition, errorID, suggestionError, suggestionOK) ((void)0)

#define WATCH(whichVar, varValue, className) ((void)0)

#define GET_LAST_RESULT() ((void)0)

#define GET_LAST_RESULT_STRUCT() ((void)0)

#define GET_FIRST_ERROR_STRUCT() ((void)0)

#define RESET_ERRORS() ((void)0)

#endif

#if defined(SWITCH_ON_BRT_ERRORHANDLER)

///////////////////////////////////////////////////
/// Macro definitions for asserts, setting results and watching variables

/** \brief Macro used for easy access to error handler singleton
*/
#define BRT_ERRORHANDLER Common::CErrorHandler::Instance()

/** \brief Macro used by internal classes for reporting results to error handler
*/
#define SET_RESULT(errorID, suggestion) Common::CErrorHandler::Instance().SetResult(errorID, suggestion, __FILE__, __LINE__)

/** \brief Macro used by internal classes for throwing asserts to error handler
*/
#define ASSERT(condition, errorID, suggestionError, suggestionOK) Common::CErrorHandler::Instance().AssertTest(condition, errorID, suggestionError, suggestionOK, __FILE__, __LINE__)

/** \brief Macro used by internal classes to allow watch of internal variables 
*/
#define WATCH(whichVar, varValue, className) Common::CErrorHandler::Instance().Watch<className>(whichVar, varValue)

/** \brief Macro for getting (only the ID of) the last result reported to the error handler
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\retval result ID of last result/error/warning
*/
#define GET_LAST_RESULT() Common::CErrorHandler::Instance().GetLastResult()

/** \brief Macro for getting (the full structure of) the last result reported to the error handler
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\retval resultStruct full structure with information on the last result/error/warning (See \link TResultStruct \endlink)
*/
#define GET_LAST_RESULT_STRUCT() Common::CErrorHandler::Instance().GetLastResultStruct()

/** \brief Macro for getting (the full structure of) the first error reported to the error handler in a block of code
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\pre The starting point of the block of code is marked using \link ResetErrors \endlink
*	\retval errorStruct full structure with information on the first result/error/warning (See \link TResultStruct \endlink)
*/
#define GET_FIRST_ERROR_STRUCT() Common::CErrorHandler::Instance().GetFirstErrorStruct()

/** \brief Macro for doing a reset of last reported result
*/
#define RESET_ERRORS() Common::CErrorHandler::Instance().ResetErrors()

#endif

#if defined(SWITCH_ON_BRT_ERRORHANDLER) || defined(_BRT_ANDROID_ERRORHANDLER)

//
// Result/Error data structures
//

/** \brief ID of result reported to the error handler
*/
enum TResultID
{ 
	// No error
	RESULT_OK,						///< No error. Everything went ok

	// General errors
	RESULT_ERROR_UNKNOWN,			///< Unknown error (use only for weird situations, when you don't have any clue of the error source)
	RESULT_ERROR_NOTSET,			///< The value of some parameter was not set
	RESULT_ERROR_BADALLOC,			///< Memory allocation failure
	RESULT_ERROR_NULLPOINTER,		///< Trying to use a pointer which is null
	RESULT_ERROR_DIVBYZERO,			///< Division by zero
	RESULT_ERROR_CASENOTDEFINED,	///< Some case in a switch was not defined (typically, use this for the "default" case of a switch)	
	RESULT_ERROR_PHYSICS,			///< Trying to do something which is not physically correct
	RESULT_ERROR_INVALID_PARAM,     ///< Param value is not valid
	RESULT_ERROR_OUTOFRANGE,		///< Trying to access an array or vector position outside its size
	RESULT_ERROR_BADSIZE,			///< Trying to fill a data structure with a bad data size
	RESULT_ERROR_NOTINITIALIZED,	///< Using or returning a value which is not initialized
	RESULT_ERROR_SYSTEMCALL,		///< A system call returned an error
	RESULT_ERROR_NOTALLOWED,		///< Trying to do something which is not allowed in the current context
	RESULT_ERROR_NOTIMPLEMENTED,	///< A method was defined in the interface for future versions, but it is not implemented yet
	RESULT_ERROR_FILE,				///< Error trying to handle a file
	RESULT_ERROR_EXCEPTION,			///< Exception caught

	// More errors...

	// Warnings
	RESULT_WARNING					///< Description to be specified in suggestion	
};

/** Struct with full information about one error/result/warning
*/
struct TResultStruct
{
	TResultID id;			///< ID of result
	std::string description;		///< Description of result
	std::string suggestion;		///< Suggestion for fixing error or further information about result
	std::string filename;		///< File from which result was reported
	int linenumber;			///< Line number at which result was reported (within filename file)
};

/** \brief Stream output of \link TResultStruct \endlink
*/
inline std::ostream & operator<<(std::ostream & out, const TResultStruct & r)
{
	out << "RESULT #" << r.id << " in File " << r.filename << "(" << r.linenumber << "): " << r.description << " - " << r.suggestion;
	return out;
}

//
// Verbosity modes data structures and presets
//

/*********************************************/

/** \brief Preset verbosity modes
*/
#define VERBOSITY_MODE_SILENT			0	///< Nothing to show
#define VERBOSITYMODE_ERRORSANDWARNINGS	1	///< Show errors and warnings, but not OK results
#define VERBOSITY_MODE_ONLYERRORS		2	///< Show only errors, not OK nor warnings
#define VERBOSITY_MODE_ALL				3	///< Show every type of result: error, warning and OK. Use this with caution, may report a huge amount of information...

/*********************************************/

/** \brief Type definition for verbosity modes
*/
struct TVerbosityMode
{
	bool showErrors;		///< Do show error results
	bool showWarnings;		///< Do show warning results
	bool showOk;			///< Do show OK results

	bool showID;			///< Do show ID of result
	bool showDescription;	///< Do show description of result
	bool showSuggestion;	///< Do show suggestion of result
	bool showFilename;		///< Do show filename of result
	bool showLinenumber;	///< Do show linenumber of result
};

//
// Assert modes 
//

/** \brief Type definition of assert modes
*/
enum TAssertMode	{ASSERT_MODE_EMPTY,		///< Do nothing. Ignore even result reporting. The error handler becomes useless with this setting. For maximum performance, undefine \link SWITCH_ON_BRT_ERRORHANDLER \endlink
					ASSERT_MODE_CONTINUE,	///< Allow reporting of results, but do nothing with them. Will never terminate program execution
					ASSERT_MODE_ABORT,		///< Abort execution when an ASSERT is evaluated as false. The error will be reported/logged before terminating
					ASSERT_MODE_PARANOID	///< Abort execution if any error is reported to the error handler, even if it was reported using SET_RESULT rather than ASSERT. The error will be reported/logged before terminating
					};


//
// Definitions of variables for variable watcher
//

/** \brief Definition of variables reported to the variable watcher
*	\details This is just an example, you can add here any variables you may need
*/
enum TWatcherVariable	{WV_ANECHOIC_AZIMUTH_LEFT,		///< Azimuth of an audio source for listener left ear
						WV_ANECHOIC_AZIMUTH_RIGHT,		///< Azimuth of an audio source for listener right ear
						WV_ANECHOIC_OUTPUT_LEFT,		///< Left output buffer of anechoic process for one source
						WV_ANECHOIC_OUTPUT_RIGHT,		///< Right output buffer of anechoic process for one source
	                    WV_ENVIRONMENT_OUTPUT_LEFT,		///< Left output buffer of environment process 
						WV_ENVIRONMENT_OUTPUT_RIGHT,	///< Right output buffer of environment process 
						WV_HEARINGLOSS_OUTPUT_LEFT,		///< Left output buffer of hearing loss simulation process 
						WV_HEARINGLOSS_OUTPUT_RIGHT,	///< Right output buffer of hearing loss simulation process 
						WV_HEARINGAID_OUTPUT_LEFT,		///< Left output buffer of hearing aid simulation process 
						WV_HEARINGAID_OUTPUT_RIGHT,		///< Right output buffer of hearing aid simulation process 
						WV_LISTENER_POSITION,			///< Listener position
						// .... Add here your own variable watches... 
						WV_END};

/*********************************************/

namespace Common {

	/** \details Error handler class for error reporting and watching variables
	*	\details Follows Meyers Singleton design pattern
	*/
	class CErrorHandler
	{
	public:
		// PUBLIC METHODS:

			/** \brief Access to singleton instance with lazy initialization
			*	\details Use CErrorHandler::Instance().Method to call any error handler method, or use the defined MACROS instead
			*	\sa SET_RESULT, ASSERT, GET_RESULT, GET_RESULT_STRUCT, GET_FIRST_ERROR_STRUCT, WATCH
			*/
		static CErrorHandler& Instance()
		{
			static CErrorHandler singletonInstance;
			return singletonInstance;
		}

		//
		// Result reporting
		//

		/** \brief Get a struct with the info of the last reported result
		*	\retval resultStruct info of last reported result
		*   \eh Nothing is reported to the error handler.
		*/
		TResultStruct GetLastResultStruct()		
		{
			return lastResult;
		}

		/** \brief Get the ID of the last reported result
		*	\retval result ID of last reported result
		*   \eh Nothing is reported to the error handler.
		*/		
		TResultID GetLastResult()
		{
			return lastResult.id;
		}

		/** \brief Set result of last operation
		*	\note Instead of calling this method, using the macros \link SET_RESULT \endlink or \link ASSERT \endlink is recommended
		*	\param [in] resultID ID of result
		*	\param [in] suggestion suggestion or further information about result
		*	\param [in] filename file from which result is being reported
		*	\param [in] linenumber line number at which result is being reported (whithin filename file)
		*/		
		void SetResult(TResultID resultID, std::string suggestion, std::string filename, int linenumber)
		{
			if (assertMode != ASSERT_MODE_EMPTY)	// Alternative: put this before logging to file
			{
				std::lock_guard<std::mutex> lock(errorHandlerMutex);

				// Set result struct

				lastResult.id = resultID;
				lastResult.linenumber = linenumber;
				lastResult.filename = filename;

				// Set specific strings for each result type. Suggestions are generic and might be replaced with the one specified
				std::string defaultDescription, defaultSuggestion;
				GetDescriptionAndSuggestion(lastResult.id, defaultDescription, defaultSuggestion);
				lastResult.description = defaultDescription;

				// Replace default suggestion with the provided one, if it was specified
				if (suggestion != "")
					lastResult.suggestion = suggestion;
				else
					lastResult.suggestion = defaultSuggestion;

				// For filename, remove the path (WARNING! This may be platform-dependent)
				const size_t last_slash = lastResult.filename.find_last_of("\\/");
				if (std::string::npos != last_slash)
					lastResult.filename.erase(0, last_slash + 1);

				// SET FIRST ERROR 
				if (resultID != RESULT_OK)
				{
					if (firstError.id == RESULT_OK)
					{
						firstError = lastResult;
					}
				}

				// LOG TO FILE
				if (errorLogFile.is_open())
					LogErrorToFile(lastResult);

				// LOG TO STREAM
				if (logToStream)
					LogErrorToStream(*errorLogStream, lastResult);

				// TERMINATE PROGRAM IF ERROR IN PARANOID MODE 
				// TO THINK: Do we include RESULT_WARNING here????
				if ((lastResult.id != RESULT_OK) && (assertMode == ASSERT_MODE_PARANOID))
				{
					std::terminate();
				}
			}
		}

#if defined (_BRT_ANDROID_ERRORHANDLER)
		void AndroidSetResult(TResultID resultID, string suggestion, string filename, int linenumber)
		{
			string newdescription;
			string newsuggestion;
			GetDescriptionAndSuggestion(resultID, newdescription, newsuggestion);

			if (suggestion != "")
				newsuggestion = suggestion;

			if (resultID == RESULT_OK)
				LOGV("OK: %s in file %s (%d)", newsuggestion.c_str(), filename.c_str(), linenumber);
			else
			{
				if (resultID == RESULT_WARNING)
					LOGW("WARNING: %s in file %s (%d)", newsuggestion.c_str(), filename.c_str(), linenumber);
				else
					LOGE("ERROR (%s): %s in file %s (%d)", newdescription.c_str(), newsuggestion.c_str(), filename.c_str(), linenumber);
			}
		}
#endif

		//
		// First error (error reporting in blocks of code)
		//

		/** \brief Inits the first error report, so that the next error will be stored as the first error
		*	\details Used to mark the starting point of the code block
		*   \eh Nothing is reported to the error handler.
		*/		
		void ResetErrors()
		{
			if (assertMode != ASSERT_MODE_EMPTY)
			{
				std::string description, suggestion;
				firstError.id = RESULT_OK;
				GetDescriptionAndSuggestion(firstError.id, description, suggestion);
				firstError.description = description;
				firstError.suggestion = suggestion;
				firstError.filename = "Nobody";
				firstError.linenumber = -1;
			}
		}

		/** \brief Get a struct with the info of the first reported error in code block
		*	\retval resultStruct info of first reported error in code block
		*   \eh Nothing is reported to the error handler.
		*/		
		TResultStruct GetFirstErrorStruct()
		{
			return firstError;
		}

		/** \brief Get the ID of the first reported error in code block
		*	\retval result ID of first reported error in code block
		*   \eh Nothing is reported to the error handler.
		*/		
		TResultID GetFirstError()
		{
			return firstError.id;
		}

		//
		// Verbosity modes
		//

		/** \brief Set verbosity mode from one of the presets
		*	\sa VERBOSITY_MODE_SILENT, VERBOSITYMODE_ERRORSANDWARNINGS, VERBOSITY_MODE_ONLYERRORS, VERBOSITY_MODE_ALL
		*   \eh Nothing is reported to the error handler.
		*/		
		void SetVerbosityMode(int presetMode)
		{
			// By default, all presets show all attributes of the error handler result
			verbosityMode.showID = true;
			verbosityMode.showDescription = true;
			verbosityMode.showSuggestion = true;
			verbosityMode.showFilename = true;
			verbosityMode.showLinenumber = true;

			// What type of results to show, depending on preset
			switch (presetMode)
			{
			case VERBOSITY_MODE_SILENT:
				verbosityMode.showErrors = false;
				verbosityMode.showOk = false;
				verbosityMode.showWarnings = false;
				//SET_RESULT(RESULT_OK, "Verbosity mode changed to Silent.");	// actually, setting this result is nonsense :)
				break;
			case VERBOSITY_MODE_ONLYERRORS:
				verbosityMode.showErrors = true;
				verbosityMode.showOk = false;
				verbosityMode.showWarnings = false;
				//SET_RESULT(RESULT_OK, "Verbosity mode changed to Only Errors.");	// actually, setting this result is nonsense :)
				break;
			case VERBOSITY_MODE_ALL:
				verbosityMode.showErrors = true;
				verbosityMode.showOk = true;
				verbosityMode.showWarnings = true;
				//SET_RESULT(RESULT_OK, "Verbosity mode changed to All.");
				break;
			case VERBOSITYMODE_ERRORSANDWARNINGS:
				verbosityMode.showErrors = true;
				verbosityMode.showOk = false;
				verbosityMode.showWarnings = true;
				//SET_RESULT(RESULT_OK, "Verbosity mode changed to Errors and Warnings.");
				break;
			default:
				verbosityMode.showErrors = false;
				verbosityMode.showOk = false;
				verbosityMode.showWarnings = false;
				//SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Preset not found for verbosity mode.");
				break;
			}
		}

		/** \brief Set custom verbosity mode
		*	\param [in] _verbosityMode definition of custom verbosity mode
		*   \eh Nothing is reported to the error handler.
		*/		
		void SetVerbosityMode(TVerbosityMode _verbosityMode)
		{
			verbosityMode = _verbosityMode;
		}

		//
		// Logging to file
		//

		/** \brief Enable/disable log of reported results to file, using current verbosity mode
		*	\param [in] filename name of log file
		*	\param [in] logOn switch on/off logging to file (default, true)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetErrorLogFile(std::string filename, bool logOn = true)
		{
			// TO DO: check errors!

			if (errorLogFile.is_open())
				errorLogFile.close();

			if (logOn)
			{
				errorLogFile.open(filename, std::ofstream::out | std::ofstream::app);	// Using append, we allow enabling/disabling log to the same file in runtime
				// TO DO: Put a text header in log file each time you open it? (for example, with a time stamp, but this might be platform-dependent)
			}
		}

		/** \brief Enable log of reported results to output stream, using current verbosity mode
		*	\param [in] outStream output stream
		*	\param [in] logOn switch on/off logging to stream (default, true)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetErrorLogStream(std::ostream* outStream, bool logOn = true)
		{
			errorLogStream = outStream;
			logToStream = logOn;
		}

		//
		// Assert modes
		//

		/** \brief Set an assert mode
		*	\details Defines what to do when an error is reported
		*	\param [in] _assertMode one of the preset assert modes
		*	\sa ASSERT_MODE_EMPTY, ASSERT_MODE_CONTINUE, ASSERT_MODE_ABORT, ASSERT_MODE_PARANOID
		*/		
		void SetAssertMode(TAssertMode _assertMode)
		{
			assertMode = _assertMode;
			if (assertMode == ASSERT_MODE_EMPTY)
			{
				lastResult.id = RESULT_OK;
				lastResult.description = "No results";
				lastResult.suggestion = "Assert mode is empty; results are not being reported.";
				lastResult.filename = "";
				lastResult.linenumber = -1;
				firstError = lastResult;
			}
		}

		/** \brief Test a condition and report error if false, doing the action specified by the assert mode
		*	\details Internally used by the \link ASSERT \endlink macro. Using the macro instead of this method is recommended
		*	\param [in] condition condition to evaluate
		*	\param [in] errorID ID of error to report if condition is evaluated false
		*	\param [in] suggestionError suggestion for reported result struct if condition is evaluated false (result error)
		*	\param [in] suggestionOK suggestion for reported result struct if condition is evaluated true (result OK)
		*	\param [in] filename filename for reported result struct
		*	\param [in] linenumber linenumber for reported result struct
		*/
		void AssertTest(bool condition, TResultID errorID, std::string suggestionError, std::string suggestionOK, std::string filename, int linenumber)
		{
			if (assertMode != ASSERT_MODE_EMPTY)
			{
				if (condition)
				{
					if (suggestionOK != "")
						SetResult(RESULT_OK, suggestionOK, filename, linenumber);
				}
				else
				{
					SetResult(errorID, suggestionError, filename, linenumber);

					if (assertMode == ASSERT_MODE_ABORT)
					{
						std::terminate();
					}
				}
			}
		}

#if defined (_BRT_ANDROID_ERRORHANDLER)
		void AndroidAssertTest(bool condition, TResultID errorID, string suggestionError, string suggestionOK, string filename, int linenumber)
		{
			if (condition)
			{
				if (suggestionOK != "")
					AndroidSetResult(RESULT_OK, suggestionOK, filename, linenumber);
			}
			else
				AndroidSetResult(errorID, suggestionError, filename, linenumber);
		}
#endif

		//
		// Variable watcher
		//

		/** \brief Add a variable to the list of variables to watch
		*	\param [in] whichVar variable to watch, which has to be added first to the \link TWatcherVariable \endlink enum
		*/
		void AddVariableWatch(TWatcherVariable whichVar)		
		{
			watcherVariables[whichVar] = true;
		}

		/** \brief Remove a variable from the list of variables to watch
		*	\param [in] whichVar which variable to stop watching
		*	\pre Variable was added to watch first
		*/		
		void RemoveVariableWatch(TWatcherVariable whichVar)
		{
			watcherVariables[whichVar] = false;
		}		

		/** \brief Enable/disable log to file of a specific watched variable
		*	\param [in] whichVar which variable to log to file
		*	\param [in] filename name of file with the log
		*	\param [in] logOn switch on/off file logging for this war (default, true)
		*	\pre Variable was added to watch first
		*/
		void SetWatcherLogFile(TWatcherVariable whichVar, std::string filename, bool logOn = true)
		{
			// TO DO: check errors!

			if (watcherLogFiles[whichVar].is_open())
				watcherLogFiles[whichVar].close();

			if (logOn)
			{
				watcherLogFiles[whichVar].open(filename, std::ofstream::out | std::ofstream::app);	// Using append, we allow enabling/disabling log to the same file in runtime
				// TO DO: Put a text header in log file each time you open it? (for example, with a time stamp, but this might be platform-dependent)			
			}
		}

		/** \brief Sends the value of a variable to the watcher
		*	\details The value will be recorded ONLY if the variable is on the list of watched variables. No overhead if the variable is not in the list
		*	\param [in] whichVar from which variable are we reporting value
		*	\param [in] varValue the value we are reporting
		*/
		template <class T>
		void Watch(TWatcherVariable whichVar, const T& varValue)
		{
			// Check first if this variable is being watched
			if (!watcherVariables[whichVar])
				return;

			// Log to file
			if (watcherLogFiles[whichVar].is_open())
			{
				watcherLogFiles[whichVar] << varValue << std::endl;
			}
		}


	// PRIVATE METHODS
	protected:

		/* Default constructor
		*	Resets all result and error reporting, sets verbosity mode to \link VERBOSITY_MODE_ONLYERRORS \endlink and assert mode to \link ASSERT_MODE_ABORT \endlink
		*/
		CErrorHandler()
		{
			lastResult.id = RESULT_OK;
			lastResult.filename = "Nobody";
			lastResult.linenumber = -1;
			lastResult.suggestion = "Nothing has been reported to the error handler yet";

			ResetErrors();
			SetVerbosityMode(VERBOSITY_MODE_ONLYERRORS);
			SetAssertMode(ASSERT_MODE_ABORT);
			ResetWatcher();
			logToStream = false;
			errorLogStream = NULL;
		}

		/* Destructor
		*  Closes all open files (error log and watcher)
		*/
		~CErrorHandler()
		{
			if (errorLogFile.is_open())
				errorLogFile.close();
			for (int i = 0; i < WV_END; i++)
			{
				if (watcherLogFiles[i].is_open())
					watcherLogFiles[i].close();
			}
		}

		// Generic method for obtaining description and suggestions of a result ID		
		void GetDescriptionAndSuggestion(TResultID result, std::string& description, std::string& suggestion)
		{
			// Set specific strings for each error type. Suggestions are generic and might be replaced with the one specified when calling to SetResult
			switch (result)
			{
			case RESULT_OK: { description = "OK"; suggestion = "Nothing to do"; break;  }
			case RESULT_ERROR_UNKNOWN: { description = "Unknown error"; suggestion = "There are no specific details about this error type"; break;  }
			case RESULT_ERROR_NOTSET: { description = "Value not set"; suggestion = "Tried to use a parameter and its value was not set"; break;  }
			case RESULT_ERROR_BADALLOC: { description = "Memory allocation failure"; suggestion = "Bad alloc exception thrown using New"; break;  }
			case RESULT_ERROR_NULLPOINTER: { description = "Null pointer"; suggestion = "Attempt to use a null pointer"; break;  }
			case RESULT_ERROR_DIVBYZERO: { description = "Division by zero"; suggestion = ""; break;  }
			case RESULT_ERROR_CASENOTDEFINED: { description = "Case not defined"; suggestion = "A switch statement went through an unexpected default case"; break;  }
			case RESULT_ERROR_PHYSICS: { description = "Violation of physics"; suggestion = "You tried to do something which is not physically correct"; break;  }
			case RESULT_ERROR_OUTOFRANGE: { description = "Out of range"; suggestion = "Trying to access an array or vector position outside its size"; break;  }
			case RESULT_ERROR_BADSIZE: { description = "Bad size"; suggestion = "Trying to fill a data structure with a bad size"; break;  }
			case RESULT_ERROR_NOTINITIALIZED: { description = "Not initialized"; suggestion = "Using or returning a value which was not initialized"; break;  }
			case RESULT_ERROR_INVALID_PARAM: { description = "Invalid parameter"; suggestion = "One or more parameters passed to a method have an incorrect value"; break;  }
			case RESULT_ERROR_SYSTEMCALL: { description = "Error in System Call"; suggestion = "Some platform-specific system call returned an error"; break;  }
			case RESULT_ERROR_NOTALLOWED: { description = "Not allowed"; suggestion = "Attempt to do something which is not allowed in the current context"; break;  }
			case RESULT_ERROR_NOTIMPLEMENTED: { description = "Not implemented yet"; suggestion = "Call to a method not implemented yet in this version of the toolkit core"; break;  }
			case RESULT_ERROR_FILE: { description = "File handling error"; suggestion = "Wrong attempt to open, read or write a file"; break;  }
			case RESULT_ERROR_EXCEPTION: { description = "Exception cuaght"; suggestion = "An exception was thrown and caught"; break;  }
			case RESULT_WARNING: { description = "Warning!"; suggestion = "This is not an error, only a warning"; break;  }
			default: { description = "Unknown error type"; suggestion = "The error handler was not properly used for setting result"; }
			}
		}

		// Log to file/stream		
		void LogErrorToFile(TResultStruct result)
		{
			LogErrorToStream(errorLogFile, result);
		}
		
		void LogErrorToStream(std::ostream& outStream, TResultStruct result)
		{
			// Return if we want to log an OK in a verbosity mode not logging OK
			if ((!verbosityMode.showOk) && (result.id == RESULT_OK))
				return;

			// Return if we want to log an error in a verbosity mode not logging errors
			if ((!verbosityMode.showErrors) && (result.id != RESULT_OK))
				return;

			// Return if we want to log a warning in a verbosity mode not logging warnings
			if ((!verbosityMode.showWarnings) && (result.id == RESULT_WARNING))
				return;

			// Go ahead with loggin in any other case
			// TO DO: coherent text, brackets, etc for custom verbosity modes
			if (verbosityMode.showID)
			{
				if (result.id == RESULT_OK)
					outStream << "    OK";	// We put spaces to clearly spot errors at first sight
				else
				{
					if (result.id == RESULT_WARNING)
						outStream << "  Warning";
					else
						outStream << "ERROR #" << result.id;
				}
			}
			if (verbosityMode.showFilename)
			{
				outStream << " in " << result.filename << " (";
			}
			if (verbosityMode.showLinenumber)
			{
				outStream << result.linenumber << "): ";
			}
			if (verbosityMode.showDescription)
			{
				outStream << result.description;
			}
			if (verbosityMode.showSuggestion)
			{
				outStream << " - " << result.suggestion;
			}
			outStream << std::endl;
		}

		// Reset all watches		
		void ResetWatcher()
		{
			for (int i = 0; i < WV_END; i++)
			{
				watcherVariables[i] = false;
			}
		}

	private:
		// ATTRIBUTES:

		std::mutex errorHandlerMutex;

		// Last Result handling
		TResultStruct lastResult;

		// First error (error reporting in blocks of code)
		TResultStruct firstError;

		// Verbosity modes
		// TO DO: we can think in having different modes for different things: log to file, stream output...
		TVerbosityMode verbosityMode;

		// Logging to file/stream
		std::ofstream errorLogFile;
		std::ostream* errorLogStream;
		bool logToStream;

		// Assert modes
		TAssertMode assertMode;

		// Variable watcher
		bool watcherVariables[TWatcherVariable::WV_END];
		std::ofstream watcherLogFiles[TWatcherVariable::WV_END];
		//std::ostream watcherStream;	
	};
}

#endif

#endif 
