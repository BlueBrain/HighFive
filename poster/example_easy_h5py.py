import h5py
import numpy as np

A = np.ones([10, 3])

with h5py.File("tmp.h5", "w") as file:

    # write dataset (automatically creates groups if needed)
    file["/path/to/A"] = A

    # read from dataset
    B = file["/path/to/A"]

    # write attribute
    file["/path/to/A"].attrs["date"] = "today"

    # read from attribute
    d = file["/path/to/A"].attrs["date"]

    # create extendible dataset and extend it
    dset = file.create_dataset("/path/to/extendible", (1,), maxshape=(None,))
    dset[0] = 0

    for i in range(1, 10):
        dset.resize((i + 1,))
        dset[i] = i
