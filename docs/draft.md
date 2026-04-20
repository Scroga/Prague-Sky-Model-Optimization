## Possible optimizations

The first and most important optimization is to avoid generating the second, opposite-facing view. Since the sky dome is symmetrical, it is sufficient to generate only one side-facing view of the sky dome.

Another useful optimization is caching frame-constant interpolation and parameter computations. In particular, the `evaluateModel` and `computeParameters` methods recompute `visibilityParam`, `albedoParam`, `altitudeParam`, and `elevationParam`, even though these values may remain unchanged for the whole frame or for multiple consecutive frames. In such cases, they should be computed once and reused instead of being recalculated for every query.

It is also possible to extend the `initialize` method so that some parameters, such as fixed `albedo`, `altitude`, or `solar elevation`, are precomputed in advance. This would reduce the amount of interpolation needed in `evaluateModel`, because the method would no longer need to perform the full interpolation across all parameters at runtime.

More specifically, we can precompute the final reconstructed output for a grid of atmospheric states, such as `visibility`, `albedo`, `altitude`, and `solar elevation`, and store this output in a texture or lookup table. Then, at runtime, the model would only perform texture sampling and interpolation, instead of executing the full *CPD/SVD* reconstruction for every query. This shifts more work to preprocessing, but can significantly improve runtime performance.

Camera rotation will not recompute the sky dome but rotate the fixed sky dome.


Split parameters into two types: 
- Parameters that are changed at runtime and will be updated every frame: ``altitude``, (``solar elevation``?).
- Parameters that will be set once at the start of the program: `visibility`, ``albedo``, (``solar elevation``?).

One of the following parameters might be precomputed:
- altitude: there will be multiple objects with different altitudes in the scene.
- solar elevation: the is fixed amount of solar elevations, but this parameter will not change every frame.
