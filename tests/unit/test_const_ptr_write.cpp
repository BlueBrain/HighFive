#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

const std::string FILE_NAME("3d_dataset_from_flat.h5");
const std::string DATASET_NAME("dset");
constexpr std::size_t dim = 3;


template<typename Data>
static Data const*const*const* flat_to_3d(Data const * const data)
{
    return reinterpret_cast<Data const*const*const* >(data);
}

template<typename Data>
void write_read()
{
    using namespace HighFive;
    try
    {
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        std::vector<std::size_t> shape(dim, dim);
        DataSpace dataspace = DataSpace(shape);

        DataSet dataset = file.createDataSet<double>(DATASET_NAME, dataspace);

        std::vector<double> const t1(dim * dim * dim, 1);
        dataset.write(flat_to_3d(t1.data()));

        std::vector<std::vector<std::vector<Data>>> result;
        dataset.read(result);

        for (auto slab : result)
        {
            for (auto row : slab)
            {
                for (auto col : row)
                    std::cout << " " << col;
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }
    catch (const Exception& err)
    {
        std::cerr << err.what() << std::endl;
    }
}

int main(void)
{
    write_read<double>();
    return 0;
}
