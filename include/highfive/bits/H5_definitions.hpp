#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define H5_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define H5_DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: Compiler doesnt support deprecation")
#define H5_DEPRECATED
#endif


// Fw declarations

namespace HighFive {

enum class ObjectType;
enum class PropertyType;

class Attribute;
class DataSet;
class DataSpace;
class DataType;
class Exception;
class File;
class FileDriver;
class Object;
class ObjectInfo;
class Reference;
class Selection;
class SilenceHDF5;

template <typename T>
class AtomicType;

template <typename Derivate>
class AnnotateTraits;

template <typename Derivate>
class NodeTraits;

template <PropertyType T>
class PropertyList;

}  // namespace HighFive

