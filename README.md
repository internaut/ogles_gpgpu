# ogles_gpgpu - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0

## Features

* fast and portable C++ code
* platform optimizations for fast texture access
 * on iOS: [Core Video Texture Cache API](http://allmybrain.com/2011/12/08/rendering-to-a-texture-with-ios-5-texture-cache-api/)
 * on Android: EGL pixelbuffers and [KHRImage extensions](http://snorp.net/2011/12/16/android-direct-texture.html)
* well documented
* contains several example applications
* LGPL licensed

## Tested platforms

* iOS 7.1
* Android 5.0

## Tested devices

* Apple iPad 2
* Apple iPad 4
* Apple iPhone 4s
* XCode Simulator
* Google Nexus 10 (2013)

## Example projects

### iOS examples

These examples should run out-of-the-box.

* OGLESGPGPUStillImage - *Simple project that performs GPU-powered adaptive thresholding on different images*
* OGLESGPGPUVideoCam - *Project that shows real-time GPU-powered image processing on video camera frames*

### Android examples

These projects were created with the Eclipse ADT and use the Android NDK.

To compile, you need to install the [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html) first. Then, do the following: Right click on the project in Eclipse, select *Properties > C/C++ Build > Environment*. Edit the variable *NDK_PATH* to point to your NDK root directory.

* OGStillImageDroid - *Simple project that performs GPU-powered adaptive thresholding on different images*

## TODO

### Milestone 2

* android: camera example
* test ipad3 / ios 8

### Milestone 3

* rasp pi port

### Milestone 4

* comparisons with openCV
* AR support (include into ocv_ar)

### Milestone 5

* more dynamic filters (-> shader code generator)
* create own multipass filter (multiple gauss filters?)

### Milestone 6

* native iOS YUV input support

## License

This project is licensed under [GNU LGPL v2.1](http://choosealicense.com/licenses/lgpl-2.1/). See `LICENSE` file for the full text.
