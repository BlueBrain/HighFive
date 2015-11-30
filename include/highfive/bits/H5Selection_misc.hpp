#ifndef H5SELECTION_MISC_HPP
#define H5SELECTION_MISC_HPP

#include "../H5Selection.hpp"

namespace HighFive{


Selection::Selection(const DataSpace & memspace, const DataSpace & file_space, DataSet & set) :
    _mem_space(memspace),
    _file_space(file_space),
    _set(set){

}


DataSpace Selection::getSpace() const{
    return _file_space;
}

DataSpace Selection::getMemSpace() const{
    return _mem_space;
}

DataSet & Selection::getDataset(){
    return _set;
}

}


#endif // H5SELECTION_MISC_HPP
