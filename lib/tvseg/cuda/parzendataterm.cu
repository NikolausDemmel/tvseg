
#include "parzendataterm.h"
#include "errorhandling.h"
#include "config.h"
#include "utils.h"

#include <limits.h>
#include <float.h>


// constant used in computaion of gauss kernel
#define SQRT_2PI 2.50662827463

// small epsilon to avoid log(0)
#define LOG0_EPSILON 0.0000001f

// constants indication special cases
#define LIKELIHOOD_FIXED_THIS_LABEL (-0.0001f) // pick negative value close to 0 so visualization of normalized probability is not off
#define LIKELIHOOD_FIXED_OTHER_LABEL (-0.0002f) // pick negative value close to 0 so visualization of normalized probability is not off
#define ENERGY_FIXED_OTHER_LABEL 1000


namespace tvseg {
namespace cuda {


using std::cout;
using std::cerr;
using std::endl;


// TODO: optimize this by precomputing parts of the expression
__device__ float kernel_gauss(const float x, const float var)
{
    return expf(-0.5f*x*x/(var*var))/(var*SQRT_2PI);
}


// given dataterm likelihood, compute the dataterm energy for one label as the negative log, taking into account special cases
__global__ void kernel_datatermEnergy(float * const dataterm, const int pitch, const Dim2 dim, const float min, const float max, const bool energyFixedOtherLabelUseMin)
{
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    if (x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y*pitch;

    const float prob = dataterm[idx];

    if (prob == LIKELIHOOD_FIXED_THIS_LABEL) {
        // -1  =>  scribble from current label on current pixel  =>  very low energy
        dataterm[idx] = 0;
    } else if(prob == LIKELIHOOD_FIXED_OTHER_LABEL) {
        // -2  =>  indicates scribble from different label on current pixel  =>  very high energy
        if (energyFixedOtherLabelUseMin) {
            dataterm[idx] = -log(LOG0_EPSILON);
        } else {
            dataterm[idx] = ENERGY_FIXED_OTHER_LABEL;
        }
    } else {
        if (min != max) {
            // dataterm contains likelihood which is >= 0. Normalization converts this to
            // range ]0,1], for which taking the negative log gives energies in the range [0, +inf[
            dataterm[idx] = -log((prob - min) / (max - min) * (1 - LOG0_EPSILON) + LOG0_EPSILON);
        } else {
            // in general min != max, since otherwise the likelihood for all pixels and all labels is the same,
            // i.e. no meaningful result is possible; handle it anyway to be safe
            dataterm[idx] = -log(prob + LOG0_EPSILON);
        }
    }
}



// get minimum and maximum value of dataterm likelihood, ignoring negative values (which have special meaning)
__global__ void kernel_likelihoodColumnsMinMax(const float * const dataterm, const int pitch, const Dim2 dim, float * const columnsMin, float * const columnsMax)
{
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;

    if (x >= dim.width)
        return;

    float min = FLT_MAX;
    float max = -FLT_MAX;

    for (int y = 0; y < dim.height; ++y)
    {
        const float value = dataterm[x + y*pitch];
        if (value >= 0) { // ignore negative values as they indicate special cases
            if (value < min) {
                min = value;
            }
            if (value > max) {
                max = value;
            }
        }
    }
    columnsMin[x] = min;
    columnsMax[x] = max;
}


__global__ void kernel_datatermLikelihood(
        float * const dataterm,
        float * const rhos,
        const float * const color,
        const float * const depth,
        const uint pitch, // dataterm, rhos, and depth
        const uint pitch3, // color
        const int * const scribbles,
        const int * const numScribbles,
        const float depthNormalization,
        const Intrinsics intr,
        const Dim3 dim,
        const ParzenParams params)
{

    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;
    if (x >= dim.width || y >= dim.height)
        return;

    const uint datatermImageSize = dim.height * pitch;

    const float depthScale = params.depthScale / depthNormalization;
    float xyScale = 1.0;
    if (params.normalizeSpaceScale) {
        if (params.scribbleDistancePerspective) {
            xyScale = depthScale;
        } else {
            xyScale = 1.0f / max(dim.width, dim.height);
        }
    }

    const float alpha = params.scribbleDistanceFactor;
    const float sigma = params.colorVariance;
    const float tau = params.depthVariance;

    // compute nearest neighbor scribbles
    int currentNumScribbles;
    for (uint i = 0, scribbleOffset = 0; i < dim.labels; ++i, scribbleOffset += currentNumScribbles*2) {
        currentNumScribbles = numScribbles[i];
        float rho = 0;

        if (params.useDistanceKernel) {
            // find distance to nearest neighbor scribble of this label
            float minDist = FLT_MAX;
            for (int n = scribbleOffset; n < scribbleOffset + currentNumScribbles*2; n += 2) {
                const int sx = scribbles[n];
                const int sy = scribbles[n + 1];
                float dx, dy, dz;
                if (params.scribbleDistanceDepth) {
                    if (params.scribbleDistancePerspective) {
                        // backproject pixel positions to 3d space using depth and camera intrinsics
                        const float fz = depth[x + y*pitch];
                        const float fsz = depth[sx + sy*pitch];
                        const float fx = fz*(x - intr.o_x) / intr.s_x;
                        const float fy = fz*(y - intr.o_y) / intr.s_y;
                        const float fsx = fsz*(sx - intr.o_x) / intr.s_x;
                        const float fsy = fsz*(sy - intr.o_y) / intr.s_y;
                        dx = fsx - fx;
                        dy = fsy - fy;
                        dz = fsz - fz;
                    } else {
                        // isometric projection
                        const float fz = depth[x + y*pitch];
                        const float fsz = depth[sx + sy*pitch];
                        dx = sx - (int)x;
                        dy = sy - (int)y;
                        dz = fsz - fz;
                    }
                } else {
                    dx = sx - (int)x;
                    dy = sy - (int)y;
                    dz = 0;
                }
                dx *= xyScale;
                dy *= xyScale;
                dz *= depthScale;
                float dist = dx*dx + dy*dy + dz*dz;
                if (dist < minDist) {
                    minDist = dist;
                }
            }
            if (params.fixScribblePixels && !params.fixScribblePixelsThisOnly && minDist == 0) {
                // there must be scribble of current label on this pixel  ==>  set hard constraints for all labels
                for (int j = 0; j < dim.labels; ++j) {
                    rhos[x + y*pitch + j*datatermImageSize] = 0; // use 0 for all labels here as this is only used for visualization
                    if (i == j) {
                        dataterm[x + y*pitch + j*datatermImageSize] = LIKELIHOOD_FIXED_THIS_LABEL;
                    } else {
                        dataterm[x + y*pitch + j*datatermImageSize] = LIKELIHOOD_FIXED_OTHER_LABEL;
                    }
                }
                return; // nothing else to do when hard constraints are set
            }
            // when there are no scribbles, minDist == FLT_MAX  ==>  leave rho as default value for sensible visualization
            if (minDist < FLT_MAX) {
                // when alpha == 0, indicating disabled space kernel, rho will also be 0
                rho = alpha * sqrtf(minDist);
            }
        }
        rhos[x + y*pitch + i*datatermImageSize] = rho;
    }

    for (uint i = 0, scribbleOffset = 0; i < dim.labels; scribbleOffset += currentNumScribbles*2, ++i)  {
        const float rho = rhos[x + y*pitch + i*datatermImageSize];
        currentNumScribbles = numScribbles[i];
        int numActiveScribbles = 0;
        bool fixedThisLabel = false;
        float likely_a = 0; // active scribbles close to the pixel
        float likely_i = 0; // inactive pixels further away
        if (currentNumScribbles > 0) {
            for (int n = scribbleOffset; n < scribbleOffset + currentNumScribbles*2 && !fixedThisLabel; n += 2) {
                bool active = true;
                float space_factor = 1.0;
                float color_factor = 1.0;
                float depth_factor = 1.0;
                const int sx = scribbles[n];
                const int sy = scribbles[n + 1];

                // space kernel
                if (params.useDistanceKernel && rho > 0) {
                    float dx, dy, dz;
                    if (params.scribbleDistanceDepth) {
                        if (params.scribbleDistancePerspective) {
                            // backproject pixel positions to 3d space using depth and camera intrinsics
                            const float fz = depth[x + y*pitch];
                            const float fsz = depth[sx + sy*pitch];
                            const float fx = fz*(x - intr.o_x) / intr.s_x;
                            const float fy = fz*(y - intr.o_y) / intr.s_y;
                            const float fsx = fsz*(sx - intr.o_x) / intr.s_x;
                            const float fsy = fsz*(sy - intr.o_y) / intr.s_y;
                            dx = fsx - fx;
                            dy = fsy - fy;
                            dz = fsz - fz;
                        } else {
                            // isometric projection
                            const float fz = depth[x + y*pitch];
                            const float fsz = depth[sx + sy*pitch];
                            dx = sx - (int)x;
                            dy = sy - (int)y;
                            dz = fsz - fz;
                        }
                    } else {
                        dx = sx - (int)x;
                        dy = sy - (int)y;
                        dz = 0;
                    }
                    dx *= xyScale;
                    dy *= xyScale;
                    dz *= depthScale;
                    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                    if (params.useSpatiallyActiveScribbles) {
                        if (dist <= (rho * params.spatialActivationFactor) / alpha) {
                            ++numActiveScribbles;
                        } else {
                            active = false;
                        }
                    } else {
                        // all scribbles active
                        ++numActiveScribbles;
                    }

                    space_factor = kernel_gauss(dist, rho);
                } else if (params.useDistanceKernel && rho == 0 && alpha != 0 &&
                           params.fixScribblePixels && params.fixScribblePixelsThisOnly) {
                    dataterm[x + y*pitch + i*datatermImageSize] = LIKELIHOOD_FIXED_THIS_LABEL;
                    fixedThisLabel = true;
                }

                // color kernel
                if (params.useColorKernel && sigma > 0)
                {
                    // Euclidean distance in RGB color space
                    // TODO: is rgb really in that order for us? (doesn't really matter for euclidean distance)
                    const float r = color[3*x + y*pitch3    ] - color[3*sx + sy*pitch3    ];
                    const float g = color[3*x + y*pitch3 + 1] - color[3*sx + sy*pitch3 + 1];
                    const float b = color[3*x + y*pitch3 + 2] - color[3*sx + sy*pitch3 + 2];
                    const float dist = sqrtf(r*r + g*g + b*b);
                    color_factor = kernel_gauss(dist * params.colorScale, sigma);
                }

                // depth kernel
                if (params.useDepthKernel && tau > 0)
                {
                    const float d = depth[x + y*pitch] - depth[sx + sy*pitch];
                    depth_factor = kernel_gauss(d * depthScale, tau);
                }

                if (active) {
                    likely_a += space_factor * color_factor * depth_factor;
                } else {
                    likely_i += space_factor * color_factor * depth_factor;
                }
            }
            const int numInactiveScribbles = currentNumScribbles - numActiveScribbles;
//            if (x == 0 && y == 0) {
//                printf("currentNumScribbles: %d, activescribbles: %d, numinactivescribbles: %d\n", currentNumScribbles, numActiveScribbles, numInactiveScribbles);
//            }
//            if (x == 0 && y == 0) {
//                printf("likely_a: %f, likely_i: %f\n", likely_a, likely_i);
//            }
            if (params.useSpatiallyActiveScribbles && (((float)numActiveScribbles) / currentNumScribbles) < params.activeScribbleLambda && numActiveScribbles > 0) {
                // if active scribbles have a smaller portion than params.activeScribbleLambda, make sure they have overall weight of params.activeScribbleLambda
                likely_a /= numActiveScribbles;
                if (numInactiveScribbles > 0) {
                    likely_i /= numInactiveScribbles;
                }
            } else {
                // if active scribbles already have a larger portion than params.activeScribbleLambda, don't adjust weight at all
                likely_a = (likely_a + likely_i) / currentNumScribbles;
                likely_i = likely_a;
            }
//            if (x == 0 && y == 0) {
//                printf("likely_a: %f, likely_i: %f\n", likely_a, likely_i);
//            }
        }
        if (fixedThisLabel) {
            dataterm[x + y*pitch + i*datatermImageSize] = LIKELIHOOD_FIXED_THIS_LABEL;
        } else {
            dataterm[x + y*pitch + i*datatermImageSize] = likely_a * params.activeScribbleLambda + likely_i * (1 - params.activeScribbleLambda);
        }
    }
}


float normalizationDepthScale(const float * const depth, const Dim2 dim, const bool perspective, const Intrinsics &intr) {
    if (dim.width == 0 || dim.height == 0) {
        return 1.0;
    }
    float minx = FLT_MAX;
    float miny = FLT_MAX;
    float minz = FLT_MAX;
    float maxx = -FLT_MAX;
    float maxy = -FLT_MAX;
    float maxz = -FLT_MAX;
    for (uint x = 0; x < dim.width; ++x) {
        for (uint y = 0; y < dim.height; ++y) {
            const float fz = depth[x + y*dim.width];
            const float fx = fz*(x - intr.o_x) / intr.s_x;
            const float fy = fz*(y - intr.o_y) / intr.s_y;
            minx = min(fx, minx);
            miny = min(fy, miny);
            minz = min(fz, minz);
            maxx = max(fx, maxx);
            maxy = max(fy, maxy);
            maxz = max(fz, maxz);
        }
    }
    const float dx = maxx - minx;
    const float dy = maxy - miny;
    const float dz = maxz - minz;
    float scale;
    if (perspective) {
        scale = max(dx, max(dy, dz));
    } else {
        scale = dz;
    }
    if (scale == 0) {
        scale = 1;
    }
    return scale;
}


void computeParzenDataterm(
        float* dataterm,
        const float *color,
        const float *depth,
        const int *scribbles,
        const int *numScribbles,
        const Intrinsics &intr,
        const Dim3 &dim,
        const ParzenParams &params,
        const Feedback *feedback)
{
    // debugging info
    cout << "Begin: computeParzenDataterm" << endl;
    cout << "\tdim: " << dim << endl;
    cout << "\tparams: " << params << endl;
    cout << "\tscribble counts: ";
    for (int i = 0; i < dim.labels; ++i) {
        cout << numScribbles[i] << ", ";
    }
    cout << endl;
//    cout << "\tscribbles: " << endl;
//    uint currentNumScribbles;
//    for (uint i = 0, scribbleOffset = 0; i < dim.labels; i++, scribbleOffset += currentNumScribbles*2) {
//        currentNumScribbles = numScribbles[i];
//        cout << "\t\t" << i+1 << " : ";
//        for (int n = scribbleOffset; n < scribbleOffset + currentNumScribbles*2; n += 2) {
//            cout << "(" << scribbles[n] << "," << scribbles[n+1] << ")  ";
//        }
//        cout << endl;
//    }

    // Prepare grid size
    dim3 dimBlock(BLOCKDIMX, BLOCKDIMY);
    dim3 dimGrid;
    dimGrid.x = (dim.width % dimBlock.x) ? (dim.width/dimBlock.x + 1) : (dim.width/dimBlock.x);
    dimGrid.y = (dim.height % dimBlock.y) ? (dim.height/dimBlock.y + 1) : (dim.height/dimBlock.y);

    // auxiliare variables
    size_t pitch; // pitch for dataterm and depth
    size_t pitch3; // pitch for color
    int totalNumScribbles = 0;
    for (int i = 0; i < dim.labels; ++i) {
        int num = numScribbles[i];
        if (num < 0 || num > dim.width * dim.height) {
            cerr << "unreasonable scribble count " << num << "... aborting" << endl;
            return;
        }
        totalNumScribbles += num;
    }
    cout << "\ttotalNumScribbles: " << totalNumScribbles << endl;
    int steps = 0; // for feedback
    const int maxSteps = 4 + dim.labels;
    feedback->progress(steps++, maxSteps, 0);

    // normalize depth
    float depthNormalization = 1.0;
    if (params.normalizeSpaceScale) {
//        bool perspectiveNormalization = params.scribbleDistancePerspective;
        bool perspectiveNormalization = false;
        depthNormalization = normalizationDepthScale(depth, dim2(dim), perspectiveNormalization, intr);
    }

    // /////////////////////////////////////////////////////////////////
    // 1. allocate memory on the GPU and copy data or set initial value

    float *gpu_dataterm = NULL;
    float *gpu_rhos = NULL;
    float *gpu_color = NULL;
    float *gpu_depth = NULL;
    int *gpu_scribbles = NULL;
    int *gpu_numScribbles = NULL;
    float *gpu_columnsMin = NULL;
    float *gpu_columnsMax = NULL;

    const uint widthBytes = dim.width * sizeof(float);
    const uint heightTotal = dim.height * dim.labels;

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_dataterm, &pitch, widthBytes, heightTotal));
    CUDA_CHECK(cudaMemset2DAsync(gpu_dataterm, pitch, 0, widthBytes, heightTotal));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_rhos, &pitch, widthBytes, heightTotal));
    CUDA_CHECK(cudaMemset2DAsync(gpu_rhos, pitch, 0, widthBytes, heightTotal));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_color, &pitch3, 3*widthBytes, dim.height));
    CUDA_CHECK(cudaMemcpy2DAsync(gpu_color, pitch3, color, 3*widthBytes, 3*widthBytes, dim.height, cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_depth, &pitch, widthBytes, dim.height));
    CUDA_CHECK(cudaMemcpy2DAsync(gpu_depth, pitch, depth, widthBytes, widthBytes, dim.height, cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc((void**)&gpu_scribbles, 2 * totalNumScribbles * sizeof(int)));
    CUDA_CHECK(cudaMemcpyAsync(gpu_scribbles, scribbles, 2 * totalNumScribbles * sizeof(int), cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc((void**)&gpu_numScribbles, dim.labels * sizeof(int)));
    CUDA_CHECK(cudaMemcpyAsync(gpu_numScribbles, numScribbles, dim.labels * sizeof(int), cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc(&gpu_columnsMin, widthBytes));
    CUDA_CHECK(cudaMalloc(&gpu_columnsMax, widthBytes));

    feedback->progress(steps++, maxSteps, 0);

    // /////////////////////////////////////////////////////////////////
    // 2.  compute dataterm likelihood on GPU
    CUDA_CHECK_KERNEL_LAUNCH_AND_SYNCHRONIZE();
    kernel_datatermLikelihood<<< dimGrid, dimBlock >>>(gpu_dataterm, gpu_rhos, gpu_color, gpu_depth,
                                                       pitch/sizeof(float), pitch3/sizeof(float),
                                                       gpu_scribbles, gpu_numScribbles,
                                                       depthNormalization, intr, dim, params);
    CUDA_CHECK_KERNEL_LAUNCH_AND_SYNCHRONIZE();
    std::string winName = "rhos";
    CUDA_CHECK(cudaMemcpy2D((void*)dataterm, widthBytes, gpu_rhos, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToHost));
    feedback->displayImage(dataterm, dim, winName);
    winName = "likely";
    CUDA_CHECK(cudaMemcpy2D((void*)dataterm, widthBytes, gpu_dataterm, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToHost));
    feedback->displayImage(dataterm, dim, winName);

    feedback->progress(steps++, maxSteps, 0);

    // /////////////////////////////////////////////////////////////////
    // 3.  compute min/mix of likelihood for normalization of likelihood values

    dim3 dimBlockMinMax(BLOCKDIMX * BLOCKDIMY, 1);
    dim3 dimGridMinMax;
    dimGridMinMax.x = (dim.width % dimBlockMinMax.x) ? (dim.width / dimBlockMinMax.x + 1) : (dim.width / dimBlockMinMax.x);
    dimGridMinMax.y = 1;

    float *columnsMin = new float[dim.width];
    float *columnsMax = new float[dim.width];
    float min = FLT_MAX;
    float max = -FLT_MAX;

    // TODO: maybe compute for the long columns of all labels with one kernel call?
    for (int i = 0; i < dim.labels; ++i) {
        if (feedback->isCanceled()) {
            break;
        }
        kernel_likelihoodColumnsMinMax<<< dimGridMinMax, dimBlockMinMax >>>(gpu_dataterm + i * pitch/sizeof(float) * dim.height,
                                                                            pitch/sizeof(float),
                                                                            dim2(dim),
                                                                            gpu_columnsMin,
                                                                            gpu_columnsMax);
        CUDA_CHECK_KERNEL_LAUNCH_AND_SYNCHRONIZE();
        CUDA_CHECK(cudaMemcpy(columnsMin, gpu_columnsMin, widthBytes, cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(columnsMax, gpu_columnsMax, widthBytes, cudaMemcpyDeviceToHost));
        for (int j = 0; j < dim.width; ++j) {
            if (columnsMin[j] < min) {
                min = columnsMin[j];
            }
            if (columnsMax[j] > max) {
                max = columnsMax[j];
            }
        }
        feedback->progress(steps++, maxSteps, 0);
    }
    delete [] columnsMin;
    delete [] columnsMax;

//    cout << "likelihood range: [" << min << "," << max << "]" << endl;

    // /////////////////////////////////////////////////////////////////
    // 4.  compute energy from likelihood
    for (uint i = 0; i < dim.labels; ++i) {
        if (feedback->isCanceled()) {
            break;
        }
        kernel_datatermEnergy<<< dimGrid, dimBlock >>>(gpu_dataterm + i * pitch/sizeof(float) * dim.height,
                                                       pitch/sizeof(float),
                                                       dim2(dim), min, max,
                                                       params.fixScribblePixelsOtherLabelUseMin);
        CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();
        feedback->progress(steps++, maxSteps, 0);
    }

    // copy result back to CPU
    if (!feedback->isCanceled()) {
        CUDA_CHECK(cudaMemcpy2D((void*)dataterm, dim.width * sizeof(float), gpu_dataterm, pitch, dim.width * sizeof(float), dim.height * dim.labels, cudaMemcpyDeviceToHost));
        feedback->progress(steps++, maxSteps, 0);
    }

    // delete all data on GPU
    CUDA_CHECK(cudaFree(gpu_dataterm));
    CUDA_CHECK(cudaFree(gpu_rhos));
    CUDA_CHECK(cudaFree(gpu_color));
    CUDA_CHECK(cudaFree(gpu_depth));
    CUDA_CHECK(cudaFree(gpu_scribbles));
    CUDA_CHECK(cudaFree(gpu_numScribbles));
}


}
}
