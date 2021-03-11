#include <H5Ipublic.h>

#include "H5Utils.hpp"
#include "H5Path_traits.hpp"

namespace HighFive{

template <typename Derivate>
inline PathTraits<Derivate>::PathTraits() {
    const hid_t hid = static_cast<Derivate*>(this)->getId();
    hid_t file_id = H5Iget_file_id(hid);
    if (file_id > 0 and file_id != hid) {
        _file_obj.reset(new File(file_id));
    }
}

template <typename Derivate>
inline std::string PathTraits<Derivate>::getPath() const {
    return details::get_name([this](char* buffer, hsize_t length) {
        return H5Iget_name(static_cast<const Derivate*>(this)->getId(), buffer, length);
    });
}

template <typename Derivate>
inline File& PathTraits<Derivate>::getFile() const noexcept {
    return *_file_obj;
}

}  // namespace HighFive