> [!WARNING]
> The BlueBrainProject closed at the end of 2024. This unfortunately required
> archiving all repositories under `BlueBrain` including HighFive.
>
> The development of HighFive will continue at:
>   https://github.com/highfive-devs/highfive


# HighFive - HDF5 header-only C++ Library

The releases for versions 2.x.y and two prereleases of v3 can be found at:
* https://github.com/BlueBrain/HighFive/releases
* https://zenodo.org/doi/10.5281/zenodo.10679422

All future development and full releases can be found at:
* https://github.com/highfive-devs/highfive

## Example

```c++
using namespace HighFive;

File file("foo.h5", File::Truncate);

{
    std::vector<int> data(50, 1);
    file.createDataSet("grp/data", data);
}

{
    auto dataset = file.getDataSet("grp/data");

    // Read back, automatically allocating:
    auto data = dataset.read<std::vector<int>>();

    // Alternatively, if `data` has the correct
    // size, without reallocation:
    dataset.read(data);
}
```

# Funding & Acknowledgment

The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.

HighFive releases are uploaded to Zenodo. If you wish to cite HighFive in a
scientific publication you can use the DOIs for the
[Zenodo records](https://zenodo.org/doi/10.5281/zenodo.10679422).

Copyright © 2015-2024 Blue Brain Project/EPFL


### License

Boost Software License 1.0
