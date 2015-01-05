//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0 
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015 
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "og_pipeline.h"

static ogles_gpgpu::GrayscaleProc grayscaleProc;
static ogles_gpgpu::AdaptThreshProc adaptThreshProc;

void ogPipelineSetup(ogles_gpgpu::Core *core) {
	core->setUseMipmaps(false);

	// set up grayscale processor
	grayscaleProc.setOutputSize(0.5f);

	// set up adaptive thresholding processor
	// (nothing to set up here ...)

	// add the processors to the processing pipeline
	core->addProcToPipeline(&grayscaleProc);
//	core->addProcToPipeline(&adaptThreshProc);
}
