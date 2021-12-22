What is this project ?
----------------------

This project is **NOT** the official VTK-m repository.

It is a fork of VTK-m sources hosted at https://github.com/Kitware/VTK-m

It is used as staging area to maintain topics specific to [Slicer Custom Application](https://github.com/KitwareMedical/SlicerCustomAppTemplate#readme) that will eventually be contributed back to the official repository.


What is the branch naming convention ?
--------------------------------------

Each branch is named following the pattern `kitware-custom-app-Y.Y.Z-YYYY-MM-DD-SHA{N}`

where:

* `kitware-custom-app` is the name of the custom application. Two cases:
  * can be publicly disclosed: replace with the name of the application (e.g `cell-locator`)
  * can **NOT** be publicly disclosed: use the literal prefix `kitware-custom-app` or `k12345-custom-app` where `12345` is the internal Kitware ID)
* `vX.Y.Z` is the version of VTK-m
* `YYYY-MM-DD` is the date of the last official commit associated with the branch.
* `SHA{N}` are the first N characters of the last official commit associated with the branch.

