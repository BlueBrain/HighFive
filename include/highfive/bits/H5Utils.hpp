#ifndef H5UTILS_HPP
#define H5UTILS_HPP

// internal utilities functions
#include <vector>

namespace HighFive{

namespace details{


// determine at compile time number of dimensions of in memory datasets
template<typename T>
struct array_dims { static const size_t value = 0; };

template<typename T>
struct array_dims<std::vector<T> > { static const size_t value = 1 + array_dims<T>::value; };

template<typename T>
struct array_dims<T*> { static const size_t value = 1 + array_dims<T>::value; };

template<typename T, std::size_t N>
struct array_dims<T [N]> { static const size_t value = 1 + array_dims<T>::value;  };

// determine recursively the size of each dimension of a N dimension vector
template<typename T>
void get_dim_vector_rec(const T & vec, std::vector<size_t> & dims){
    (void) dims;
    (void) vec;
}

template<typename T>
void get_dim_vector_rec(const std::vector<T> & vec, std::vector<size_t> & dims){
    dims.push_back(vec.size());
    get_dim_vector_rec(vec[0], dims);
}

template<typename T>
std::vector<size_t> get_dim_vector(const T & vec){
    std::vector<size_t> dims;
    get_dim_vector_rec(vec, dims);
    return dims;
}



// determine at compile time recursively the basic type of the data
template<typename T>
struct type_of_array { typedef T type; };

template<typename T>
struct type_of_array<std::vector<T> > { typedef typename type_of_array<T>::type type; };

template<typename T>
struct type_of_array<T*> { typedef typename type_of_array<T>::type type; };

template<typename T, std::size_t N>
struct type_of_array<T [N]> { typedef typename type_of_array<T>::type type; };


// same type compile time check
template<typename T, typename U>
struct is_same{
    static const bool value = false;
};

template<typename T>
struct is_same<T, T>{
    static const bool value = true;
};

// check if the type is a container ( only vector supported for now )
template<typename >
struct is_container{
    static const bool value = false;
};

template<typename T>
struct is_container<std::vector<T> >{
    static const bool value = true;
};

// enable if implem for not c++11 compiler
template <bool Cond, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> { typedef T type; };


}


}

#endif // H5UTILS_HPP
