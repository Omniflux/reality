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
