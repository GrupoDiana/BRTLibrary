# Changelog

All notable changes to the Binaural Rendering Toolbox (BRT) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
