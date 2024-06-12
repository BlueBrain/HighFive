#pragma once

namespace HighFive {
namespace testing {

/// \brief Trait for `createAttribute`.
///
/// The point of these is to simplify testing. The typical issue is that we
/// need to write the tests twice, one with `createDataSet` and then again with
/// `createAttribute`. This trait allows us to inject this difference.
struct AttributeCreateTraits {
    using type = Attribute;

    template <class Hi5>
    static Attribute get(Hi5& hi5, const std::string& name) {
        return hi5.getAttribute(name);
    }


    template <class Hi5, class Container>
    static Attribute create(Hi5& hi5, const std::string& name, const Container& container) {
        return hi5.createAttribute(name, container);
    }

    template <class Hi5>
    static Attribute create(Hi5& hi5,
                            const std::string& name,
                            const DataSpace& dataspace,
                            const DataType& datatype) {
        return hi5.createAttribute(name, dataspace, datatype);
    }

    template <class T, class Hi5>
    static Attribute create(Hi5& hi5, const std::string& name, const DataSpace& dataspace) {
        auto datatype = create_datatype<T>();
        return hi5.template createAttribute<T>(name, dataspace);
    }
};

/// \brief Trait for `createDataSet`.
struct DataSetCreateTraits {
    using type = DataSet;

    template <class Hi5>
    static DataSet get(Hi5& hi5, const std::string& name) {
        return hi5.getDataSet(name);
    }

    template <class Hi5, class Container>
    static DataSet create(Hi5& hi5, const std::string& name, const Container& container) {
        return hi5.createDataSet(name, container);
    }

    template <class Hi5>
    static DataSet create(Hi5& hi5,
                          const std::string& name,
                          const DataSpace& dataspace,
                          const DataType& datatype) {
        return hi5.createDataSet(name, dataspace, datatype);
    }

    template <class T, class Hi5>
    static DataSet create(Hi5& hi5, const std::string& name, const DataSpace& dataspace) {
        auto datatype = create_datatype<T>();
        return hi5.template createDataSet<T>(name, dataspace);
    }
};

}  // namespace testing
}  // namespace HighFive
