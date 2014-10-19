## `input/` ##

parameters                        | description | type | typical values
--------------------------------- | --- | --- | ---
`color`                           | path to color input image | `string` | `images/example_color.png`
`colorMatch`                      | regular expression matching `input/color` | `string` | `_color(\\..*)`
`depthReplace`                    | replacement for `input/colorMatch` to obtain filename for depth input | `string` |  `_depth_filled_colorization\\1`
`depthScale`                      | scale of depth input (input is divided by this value to obtain meters) | `float` | `10000`
`groundTruthLabelMappingOverride` | mapping of regions in the ground truth input | vector of uint | `"0,0,0,2,0,1,1,0,0,3"`
`groundTruthLabelMappingReplace`  | replacement for `input/colorMatch` to obtain filename for label mapping input | `string` | `_label_mapping.txt`
`groundTruthReplace`              | replacement for `input/colorMatch` to obtain filename for depth input | `string` | `_label\\1`
`intrinsics`                      | camera intrinsics (row-wise) | list of float size 9 | `"525,0,319.5,0,525,239.5,0,0,1"`
`loadDepth`                       | flag to enable loading depth input file | `bool` | `true`
`loadGroundTruth`                 | flag to enable loading ground truth input file | `bool` | `true`
`loadScribbles`                   | flag to enable loading scribble input file | `bool` | `true`
`loadSettings`                    | flag to enable loading settings input file | `bool` | `true`
`overrideGroundTruthLabelMapping` | flag to overwrite the ground truth label mapping with the value of `groundTruthReplace` | `bool` | `false`
`overrideScribbles`               | flag to overwrite the scribble input with the file `scribbles.png` in the current directory | `bool` | `false`
`scribblesReplace`                | replacement for `input/colorMatch` to obtain filename for scribble input | `string` | `_scribbles\\1`
`settingsReplace`                 | replacement for `input/colorMatch` to obtain filename for settings input | `string` | `_settings.ini`



## `main_window/` ##

parameters    | description     | type         | typical values
------------- | --------------- | ------------ | --------------
`geometry`    | Qt window state | binary array | ...
`windowState` | Qt window state | binary array | ...



## `output/` ##

parameters             | description | type | typical values
---------------------- | --- | --- | ---
`datatermReplace`      | replacement for `input/colorMatch` to obtain filename for result dataterm output | `string` | `_dataterm_normalized\\1`
`metricsReplace`       | replacement for `input/colorMatch` to obtain filename for result metrics output | `string` | `_metrics.txt`
`prefixDateTime`       | prefix datetime string to result output files | `bool` | `true`
`resultPath`           | path to the folder where results are stores | `string` | `results`
`solutionReplace`      | replacement for `input/colorMatch` to obtain filename for result solution output | `string` | `_solution\\1`
`subFolderByDate`      | group results in a subfolder for the current date inside `output/resultPath` | `bool` | `true`
`visualizationReplace` |   | `_visualization\\1`
`weightReplace`        | replacement for `input/colorMatch` to obtain filename for result weight output | `string` | `_weight_normalized\\1`



## `parameters/` ##

parameters | description | type | typical values
--- | --- | --- | ---
`numLabels` | number of labels in the segmentation | uint >= 2 | 2



## `parzen/` ##

parameters                     | description | type | typical values
------------------------------ | --- | --- | ---
`activeScribbleLambda`         | minimum fraction of weight going to active scribbles | `float` in [0,1] | `0.8`
`colorScale`                   | constant scale applied to color channels | `float` | `1`
`colorVariance`                | bandwidth "sigma" of the color kernel | `float` | we use `0.05`; Claudia suggests ca. `0.02` for color-only
`depthScale`                   | constant scale applied to depth value (after normalization) | `float` | `1`
`depthVariance`                | bandwidth "sigma" of the color kernel | `float` | 0.2 for relatively weak depth kernel (typically 0.5 is too large, 0.02 too small)
`fixScribblePixels`            | activates special treatment of pixels where scribbles are placed | `bool` | `true`
`normalizeSpaceScale`          | activates normalization of distance space for distance kernel | `bool` | `true`
`scribbleDistanceDepth`        | activates 3D distance computation | `bool` | `true`
`scribbleDistanceFactor`       | | `float` | `1000`
`scribbleDistancePerspective`  | | `bool` | `false`
`spatialActivationFactor`      | | `float` | `3`
`useColorKernel`               | | `bool` | `true`
`useDepthKernel`               | | `bool` | `true`
`useDistanceKernel`            | | `bool` | `true`
`useSpatiallyActiveScribbles`  | | `bool` | `true`


