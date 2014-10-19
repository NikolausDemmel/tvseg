
#include "solver.h"
#include "errorhandling.h"
#include "config.h"
#include "cvimageshow.h"
#include "utils.h"

#include <nppi.h>

#include <iostream>
#include <time.h>



namespace tvseg {
namespace cuda {


using std::cout;
using std::endl;


// FIXME: use debug logging provided

// FIXME: clean up (memory) on stack unwind


// get the difference for each pixel between u and u_saved
__global__ void kernel_getPixelDifferences(
        float *pixelDifferences,
        const float *u,
        const float *u_saved,
        const Dim3 dim,
        const uint indexPitch)
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if (x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y * indexPitch;
    const uint labelOffset = indexPitch * dim.height;

    pixelDifferences[idx] = 0.0f;

    for (uint i = 0; i < dim.labels; ++i) {
        // add up pixel differences
        // NOTE: Since the changes for a single pixel sum up to zero, each change is counted
        //       twice. We take care of this when summing up pixelDifferences.
        pixelDifferences[idx] += abs(u[idx + i * labelOffset] - u_saved[idx + i * labelOffset]);
    }
}



// get energy of a solution
__global__ void kernel_getEnergy(
        float *energy,
        const float *u,
        const float *g,
        const float *dataterm,
        const float lambda,
        const Dim3 dim,
        const uint indexPitch
        )
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if(x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y * indexPitch;
    const uint labelOffset = indexPitch * dim.height;
    const float g_scaled = lambda * g[idx];

    // accumulator for energy (energy_xy = energy[x + y * pitch]); summing over all labels
    float energy_xy_acc = 0.0f;

    for (int i = 0; i < dim.labels; ++i) {
        const int pos_u = idx + i * labelOffset;
        const float u_pos_u = u[pos_u];

        // compute norm of gradient of u
        const float grad_u_x = (x + 1 < dim.width ) ? (u[pos_u + 1         ] - u_pos_u) : 0.0f;
        const float grad_u_y = (y + 1 < dim.height) ? (u[pos_u + indexPitch] - u_pos_u) : 0.0f;
        const float norm_grad_u = sqrtf(grad_u_x * grad_u_x + grad_u_y * grad_u_y);

        // determine value of standard energy ( |gradU| = - divXi  )
        energy_xy_acc += dataterm[pos_u] * u_pos_u + g_scaled * norm_grad_u;
    }

    energy[idx] = energy_xy_acc;
}



// get thresholded u (to show it)
__global__ void kernel_getUThresh(
        float *u_thresh,
        const float *u,
        const Dim3 dim,
        const uint indexPitch)
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if(x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y * indexPitch;
    const uint labelOffset = indexPitch * dim.height;

    // compute maximum and index of maximum
    // we set all labels to 0.0, except the maximum, which is set to 1.0
    int pos_u_max = idx;
    float u_max = u[idx];
    u_thresh[idx] = 0.0f;

    for (int i = 1; i < dim.labels; ++i) {
        const uint pos_u  = idx + i * labelOffset;
        u_thresh[pos_u] = 0.0f;
        const float u_pos_u = u[pos_u];
        if (u_max < u_pos_u) {
            pos_u_max = pos_u;
            u_max = u_pos_u;
        }
    }

    u_thresh[pos_u_max] = 1.0f;
}



// get the segmentation as image of label indices
__global__ void kernel_getSegmentation(
        unsigned char *segmentation,
        const float *u_thresh,
        Dim3 dim,
        const int indexPitchUChar,
        const int indexPitchFloat)
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if(x >= dim.width || y >= dim.height)
        return;

    const uint idxFloat = x + y * indexPitchFloat;
    const uint idxUChar = x + y * indexPitchUChar;
    const uint labelOffset = indexPitchFloat * dim.height;

    // find index where thresholded u == 1.0 (should be exactly one)
    unsigned char index = 0;
    for (int i = 1; i < dim.labels; ++i) {
        if (1.0f == u_thresh[idxFloat + i * labelOffset]) {
            index = i;
        }
    }

    segmentation[idxUChar] = index;
}



// calculate gradient ascent = update dual variables
__global__ void kernel_grad_ascent(
        float *xi,                // dual variables
        float *psi,               // dual variables
        const float *u_overrelax, // overrelaxed primal variables
        const float *sum_u,       // current pixelwise sum of u; is optimized to be 1.0
        const float *g,           // weight for penailzing region jumps
        const float lambda,       // parameter for smoothness vs correctness
        const Dim3 dim,     // image dimensions
        const int indexPitch      // image pitch in indices (not bytes)
        )
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if(x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y * indexPitch;
    const uint labelOffset = indexPitch * dim.height;

    // step sizes tau
    const float scaleTau = 2.0f;
    const float tau_xi   = 1.0f / (2          * scaleTau);
    const float tau_psi  = 1.0f / (dim.labels * scaleTau);

    // combine g and lambda
    const float g_scaled = g[idx] * lambda;

    // update xi for each label
    for (uint i = 0; i < dim.labels; ++i) {

        // set positions within arrays
        const uint pos_u  = idx + i * labelOffset;
        const uint pos_xi = idx + i * labelOffset * 2;

        // cache values from global memory
        const float u_overrelax_pos_u = u_overrelax[pos_u];
        float xi_x = xi[pos_xi];
        float xi_y = xi[pos_xi + labelOffset];

        // update xi   (gradient as forward difference)
        // ================================================
        // compute:   xi  =  xi - tau_xi * grad(u_overrelax)
        //            project xi

        // FIMXE: Why not -= in the following??
        // TODO: Does higher-order discrete differences make sense?

        // for (x == dim.width - 1) we always have xi_x == 0.0 from initialization
        if (x + 1 < dim.width) {
            xi_x += tau_xi * (u_overrelax[pos_u + 1]          - u_overrelax_pos_u);
        }

        // for (y == dim.height - 1) we always have xi_y == 0.0 from initialization
        if (y + 1 < dim.height) {
            xi_y += tau_xi * (u_overrelax[pos_u + indexPitch] - u_overrelax_pos_u);
        }

        // project xi
        const float norm_xi = sqrtf(xi_x*xi_x + xi_y*xi_y) / g_scaled;
        if (norm_xi > 1.0f) {
            xi_x /= norm_xi;
            xi_y /= norm_xi;
        }

        // write xi to global memory
        xi[pos_xi] = xi_x;
        xi[pos_xi + labelOffset] = xi_y;
    }

    // update psi
    // ================================================
    // compute:   psi = psi + tau_psi * (sum_u - 1)    where sum_u is determined by using u_overrelax
    psi[idx] += tau_psi * (sum_u[idx] - 1.0f);

}


// calculate gradient descent = update primal variables
__global__ void kernel_grad_descent(
        float *u,
        float *u_overrelax,
        float *sum_u,
        const float *xi,
        const float *psi,
        const float *dataterm,
        const Dim3 dim,
        const uint indexPitch)
{
    // which thread are we?
    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    // is it within image?
    if(x >= dim.width || y >= dim.height)
        return;

    const uint idx = x + y * indexPitch;
    const uint labelOffset = indexPitch * dim.height;

    // compute step width
    const float scaleTau  = 2.0f;
    const float tau_u     = (scaleTau * 1.0f)/6.0f;

    // accumulator for sum_u
    float sum_u_tmp = 0.0f;

    // cache values from global memory
    const float psi_idx = psi[idx];

    // update u for each label
    for (uint i = 0; i < dim.labels; ++i) {

        // set positions within arrays
        const uint pos_u = idx + i * labelOffset;
        const uint pos_xi = idx + i * labelOffset * 2;

        // compute div_xi   (backward differences)
        // ================================================
        float div_xi;
        if (x == 0 && y == 0)
            div_xi = 0;
        else if (x == 0)
            div_xi = xi[pos_xi + labelOffset] - xi[pos_xi + labelOffset - indexPitch];
        else if (y == 0)
            div_xi = xi[pos_xi]               - xi[pos_xi               - 1];
        else {
            // divXi = backwDiff_x + backwDiff_y
            div_xi = xi[pos_xi]               - xi[pos_xi               - 1]
                   + xi[pos_xi + labelOffset] - xi[pos_xi + labelOffset - indexPitch];
        }

        // update u
        // ================================================
        // compute:   u = u - tau_u * (dataterm + div_xi + 1/numLabels * psi)
        //            project u
        //            extrapolate u

        // cache values from global memory
        float u_pos_u = u[pos_u];
        const float u_pos_u_old = u_pos_u;

        // FIXME: plus or minus?? also check order of div_xi computation
        //u_pos_u -= tau_u * (dataTerm[pos_u] + divXi + 1.0f/nRegions * psi_temp);
        u_pos_u -= tau_u * (dataterm[pos_u] - div_xi + 1.0f/dim.labels * psi_idx);

        // project u
        if(u_pos_u > 1.0f)
           u_pos_u = 1.0f;
        if(u_pos_u < 0.0f)
           u_pos_u = 0.0f;

        // determine overrelaxed u and accumulate sum_u
        const float u_overrelax_pos_u = 2.0f * u_pos_u - u_pos_u_old;
        sum_u_tmp += u_overrelax_pos_u;

        // commit to global memory
        u[pos_u]           = u_pos_u;
        u_overrelax[pos_u] = u_overrelax_pos_u;
    }

    // commit accumulated sum to global memory
    sum_u[idx] = sum_u_tmp;
}



void computeSolution(
        unsigned char *segmentation,
        float *u,
        const float *dataterm,
        const float *weight,
        const Dim3 &dim,
        const SolverParams &params,
        const Feedback *feedback,
        SolverOutput &output
        )
{
    //FIXME: Check input for =! NULL


    cout << "Begin: computeSolution" << endl;

    cout << "\tdim: " << dim << endl;
    cout << "\tparams: " << params << endl;


    ///////////////////////////////////////////////////////////////////////////
    /// 1) Prepare grid size
    ///////////////////////////////////////////////////////////////////////////
    dim3 dimBlock(BLOCKDIMX, BLOCKDIMY);
    dim3 dimGrid;
    size_t pitch;
    size_t pitchUChar;
    dimGrid.x = (dim.width % dimBlock.x) ? (dim.width/dimBlock.x + 1) : (dim.width/dimBlock.x);
    dimGrid.y = (dim.height % dimBlock.y) ? (dim.height/dimBlock.y + 1) : (dim.height/dimBlock.y);

    ///////////////////////////////////////////////////////////////////////////
    /// 2) allocate the memory on the GPU & copy the data there
    ///////////////////////////////////////////////////////////////////////////
    float *gpu_dataterm = NULL;
    float *gpu_g = NULL;
    float *gpu_u = NULL;
    float *gpu_u_overrelax = NULL;
    float *gpu_u_thresh = NULL;
    float *gpu_u_saved = NULL;
    float *gpu_sum_u = NULL;
    float *gpu_pixelDifferences = NULL;
    float *gpu_xi = NULL;
    float *gpu_psi = NULL;
    unsigned char *gpu_segmentation = NULL;
    float *gpu_energy_relax = NULL;
    float *gpu_energy_thresh = NULL;

    const uint widthBytes = dim.width * sizeof(float);
    const uint heightTotal = dim.height * dim.labels;
    const NppiSize nppiSize = { dim.width, dim.height };

    // ========================================================================
    // Input
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_dataterm, &pitch, widthBytes, heightTotal));             // gpu_dataterm
    CUDA_CHECK(cudaMemcpy2D(gpu_dataterm, pitch, dataterm, widthBytes, widthBytes, heightTotal, cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_g, &pitch, widthBytes, dim.height));                     // gpu_g
    CUDA_CHECK(cudaMemcpy2D(gpu_g, pitch, weight, widthBytes, widthBytes, dim.height, cudaMemcpyHostToDevice));

    // ========================================================================
    // Primal variables and helpers for primal-dual algo
    // FIXME: check that host data is sensible?
    // initialize u with supplied host data
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_u, &pitch, widthBytes, heightTotal));                    // gpu_u
    CUDA_CHECK(cudaMemcpy2D(gpu_u, pitch, u, widthBytes, widthBytes, heightTotal, cudaMemcpyHostToDevice));

    // FIXME: is initialization the right thing (tm) ???
    // initialize u_overrelax with u
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_u_overrelax, &pitch, widthBytes, heightTotal));          // gpu_u_overrelax
    CUDA_CHECK(cudaMemcpy2D(gpu_u_overrelax, pitch, gpu_u, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToDevice));

    // FIXME: should initialize with actual sum??
    // initialize sum_u with 1
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_sum_u, &pitch, widthBytes, dim.height));                 // gpu_sum_u
    CUDA_CHECK(nppiSet_32f_C1R(1.0f, gpu_sum_u, pitch, nppiSize));

    // no need to initialize gpu_u_thresh; is set before first use
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_u_thresh, &pitch, widthBytes, heightTotal));             // gpu_u_thresh

    // ========================================================================
    // Dual Variables
    // FIXME: initialization with 0 sensible for xi and psi???
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_xi, &pitch, widthBytes, heightTotal * 2));               // gpu_xi
    CUDA_CHECK(cudaMemset2D(gpu_xi, pitch, 0, widthBytes, heightTotal * 2));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_psi, &pitch, widthBytes, dim.height));                   // gpu_psi
    CUDA_CHECK(cudaMemset2D(gpu_psi, pitch, 0, widthBytes, dim.height));

    // ========================================================================
    // Helpers for checking abort criteria
    // initialize u_saved with 0, assuming that this is different from the initialization of u
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_u_saved, &pitch, widthBytes, heightTotal));              // gpu_u_saved
    CUDA_CHECK(cudaMemset2D(gpu_u_saved, pitch, 0, widthBytes, heightTotal));

    // no need to initialize pixelDifferences; is set before first use
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_pixelDifferences, &pitch, widthBytes, dim.height));      // gpu_pixelDifferences

    // ========================================================================
    // Helpers to determine the energy gap
    // no need to initialize energies; are set before first use
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_energy_relax , &pitch, widthBytes, dim.height));         // gpu_energy_relax
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_energy_thresh, &pitch, widthBytes, dim.height));         // gpu_energy_thresh

    // ========================================================================
    // Output segmentation
    // no need to initialize; is set before first use
    CUDA_CHECK(cudaMallocPitch((void**)&gpu_segmentation, &pitchUChar, dim.width, dim.height));    // gpu_segmentation

    ///////////////////////////////////////////////////////////////////////////
    /// 3) Additional Parameters
    ///////////////////////////////////////////////////////////////////////////
    // FIXME: what should this be?
    //float breakValue = 5e-06;     // mein Vorschlag: 0.0001f;  Evgeny: 0.001f;  => 0.0005f;
    //const double epsilon = 5e-4;

    ///////////////////////////////////////////////////////////////////////////
    /// 4) Helper variables
    ///////////////////////////////////////////////////////////////////////////

    Npp8u *gpu_nppDeviceBuffer = NULL;
    int nppBufferSize = 0;
    nppiSumGetBufferHostSize_32f_C1R(nppiSize, &nppBufferSize);
    CUDA_CHECK(cudaMalloc((void **)(&gpu_nppDeviceBuffer), nppBufferSize));
    Npp64f *gpu_nppSum = NULL;
    CUDA_CHECK(cudaMalloc((void **)(&gpu_nppSum), sizeof(Npp64f) * 1));

    // helper to display thresholded u
    float *u_thresh = new float [dim.width * heightTotal];

    // set output steps; overwritten when we abort before reaching maxSteps
    output.steps = params.maxSteps;
    output.cancelled = false;

    // start timing
    const clock_t timeStart = clock();

    ///////////////////////////////////////////////////////////////////////////
    /// 5) Run the primal-dual aglorithm iterations
    ///////////////////////////////////////////////////////////////////////////

    feedback->progress(0, params.maxSteps, 0);
    for (uint step = 1; step <= params.maxSteps; ++step) {

        // ===============================================================
        // Check for cancel
        // ===============================================================

        if (feedback->isCanceled()) {
            cout << "cancelled" << endl;
            output.cancelled = true;
            break;
        }

        // ===============================================================
        // Check abort criteria
        // ===============================================================
        if(step % params.checkConverganceEvery == 0) {
            cout << "step = " << step << "\t\t";

            // get the difference between the current u and the former iteration (u_saved)
            // also assigns u_saved to the current u
            kernel_getPixelDifferences<<< dimGrid, dimBlock >>>(gpu_pixelDifferences, gpu_u, gpu_u_saved, dim, pitch/sizeof(float));
            CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

            // Sum up the pixel differences and divide by the number of pixels.
            // Since each change has to affect 2 regions equally (the sum of all regions remains 1),
            // we also have to divide by 2 to only count each change once.
            Npp64f change;
            CUDA_CHECK(nppiSum_32f_C1R(gpu_pixelDifferences, pitch, nppiSize, gpu_nppDeviceBuffer, gpu_nppSum));
            CUDA_CHECK(cudaMemcpy(&change, gpu_nppSum, sizeof(Npp64f), cudaMemcpyDeviceToHost));
            change /= 2.0 * dim.width * dim.height;
            cout << "change = " << change << endl;

            // Small change => close to convergance => stop
            if (change < params.epsilon && step >= params.minSteps) {
                cout << "Change is less than epsilon (" << params.epsilon << "). Abort." << endl;
                output.steps = step;
                break;
            }
        }

        // Save the current value of u in u_saved
        CUDA_CHECK(cudaMemcpy2D((void*)gpu_u_saved, pitch, gpu_u, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToDevice));

        // ===============================================================
        // Updates of dual and primal variables
        // ===============================================================

        // update xi, psi
        kernel_grad_ascent<<< dimGrid, dimBlock >>>(gpu_xi, gpu_psi, gpu_u_overrelax, gpu_sum_u, gpu_g, params.lambda, dim, pitch/sizeof(float));
        CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

        // update u
        kernel_grad_descent<<< dimGrid, dimBlock >>>(gpu_u, gpu_u_overrelax, gpu_sum_u, gpu_xi, gpu_psi, gpu_dataterm, dim, pitch/sizeof(float));
        CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

        // ===============================================================
        // Display current segmentation
        // ===============================================================
        if (params.displayIteration && (step % params.displayIterationEvery == 0)) {

            // dispaly u
            std::string winName = "u";
            CUDA_CHECK(cudaMemcpy2D((void*)u, widthBytes, gpu_u, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToHost));
            feedback->displayImage(u, dim, winName);
            //cvImageShow(u, dim.height, dim.width, dim.labels, winName.c_str());

            // get uThresh
            kernel_getUThresh<<< dimGrid, dimBlock >>>(gpu_u_thresh, gpu_u, dim, pitch/sizeof(float));
            CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

            // dispaly thresholded u
            winName = "u_thresh";
            CUDA_CHECK(cudaMemcpy2D((void*)u_thresh, widthBytes, gpu_u_thresh, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToHost));
            feedback->displayImage(u_thresh, dim, winName);
        }

        feedback->progress(step, params.maxSteps, 0);
    }
    output.time = (clock() - timeStart)/(float)CLOCKS_PER_SEC;

    cout << "took " << output.time << "s to solve with " << output.steps << "iterations" << endl;

    ///////////////////////////////////////////////////////////////////////////
    /// 6) determine energy gap
    ///////////////////////////////////////////////////////////////////////////

    // threshold u
    kernel_getUThresh<<< dimGrid, dimBlock >>>(gpu_u_thresh, gpu_u, dim, pitch/sizeof(float));
    CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

    // determine energy for u
    Npp64f energy_relax;
    kernel_getEnergy<<< dimGrid, dimBlock >>>(gpu_energy_relax, gpu_u, gpu_g, gpu_dataterm, params.lambda, dim, pitch/sizeof(float));
    CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();
    CUDA_CHECK(nppiSum_32f_C1R(gpu_energy_relax, pitch, nppiSize, gpu_nppDeviceBuffer, gpu_nppSum));
    CUDA_CHECK(cudaMemcpy(&energy_relax, gpu_nppSum, sizeof(Npp64f), cudaMemcpyDeviceToHost));

    // determine energy for uThresh (energyThresh)
    Npp64f energy_thresh;
    kernel_getEnergy<<< dimGrid, dimBlock >>>(gpu_energy_thresh, gpu_u_thresh, gpu_g, gpu_dataterm, params.lambda, dim, pitch/sizeof(float));
    CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();
    CUDA_CHECK(nppiSum_32f_C1R(gpu_energy_thresh, pitch, nppiSize, gpu_nppDeviceBuffer, gpu_nppSum));
    CUDA_CHECK(cudaMemcpy(&energy_thresh, gpu_nppSum, sizeof(Npp64f), cudaMemcpyDeviceToHost));

    cout << "energy_thresh = " << energy_thresh << " and energy_relax = " << energy_relax << endl;
    output.energyGap = abs((energy_thresh - energy_relax)/energy_relax);
    cout << "energy_gap = " << output.energyGap << endl;

    ///////////////////////////////////////////////////////////////////////////
    /// 7) Compute segementation and copy result
    ///////////////////////////////////////////////////////////////////////////

    kernel_getSegmentation<<< dimGrid, dimBlock >>>(gpu_segmentation, gpu_u_thresh, dim, pitchUChar, pitch/sizeof(float));
    CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

    // copy result back to CPU
    CUDA_CHECK(cudaMemcpy2D((void*)u, widthBytes, gpu_u, pitch, widthBytes, heightTotal, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy2D((void*)segmentation, dim.width, gpu_segmentation, pitchUChar, dim.width, dim.height, cudaMemcpyDeviceToHost));

    ///////////////////////////////////////////////////////////////////////////
    /// 8) clean up
    ///////////////////////////////////////////////////////////////////////////

    // free device memory
    CUDA_CHECK(cudaFree(gpu_dataterm));
    CUDA_CHECK(cudaFree(gpu_g));
    CUDA_CHECK(cudaFree(gpu_u));
    CUDA_CHECK(cudaFree(gpu_u_overrelax));
    CUDA_CHECK(cudaFree(gpu_u_thresh));
    CUDA_CHECK(cudaFree(gpu_u_saved));
    CUDA_CHECK(cudaFree(gpu_sum_u));
    CUDA_CHECK(cudaFree(gpu_pixelDifferences));
    CUDA_CHECK(cudaFree(gpu_xi));
    CUDA_CHECK(cudaFree(gpu_psi));
    CUDA_CHECK(cudaFree(gpu_segmentation));
    CUDA_CHECK(cudaFree(gpu_energy_relax));
    CUDA_CHECK(cudaFree(gpu_energy_thresh));
    CUDA_CHECK(cudaFree(gpu_nppDeviceBuffer));
    CUDA_CHECK(cudaFree(gpu_nppSum));

    // free host memory
    delete u_thresh;

    cout << "End: computeSolution" << endl;
}


} // namespace cuda
} // namespace tvseg
