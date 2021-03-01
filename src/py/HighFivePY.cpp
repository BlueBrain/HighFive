#define H5_USE_EIGEN  // should be defined before including HighFive

#include <highfive/H5Object.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

#include <Eigen/Dense>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<int>)
PYBIND11_MAKE_OPAQUE(Eigen::MatrixX<int>)

/* to generate .pyi the returned type should be declared before it
 * is called. For example `createGroup` returns `Group` so I need to
 * place `py::class_<Group, Object>(m, "Group")`
 * before calling `.def("createGroup", ...` ) */

/* py::arithmetic() -> create an enumeration that also supports
 * rudimentary arithmetic and bit-level operations like
 * comparisons, and, or, xor, negation, etc. */

/* export_values() -> export the enum entries into the parent scope,
 * which should be skipped for newer C++11-style strongly typed enums */

/* py::const_ -> is neccessary when binding overoaded functions */

namespace HighFive {

std::string moduleInfo(){
  return "HDF5 wrapper originally written in C++: "
"https://github.com/BlueBrain/HighFive";
}

py::enum_<ObjectType> ObjectType_enum(py::module &m) {
  return py::enum_<ObjectType>(m, "ObjectType", py::arithmetic())
      .value("File", ObjectType::File)
      .value("Group", ObjectType::Group)
      .value("UserDataType", ObjectType::UserDataType)
      .value("DataSpace", ObjectType::DataSpace)
      .value("Dataset", ObjectType::Dataset)
      .value("Attribute", ObjectType::Attribute)
      .value("Other", ObjectType::Other);
}

py::enum_<LinkType> LinkType_enum(py::module &m) {
  return py::enum_<LinkType>(m, "LinkType", py::arithmetic())
      .value("Hard", LinkType::Hard)
      .value("Soft", LinkType::Soft)
      .value("External", LinkType::External)
      .value("Other", LinkType::Other);
}

py::enum_<PropertyType> PropertyType_enum(py::module &m) {
  return py::enum_<PropertyType>(m, "PropertyType", py::arithmetic())
      .value("OBJECT_CREATE", PropertyType::OBJECT_CREATE)
      .value("FILE_CREATE", PropertyType::FILE_CREATE)
      .value("FILE_ACCESS", PropertyType::FILE_ACCESS)
      .value("DATASET_CREATE", PropertyType::DATASET_CREATE)
      .value("DATASET_ACCESS", PropertyType::DATASET_ACCESS)
      .value("DATASET_XFER", PropertyType::DATASET_XFER)
      .value("GROUP_CREATE", PropertyType::GROUP_CREATE)
      .value("GROUP_ACCESS", PropertyType::GROUP_ACCESS)
      .value("DATATYPE_CREATE", PropertyType::DATATYPE_CREATE)
      .value("DATATYPE_ACCESS", PropertyType::DATATYPE_ACCESS)
      .value("STRING_CREATE", PropertyType::STRING_CREATE)
      .value("ATTRIBUTE_CREATE", PropertyType::ATTRIBUTE_CREATE)
      .value("OBJECT_COPY", PropertyType::OBJECT_COPY)
      .value("LINK_CREATE", PropertyType::LINK_CREATE)
      .value("LINK_ACCESS", PropertyType::LINK_ACCESS);
}

py::enum_<DataTypeClass> DataTypeClass_enum(py::module &m) {
  return py::enum_<DataTypeClass>(m, "DataTypeClass", py::arithmetic())
      .value("Time", DataTypeClass::Time)
      .value("Integer", DataTypeClass::Integer)
      .value("Float", DataTypeClass::Float)
      .value("String", DataTypeClass::String)
      .value("BitField", DataTypeClass::BitField)
      .value("Opaque", DataTypeClass::Opaque)
      .value("Compound", DataTypeClass::Compound)
      .value("Reference", DataTypeClass::Reference)
      .value("Enum", DataTypeClass::Enum)
      .value("VarLen", DataTypeClass::VarLen)
      .value("Array", DataTypeClass::Array)
      .value("Invalid", DataTypeClass::Invalid);
}

py::enum_<File::OpenFlag> OpenFlag_enum(py::module &m) {
  return py::enum_<File::OpenFlag>(m, "OpenFlag", py::arithmetic())
      .value("ReadOnly", File::ReadOnly)
      .value("ReadWrite", File::ReadWrite)
      .value("Truncate", File::Truncate)
      .value("Excl", File::Excl)
      .value("Debug", File::Debug)
      .value("Create", File::Create)
      .value("Overwrite", File::Overwrite)
      .value("OpenOrCreate", File::OpenOrCreate)
      .export_values();
}

template <PropertyType T>
py::class_<PropertyList<T> > PropertyList_decl(py::module &m, const std::string& typeStr) {
  return py::class_<PropertyList<T> >(m, typeStr.c_str());
}

template <PropertyType T>
void PropertyList_def(py::class_<PropertyList<T> >& py_obj) {
  py_obj.def("getObjectType", &PropertyList<T>::getObjectType)
      .def("getId", &PropertyList<T>::getId);
}

py::class_<LinkCreateProps> LinkCreateProps_decl(py::module &m){
  return py::class_<LinkCreateProps,
      PropertyList<PropertyType::LINK_CREATE> >(
        m, "LinkCreateProps");
}

void LinkCreateProps_def(py::class_<LinkCreateProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<LinkAccessProps> LinkAccessProps_decl(py::module &m){
  return py::class_<LinkAccessProps,
      PropertyList<PropertyType::LINK_ACCESS> >(
        m, "LinkAccessProps");
}

void LinkAccessProps_def(py::class_<LinkAccessProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<GroupCreateProps> GroupCreateProps_decl(py::module &m){
  return py::class_<GroupCreateProps,
      PropertyList<PropertyType::GROUP_CREATE> >(
        m, "GroupCreateProps");
}

void GroupCreateProps_def(py::class_<GroupCreateProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<GroupAccessProps> GroupAccessProps_decl(py::module &m){
  return py::class_<GroupAccessProps,
      PropertyList<PropertyType::GROUP_ACCESS> >(
        m, "GroupAccessProps");
}

void GroupAccessProps_def(py::class_<GroupAccessProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<DataSetCreateProps> DataSetCreateProps_decl(py::module &m){
  return py::class_<DataSetCreateProps,
      PropertyList<PropertyType::DATASET_CREATE> >(
        m, "DataSetCreateProps");
}

void DataSetCreateProps_def(py::class_<DataSetCreateProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<DataSetAccessProps> DataSetAccessProps_decl(py::module &m){
  return py::class_<DataSetAccessProps,
      PropertyList<PropertyType::DATASET_ACCESS> >(
        m, "DataSetAccessProps");
}

void DataSetAccessProps_def(py::class_<DataSetAccessProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<DataTypeCreateProps> DataTypeCreateProps_decl(py::module &m){
  return py::class_<DataTypeCreateProps,
      PropertyList<PropertyType::DATATYPE_CREATE> >(
        m, "DataTypeCreateProps");
}

void DataTypeCreateProps_def(py::class_<DataTypeCreateProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<DataTypeAccessProps> DataTypeAccessProps_decl(py::module &m){
  return py::class_<DataTypeAccessProps,
      PropertyList<PropertyType::DATATYPE_ACCESS> >(
        m, "DataTypeAccessProps");
}

void DataTypeAccessProps_def(py::class_<DataTypeAccessProps>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<ObjectInfo>
ObjectInfo_decl(py::module &m){
  return py::class_<ObjectInfo>(m, "ObjectInfo");
}

void ObjectInfo_def(py::class_<ObjectInfo>& py_obj){
  py_obj.def("getAddress", &ObjectInfo::getAddress)
      .def("getHardLinkRefCount", &ObjectInfo::getHardLinkRefCount)
      .def("getCreationTime", &ObjectInfo::getCreationTime)
      .def("getModificationTime", &ObjectInfo::getModificationTime);
}

py::class_<LinkInfo>
LinkInfo_decl(py::module &m){
  return py::class_<LinkInfo>(m, "LinkInfo");
}

void LinkInfo_def(py::class_<LinkInfo>& py_obj){
  py_obj.def("getLinkType", &LinkInfo::getLinkType)
      .def("creationOrderValid", &LinkInfo::creationOrderValid)
      .def("getCreationOrder", &LinkInfo::getCreationOrder)
      .def("getLinkNameCharacterSet", &LinkInfo::getLinkNameCharacterSet)
    #if (H5Lget_info_vers < 2)
      .def("getAddress", &LinkInfo::getAddress)
    #else
      .def("getHardLinkToken", &LinkInfo::getHardLinkToken)
    #endif
      .def("getSoftLinkSize", &LinkInfo::getSoftLinkSize);
}

py::class_<ElementSet>
ElementSet_decl(py::module &m){
  return py::class_<ElementSet>(m, "ElementSet");
}

void ElementSet_def(py::class_<ElementSet>& py_obj) {
  //  py_obj.def(py::init<std::initializer_list<std::size_t> >())
  //      .def(py::init<std::initializer_list<std::vector<std::size_t> > >())
  py_obj.def(py::init<const std::vector<std::size_t>& >())
      .def(py::init<const std::vector<std::vector<std::size_t> >& >());
}

template <typename Derivate>
py::class_<SliceTraits<Derivate> > SliceTraits_decl(py::module &m, const std::string& typeStr) {
  return py::class_<SliceTraits<Derivate> >(m, typeStr.c_str());
}

template <typename Derivate>
void SliceTraits_def(py::class_<SliceTraits<Derivate> >& py_obj){
  py_obj.def("select", py::overload_cast<
             const std::vector<size_t>&,
             const std::vector<size_t>&,
             const std::vector<size_t>&>(
               &SliceTraits<Derivate>::select, py::const_),
             py::arg("offset"),
             py::arg("count"),
             py::arg_v("stride", std::vector<size_t>(), "vector<size_t>()"))
      .def("select", py::overload_cast<
           const std::vector<size_t>&>(
             &SliceTraits<Derivate>::select, py::const_),
           py::arg("columns"))
      .def("select", py::overload_cast<
           const ElementSet&>(
             &SliceTraits<Derivate>::select, py::const_),
           py::arg("elements"))
      .def("write", &SliceTraits<Derivate>::template write<char>)
      .def("write", &SliceTraits<Derivate>::template write<signed char>)
      .def("write", &SliceTraits<Derivate>::template write<unsigned char>)
      .def("write", &SliceTraits<Derivate>::template write<short>)
      .def("write", &SliceTraits<Derivate>::template write<unsigned short>)
      .def("write", &SliceTraits<Derivate>::template write<int>)
      .def("write", &SliceTraits<Derivate>::template write<unsigned>)
      .def("write", &SliceTraits<Derivate>::template write<long>)
      .def("write", &SliceTraits<Derivate>::template write<unsigned long>)
      .def("write", &SliceTraits<Derivate>::template write<long long>)
      .def("write", &SliceTraits<Derivate>::template write<unsigned long long>)
      .def("write", &SliceTraits<Derivate>::template write<float>)
      .def("write", &SliceTraits<Derivate>::template write<double>)
      .def("write", &SliceTraits<Derivate>::template write<bool>);
//      .def("write", &SliceTraits<Derivate>::template write<std::complex<double> >);

  py_obj.def("write", &SliceTraits<Derivate>::template write<std::vector<char> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<signed char> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<unsigned char> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<short> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<unsigned short> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<int> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<unsigned> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<long> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<unsigned long> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<long long> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<unsigned long long> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<float> >)
      .def("write", &SliceTraits<Derivate>::template write<std::vector<double> >);
//      .def("write", &SliceTraits<Derivate>::template write<std::vector<bool> >);
//      .def("write", &SliceTraits<Derivate>::template write<std::vector<std::complex<double> > >);

  py_obj.def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<char> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<signed char> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<unsigned char> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<short> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<unsigned short> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<int> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<unsigned> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<long> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<unsigned long> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<long long> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<unsigned long long> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<float> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<double> >)
      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<bool> >);
//      .def("write", &SliceTraits<Derivate>::template write<Eigen::MatrixX<std::complex<double> > >);

//  py_obj.def("read", py::overload_cast<char&>(&SliceTraits<Derivate>::template read<char>, py::const_))
//      .def("read", py::overload_cast<signed char&>(&SliceTraits<Derivate>::template read<signed char>, py::const_))
//      .def("read", py::overload_cast<unsigned char&>(&SliceTraits<Derivate>::template read<unsigned char>, py::const_))
//      .def("read", py::overload_cast<short&>(&SliceTraits<Derivate>::template read<short>, py::const_))
//      .def("read", py::overload_cast<unsigned short&>(&SliceTraits<Derivate>::template read<unsigned short>, py::const_))
//      .def("read", py::overload_cast<int&>(&SliceTraits<Derivate>::template read<int>, py::const_))
//      .def("read", py::overload_cast<unsigned&>(&SliceTraits<Derivate>::template read<unsigned>, py::const_))
//      .def("read", py::overload_cast<long&>(&SliceTraits<Derivate>::template read<long>, py::const_))
//      .def("read", py::overload_cast<unsigned long&>(&SliceTraits<Derivate>::template read<unsigned long>, py::const_))
//      .def("read", py::overload_cast<long long&>(&SliceTraits<Derivate>::template read<long long>, py::const_))
//      .def("read", py::overload_cast<unsigned long long&>(&SliceTraits<Derivate>::template read<unsigned long long>, py::const_))
//      .def("read", py::overload_cast<float&>(&SliceTraits<Derivate>::template read<float>, py::const_))
//      .def("read", py::overload_cast<double&>(&SliceTraits<Derivate>::template read<double>, py::const_))
//      .def("read", py::overload_cast<bool&>(&SliceTraits<Derivate>::template read<bool>, py::const_));
//      .def("read", py::overload_cast<std::complex<double>&>(&SliceTraits<Derivate>::template read<std::complex<double> >, py::const_));

//  py_obj.def("read", py::overload_cast<std::vector<char>&>(&SliceTraits<Derivate>::template read<std::vector<char> >, py::const_))
//      .def("read", py::overload_cast<std::vector<signed char>&>(&SliceTraits<Derivate>::template read<std::vector<signed char> >, py::const_))
//      .def("read", py::overload_cast<std::vector<unsigned char>&>(&SliceTraits<Derivate>::template read<std::vector<unsigned char> >, py::const_))
//      .def("read", py::overload_cast<std::vector<short>&>(&SliceTraits<Derivate>::template read<std::vector<short> >, py::const_))
//      .def("read", py::overload_cast<std::vector<unsigned short>&>(&SliceTraits<Derivate>::template read<std::vector<unsigned short> >, py::const_))
//      .def("read", py::overload_cast<std::vector<int>&>(&SliceTraits<Derivate>::template read<std::vector<int> >, py::const_))
//      .def("read", py::overload_cast<std::vector<unsigned>&>(&SliceTraits<Derivate>::template read<std::vector<unsigned> >, py::const_))
//      .def("read", py::overload_cast<std::vector<long>&>(&SliceTraits<Derivate>::template read<std::vector<long> >, py::const_))
//      .def("read", py::overload_cast<std::vector<unsigned long>&>(&SliceTraits<Derivate>::template read<std::vector<unsigned long> >, py::const_))
//      .def("read", py::overload_cast<std::vector<long long>&>(&SliceTraits<Derivate>::template read<std::vector<long long> >, py::const_))
//      .def("read", py::overload_cast<std::vector<unsigned long long>&>(&SliceTraits<Derivate>::template read<std::vector<unsigned long long> >, py::const_))
//      .def("read", py::overload_cast<std::vector<float>&>(&SliceTraits<Derivate>::template read<std::vector<float> >, py::const_))
//      .def("read", py::overload_cast<std::vector<double>&>(&SliceTraits<Derivate>::template read<std::vector<double> >, py::const_));
//      .def("read", py::overload_cast<std::vector<bool>&>(&SliceTraits<Derivate>::template read<std::vector<bool> >, py::const_));
//      .def("read", py::overload_cast<std::vector<std::complex<double> >&>(&SliceTraits<Derivate>::template read<std::vector<std::complex<double> > >, py::const_));

  py_obj.def("read", [](const SliceTraits<Derivate> &slice)->Eigen::MatrixX<double> {
    Eigen::MatrixX<double> M;
    slice.read(M);
    return M;
  });

  py_obj.def("read", [](const SliceTraits<Derivate> &slice)->Eigen::MatrixX<float> {
    Eigen::MatrixX<float> M;
    slice.read(M);
    return M;
  });

  py_obj.def("read", [](const SliceTraits<Derivate> &slice)->Eigen::MatrixX<int> {
    Eigen::MatrixX<int> M;
    slice.read(M);
    return M;
  });

//  [](const SliceTraits<Derivate> &slice)->Eigen::MatrixX<double> {
//    Eigen::MatrixX<double> M;
//    slice.read(M);
//    return M;
//  };
//  py_obj.def("read", py::overload_cast<py::EigenDRef<Eigen::MatrixX<double>>&>(
//               &SliceTraits<Derivate>::template read<py::EigenDRef<Eigen::MatrixX<double> > >, py::const_));
//  py_obj.def("read", py::overload_cast<py::EigenDRef<Eigen::MatrixX<char> > >(&SliceTraits<Derivate>::template read<Eigen::MatrixX<char> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<signed char>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<signed char> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<unsigned char>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<unsigned char> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<short>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<short> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<unsigned short>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<unsigned short> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<int>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<int> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<unsigned>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<unsigned> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<long>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<long> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<unsigned long>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<unsigned long> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<long long>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<long long> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<unsigned long long>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<unsigned long long> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<float>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<float> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<double>&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<double> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<bool> &>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<bool> >, py::const_))
//      .def("read", py::overload_cast<Eigen::MatrixX<std::complex<double> >&>(&SliceTraits<Derivate>::template read<Eigen::MatrixX<std::complex<double> > >, py::const_));

//  py_obj.def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<char> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<signed char> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<unsigned char> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<short> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<unsigned short> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<int> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<unsigned> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<long> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<unsigned long> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<long long> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<unsigned long long> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<float> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<double> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<bool> > >)
//      .def("write", &SliceTraits<Derivate>::template write<py::EigenDRef<Eigen::MatrixX<std::complex<double> > > >);
}

py::class_<Object>
Object_decl(py::module &m){
  return py::class_<Object>(m, "Object");
}

void Object_def(py::class_<Object>& py_obj) {
  py_obj.def("isValid", &Object::isValid)
      .def("refresh", &Object::refresh)
      .def("getId", &Object::getId)
      .def("getFileId", &Object::getFileId)
      .def("getFileName", &Object::getFileName)
      .def("getIdRefCount", &Object::getIdRefCount)
      .def("getObjectInfo", &Object::getObjectInfo)
      .def("getObjectType", &Object::getObjectType);
}

py::class_<DataType, Object>
DataType_decl(py::module &m){
  return py::class_<DataType, Object>(m, "DataType");
}

void DataType_def(py::class_<DataType, Object>& py_obj) {
  py_obj.def(py::init<>())
      .def("getClass", &DataType::getClass)
      .def("getSize", &DataType::getSize)
      .def("string", &DataType::string)
      .def("isVariableStr", &DataType::isVariableStr)
      .def("isFixedLenStr", &DataType::isFixedLenStr)
      .def("empty", &DataType::empty)
      .def("isReference", &DataType::isReference);
}

template <typename T>
py::class_<AtomicType<T>, DataType> AtomicType_decl(py::module &m, const std::string& typeStr) {
  return py::class_<AtomicType<T>, DataType>(m, typeStr.c_str());
}

template <typename T>
void AtomicType_def(py::class_<AtomicType<T>, DataType>& py_obj){
  py_obj.def(py::init<>());
}

py::class_<DataSpace, Object>
DataSpace_decl(py::module &m){
  return py::class_<DataSpace, Object>(m, "DataSpace");
}

void DataSpace_def(py::class_<DataSpace, Object>& py_obj) {
  py_obj.def(py::init<const std::vector<size_t>&>());
  //  py_obj.def(py::init<const std::initializer_list<size_t>&>());
}

template <typename Derivate>
py::class_<NodeTraits<Derivate> > NodeTraits_decl(py::module &m, const std::string& typeStr) {
  return py::class_<NodeTraits<Derivate> >(m, typeStr.c_str());
}

template <typename Derivate>
void NodeTraits_def(py::class_<NodeTraits<Derivate> >& py_obj){
  py_obj.def("createDataSet", py::overload_cast<
             const std::string&,
             const DataSpace&,
             const DataType&,
             const LinkCreateProps&,
             const DataSetCreateProps&,
             const DataSetAccessProps&>(
               &NodeTraits<Derivate>::template createDataSet<void>),
             py::arg("dataset_name"),
             py::arg("space"),
             py::arg("type"),
             py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
             py::arg_v("dsetCreateProps", DataSetCreateProps(), "DataSetCreateProps()"),
             py::arg_v("dsetAccessProps", DataSetAccessProps(), "DataSetAccessProps()"))
      .def("getDataType", &NodeTraits<Derivate>::getDataType,
           py::arg("dtype_name"),
           py::arg_v("dtypeAccessProps", DataTypeAccessProps(), "DataTypeAccessProps()"))
      .def("getDataSet", &NodeTraits<Derivate>::getDataSet,
           py::arg("dset_name"),
           py::arg_v("dsetAccessProps", DataSetAccessProps(), "DataSetAccessProps()"))
      .def("createGroup", &NodeTraits<Derivate>::createGroup,
           py::arg("group_name"),
           py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
           py::arg_v("groupCreateProps", GroupCreateProps(), "GroupCreateProps()"),
           py::arg_v("groupAccessProps", GroupAccessProps(), "GroupAccessProps()"))
      .def("getGroup", &NodeTraits<Derivate>::getGroup,
           py::arg("group_name"),
           py::arg_v("groupAccessProps", GroupAccessProps(), "GroupAccessProps()"))
      .def("getObjectName", &NodeTraits<Derivate>::getObjectName,
           py::arg("index"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("rename", &NodeTraits<Derivate>::rename,
           py::arg("src_path"),
           py::arg("dest_path"),
           py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("listObjectNames", &NodeTraits<Derivate>::listObjectNames)
      .def("exist", &NodeTraits<Derivate>::exist,
           py::arg("obj_name"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("hasObject", &NodeTraits<Derivate>::hasObject,
           py::arg("objName"),
           py::arg("objectType"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("unlink", &NodeTraits<Derivate>::unlink,
           py::arg("obj_name"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("getLinkType", &NodeTraits<Derivate>::getLinkType,
           py::arg("obj_name"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("getObjectType", &NodeTraits<Derivate>::getObjectType,
           py::arg("obj_name"),
           py::arg_v("linkAccessProps", LinkAccessProps(), "LinkAccessProps()"))
      .def("createLink", py::overload_cast<
           const File&,
           const std::string&,
           const LinkType&,
           const LinkCreateProps&,
           const LinkAccessProps&,
           const GroupAccessProps&>(&NodeTraits<Derivate>::template createLink<File>),
           py::arg("file"),// "target object",
           py::arg("linkName"),// "name for a new link",
           py::arg("linkType"),
           py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
           py::arg_v("linkAccesProps", LinkAccessProps(), "LinkAccessProps()"),
           py::arg_v("groupAccessProps", GroupAccessProps(), "GroupAccessProps()"))
      .def("createLink", py::overload_cast<
           const Group&,
           const std::string&,
           const LinkType&,
           const LinkCreateProps&,
           const LinkAccessProps&,
           const GroupAccessProps&>(&NodeTraits<Derivate>::template createLink<Group>),
           py::arg("group"),// "target object",
           py::arg("linkName"),// "name for a new link",
           py::arg("linkType"),
           py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
           py::arg_v("linkAccesProps", LinkAccessProps(), "LinkAccessProps()"),
           py::arg_v("groupAccessProps", GroupAccessProps(), "GroupAccessProps()"))
      .def("createLink", py::overload_cast<
           const DataSet&,
           const std::string&,
           const LinkType&,
           const LinkCreateProps&,
           const LinkAccessProps&,
           const DataSetAccessProps&>(&NodeTraits<Derivate>::template createLink<void>),
           py::arg("dset"),// "target object",
           py::arg("linkName"),// "name for a new link",
           py::arg("linkType"),
           py::arg_v("linkCreateProps", LinkCreateProps(), "LinkCreateProps()"),
           py::arg_v("linkAccesProps", LinkAccessProps(), "LinkAccessProps()"),
           py::arg_v("dsetAccessProps", DataSetAccessProps(), "DataSetAccessProps()"));
}

py::class_<Selection,
SliceTraits<Selection> >
Selection_decl(py::module &m){
  return py::class_<Selection,
      SliceTraits<Selection> >(m, "Selection");
}

void Selection_def(py::class_<Selection,
                   SliceTraits<Selection> >& py_obj){

}

py::class_<File, Object,
NodeTraits<File> >
File_decl(py::module &m){
  return py::class_<File, Object,
      NodeTraits<File> >(m, "File");
}

void File_def(py::class_<File, Object,
              NodeTraits<File> >& py_obj){
  py_obj.def(py::init<const std::string&, unsigned>());
}

py::class_<Group, Object,
NodeTraits<Group> >
Group_decl(py::module &m){
  return py::class_<Group, Object,
      NodeTraits<Group> >(m, "Group");
}

void Group_def(py::class_<Group, Object,
               NodeTraits<Group> >& py_obj){

}

py::class_<DataSet, Object,
SliceTraits<DataSet> >
DataSet_decl(py::module &m){
  return py::class_<DataSet, Object,
      SliceTraits<DataSet> >(m, "DataSet");
}

void DataSet_def(py::class_<DataSet, Object,
                 SliceTraits<DataSet> >& py_obj){

}

PYBIND11_MODULE(HighFivePY, m) {
  m.doc() = moduleInfo();

  py::bind_vector<std::vector<int> >(m, "VectorInt");
  m.def("add_any", [](py::EigenDRef<Eigen::MatrixXd> x, int r, int c, double v) { x(r,c) += v; });

  // DECLARATION
  auto pyOjType = ObjectType_enum(m);
  auto pyLinkType = LinkType_enum(m);
  auto pyPropertyType = PropertyType_enum(m);
  auto pyOpenFlag = OpenFlag_enum(m);
  auto pyDTypeClass = DataTypeClass_enum(m);

  PropertyList_decl<PropertyType::LINK_CREATE>(m, "_LinkCreateProps");
  PropertyList_decl<PropertyType::LINK_ACCESS>(m, "_LinkAccessProps");
  PropertyList_decl<PropertyType::GROUP_CREATE>(m, "_GroupCreateProps");
  PropertyList_decl<PropertyType::GROUP_ACCESS>(m, "_GroupAccessProps");
  PropertyList_decl<PropertyType::DATASET_CREATE>(m, "_DataSetCreateProps");
  PropertyList_decl<PropertyType::DATASET_ACCESS>(m, "_DataSetAccessProps");
  PropertyList_decl<PropertyType::DATATYPE_CREATE>(m, "_DataTypeCreateProps");
  PropertyList_decl<PropertyType::DATATYPE_ACCESS>(m, "_DataTypeAccessProps");

  auto pyLinkCreateProps = LinkCreateProps_decl(m);
  auto pyLinkAccessProps = LinkAccessProps_decl(m);
  auto pyGroupCreateProps = GroupCreateProps_decl(m);
  auto pyGroupAccessProps = GroupAccessProps_decl(m);
  auto pyDsetCreateProps = DataSetCreateProps_decl(m);
  auto pyDsetAccessProps = DataSetAccessProps_decl(m);
  auto pyDTypeCreateProps = DataTypeCreateProps_decl(m);
  auto pyDTypeAccessProps = DataTypeAccessProps_decl(m);

  auto pyElementSet = ElementSet_decl(m);
  auto pyObjectInfo = ObjectInfo_decl(m);
  auto pyLinkInfo = LinkInfo_decl(m);
  auto pyObject = Object_decl(m);
  auto pyDSpace = DataSpace_decl(m);
  auto pyDType = DataType_decl(m);

  /* atomic declarations should be invoked after `DataType_decl` as
   * atomic types are children of DataType */
  auto pyAtomicChar = AtomicType_decl<char>(m, "AtomicChar");
  auto pyAtomicSChar = AtomicType_decl<signed char>(m, "AtomicSChar");
  auto pyAtomicUChar = AtomicType_decl<unsigned char>(m, "AtomicUChar");
  auto pyAtomicShort = AtomicType_decl<short>(m, "AtomicShort");
  auto pyAtomicUShort = AtomicType_decl<unsigned short>(m, "AtomicUShort");
  auto pyAtomicInt = AtomicType_decl<int>(m, "AtomicInt");
  auto pyAtomicUInt = AtomicType_decl<unsigned>(m, "AtomicUInt");
  auto pyAtomicLong = AtomicType_decl<long>(m, "AtomicLong");
  auto pyAtomicULong = AtomicType_decl<unsigned long>(m, "AtomicULong");
  auto pyAtomicLLong = AtomicType_decl<long long>(m, "AtomicLLong");
  auto pyAtomicULLong = AtomicType_decl<unsigned long long>(m, "AtomicULLong");
  auto pyAtomicFloat = AtomicType_decl<float>(m, "AtomicFloat");
  auto pyAtomicDouble = AtomicType_decl<double>(m, "AtomicDouble");
  auto pyAtomicBool = AtomicType_decl<bool>(m, "AtomicBool");
  auto pyAtomicString = AtomicType_decl<std::string>(m, "AtomicString");
  auto pyAtomicComplex = AtomicType_decl<std::complex<double> >(m, "AtomicComplex");

  auto pySliceTraits_Selection = SliceTraits_decl<Selection>(m, "_SelectionSlice");
  auto pySliceTraits_Dset = SliceTraits_decl<DataSet>(m, "_DsetSlice");
  auto pyNodeTraits_File = NodeTraits_decl<File>(m, "_FileNode"); // _Name <- underlined because user don't need to use it
  auto pyNodeTraits_Group = NodeTraits_decl<Group>(m, "_GroupNode");

  auto pySelection = Selection_decl(m);
  auto pyFile = File_decl(m);
  auto pyGroup = Group_decl(m);
  auto pyDset = DataSet_decl(m);

  // DEFINITION
  LinkCreateProps_def(pyLinkCreateProps);
  LinkAccessProps_def(pyLinkAccessProps);
  GroupCreateProps_def(pyGroupCreateProps);
  GroupAccessProps_def(pyGroupAccessProps);
  DataSetCreateProps_def(pyDsetCreateProps);
  DataSetAccessProps_def(pyDsetAccessProps);
  DataTypeCreateProps_def(pyDTypeCreateProps);
  DataTypeAccessProps_def(pyDTypeAccessProps);

  ElementSet_def(pyElementSet);
  ObjectInfo_def(pyObjectInfo);
  LinkInfo_def(pyLinkInfo);
  Object_def(pyObject);
  DataSpace_def(pyDSpace);
  DataType_def(pyDType);

  AtomicType_def(pyAtomicChar);
  AtomicType_def(pyAtomicSChar);
  AtomicType_def(pyAtomicUChar);
  AtomicType_def(pyAtomicShort);
  AtomicType_def(pyAtomicUShort);
  AtomicType_def(pyAtomicInt);
  AtomicType_def(pyAtomicUInt);
  AtomicType_def(pyAtomicLong);
  AtomicType_def(pyAtomicULong);
  AtomicType_def(pyAtomicLLong);
  AtomicType_def(pyAtomicULLong);
  AtomicType_def(pyAtomicFloat);
  AtomicType_def(pyAtomicDouble);
  AtomicType_def(pyAtomicBool);
  AtomicType_def(pyAtomicString);
  AtomicType_def(pyAtomicComplex);

  SliceTraits_def(pySliceTraits_Selection);
  SliceTraits_def(pySliceTraits_Dset);
  NodeTraits_def(pyNodeTraits_File);
  NodeTraits_def(pyNodeTraits_Group);

  Selection_def(pySelection);
  File_def(pyFile);
  Group_def(pyGroup);
  DataSet_def(pyDset);
}

}
