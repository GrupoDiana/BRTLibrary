# Changelog

All notable changes to the Binaural Rendering Toolbox (BRT) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.0.2] - 2026-03-27

## Fixed
- If near-field simulation was enabled and you left the near-field zone, the sound would go silent. This bug appears to have been introduced in the latest release.

## [3.0.1] - 2026-03-24

## Fixed
- In the SDN simulation, the distance of the direct route was limited to a minimum of one metre. This has been changed to remove that restriction.

## [3.0.0] - 2026-03-20

### Changed
- Major refactoring of the data structs for services modules.
- Major refactoring of the base class (inteface definition) of the services and all associated classes.
- Major refactoring of the HRTF class to support multiple IR measurement distances. More minor changes in related classes.
- The HRTF class, with all the changes, is now called CSphericalInterpolatedFIRTable. It now supports the storage of various data types, not just HRTFs.
- The service for storing SOS IIR filters has been refactored and renamed to CSphericalSOSTable.
- Many of the processing classes have had to be modified to adapt to the major changes in the service modules.
- Changes to the SOFA file readers.

### Added
- New data structures for storing information based on a reference point and distance.
- A new generic data class, called SphericalFIRTable, which allows FIR data to be stored. Specifically, it allows HRTFs, BRIR, source directivity and FIR filter characteristics to be stored. It organises them according to reference position and distance. It does not perform any form of interpolation.
- Support for a new source directivity convention called SourceDirectivityFIR.
- New SOFA readers

## Removed
- Support for Directivity TF-based files has been removed.
- The FIR filter service has been removed; this is now supported by the new SphericalFIRTable service

## Fixed
- A few minor errors have been corrected here and there.
- Standardise the names of the methods in the bilateral filters.
- The absorption coefficients for the walls were not updated correctly in the SDN model if changes were made before the room was defined within it. 

## [2.5.0] - 2026-01-12

### Changed
- The classes related to filtering have been refactored. The aim was to achieve a generic and flexible implementation based on a general interface. 
This will allow them to be used in the same way in any processing chain we wish. This has meant that:
	- existing classes, in addition to being modified, have been moved from one folder and namespace to another.
	- new classes and folders have been created.
- The basic processing classes CBiquadFilter and CBiquadFilterChain have been moved to the processing modules folder and namespace. CBiquadFilterChain has been renamed from its original name.
- Classes that perform filtering, such as the SOS and CascadeGraphicEq9OctaveBands filters, have been moved to the filters folder and namespace. 
In addition to this, new filtering elements have been implemented. See the ‘Additions’ section.
- The name of the service module that stores the SOS filter coefficients has been changed. We believe the previous name was confusing. 
- The name of the bilateral filter models has been changed, with the suffix ‘Models’ added. This change is based on maintaining the same naming convention as the rest of the models. 
- The SOFA loader has been modified to read responses to the impulse of FIR filters and store them in the new GeneralFIR service module.
- The parent class of the models has been improved to unify common attributes and methods.

### Added
- Signal processing classes for filtering have been added (and refactored). Now there are classes for: 
	- FIR filtering based on convolution with impulse responses. For spatialisation-oriented and non-spatialisation-oriented situations.
	- IIR filtering based on biquad filter chains. For spatialisation-oriented and non-spatialisation-oriented situations.
- The signal processing class ‘CMultichannelBiquadFilterChain’ has been created, allowing the implementation of simultaneous N-channel biquad IIR filters. 
- A new class has been added for convolution in the frequency domain of the signal with the impulse response of FIR filters.
- A new service module has been created to store the impulse response. This new service module does not perform any type of interpolation after loading the impulse responses.
- A class has been added to search for the response closest to a given impulse using a KD tree.
	
## Fixed
- The source position was not propagated when the free environment model was disabled.
- Distance attenuation is added to the propagation model of virtual sources in the ISM environment model.
- Fixed a bug so that in bilateral filter models, the gain is not applied when the model is disabled.

## [2.4.0] - 2025-11-18

### Added
- New environment simulation model based on the image method (ISM)
- Added new 9 octave graphic equaliser filter.
- New OBJ file reader for room definition.

### Changed
- The waveguides now include a configurable filter. For now, they only support 9 octave graphic equaliser filter, but this limitation will be expanded in the next release.
- The definitions of rooms and walls have been improved and expanded.
- Rooms become a service, like other resources. This means they can be changed and configured in real time.

## Fixed
- The SOFA file reader now controls more parameters to prevent the loading of malformed files.
- The BRTManager adds lock control to prevent certain types of collisions.

## [2.3.1] - 2025-08-28

### Changed
- After testing it, we have decided that the normalization feature is not useful, so we have finally removed it from the audio mixer. In fact, including it in the previous release was a mistake.
From now on, we will revert to the classic behavior of simply adding together buffers from different sources. Any problems with samples that fall outside the dynamic range will need to be addressed from the application's point of view.

## [2.3.0] - 2025-07-29

### Changed
- The names of all listener models have been changed, including the bilateral filter (previously called binaural) and the omnidirectional source model (previously called simple). 
This way, a more accurate nomenclature of names is now used, which we believe will be easier to understand. This new nomenclature of names was presented in an article entitled "NEW RELEASE OF THE BINAURAL RENDERING TOOLBOX: INTRODUCING VERSION 2.0" presented at the Acusticum 2025 Forum. https://www.fa-euronoise2025.org/w/forumacusticumeuronoise/getView/posterView?pos=6794b42969545&section=178145&referer=848202

## Fixed
- Fixed several minor errors in the code that caused compilation errors on platforms other than Visual Studio.

## [2.2.0] - 2025-04-04

### Added
- Adds a distance attenuator to the listener and enviroment models.

### Changed
- The free field environment model allows you to configure its attenuation factor value per distance individually. 
- Changes the behaviour of the SDN environment model when disabled, it now lets the signal pass through unaltered.
- Renames loadBRIR window parameters

### Fixed
- Fixes errors when reconfiguring the ambisonic during rendering
- Solve SDN model failure when receiving an internal control message


## [2.1.0] - 2025-02-26

### Added
- An RMS meter is added, which will allow the dBFS signal level to be measured when required. It works by means of a sliding window of configurable size.
- A new centralized audio mixer that adds normalization

### Changed
- The libmysofa library has been updated to the latest commit, dated 7 February 2025. 
- Some warning messages that go to the log file are changed in the HRTF and Vector3 classes. This was done in order to reduce the number of error lines and to clarify which ones are logged.

### Fixed
- Fixed environment model bug that prevented different environment model instances from generating virtual sources from the same sound source.
- Resolves an error in the binaural filter class when no filter coefficients are found in the table. This can occur in exceptional situations and the failure caused the system to crash.
- Fixed a problem that arose when more than one listener model was connected to a binaural filter and one of those listeners was not connected to any source in the rederising. This problem caused the binaural filter to lose frames and therefore a bad sound.
- Fixed several bugs in the waveguide that arose when the listener moved so close to the source that they put it inside their head and also when they moved while keeping the source inside their head.A situation that seems strange but which can occur depending on the desired simulation.
- Fixes inter-thread collision in HRTF Convolver when resetting buffers while a frame was being processed. Related with Issue #16
- Fixed head radius loaded from SOFA file not being stored correctly.


## [2.0.1] - 2025-01-16

### Fixed
- When the propagation delay was deactivated the waveguide did not correctly provide the instantaneous source position, this led to a malfunction of the free environment model.
- The binaural filter processor checks that the distance between source and listener is zero, to avoid a catastrophic failure of the application.  Instead it generates an error, to the log file, and continues to run. 

## [2.0.0] - 2024-12-20

### Added
- A new namespace and folder has been created to centralise the classes that manage the connectivity between modules.
- Added implementation of binaural filters based on second order sections loaded from a SOFA file.
- An independent and adjustable gain control has been added for each model.
- Propagation delay simulation has been added to the free-field environment model.

### Changed
- The classes related to connectivity have been moved to the new folder and namespace created for this purpose.
- The location and namespace of the base classes of the environment, listener and source models have been changed. 
- The source models have been simplified to make them easier to use internally, in the same style as the rest of the model.
- Nearfield processing is now handled by a more generic class called BinauralFilter.
- The method for reading SOFA files of type SOS has been renamed.
- The headers for all files have been updated.

### Fixed
- Audio sources will not be erased properly when using the free field environment model.

## [1.7.0] - 2024-10-29

### Added
- New free field environment model. This is a preliminary version of a free-field environment model, so far it only implements distance attenuation. It will soon include propagation delay and long-distance filtering of the signal.
- A couple of classes, BRTConnectivity and ModelBase, have been created to simplify the architecture of inheritance between classes.

### Changed
- Classes and folders of the SDN environment model have been renamed for a simpler and more understandable architecture.

### Removed
- Outdated example classes

## [1.6.0] - 2024-10-15

### Added
- New model of virtual source environment based on SDN.
- Add room definition classes from 3DTI-Toolkit.

### Changed
- Add environment connection and disconnection methods to HRTF and Ambisonic HRTF models.
- Update Environment Model Base
- Update source type options.
- Update virtual source definition
- Adds funcionalities to some common clases


## [1.5.0] - 2024-07-30

### Added
- Added a couple of new listener models for reverberation rendering based on BRIR. One of the models performs direct convolution with the BRIR, the other one performs convolution in the ambisonic domain.
- The listener class has been created as its own entity separate from the models, its motivation is to represent real listeners. Among other things it acts as a mixer for different rendering models. This does not allow to render a scene for different listeners simultaneously.
- Loading of SOFA files with the FIR-E type is now supported.
- The BRIR charger supports truncation of the impulse responses both at the beginning and at the end. That is, you can fade-in and fade-out the impulse responses.

### Fixed
- The source directivity file is not loaded if the buffer size does not match the set buffer size.

### Changed
- The SOFA charger is now convention-agnostic. The loading is done on a data type basis and not on a convention basis. Right now we support FIR or FIR-E types for HRTF or BRIR loading. The TF type for loading the source directivity and the SOS type for loading the coefficients of the near-field simulation filters.
- The pre-existing listener models have been modified to accommodate the changes in the architecture.


## [1.4.0] - 2024-06-04

### Fixed

- Fixes an accuracy error in a calculation performed during the SOFA interpolation offline process. 

### Changed

- The operation of the ITD simulation and parallax is fine-tuned. From now on, the ITD simulation and parallax can be activated and deactivated via the listener model. It is in the HRTF class where the radius of the listener's head can be configured and it can be configured to calculate the delays (for the ITD simulation) by means of the woodoworth formula or by reading the integrated ones in the SOFA file.


## [1.3.1] - 2024-03-19

### Fixed

- To avoid errors, it is checked, when loading a SOFA file of HRTFs, that the IRs are at a distance greater than zero. 
- Fixes error that occur when loading an HRTFs SOFA file that only contains IRs in an azimuth ring.
- Fixed listener configuration loss when deleting all present sources.
- The near field compensation crossfading process is reset when the processor is reset.
- Crossfading process of the near field compensation is now not applied in the first buffer.
- A number of minor errors have been resolved.

### Changed

- ILD class has been renamed to NearFieldCompensationFilters.
