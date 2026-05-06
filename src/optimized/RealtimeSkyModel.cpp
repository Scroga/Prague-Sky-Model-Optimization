// Copyright 2022 Charles University
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <cstring>

#include "RealtimeSkyModel.h"

/////////////////////////////////////////////////////////////////////////////////////
// Data reading
/////////////////////////////////////////////////////////////////////////////////////

void RealtimeSkyModel::readRadiance(FILE* handle, const double singleVisibility) {
				// Read metadata.

				// Structure of the metadata part of the data file:
				// visibilityCount      (1 * int), visibilities													   (visibilityCount * double),
				// albedoCount          (1 * int), albedos																						   (albedoCount * double),
				// altitudeCount        (1 * int), altitudes																				 (altitudeCount * double),
				// elevationCount       (1 * int), elevations																			(elevationCount * double),
				// channels             (1 * int), channelStart   (1 * double), channelWidth (1 * double), 
				// rankRad              (1 * int), 
				// sunBreaksCountRad    (1 * int), sunBreaksRad													 (sunBreaksCountRad * double), 
				// zenithBreaksCountRad (1 * int), zenithBreaksRad								(zenithBreaksCountRad * double), 
				// emphBreaksCountRad   (1 * int), emphBreaksRad								    (emphBreaksCountRad * double)

				size_t valsRead;

				int visibilityCount = 0;
				valsRead = fread(&visibilityCount, sizeof(int), 1, handle);
				if (valsRead != 1 || visibilityCount < 1)
								throw DatasetReadException("visibilityCountRad");

				std::vector<double> visibilitiesRadInFile;
				visibilitiesRadInFile.resize(visibilityCount);
				valsRead = fread(visibilitiesRadInFile.data(), sizeof(double), visibilityCount, handle);
				if (valsRead != visibilityCount)
								throw DatasetReadException("visibilitesRad");

				int skippedVisibilities = 0;
				if (singleVisibility <= 0.0 || visibilityCount <= 1) {
								// If the given single visibility value is not valid or there is just one visibility present (so there
								// is nothing to save), all visibilities present are loaded.
								visibilitiesRad.resize(visibilityCount);
								visibilitiesRad = visibilitiesRadInFile;
				}
				else {
								// Otherwise, only the two visibilities closest to the single visibility value are loaded and the rest
								// is skipped.
								if (singleVisibility <= visibilitiesRadInFile.front()) {
												visibilitiesRad.resize(1);
												visibilitiesRad[0] = visibilitiesRadInFile.front();
								}
								else if (singleVisibility >= visibilitiesRadInFile.back()) {
												visibilitiesRad.resize(1);
												visibilitiesRad[0] = visibilitiesRadInFile.back();
												skippedVisibilities = visibilitiesRadInFile.size() - 1;
								}
								else {
												int visIdx = 0;
												while (singleVisibility >= visibilitiesRadInFile[visIdx]) {
																++visIdx;
												}
												visibilitiesRad.resize(2);
												visibilitiesRad[0] = visibilitiesRadInFile[visIdx - 1];
												visibilitiesRad[1] = visibilitiesRadInFile[visIdx];
												skippedVisibilities = visIdx - 1;
								}
				}

				int albedoCount = 0;
				valsRead = fread(&albedoCount, sizeof(int), 1, handle);
				if (valsRead != 1 || albedoCount < 1)
								throw DatasetReadException("albedoCountRad");

				albedosRad.resize(albedoCount);
				valsRead = fread(albedosRad.data(), sizeof(double), albedoCount, handle);
				if (valsRead != albedoCount)
								throw DatasetReadException("albedosRad");

				int altitudeCount = 0;
				valsRead = fread(&altitudeCount, sizeof(int), 1, handle);
				if (valsRead != 1 || altitudeCount < 1)
								throw DatasetReadException("altitudeCountRad");

				altitudesRad.resize(altitudeCount);
				valsRead = fread(altitudesRad.data(), sizeof(double), altitudeCount, handle);
				if (valsRead != altitudeCount)
								throw DatasetReadException("altitudesRad");

				int elevationCount = 0;
				valsRead = fread(&elevationCount, sizeof(int), 1, handle);
				if (valsRead != 1 || elevationCount < 1)
								throw DatasetReadException(" elevationCountRad");

				elevationsRad.resize(elevationCount);
				valsRead = fread(elevationsRad.data(), sizeof(double), elevationCount, handle);
				if (valsRead != elevationCount)
								throw DatasetReadException("elevationsRad");

				valsRead = fread(&channels, sizeof(int), 1, handle);
				if (valsRead != 1 || channels < 1)
								throw DatasetReadException("channels");

				valsRead = fread(&channelStart, sizeof(double), 1, handle);
				if (valsRead != 1 || channelStart < 0)
								throw DatasetReadException("channelStart");

				valsRead = fread(&channelWidth, sizeof(double), 1, handle);
				if (valsRead != 1 || channelWidth <= 0)
								throw DatasetReadException("channelWidth");

				totalConfigs =
								channels * elevationsRad.size() * altitudesRad.size() * albedosRad.size() * visibilitiesRad.size();

				skippedConfigsBegin =
								channels * elevationsRad.size() * altitudesRad.size() * albedosRad.size() * skippedVisibilities;

				skippedConfigsEnd = channels * elevationsRad.size() * altitudesRad.size() * albedosRad.size() *
								(visibilitiesRadInFile.size() - skippedVisibilities - visibilitiesRad.size());

				valsRead = fread(&metadataRad.rank, sizeof(int), 1, handle);
				if (valsRead != 1 || metadataRad.rank < 1)
								throw DatasetReadException("rankRad");

				int sunBreaksCount = 0;
				valsRead = fread(&sunBreaksCount, sizeof(int), 1, handle);
				if (valsRead != 1 || sunBreaksCount < 2)
								throw DatasetReadException("sunBreaksCountRad");

				metadataRad.sunBreaks.resize(sunBreaksCount);
				valsRead = fread(metadataRad.sunBreaks.data(), sizeof(double), sunBreaksCount, handle);
				if (valsRead != sunBreaksCount)
								throw DatasetReadException("sunBreaksRad");

				int zenitBreaksCount = 0;
				valsRead = fread(&zenitBreaksCount, sizeof(int), 1, handle);
				if (valsRead != 1 || zenitBreaksCount < 2)
								throw DatasetReadException("zenitBreaksCountRad");

				metadataRad.zenithBreaks.resize(zenitBreaksCount);
				valsRead = fread(metadataRad.zenithBreaks.data(), sizeof(double), zenitBreaksCount, handle);
				if (valsRead != zenitBreaksCount)
								throw DatasetReadException("zenithBreaksRad");

				int emphBreaksCount = 0;
				valsRead = fread(&emphBreaksCount, sizeof(int), 1, handle);
				if (valsRead != 1 || emphBreaksCount < 2)
								throw DatasetReadException("emphBreaksCountRad");

				metadataRad.emphBreaks.resize(emphBreaksCount);
				valsRead = fread(metadataRad.emphBreaks.data(), sizeof(double), emphBreaksCount, handle);
				if (valsRead != emphBreaksCount)
								throw DatasetReadException("emphBreaksRad");

				// Calculate offsets and strides.

				metadataRad.sunOffset = 0;
				metadataRad.sunStride = metadataRad.sunBreaks.size() + metadataRad.zenithBreaks.size();

				metadataRad.zenithOffset = metadataRad.sunOffset + metadataRad.sunBreaks.size();
				metadataRad.zenithStride = metadataRad.sunStride;

				metadataRad.emphOffset = metadataRad.sunOffset + metadataRad.rank * metadataRad.sunStride;

				metadataRad.totalCoefsSingleConfig =
								metadataRad.emphOffset + metadataRad.emphBreaks.size(); // this is for one specific configuration
				metadataRad.totalCoefsAllConfigs = metadataRad.totalCoefsSingleConfig * totalConfigs;

				// Read data.

				// Structure of the data part of the data file:
				// [[[[[[ sunCoefsRad       (sunBreaksCountRad * half), zenithScale (1 * double), 
				//        zenithCoefsRad (zenithBreaksCountRad * half) ] * rankRad, 
				//        emphCoefsRad     (emphBreaksCountRad * half) ]
				//  * channels ] * elevationCount ] * altitudeCount ] * albedoCount ] * visibilityCount

				int offset = 0;
				dataRad.resize(metadataRad.totalCoefsAllConfigs);

				std::vector<uint16> radianceTemp;
				radianceTemp.resize(std::max(metadataRad.sunBreaks.size(),
								std::max(metadataRad.zenithBreaks.size(), metadataRad.emphBreaks.size())));

				size_t oneConfigByteCount =
								((metadataRad.sunBreaks.size() + metadataRad.zenithBreaks.size()) * sizeof(uint16) + sizeof(double)) *
								metadataRad.rank +
								metadataRad.emphBreaks.size() * sizeof(uint16);

				// If a single visibility was requested, skip all configurations from the beginning till those needed for
				// the requested visibility.
				fseek(handle, oneConfigByteCount * skippedConfigsBegin, SEEK_CUR);

				// Read configurations needed for the requested visibility (or all if none requested).
				for (int con = 0; con < totalConfigs; ++con) {
								for (int r = 0; r < metadataRad.rank; ++r) {
												// Read sun params.
												valsRead = fread(radianceTemp.data(), sizeof(uint16), metadataRad.sunBreaks.size(), handle);
												if (valsRead != metadataRad.sunBreaks.size())
																throw DatasetReadException("sunCoefsRad");

												// Unpack sun params from half.
												for (int i = 0; i < metadataRad.sunBreaks.size(); ++i) {
																dataRad[offset++] = float(doubleFromHalf(radianceTemp[i]));
												}

												// Read scaling factor for zenith params.
												double zenithScale;
												valsRead = fread(&zenithScale, sizeof(double), 1, handle);
												if (valsRead != 1)
																throw DatasetReadException("zenithScaleRad");

												// Read zenith params.
												valsRead = fread(radianceTemp.data(), sizeof(uint16), metadataRad.zenithBreaks.size(), handle);
												if (valsRead != metadataRad.zenithBreaks.size())
																throw DatasetReadException("zenithCoefsRad");

												// Unpack zenith params from half (these need additional rescaling).
												for (int i = 0; i < metadataRad.zenithBreaks.size(); ++i) {
																dataRad[offset++] = float(doubleFromHalf(radianceTemp[i]) / zenithScale);
												}
								}

								// Read emphasize params.
								valsRead = fread(radianceTemp.data(), sizeof(uint16), metadataRad.emphBreaks.size(), handle);
								if (valsRead != metadataRad.emphBreaks.size())
												throw DatasetReadException("emphCoefsRad");

								// Unpack emphasize params from half.
								for (int i = 0; i < metadataRad.emphBreaks.size(); ++i) {
												dataRad[offset++] = float(doubleFromHalf(radianceTemp[i]));
								}
				}

				// Skip remaining configurations till the end.
				fseek(handle, oneConfigByteCount * skippedConfigsEnd, SEEK_CUR);
}

void RealtimeSkyModel::readTransmittance(FILE* handle) {
				// Read metadata

				size_t valsRead;

				valsRead = fread(&dDim, sizeof(int), 1, handle);
				if (valsRead != 1 || dDim < 1)
								throw DatasetReadException("dDim");

				valsRead = fread(&aDim, sizeof(int), 1, handle);
				if (valsRead != 1 || aDim < 1)
								throw DatasetReadException("aDim");

				int visibilitiesCount = 0;
				valsRead = fread(&visibilitiesCount, sizeof(int), 1, handle);
				if (valsRead != 1 || visibilitiesCount < 1)
								throw DatasetReadException("visibilitiesCountTrans");

				int altitudesCount = 0;
				valsRead = fread(&altitudesCount, sizeof(int), 1, handle);
				if (valsRead != 1 || altitudesCount < 1)
								throw DatasetReadException("altitudesCountTrans");

				valsRead = fread(&rankTrans, sizeof(int), 1, handle);
				if (valsRead != 1 || rankTrans < 1)
								throw DatasetReadException("rankTrans");

				std::vector<float> temp;
				temp.resize(std::max(altitudesCount, visibilitiesCount));

				altitudesTrans.resize(altitudesCount);
				valsRead = fread(temp.data(), sizeof(float), altitudesCount, handle);
				if (valsRead != altitudesCount)
								throw DatasetReadException("altitudesTrans");
				for (int i = 0; i < altitudesCount; i++) {
								altitudesTrans[i] = double(temp[i]);
				}

				visibilitiesTrans.resize(visibilitiesCount);
				valsRead = fread(temp.data(), sizeof(float), visibilitiesCount, handle);
				if (valsRead != visibilitiesCount)
								throw DatasetReadException("visibilitiesTrans");
				for (int i = 0; i < visibilitiesCount; i++) {
								visibilitiesTrans[i] = double(temp[i]);
				}

				const size_t totalCoefsU = dDim * aDim * rankTrans * altitudesTrans.size();
				const size_t totalCoefsV = visibilitiesTrans.size() * rankTrans * channels * altitudesTrans.size();

				// Read data

				dataTransU.resize(totalCoefsU);
				valsRead = fread(dataTransU.data(), sizeof(float), totalCoefsU, handle);
				if (valsRead != totalCoefsU)
								throw DatasetReadException("datasetTransU");

				dataTransV.resize(totalCoefsV);
				valsRead = fread(dataTransV.data(), sizeof(float), totalCoefsV, handle);
				if (valsRead != totalCoefsV)
								throw DatasetReadException("datasetTransV");
}

void RealtimeSkyModel::readPolarisation(FILE* handle) {
				// Read metadata.

				// Structure of the metadata part of the data file:
				// rankPol              (1 * int), 
				// sunBreaksCountPol    (1 * int), sunBreaksPol    (sunBreaksCountPol * double),
				// zenithBreaksCountPol (1 * int), zenithBreaksPol (zenithBreaksCountPol * double), 

				size_t valsRead;

				valsRead = fread(&metadataPol.rank, sizeof(int), 1, handle);
				if (valsRead != 1) {
								// Polarisation dataset not present
								metadataPol.rank = 0;
								return;
				}

				int sunBreaksCount = 0;
				valsRead = fread(&sunBreaksCount, sizeof(int), 1, handle);
				if (valsRead != 1 || sunBreaksCount < 1)
								throw DatasetReadException("sunBreaksCountPol");

				metadataPol.sunBreaks.resize(sunBreaksCount);
				valsRead = fread(metadataPol.sunBreaks.data(), sizeof(double), sunBreaksCount, handle);
				if (valsRead != sunBreaksCount)
								throw DatasetReadException("sunBreaksPol");

				int zenithBreaksCount = 0;
				valsRead = fread(&zenithBreaksCount, sizeof(int), 1, handle);
				if (valsRead != 1 || zenithBreaksCount < 1)
								throw DatasetReadException("zenithBreaksCountPol");

				metadataPol.zenithBreaks.resize(zenithBreaksCount);
				valsRead = fread(metadataPol.zenithBreaks.data(), sizeof(double), zenithBreaksCount, handle);
				if (valsRead != zenithBreaksCount)
								throw DatasetReadException("zenithBreaksPol");

				metadataPol.emphBreaks.clear();

				// Calculate offsets and strides.

				metadataPol.sunOffset = 0;
				metadataPol.sunStride = metadataPol.sunBreaks.size() + metadataPol.zenithBreaks.size();

				metadataPol.zenithOffset = metadataPol.sunOffset + metadataPol.sunBreaks.size();
				metadataPol.zenithStride = metadataPol.sunStride;

				metadataPol.emphOffset = 0;

				metadataPol.totalCoefsSingleConfig =
								metadataPol.sunOffset +
								metadataPol.rank * metadataPol.sunStride; // this is for one specific configuration
				metadataPol.totalCoefsAllConfigs = metadataPol.totalCoefsSingleConfig * totalConfigs;

				// Read data.

				// Structure of the data part of the data file:
				// [[[[[[ sunCoefsPol       (sunBreaksCountPol * float), 
				//        zenithCoefsPol (zenithBreaksCountPol * float) ] * rankPol] 
				// * channels ] * elevationCount ] * altitudeCount ] * albedoCount ] * visibilityCount

				size_t offset = 0;
				dataPol.resize(metadataPol.totalCoefsAllConfigs);

				size_t oneConfigByteCount =
								(metadataPol.sunBreaks.size() + metadataPol.zenithBreaks.size()) * sizeof(float) * metadataPol.rank;

				// If a single visibility was requested, skip all configurations from the beginning till those needed for
				// the requested visibility.
				fseek(handle, oneConfigByteCount * skippedConfigsBegin, SEEK_CUR);

				for (int con = 0; con < totalConfigs; ++con) {
								for (int r = 0; r < metadataPol.rank; ++r) {
												// Read sun params.
												valsRead = fread(dataPol.data() + offset, sizeof(float), metadataPol.sunBreaks.size(), handle);
												if (valsRead != metadataPol.sunBreaks.size())
																throw DatasetReadException("sunCoefsPol");
												offset += metadataPol.sunBreaks.size();

												// Read zenith params.
												valsRead = fread(dataPol.data() + offset, sizeof(float), metadataPol.zenithBreaks.size(), handle);
												if (valsRead != metadataPol.zenithBreaks.size())
																throw DatasetReadException("zenithCoefsPol");
												offset += metadataPol.zenithBreaks.size();
								}
				}
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialization
/////////////////////////////////////////////////////////////////////////////////////
void RealtimeSkyModel::initialize(const std::string& filename, const double singleVisibility) {
				if (FILE* handle = fopen(filename.c_str(), "rb")) {
								initialized = false;
								// Read data
								readRadiance(handle, singleVisibility);
								readTransmittance(handle);
								readPolarisation(handle);
								fclose(handle);
								initialized = true;
				}
				else {
								throw DatasetNotFoundException(filename);
				}
}

AvailableData RealtimeSkyModel::getAvailableData() const {
				if (!initialized) {
								throw NotInitializedException();
				}

				AvailableData available;
				available.albedoMin = albedosRad.front();
				available.albedoMax = albedosRad.back();
				available.altitudeMin = altitudesRad.front();
				available.altitudeMax = altitudesRad.back();
				available.elevationMin = elevationsRad.front();
				available.elevationMax = elevationsRad.back();
				available.visibilityMin = visibilitiesRad.front();
				available.visibilityMax = visibilitiesRad.back();
				available.polarisation = (metadataPol.rank != 0);
				available.channels = channels;
				available.channelStart = channelStart;
				available.channelWidth = channelWidth;

				return available;
}

/////////////////////////////////////////////////////////////////////////////////////
// Filling the Parameters structure
/////////////////////////////////////////////////////////////////////////////////////

Parameters RealtimeSkyModel::computeParameters(
				const Vector3& viewpoint,
				const Vector3& viewDirection,
				const double   groundLevelSolarElevationAtOrigin,
				const double   groundLevelSolarAzimuthAtOrigin,
				const double   visibility,
				const double   albedo) const {

				assert(viewpoint.z >= 0.0);
				assert(magnitude(viewDirection) > 0.0);
				assert(visibility >= 0.0);
				assert(albedo >= 0.0 && albedo <= 1.0);

				Parameters params;
				params.visibility = visibility;
				params.albedo = albedo;

				// Shift viewpoint about safety altitude up

				const Vector3 centerOfTheEarth = Vector3(0.0, 0.0, -PLANET_RADIUS);
				Vector3       toViewpoint = viewpoint - centerOfTheEarth;
				Vector3       toViewpointN = normalize(toViewpoint);
				const double  distanceToView = magnitude(toViewpoint) + SAFETY_ALTITUDE;
				Vector3       toShiftedViewpoint = toViewpointN * distanceToView;
				Vector3       shiftedViewpoint = centerOfTheEarth + toShiftedViewpoint;

				Vector3 viewDirectionN = normalize(viewDirection);

				// Compute altitude of viewpoint

				params.altitude = distanceToView - PLANET_RADIUS;
				params.altitude = std::max(params.altitude, 0.0);

				// Direction to sun

				Vector3 directionToSunN;
				directionToSunN.x = cos(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
				directionToSunN.y = sin(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
				directionToSunN.z = sin(groundLevelSolarElevationAtOrigin);

				// Solar elevation at viewpoint (more precisely, solar elevation at the point
				// on the ground directly below viewpoint)

				const double dotZenithSun = dot(toViewpointN, directionToSunN);
				params.elevation = 0.5 * PI - acos(dotZenithSun);

				// Altitude-corrected view direction

				Vector3 correctViewN;
				if (distanceToView > PLANET_RADIUS) {
								Vector3 lookAt = shiftedViewpoint + viewDirectionN;

								const double correction =
												sqrt(distanceToView * distanceToView - PLANET_RADIUS * PLANET_RADIUS) / distanceToView;

								Vector3 toNewOrigin = toViewpointN * (distanceToView - correction);
								Vector3 newOrigin = centerOfTheEarth + toNewOrigin;
								Vector3 correctView = lookAt - newOrigin;

								correctViewN = normalize(correctView);
				}
				else {
								correctViewN = viewDirectionN;
				}

				// Sun angle (gamma) - no correction

				double dotProductSun = dot(viewDirectionN, directionToSunN);
				params.gamma = acos(dotProductSun);

				// Shadow angle - requires correction

				const double effectiveElevation = groundLevelSolarElevationAtOrigin;
				const double effectiveAzimuth = groundLevelSolarAzimuthAtOrigin;
				const double shadowAngle = effectiveElevation + PI * 0.5;

				Vector3 shadowDirectionN = Vector3(cos(shadowAngle) * cos(effectiveAzimuth),
								cos(shadowAngle) * sin(effectiveAzimuth),
								sin(shadowAngle));

				const double dotProductShadow = dot(correctViewN, shadowDirectionN);
				params.shadow = acos(dotProductShadow);

				// Zenith angle (theta) - corrected version stored in otherwise unused zero
				// angle

				double cosThetaCor = dot(correctViewN, toViewpointN);
				params.zero = acos(cosThetaCor);

				// Zenith angle (theta) - uncorrected version goes outside

				double cosTheta = dot(viewDirectionN, toViewpointN);
				params.theta = acos(cosTheta);

				return params;
}

RealtimeSkyModel::FrameInterpolationParameters RealtimeSkyModel::computeFrameInterpolationParameters(
				const Vector3& viewpoint,
				const double   groundLevelSolarElevationAtOrigin,
				const double   groundLevelSolarAzimuthAtOrigin,
				const double   visibility,
				const double   albedo) {

				assert(viewpoint.z >= 0.0);
				assert(visibility >= 0.0);
				assert(albedo >= 0.0 && albedo <= 1.0);

				// Compute frame parameters
				if (initializedFrameParams == false) 
								frameParams = FrameParameters{};

				frameParams.visibility = visibility;
				frameParams.albedo = albedo;

				const Vector3 centerOfTheEarth = Vector3(0.0, 0.0, -PLANET_RADIUS);
				Vector3       toViewpoint = viewpoint - centerOfTheEarth;
				Vector3       toViewpointN = normalize(toViewpoint);
				const double  distanceToView = magnitude(toViewpoint) + SAFETY_ALTITUDE;
				Vector3       toShiftedViewpoint = toViewpointN * distanceToView;
				Vector3       shiftedViewpoint = centerOfTheEarth + toShiftedViewpoint;

				frameParams.shiftedViewpoint = shiftedViewpoint;
				frameParams.distanceToView = distanceToView;
				frameParams.toViewpointN = toViewpointN;
				
				frameParams.altitude = std::max(distanceToView - PLANET_RADIUS, 0.0);

				frameParams.directionToSunN.x = cos(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
				frameParams.directionToSunN.y = sin(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
				frameParams.directionToSunN.z = sin(groundLevelSolarElevationAtOrigin);

				const double dotZenithSun = dot(toViewpointN, frameParams.directionToSunN);
				frameParams.elevation = 0.5 * PI - acos(dotZenithSun);

				const double effectiveElevation = groundLevelSolarElevationAtOrigin;
				const double effectiveAzimuth = groundLevelSolarAzimuthAtOrigin;
				const double shadowAngle = effectiveElevation + PI * 0.5;

				Vector3 shadowDirectionN = Vector3(cos(shadowAngle) * cos(effectiveAzimuth),
								cos(shadowAngle) * sin(effectiveAzimuth),
								sin(shadowAngle));

				frameParams.shadowDirectionN = shadowDirectionN;

				initializedFrameParams = true;

				// Compute frame interpolation parameters
				FrameInterpolationParameters interParams{};
				interParams.visibility = getInterpolationParameter(frameParams.visibility, visibilitiesRad);
				interParams.albedo = getInterpolationParameter(frameParams.albedo, albedosRad);
				interParams.altitude = getInterpolationParameter(frameParams.altitude, altitudesRad);
				interParams.elevation = getInterpolationParameter(radiansToDegrees(frameParams.elevation), elevationsRad);

				return interParams;
}

RealtimeSkyModel::PixelInterpolationParameters RealtimeSkyModel::computePixelInterpolationParameters(
				const Vector3& viewDirection) const {
			
				if (!initializedFrameParams) {
								throw NotInitializedException();
				}

				const Vector3 centerOfTheEarth = Vector3(0.0, 0.0, -PLANET_RADIUS);
				Vector3 viewDirectionN = normalize(viewDirection);

				Vector3 correctViewN;
				if (frameParams.distanceToView > PLANET_RADIUS) {
								Vector3 lookAt = frameParams.shiftedViewpoint + viewDirectionN;

								const double correction =
												sqrt(frameParams.distanceToView * frameParams.distanceToView 
																- PLANET_RADIUS * PLANET_RADIUS) / frameParams.distanceToView;

								Vector3 toNewOrigin = frameParams.toViewpointN * (frameParams.distanceToView - correction);
								Vector3 newOrigin = centerOfTheEarth + toNewOrigin;
								Vector3 correctView = lookAt - newOrigin;

								correctViewN = normalize(correctView);
				}
				else {
								correctViewN = viewDirectionN;
				}

				// Compute pixel interpolation parameters
				PixelInterpolationParameters iterParams{};

				// Sun angle (gamma) - no correction
				double dotProductSun = dot(viewDirectionN, frameParams.directionToSunN);
				iterParams.gamma = acos(dotProductSun);

				// Zenith angle (theta) - uncorrected version goes outside
				double cosTheta = dot(viewDirectionN, frameParams.toViewpointN);
				iterParams.theta = acos(cosTheta);

				// Shadow angle - requires correction
				const double dotProductShadow = dot(correctViewN, frameParams.shadowDirectionN);
				iterParams.shadow = acos(dotProductShadow);

				// Zenith angle (theta) - corrected version stored in otherwise unused zero angle
				double cosThetaCor = dot(correctViewN, frameParams.toViewpointN);
				iterParams.zero = acos(cosThetaCor);

				// Translate angle values to indices and interpolation factors.
				iterParams.angle = AngleParameters{};
				iterParams.angle.gamma = getInterpolationParameter(iterParams.gamma, metadataRad.sunBreaks);
				if (!metadataRad.emphBreaks.empty()) { // for radiance
								iterParams.angle.alpha =
												getInterpolationParameter(frameParams.elevation < 0.0 ? iterParams.shadow : iterParams.zero,
																metadataRad.zenithBreaks);
								iterParams.angle.zero = getInterpolationParameter(iterParams.zero, metadataRad.emphBreaks);
				}
				else { // for polarisation
								iterParams.angle.alpha = getInterpolationParameter(iterParams.zero, metadataRad.zenithBreaks);
				}

				return iterParams;
}

/////////////////////////////////////////////////////////////////////////////////////
// Model evaluation for sky radiance and polarisation
/////////////////////////////////////////////////////////////////////////////////////

std::vector<float>::const_iterator RealtimeSkyModel::getCoefficients(const std::vector<float>& dataset,
				const int totalCoefsSingleConfig,
				const int elevation,
				const int altitude,
				const int visibility,
				const int albedo,
				const int wavelength) const {
				return dataset.cbegin() +
								(totalCoefsSingleConfig *
												(wavelength + size_t(channels) * elevation + channels * elevationsRad.size() * altitude +
																channels * elevationsRad.size() * altitudesRad.size() * albedo +
																channels * elevationsRad.size() * altitudesRad.size() * albedosRad.size() * visibility));
}

InterpolationParameter RealtimeSkyModel::getInterpolationParameter(
				const double               queryVal,
				const std::vector<double>& breaks) const {
				// Clamp the value to the valid range.
				const double clamped = std::clamp(queryVal, breaks.front(), breaks.back());

				// Get the nearest greater parameter value.
				const auto next = std::upper_bound(breaks.cbegin() + 1, breaks.cend(), clamped);

				// Compute the index and float factor.
				InterpolationParameter parameter;
				parameter.index = int(next - breaks.cbegin()) - 1;
				if (next == breaks.cend()) {
								parameter.factor = 0.0;
				}
				else {
								parameter.factor = (clamped - *(next - 1)) / (*next - *(next - 1));
				}

				assert(0 <= parameter.index && parameter.index < breaks.size() &&
								(parameter.index < breaks.size() - 1 || parameter.factor == 0.f));
				assert(0 <= parameter.factor && parameter.factor <= 1);
				return parameter;
}

double RealtimeSkyModel::reconstruct(
				const AngleParameters& radianceParameters,
				const std::vector<float>::const_iterator channelParameters,
				const Metadata& metadata) const {
				// The original image was emphasized (for radiance only), re-parametrized into gamma-alpha space,
				// decomposed into sum of rankRad outer products of 'sun' and 'zenith' vectors and these vectors were
				// stored as piece-wise polynomial approximation. Here the process is reversed (for one point).

				double result = 0.0;
				for (int r = 0; r < metadata.rank; ++r) {
								// Restore the right value in the 'sun' vector
								const double sunParam = evalPL(channelParameters + metadata.sunOffset + r * metadata.sunStride +
												radianceParameters.gamma.index,
												radianceParameters.gamma.factor);

								// Restore the right value in the 'zenith' vector
								const double zenithParam = evalPL(channelParameters + metadata.zenithOffset +
												r * metadata.zenithStride + radianceParameters.alpha.index,
												radianceParameters.alpha.factor);

								// Accumulate their "outer" product
								result += sunParam * zenithParam;
				}

				// De-emphasize (for radiance only)
				if (!metadata.emphBreaks.empty()) {
								const double emphParam =
												evalPL(channelParameters + metadata.emphOffset + radianceParameters.zero.index,
																radianceParameters.zero.factor);
								result *= emphParam;
								result = std::max(result, 0.0);
				}

				return result;
}

double RealtimeSkyModel::evaluateModel(const Parameters& params,
				const double              wavelength,
				const std::vector<float>& data,
				const Metadata& metadata) const {
				// Ignore wavelengths outside the dataset range.
				if (wavelength < channelStart || wavelength >= (channelStart + channels * channelWidth)) {
								return 0.0;
				}

				// Don't interpolate wavelengths inside the dataset range.
				const int channelIndex = int(floor((wavelength - channelStart) / channelWidth));

				// Translate angle values to indices and interpolation factors.
				AngleParameters angleParameters;
				angleParameters.gamma = getInterpolationParameter(params.gamma, metadata.sunBreaks);
				if (!metadata.emphBreaks.empty()) { // for radiance
								angleParameters.alpha =
												getInterpolationParameter(params.elevation < 0.0 ? params.shadow : params.zero,
																metadata.zenithBreaks);
								angleParameters.zero = getInterpolationParameter(params.zero, metadata.emphBreaks);
				}
				else { // for polarisation
								angleParameters.alpha = getInterpolationParameter(params.zero, metadata.zenithBreaks);
				}

				// Translate configuration values to indices and interpolation factors.
				const InterpolationParameter visibilityParam =
								getInterpolationParameter(params.visibility, visibilitiesRad);
				const InterpolationParameter albedoParam = getInterpolationParameter(params.albedo, albedosRad);
				const InterpolationParameter altitudeParam = getInterpolationParameter(params.altitude, altitudesRad);
				const InterpolationParameter elevationParam =
								getInterpolationParameter(radiansToDegrees(params.elevation), elevationsRad);

				// Prepare parameters controlling the interpolation.
				ControlParameters controlParameters;
				for (int i = 0; i < 16; ++i) {
								const int visibilityIndex = std::min(visibilityParam.index + i / 8, int(visibilitiesRad.size() - 1));
								const int albedoIndex = std::min(albedoParam.index + (i % 8) / 4, int(albedosRad.size() - 1));
								const int altitudeIndex = std::min(altitudeParam.index + (i % 4) / 2, int(altitudesRad.size() - 1));
								const int elevationIndex = std::min(elevationParam.index + i % 2, int(elevationsRad.size() - 1));

								controlParameters.coefficients[i] = getCoefficients(data,
												metadata.totalCoefsSingleConfig,
												elevationIndex,
												altitudeIndex,
												visibilityIndex,
												albedoIndex,
												channelIndex);
				}
				controlParameters.interpolationFactor[0] = visibilityParam.factor;
				controlParameters.interpolationFactor[1] = albedoParam.factor;
				controlParameters.interpolationFactor[2] = altitudeParam.factor;
				controlParameters.interpolationFactor[3] = elevationParam.factor;

				// Interpolate.
				const double result = interpolate<0, 0>(angleParameters, controlParameters, metadata);
				assert(metadata.emphBreaks.empty() || result >= 0.0); // polarisation can be negative

				return result;
}

/////////////////////////////////////////////////////////////////////////////////////
// Sky radiance
/////////////////////////////////////////////////////////////////////////////////////

double RealtimeSkyModel::skyRadiance(const Parameters& params, const double wavelength) const {
				if (!initialized) {
								throw NotInitializedException();
				}

				return evaluateModel(params, wavelength, dataRad, metadataRad);
}

double RealtimeSkyModel::skyRadiance(
				const PixelInterpolationParameters& pixelIterParams,
				const FrameInterpolationParameters& frameIterParams,
				const double wavelength) const {

				if (!initialized) {
								throw NotInitializedException();
				}
				if (!initializedFrameParams) {
								throw NotInitializedException();
				}

				// Ignore wavelengths outside the dataset range.
				if (wavelength < channelStart || wavelength >= (channelStart + channels * channelWidth)) {
								return 0.0;
				}

				// Don't interpolate wavelengths inside the dataset range.
				const int channelIndex = int(floor((wavelength - channelStart) / channelWidth));

				// Prepare parameters controlling the interpolation.
				ControlParameters controlParameters;
				for (int i = 0; i < 16; ++i) {
								const int visibilityIndex = std::min(frameIterParams.visibility.index + i / 8, int(visibilitiesRad.size() - 1));
								const int albedoIndex = std::min(frameIterParams.albedo.index + (i % 8) / 4, int(albedosRad.size() - 1));
								const int altitudeIndex = std::min(frameIterParams.altitude.index + (i % 4) / 2, int(altitudesRad.size() - 1));
								const int elevationIndex = std::min(frameIterParams.elevation.index + i % 2, int(elevationsRad.size() - 1));

								controlParameters.coefficients[i] = getCoefficients(dataRad,
												metadataRad.totalCoefsSingleConfig,
												elevationIndex,
												altitudeIndex,
												visibilityIndex,
												albedoIndex,
												channelIndex);
				}
				controlParameters.interpolationFactor[0] = frameIterParams.visibility.factor;
				controlParameters.interpolationFactor[1] = frameIterParams.albedo.factor;
				controlParameters.interpolationFactor[2] = frameIterParams.altitude.factor;
				controlParameters.interpolationFactor[3] = frameIterParams.elevation.factor;

				// Interpolate.
				const double result = interpolate<0, 0>(pixelIterParams.angle, controlParameters, metadataRad);
				assert(metadata.emphBreaks.empty() || result >= 0.0); // polarisation can be negative

				return result;
}


/////////////////////////////////////////////////////////////////////////////////////
// Sun radiance
/////////////////////////////////////////////////////////////////////////////////////

double RealtimeSkyModel::sunRadiance(const Parameters& params, const double wavelength) const {
				if (!initialized) {
								throw NotInitializedException();
				}

				// Ignore wavelengths outside the dataset range.
				if (wavelength < SUN_RAD_START || wavelength >= SUN_RAD_END) {
								return 0.0;
				}

				// Return zero for rays not hitting the sun.
				if (params.gamma > SUN_RADIUS) {
								return 0.0;
				}

				// Compute index into the sun radiance table.
				const double idx = (wavelength - SUN_RAD_START) / SUN_RAD_STEP;
				assert(idx >= 0 && idx < std::size(SUN_RAD_TABLE) - 1);
				const int    idxInt = int(floor(idx));
				const double idxFloat = idx - floor(idx);

				// Interpolate between the two closest values in the sun radiance table.
				const double sunRadiance =
								SUN_RAD_TABLE[idxInt] * (1.0 - idxFloat) + SUN_RAD_TABLE[idxInt + 1] * idxFloat;
				assert(sunRadiance > 0.0);

				// Compute transmittance towards the sun.
				const double tau = RealtimeSkyModel::transmittance(params, wavelength, std::numeric_limits<double>::max());
				assert(tau >= 0.0 && tau <= 1.0);

				// Combine.
				return sunRadiance * tau;
}


/////////////////////////////////////////////////////////////////////////////////////
// Polarisation
/////////////////////////////////////////////////////////////////////////////////////

double RealtimeSkyModel::polarisation(const Parameters& params, const double wavelength) const {
				if (!initialized) {
								throw NotInitializedException();
				}

				// If no polarisation data available
				if (metadataPol.rank == 0) {
								throw NoPolarisationException();
				}

				return -evaluateModel(params, wavelength, dataPol, metadataPol);
}


/////////////////////////////////////////////////////////////////////////////////////
// Transmittance
/////////////////////////////////////////////////////////////////////////////////////

std::vector<float>::const_iterator RealtimeSkyModel::getCoefficientsTrans(const int visibility,
				const int altitude,
				const int wavelength) const {
				return dataTransV.cbegin() +
								((visibility * altitudesTrans.size() + altitude) * channels + wavelength) * rankTrans;
}

std::vector<float>::const_iterator RealtimeSkyModel::getCoefficientsTransBase(const int altitude,
				const int a,
				const int d) const {
				return dataTransU.cbegin() + size_t(altitude) * aDim * dDim * rankTrans +
								(size_t(d) * aDim + a) * rankTrans;
}

InterpolationParameter RealtimeSkyModel::getInterpolationParameterTrans(const double value,
				const int    paramCount,
				const int power) const {
				InterpolationParameter param;
				param.index = std::min(int(value * paramCount), paramCount - 1);
				param.factor = 0.0;
				if (param.index < paramCount - 1) {
								param.factor =
												nonlerp(double(param.index) / paramCount, double(param.index + 1) / paramCount, value, power);
								param.factor = std::clamp(param.factor, 0.0, 1.0);
				}
				return param;
}

TransmittanceParameters RealtimeSkyModel::toTransmittanceParams(const double theta,
				const double distance,
				const double altitude) const {
				assert(theta >= 0.0 && theta <= PI);
				assert(distance >= 0.0);
				assert(altitude >= 0.0);

				const double rayDirX = sin(theta);
				const double rayDirY = cos(theta);
				const double rayPosY = PLANET_RADIUS + altitude; // rayPosX == 0

				constexpr double ATMOSPHERE_EDGE = PLANET_RADIUS + ATMOSPHERE_WIDTH;

				// Find intersection of the ground-to-sun ray with edge of the atmosphere (in 2D)
				double           distToIsect = -1.0;
				constexpr double LOW_ALTITUDE = 0.3;
				if (altitude < LOW_ALTITUDE) {
								// Special handling of almost zero altitude case to avoid numerical issues.
								if (theta <= 0.5 * PI) {
												distToIsect = intersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, ATMOSPHERE_EDGE);
								}
								else {
												distToIsect = 0.0;
								}
				}
				else {
								distToIsect = intersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, PLANET_RADIUS);
								if (distToIsect < 0.0) {
												distToIsect = intersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, ATMOSPHERE_EDGE);
								}
				}
				// The ray should always hit either the edge of the atmosphere or the planet (we are starting inside the
				// atmosphere).
				assert(distToIsect >= 0.0);

				distToIsect = std::min(distToIsect, distance);

				// Compute intersection coordinates
				const double isectX = rayDirX * distToIsect;
				const double isectY = rayDirY * distToIsect + rayPosY;

				// Get the internal [altitude, distance] parameters
				const auto [altitudeParam, distanceParam] = isectToAltitudeDistance(isectX, isectY);

				// Convert to interpolation parameters
				TransmittanceParameters params;
				params.altitude = getInterpolationParameterTrans(altitudeParam, aDim, 3);
				params.distance = getInterpolationParameterTrans(distanceParam, dDim, 4);

				return params;
}

double RealtimeSkyModel::reconstructTrans(const int                     visibilityIndex,
				const int                     altitudeIndex,
				const TransmittanceParameters transParams,
				const int                     channelIndex) const {
				const std::vector<float>::const_iterator coefs =
								getCoefficientsTrans(visibilityIndex, altitudeIndex, channelIndex);

				// Load transmittance values for bi-linear interpolation
				std::array<double, 4> transmittance = { 0.0, 0.0, 0.0, 0.0 };
				int                   index = 0;
				for (int a = transParams.altitude.index; a <= transParams.altitude.index + 1; ++a) {
								if (a < aDim) {
												for (int d = transParams.distance.index; d <= transParams.distance.index + 1; ++d) {
																if (d < dDim) {
																				const std::vector<float>::const_iterator baseCoefs =
																								getCoefficientsTransBase(altitudeIndex, a, d);
																				for (int i = 0; i < rankTrans; ++i) {
																								// Reconstruct transmittance value
																								transmittance[index] += double(baseCoefs[i]) * double(coefs[i]);
																				}
																				index++;
																}
												}
								}
				}

				// Perform bi-linear interpolation
				if (transParams.distance.factor > 0.f) {
								transmittance[0] = lerp(transmittance[0], transmittance[1], transParams.distance.factor);
								transmittance[1] = lerp(transmittance[2], transmittance[3], transParams.distance.factor);
				}
				transmittance[0] = std::max(transmittance[0], 0.0);

				if (transParams.altitude.factor > 0.f) {
								transmittance[1] = std::max(transmittance[1], 0.0);
								transmittance[0] = lerp(transmittance[0], transmittance[1], transParams.altitude.factor);
				}

				assert(transmittance[0] >= 0.0);
				return transmittance[0];
}

double RealtimeSkyModel::interpolateTrans(const int                     visibilityIndex,
				const InterpolationParameter  altitudeParam,
				const TransmittanceParameters transParams,
				const int                     channelIndex) const {
				// Get transmittance for the nearest lower altitude.
				double trans = reconstructTrans(visibilityIndex, altitudeParam.index, transParams, channelIndex);

				// Interpolate with transmittance for the nearest higher altitude if needed.
				if (altitudeParam.factor > 0.0) {
								const double transHigh =
												reconstructTrans(visibilityIndex, altitudeParam.index + 1, transParams, channelIndex);
								trans = lerp(trans, transHigh, altitudeParam.factor);
				}

				return trans;
}

double RealtimeSkyModel::transmittance(const Parameters& params,
				const double      wavelength,
				const double      distance) const {
				assert(distance > 0.0);

				if (!initialized) {
								throw NotInitializedException();
				}

				// Ignore wavelengths outside the dataset range.
				if (wavelength < channelStart || wavelength >= (channelStart + channels * channelWidth)) {
								return 0.0;
				}
				// Don't interpolate wavelengths inside the dataset range.
				const int channelIndex = int(floor((wavelength - channelStart) / channelWidth));

				// Translate configuration values to indices and interpolation factors.
				const InterpolationParameter visibilityParam =
								getInterpolationParameter(params.visibility, visibilitiesTrans);
				const InterpolationParameter altitudeParam = getInterpolationParameter(params.altitude, altitudesTrans);

				// Calculate position in the atmosphere.
				const TransmittanceParameters transParams = toTransmittanceParams(params.theta, distance, params.altitude);

				// Get transmittance for the nearest lower visibility.
				double trans = interpolateTrans(visibilityParam.index, altitudeParam, transParams, channelIndex);

				// Interpolate with transmittance for the nearest higher visibility if needed.
				if (visibilityParam.factor > 0.0) {
								const double transHigh =
												interpolateTrans(visibilityParam.index + 1, altitudeParam, transParams, channelIndex);

								trans = lerp(trans, transHigh, visibilityParam.factor);
				}

				// Transmittance is stored as a square root. Needs to be restored here.
				trans = trans * trans;
				assert(trans >= 0.0 && trans <= 1.0);

				return trans;
}
