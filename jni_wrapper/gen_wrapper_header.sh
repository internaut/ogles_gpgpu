#!/bin/sh

javac ogles_gpgpu/OGJNIWrapper.java && \
	javah -cp . -o og_jni_wrapper.h ogles_gpgpu.OGJNIWrapper
