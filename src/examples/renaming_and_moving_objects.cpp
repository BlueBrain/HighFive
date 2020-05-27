#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Attribute.hpp>

using namespace HighFive;

int main(int argc, char *argv[])
{
    /* We are going to create group in root directory then add
     * dataset to this group and attach attribute to the dataset.
     * Then we are trying to get path to the root, group dataset
     * and the name of the dataset.
     * Secondly we will move dataset with attached attribute to
     * some destination path. Every '/' sign means that a group
     * will be created there (it is done automatically).
     * When we have moved the dataset we can check if dataset
     * object is still valid? We do this by creating second
     * attribute */

    // Create a new file using the default property lists.
    HighFive::File file("names.h5", File::ReadWrite | File::Create | File::Truncate);

    // Create a group
    HighFive::Group group = file.createGroup("group");

    // Create a dummy dataset of one single integer
    DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
    dataset.write(100);

    // Now let's add a attribute to this dataset
    std::string string_list("very important Dataset !");
    Attribute attribute = dataset.createAttribute<std::string>("attribute", DataSpace::From(string_list));
    attribute.write(string_list);

    // Get path and names
    std::cout << "root path: \t" << file.getPath() << std::endl;
    std::cout << "group path: \t" << group.getPath() << std::endl;
    std::cout << "dataset path: \t" << dataset.getPath() << std::endl;
    std::cout << "attribute name: \t" << attribute.getName() << std::endl;
    std::cout << std::endl;

    // move dataset with its attribute to another destination path
    group.moveObject("data", file, "/NewGroup/SubGroup/movedData");

    // as you can see to reach destination path new groups were created as well
    std::cout << "dataset new path: \t" << dataset.getPath() << std::endl;
    std::cout << std::endl;

    // we can still use moved dataset
    // let's create new attribute
    Attribute attributeNew = dataset.createAttribute<std::string>("attributeNew", DataSpace::From(string_list));
    attribute.write(string_list);
    std::cout << "attribute new name: \t" << attributeNew.getName() << std::endl;
    std::cout << std::endl;

    file.flush();
}
