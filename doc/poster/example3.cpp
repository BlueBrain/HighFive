#include <highfive/highfive.hpp>


typedef struct {
    double width;
    double height;
} Size2D;


HighFive::CompoundType create_compound_Size2D() {
    return {{"width", HighFive::AtomicType<double>{}}, {"height", HighFive::AtomicType<double>{}}};
}

HIGHFIVE_REGISTER_TYPE(Size2D, create_compound_Size2D)

int data_io() {
    const std::string DATASET_NAME("points");

    HighFive::File file("compounds.h5", HighFive::File::Truncate);

    auto t1 = create_compound_Size2D();
    t1.commit(file, "Size2D");

    std::vector<Size2D> pts = {{1., 2.5}, {3., 4.5}};
    auto dataset = file.createDataSet(DATASET_NAME, pts);

    auto g1 = file.createGroup("group1");
    g1.createAttribute(DATASET_NAME, pts);
}
