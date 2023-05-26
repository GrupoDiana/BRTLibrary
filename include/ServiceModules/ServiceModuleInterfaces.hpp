
#ifndef _SERVICE_INTERFACES_H_
#define _SERVICE_INTERFACES_H_

#include <cstdint>
//#include <ServiceModules/HRTF.h>
//#include <ServiceModules/ILD.hpp>

#define MAX_DISTANCE_BETWEEN_ELEVATIONS 5
#define NUMBER_OF_PARTS 4 
#define MARGIN 10
#define ELEVATION_NORTH_POLE 90
#define ELEVATION_SOUTH_POLE 270

#define DEFAULT_GAP_THRESHOLD 10

#define SPHERE_BORDER 360.0f

#define DEFAULT_MIN_AZIMUTH 0
#define DEFAULT_MAX_AZIMUTH 360
#define DEFAULT_MIN_ELEVATION 0
#define DEFAULT_MAX_ELEVATION 360

#define ORIENTATION_RESOLUTION 0.01

#define EPSILON_SEWING 0.001

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
struct orientation
{
	float azimuth;		///< Azimuth angle in degrees
	float elevation;	///< Elevation angle in degrees	
	orientation(float _azimuth, float _elevation) :azimuth{ _azimuth }, elevation{ _elevation } {}
	orientation() :orientation{ 0,0 } {}
	bool operator==(const orientation& other) const
	{
		return ((Common::AreSame(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSame(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
	}
};

namespace std
{
	template<>
	struct hash<orientation>
	{
		// adapted from http://en.cppreference.com/w/cpp/utility/hash
		size_t operator()(const orientation& key) const
		{
			int keyAzimuth_hundredth = static_cast<int> (round(key.azimuth / ORIENTATION_RESOLUTION));
			int keyElevation_hundredth = static_cast<int> (round(key.elevation / ORIENTATION_RESOLUTION));

			size_t h1 = std::hash<int32_t>()(keyAzimuth_hundredth);
			size_t h2 = std::hash<int32_t>()(keyElevation_hundredth);
			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
		}
	};
}

namespace BRTServices {

	/** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
*/
	struct THRIRStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		CMonoBuffer<float> leftHRIR;	///< Left impulse response data
		CMonoBuffer<float> rightHRIR;	///< Right impulse response data
	};

	struct TILDStruct {
		CMonoBuffer<float> leftCoefs;	///< Left filters coefs
		CMonoBuffer<float> rightCoefs;	///< Right filters coefs
	};

	struct TDirectivityTFStruct {
		CMonoBuffer<float> dataReal;	
		CMonoBuffer<float> dataImag;	
	};

	class CServicesBase {
		
	public:
		CServicesBase() {}
		virtual ~CServicesBase() {}		
		
		virtual void BeginSetup() {}
		virtual void BeginSetup(int32_t _SRTFLength) {}
		virtual void BeginSetup(int32_t _HRIRLength, float _distance) {}
		virtual bool EndSetup() = 0;

		virtual void SetResamplingStep(int _resamplingStep) {};
		virtual void SetTitle(std::string _title) = 0;
		virtual void SetDatabaseName(std::string _databaseName) =0;
		virtual void SetListenerShortName(std::string _listenerShortName) {};
		virtual void SetFilename(std::string _fileName) = 0;

		virtual void SetNumberOfEars(int _numberOfEars) {}
		virtual void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {};

		virtual void AddHRIR(float _azimuth, float _elevation, THRIRStruct&& newHRIR) {}
		virtual void AddCoefficients(float azimuth, float distance, TILDStruct&& newCoefs) {}
		virtual void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF){}
		
	};}

#endif