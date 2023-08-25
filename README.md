# Reality for DAZ Studio and Poser
Reality is a plugin/add on for DAZ Studio and Poser to allow rendering in [LuxRender](https://web.archive.org/web/20180220011904/http://www.luxrender.net/en_GB/index).

It was a commercial product from August 2010 - September 2019 when the source code was released under the BSD 3-Clause License.

The inital commits of this repository contain the source as made available from Prêt-à-3D (Paolo Ciccone) with the following changes:
 - Remove items we do not have redistribution rights for
   - IBL single lights (SingleIBLs)
   - Studio Cubed
   - OS X Icon Template
   - DAZ Studio 4 SDK

 - Remove large dependencies available elsewhere
   - Boost 1.54
   - Qt 4.8.4
   - Python 2.6/2.7

 - Losslessly recompressed images and PDFs.


There is another open source plugin available for DAZ Studio to integrate with [LuxCoreRenderer](https://luxcorerender.org/) called [yaluxplug](https://github.com/danielbui78/yaluxplug).


## Building for Windows
DAZ Studio 4.x plugins must be built against the Visual Studio 2010 Redistributable package.

The easiest way to do this is to install the Windows 7.1 SDK. This may be difficult to do on recent versions of Windows, but can be done. Visual Studio itself is not required, but any version from 2008-2022 is compatible with the SDK if desired.
- [Microsoft Windows SDK for Windows 7](https://www.microsoft.com/en-us/download/details.aspx?id=8279)

Perl and CMake are also required
- [Perl 5](https://www.perl.org/get.html)
- [CMake 3.25](https://cmake.org/download/) - CMake is included with Visual Studio 2019+ or can be installed independently

Several other libraries must also be available and built against the same Redistributable package. The easiest way to do this is to download the official pre-built packages, although you can build them from source yourself, details to do so are not covered here. 64-bit packages are listed below, but 32-bit packages are also available.
- Boost 1.67 - [boost_1_67_0-msvc-10.0-64.exe](https://sourceforge.net/projects/boost/files/boost-binaries/1.67.0/)
- Python 2.7.18 - [python-2.7.18.amd64.msi](https://www.python.org/downloads/release/python-2718/)
- Qt 4.8.7 - [qt-4.8.7-x64-msvc2010.exe](https://sourceforge.net/projects/qt64ng/files/qt/x86-64/4.8.7/msvc2010/)
- [DAZ Studio 4.5+ SDK](https://www.daz3d.com/daz-studio-4-5-sdk)
- An OpenCL SDK - I use [NVIDIA's CUDA Toolkit](https://developer.nvidia.com/cuda-downloads), but any vendors should be fine.

Once these are installed, copy `src/CMakeUserPresets.json.template` to `src/CMakeUserPresets.json` and update to match your install locations.
- BOOST_ROOT
- QTDIR
- DAZStudioSDK_ROOT
- Add a generator field to override the generator if not using the one set in `CMakePresets.json`

Open the `src` directory in Visual Studio\
 OR\
At a command prompt in the `src` directory run
```Batchfile
cmake --preset my-windows-x64-{debug,release}
```
In the newly created build directory `build/my-windows-x64-{debug,release}` run
```Batchfile
cmake --build .
```


## Building for OS X
This does not work at the moment, need help from an OS X developer to fix `CMakePresets.json`, `CMakeUserPresets.json.template`, `CMakeLists.txt` and `FindDAZStudioSDK.cmake`

## Building installation packages
This does not work at the moment, needs to be reimplemented for [CPack](https://cmake.org/cmake/help/latest/module/CPack.html)


## TODO
- Fix OS X builds
- Implement building bundles/installers with CMake
- Port Poser 11 addon to Poser 12
  - Poser switched from Python 2 to Python 3
- Separate DAZ Studio Plugin from Reality so both DS 4 and DS 5+ plugins can exist
  - DS 4 Plugin must be Qt 4 while DS 5 Plugin will required Qt 5+
- Update build dependencies
  - Requires separating DS 4 Plugin because plugin must be built with a very old compiler (MSVC 10)
- Update Reality to Qt 5+
