#include <highfive/H5File.hpp>

using namespace HighFive;

int write_data() {
    FileDriver fdrv;

    fdrv.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    fdrv.add(MetadataBlockSize(10240));

    File file("example2.h5", File::Truncate, fdrv);

    GroupCreateProps props;
    props.add(EstimatedLinkInfo(1000, 500));
    auto group = file.createGroup("g", props);

    DataSetCreateProps dsprops;
    dsprops.add(Chunking(std::vector<hsize_t>{2, 2}));
    dsprops.add(Deflate(9));


    std::vector<int> d1(100000, 1);
    group.createDataSet("dset1", d1, dsprops);
}
