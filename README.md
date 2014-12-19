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

To compile, you need to install the [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html) first. It is also recommended to [install the Eclipse C/C++ Development Tools (CDT)](http://mhandroid.wordpress.com/2011/01/23/using-eclipse-for-android-cc-development/). Then, do the following: Right click on the project in Eclipse, select *Properties > C/C++ Build > Environment*. Edit the variable *NDK_PATH* to point to your NDK root directory.

* OGStillImageDroid - *Simple project that performs GPU-powered adaptive thresholding on different images*

## How to integrate *ogles_gpgpu* into your project

### Android

0. You need to install and set up the [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html) first. It is also recommended to [install the Eclipse C/C++ Development Tools (CDT)](http://mhandroid.wordpress.com/2011/01/23/using-eclipse-for-android-cc-development/).
1. In your Android project directory, create a folder `jni`. Then copy or link the files `og_jni_wrapper.h` and `og_jni_wrapper.cpp` from *ogles_gpgpu*'s `jni_wrapper` folder to the `jni` folder of your Android project.
2. Copy `jni_wrapper/og_pipeline.template.h` and `jni_wrapper/og_pipeline.template.cpp` to your `jni` folder and rename them to `og_pipeline.h` and `og_pipeline.cpp`, respectively. This where you can later define the image processing pipeline. For now, leave it as it is.
3. Copy `jni_wrapper/Android.template.mk` and `jni_wrapper/Application.template.mk` to your `jni` folder and rename them to `Android.mk` and `Application.mk`, respectively. Edit `Android.mk` and set the proper path to the *ogles_gpgpu* source in the variable `OG_SRC_PATH`.
4. Try compiling the native sources by executing `ndk-build` in your Android project base directory. This should already succeed and the created `libog_jni_wrapper.so` library files will be copied to `libs/`.
5. In your Android project directory, create a folder `src/ogles_gpgpu`. This is where the `OGJNIWrapper` Java source will reside. So link or copy `jni_wrapper/ogles_gpgpu/OGJNIWrapper.java` to `src/ogles_gpgpu`.
6. In Eclipse, refresh your project sources (by pressing F5).
7. In your project's Java source, you can now import `ogles_gpgpu.OGJNIWrapper` and use this class to set up an run *ogles_gpgpu*. See the Android examples on how to do that.

You are ready to go now, basically. However, each time you change something on the C/C++ side of your code (which means everything in the `jni` folder), you will need to call `ndk-build` manually from the command line. The following describes how to set up Eclipse for C/C++ development, which makes the C/C++ workflow a lot easier. However, you need to install the Eclipse CDT first.

1. In Eclipse, select *File > New > Other* and then select *Convert to C/C++ project* (don't be afraid, you will still be able to develop in Java, too!). Press *Next* and select *C++ project*. Under project options set *Makefile Project > Toolchains: Other Toolchain*. Do **not** use *Android GCC* here!
2. Open your project *Properties*. Under *Properties > C/C++ Build > Environment*, add the variable *NDK_PATH* and set it to point to your NDK root directory.
3. Still in your project *Properties*, under *Properties > C/C++ Build* in the *Builder Settings* tab, uncheck *Use default build command*. Instead, set the *Build command* to `${NDK_PATH}/ndk-build`. In the *Behaviour* tab delete the `all` value in the *Build (Incremental build)* field. This field needs to be empty.
4. Yet still in your project *Properties*, under *Properties > C/C++ General > Paths and Symbols*, press *Add...* in the *Includes* tab. Set the *directory* value to `${NDK_PATH}/platforms/android-21/arch-arm/usr/include`. Now CDT knows where to look for Android-NDK-specific header files. Press *Add...* again and set the directory to the path where your *ogles_gpgpu* source resides. In the *Symbols* tab add two symbols: `ANDROID` and `__ANDROID__` (only the names, no value).
5. Yet still in your project *Properties*, under *Properties > C/C++ General > Preprocessor Include Paths, etc.* go to the *Providers* tab and select *CDT GCC Built-In Compiler Settings*. After that, press *OK* and make sure the code analyzer's index will be rebuilt.

Now you are better suited for C++ development on Android. When you start the *Run* command in Eclipse, the C++ sources will be automatically compiled via `ndk-build`. Check the *Console* output for details. Further more, the CDT code analyzer *(CODAN)* helps editing C++ code with auto-suggestions, warnings, errors, etc. However, sometimes CODAN fails properly analyzing the code and will display errors, although `ndk-build` succeeds. You will have to manually delete the errors in the *Problems* tab then.

## Known Issues

1. When using platform optimizations on Android (which enables using the ImageKHR extension), the first processing run will not produce any output (the buffer will only contain zeros). However, any successive runs will work normally.

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
