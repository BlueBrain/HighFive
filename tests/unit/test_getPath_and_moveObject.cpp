#include <H5File.hpp>
#include <H5Group.hpp>
#include <H5DataSet.hpp>
#include <H5DataSpace.hpp>
#include <H5Attribute.hpp>
#include <H5PropertyList.hpp>

using namespace HighFive;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Create a new file using the default property lists.
    HighFive::File file("names.h5", File::ReadWrite | File::Create | File::Truncate);
    HighFive::Group emptyGroup = file.createGroup("/asd/gfg/asdg");

    HighFive::Group group = file.createGroup("group");

    // Create a dummy dataset of one single integer
    DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
    dataset.write(100);

    // Now let's add a attribute on this dataset
    std::string string_list("very important Dataset !");
    Attribute attribute = dataset.createAttribute<std::string>("attribute", DataSpace::From(string_list));
    attribute.write(string_list);

    // Get path and names
    std::cout << "root path: \t" << file.getObjectPath() << std::endl;
    std::cout << "group path: \t" << group.getObjectPath() << std::endl;
    std::cout << "dataset path: \t" << dataset.getDatasetPath() << std::endl;
    std::cout << "attribute name: \t" << attribute.getAttributeName() << std::endl;
    std::cout << std::endl;

    // move dataset with its attribute to another destination
    // as you can see to reach destination position new groups were created as well
    group.moveObject("data", file, "/NewGroup/SubGroup/movedData");
    std::cout << "dataset new path: \t" << dataset.getDatasetPath() << std::endl;
    std::cout << std::endl;

    // we can still use moved dataset
    // let's create new attribute after dataset was moved
    Attribute attributeNew = dataset.createAttribute<std::string>("attributeNew", DataSpace::From(string_list));
    attribute.write(string_list);
    std::cout << "attribute new name: \t" << attributeNew.getAttributeName() << std::endl;
    std::cout << std::endl;

    file.flush();

    return a.exec();
}
