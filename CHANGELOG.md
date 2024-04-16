# Changelog

All notable changes to the Binaural Rendering Toolbox (BRT) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


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
