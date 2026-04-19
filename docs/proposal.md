# Bachelor Thesis Proposal
## Title: Realistic precomputed sky model in a real time renderer

## Introduction
Realistic rendering of outdoor scenes requires accurate simulation of atmospheric light scattering. Physically-based approaches can reproduce these effects with high fidelity but are computationally expensive and unsuitable for real-time rendering.

Analytical sky models provide an alternative by approximating the results of physically-based simulations. The [Prague Sky Model](https://github.com/PetrVevoda/pragueskymodel) introduces a fitted radiance and attenuation model capable of reproducing atmospheric effects, such as scattering depending on altitude, attenuation at finite distances, and post sunset sky illumination. Despite its realism, evaluating this model efficiently remains challenging due to this complexity of the fitted representation and the large amount of associated data.

## Objectives
In this thesis, we propose a real-time computation approach for the integrated sky dome radiance and attenuation model described in the paper [A Fitted Radiance and Attenuation Model for Realistic Atmospheres](https://cgg.mff.cuni.cz/publications/skymodel-2021/). The main goal is to reduce the computational cost of the atmospheric model during application runtime by optimizing the evaluation of the model and precomputing as much data as possible, while minimizing the loss of visual fidelity of the atmospheric appearance.

In addition to the implementation of the atmospheric model, an interactive 3D scene will be developed to demonstrate and evaluate the rendering results. The scene will support a free-flying camera mode, allowing the user to explore the environment from different viewpoints and altitudes.

The scene will contain a terrain model with varying elevations. The material of each visible terrain region will be computed based on the evaluated atmospheric lighting. This will allow the atmospheric model to influence the appearance of the environment in a physically meaningful way.

Furthermore, a fog effect can be implemented to simulate additional atmospheric phenomena. The fog will be evaluated based on parameters such as height, density, visibility distance, and other parameters compatible with the atmospheric model in order to provide a more realistic appearance. It may be implemented using a simple ray-marching approach or another suitable method supported by the selected graphics engine (Unreal Engine, Unity, or another rendering framework capable of providing the required functionality).

## Expected Outcome
The basic solution will include:
* An interactively adjustable real-time atmospheric model based on the Prague Sky Model, capable of running at runtime with acceptable visual quality and frame rate.
* An interactive 3D environment that allows exploration of the rendered scene.
* A terrain model with varying elevations that responds to atmospheric lighting.
* Basic material systems for terrain rendering.

Additional features may be implemented depending on the capabilities of the chosen graphics engine and the available development time. These may include:
* Atmospheric fog effects.
* Better material systems for terrain rendering.
* Additional visual effects and enhancements.
