# Changelog

All notable changes to the Binaural Rendering Toolbox (BRT) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
