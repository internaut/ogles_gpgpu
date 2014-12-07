#include "og_pipeline.h"

static ogles_gpgpu::GrayscaleProc grayscaleProc;
static ogles_gpgpu::AdaptThreshProc adaptThreshProc;

void ogPipelineSetup(ogles_gpgpu::Core *core) {
	core->setUseMipmaps(false);

	// set up grayscale processor
	grayscaleProc.setOutputSize(0.5f);

	// set up adaptive thresholding processor
	// ...

	// add the processors to the processing pipeline
	core->addProcToPipeline(&grayscaleProc);
	core->addProcToPipeline(&adaptThreshProc);
}
