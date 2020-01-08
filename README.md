SimpleSeismicReader
===================

A [ParaView](http://www.paraview.org) reader plugin for simple seismic files exported from [OpendTect](http://opendtect.org/).

[![](https://zenodo.org/badge/doi/10.5281/zenodo.10509.png)](https://doi.org/10.5281/zenodo.10509)

Limitations
-----------

- The size of the exported data set must be equal in x and y direction
- Rotation of the data set is not taken into account, in ParaView the data set is axis aligned

Build requirements
------------------

- ParaView build from source (tested with 4.1)
- [CMake](http://www.cmake.org)

Build instructions
------------------

```bash
git clone https://github.com/ufz-vislab/SimpleSeismicReader.git
mkdir build
cd build
cmake ../SimpleSeismicReader -DParaView_DIR=Path/to/your/paraview/build/dir
make 
```

Usage
-----

- Copy the plugin file to ParaViews plugin folder
- Start ParaView
- Set the plugin in ParaView to auto-load
- You can now load OpendTect exported simple seismic files (*.seismic)
