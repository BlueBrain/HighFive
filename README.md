> [!WARNING]
> The BlueBrainProject closed at the end of 2024. This unfortunately required
> archiving all repositories under `BlueBrain` including HighFive.
>
> The development of HighFive will continue at:
>   https://github.com/highfive-devs/highfive


# HighFive - HDF5 header-only C++ Library

[![Zenodo](https://zenodo.org/badge/47755262.svg)](https://zenodo.org/doi/10.5281/zenodo.10679422)

## Examples

```c++
#include <highfive/highfive.hpp>

using namespace HighFive;

std::string filename = "/tmp/new_file.h5";

{
    // We create an empty HDF55 file, by truncating an existing
    // file if required:
    File file(filename, File::Truncate);

    std::vector<int> data(50, 1);
    file.createDataSet("grp/data", data);
}

{
    // We open the file as read-only:
    File file(filename, File::ReadOnly);
    auto dataset = file.getDataSet("grp/data");

    // Read back, with allocating:
    auto data = dataset.read<std::vector<int>>();

    // Because `data` has the correct size, this will
    // not cause `data` to be reallocated:
    dataset.read(data);
}
```

# Funding & Acknowledgment
 
The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.

HighFive releases are uploaded to Zenodo. If you wish to cite HighFive in a
scientific publication you can use the DOIs for the
[Zenodo records](https://zenodo.org/doi/10.5281/zenodo.10679422).
 
Copyright © 2015-2022 Blue Brain Project/EPFL


### License

Boost Software License 1.0
