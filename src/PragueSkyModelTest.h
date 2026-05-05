// Copyright 2022 Charles University
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <execution>
#include <limits>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "PragueSkyModel.h"
#include "RealtimeSkyModel.h"

/////////////////////////////////////////////////////////////////////////////////////
// Rendering
/////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// An example of using Prague Sky Model for rendering a simple fisheye RGB image of the sky.
/// </summary>
/// <param name="model">Reference to the sky model object.</param>
/// <param name="albedo">Ground albedo, value in range [0, 1].</param>
/// <param name="altitude">Altitude of view point in meters, value in range [0, 15000].</param>
/// <param name="azimuth">Sun azimuth at view point in degrees, value in range [0, 360].</param>
/// <param name="elevation">Sun elevation at view point in degrees, value in range [-4.2, 90].</param>
/// <param name="mode">Rendered quantity: sky radiance, sun radiance, polarisation, or transmittance.</param>
/// <param name="resolution">Length of resulting square image size in pixels.</param>
/// <param name="view">Rendered view: up-facing fisheye or side-facing fisheye.</param>
/// <param name="visibility">Horizontal visibility (meteorological range) at ground level in kilometers, value in range [20, 131.8].</param>
/// <param name="outResult">Buffer for storing the resulting images (index 0 = sRGB, index 1 - <# of channels in the dataset> = individual channels).</param>
void originalModelRender(const PragueSkyModel& model,
				const double                     albedo,
				const double                     altitude,
				const double                     azimuth,
				const double                     elevation,
				const Mode                       mode,
				const int                        resolution,
				const View                       view,
				const double                     visibility,
				std::vector<std::vector<float>>& outResult) {
				assert(model.isInitialized());

				// We are viewing the sky from 'altitude' meters above the origin.
				const Vector3 viewPoint = Vector3(0.0, 0.0, altitude);

				// Resize the output buffer.
				outResult.resize(SPECTRUM_CHANNELS + 1);
				outResult[0].resize(size_t(resolution) * resolution * 3); // RGB
				for (int c = 1; c < SPECTRUM_CHANNELS + 1; c++) {
								outResult[c].resize(size_t(resolution) * resolution); // mono
				}

				// Zero the output buffer.
				for (int c = 0; c < SPECTRUM_CHANNELS + 1; c++) {
								for (int i = 0; i < outResult[c].size(); i++) {
												outResult[c][i] = 0.f;
								}
				}

				std::vector<int> xs;
				xs.resize(resolution);
				for (int x = 0; x < resolution; x++) {
								xs[x] = x;
				}

				std::for_each(std::execution::par,
								xs.begin(),
								xs.end(),
								[&model,
								albedo,
								altitude,
								azimuth,
								elevation,
								mode,
								resolution,
								view,
								&viewPoint,
								visibility,
								&outResult](auto&& x) {
												for (int y = 0; y < resolution; y++) {
																// For each pixel of the rendered image get the corresponding direction in fisheye
																// projection.
																const Vector3 viewDir = pixelToDirection(x, y, resolution, view);

																// If the pixel lies outside the upper hemisphere, the direction will be zero. Such
																// a pixel is kept black.
																if (viewDir.isZero()) {
																				continue;
																}

																// Get internal model parameters for the desired configuration.
																const Parameters params = model.computeParameters(viewPoint,
																				viewDir,
																				elevation,
																				azimuth,
																				visibility,
																				albedo);

																// Based on the selected mode compute spectral sky radiance, sun radiance,
																// polarisation or transmittance.
																Spectrum spectrum;
																for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++) {
																				switch (mode) {
																				case Mode::SunRadiance:
																								spectrum[wl] = model.sunRadiance(params, SPECTRUM_WAVELENGTHS[wl]);
																								break;
																				case Mode::Polarisation:
																								spectrum[wl] =
																												std::abs(model.polarisation(params, SPECTRUM_WAVELENGTHS[wl]));
																								break;
																				case Mode::Transmittance:
																								spectrum[wl] = model.transmittance(params,
																												SPECTRUM_WAVELENGTHS[wl],
																												std::numeric_limits<double>::max());
																								break;
																				default: // Mode::SkyRadiance
																								spectrum[wl] = model.skyRadiance(params, SPECTRUM_WAVELENGTHS[wl]);
																								break;
																				}
																}

																// Convert the spectral quantity to sRGB and store it at 0 in the result buffer.
																const Vector3 rgb = spectrumToRGB(spectrum);
																outResult[0][(size_t(x) * resolution + y) * 3] = float(rgb.x);
																outResult[0][(size_t(x) * resolution + y) * 3 + 1] = float(rgb.y);
																outResult[0][(size_t(x) * resolution + y) * 3 + 2] = float(rgb.z);

																// Store the individual channels.
																for (int c = 1; c < SPECTRUM_CHANNELS + 1; c++) {
																				outResult[c][(size_t(x) * resolution + y)] = float(spectrum[c - 1]);
																}
												}
								});
}


/// <summary>
/// An example of using Realtime Sky Model for rendering a simple fisheye RGB image of the sky.
/// </summary>
/// <param name="model">Reference to the sky model object.</param>
/// <param name="albedo">Ground albedo, value in range [0, 1].</param>
/// <param name="altitude">Altitude of view point in meters, value in range [0, 15000].</param>
/// <param name="azimuth">Sun azimuth at view point in degrees, value in range [0, 360].</param>
/// <param name="elevation">Sun elevation at view point in degrees, value in range [-4.2, 90].</param>
/// <param name="mode">Rendered quantity: sky radiance, sun radiance, polarisation, or transmittance.</param>
/// <param name="resolution">Length of resulting square image size in pixels.</param>
/// <param name="view">Rendered view: up-facing fisheye or side-facing fisheye.</param>
/// <param name="visibility">Horizontal visibility (meteorological range) at ground level in kilometers, value in range [20, 131.8].</param>
/// <param name="outResult">Buffer for storing the resulting images (index 0 = sRGB, index 1 - <# of channels in the dataset> = individual channels).</param>
void optimizedModelRender(RealtimeSkyModel& model,
				const double                     albedo,
				const double                     altitude,
				const double                     azimuth,
				const double                     elevation,
				const Mode                       mode,
				const int                        resolution,
				const View                       view,
				const double                     visibility,
				std::vector<std::vector<float>>& outResult) {
				assert(model.isInitialized());

				// We are viewing the sky from 'altitude' meters above the origin.
				const Vector3 viewPoint = Vector3(0.0, 0.0, altitude);

				// Resize the output buffer.
				outResult.resize(SPECTRUM_CHANNELS + 1);
				outResult[0].resize(size_t(resolution) * resolution * 3); // RGB
				for (int c = 1; c < SPECTRUM_CHANNELS + 1; c++) {
								outResult[c].resize(size_t(resolution) * resolution); // mono
				}

				// Zero the output buffer.
				for (int c = 0; c < SPECTRUM_CHANNELS + 1; c++) {
								for (int i = 0; i < outResult[c].size(); i++) {
												outResult[c][i] = 0.f;
								}
				}

				std::vector<int> xs;
				xs.resize(resolution);
				for (int x = 0; x < resolution; x++) {
								xs[x] = x;
				}

				std::for_each(std::execution::par,
								xs.begin(),
								xs.end(),
								[&model,
								albedo,
								altitude,
								azimuth,
								elevation,
								mode,
								resolution,
								view,
								&viewPoint,
								visibility,
								&outResult](auto&& x) {
												for (int y = 0; y < resolution; y++) {
																// For each pixel of the rendered image get the corresponding direction in fisheye
																// projection.
																const Vector3 viewDir = pixelToDirection(x, y, resolution, view);

																// If the pixel lies outside the upper hemisphere, the direction will be zero. Such
																// a pixel is kept black.
																if (viewDir.isZero()) {
																				continue;
																}

																Spectrum spectrum;
																if (mode == Mode::SkyRadiance) {
																				//////////////////////////////////////
																				// THIS METHOD IS NOT THREAD SAFE !!!!!!!!!!!!!!!!!!!!!!
																				//////////////////////////////////////
																				model.setParameters(
																								viewPoint,
																								viewDir,
																								elevation,
																								azimuth,
																								visibility,
																								albedo);
																				for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++)
																								spectrum[wl] = model.skyRadiance(SPECTRUM_WAVELENGTHS[wl]);
																}
																else {
																				// Get internal model parameters for the desired configuration.
																				const Parameters params = model.computeParameters(viewPoint,
																								viewDir,
																								elevation,
																								azimuth,
																								visibility,
																								albedo);

																				// Based on the selected mode compute spectral sky radiance, sun radiance,
																				// polarisation or transmittance.
																				for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++) {
																								switch (mode) {
																								case Mode::SunRadiance:
																												spectrum[wl] = model.sunRadiance(params, SPECTRUM_WAVELENGTHS[wl]);
																												break;
																								case Mode::Polarisation:
																												spectrum[wl] =
																																std::abs(model.polarisation(params, SPECTRUM_WAVELENGTHS[wl]));
																												break;
																								case Mode::Transmittance:
																												spectrum[wl] = model.transmittance(params,
																																SPECTRUM_WAVELENGTHS[wl],
																																std::numeric_limits<double>::max());
																												break;
																								}
																				}
																}
																// Convert the spectral quantity to sRGB and store it at 0 in the result buffer.
																const Vector3 rgb = spectrumToRGB(spectrum);
																outResult[0][(size_t(x) * resolution + y) * 3] = float(rgb.x);
																outResult[0][(size_t(x) * resolution + y) * 3 + 1] = float(rgb.y);
																outResult[0][(size_t(x) * resolution + y) * 3 + 2] = float(rgb.z);

																// Store the individual channels.
																for (int c = 1; c < SPECTRUM_CHANNELS + 1; c++) {
																				outResult[c][(size_t(x) * resolution + y)] = float(spectrum[c - 1]);
																}
												}
								});
}
