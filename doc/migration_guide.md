# Migration Guide
A collection of tips for migrating away from deprecated features.

## Deprecation of `FixedLenStringArray`.
The issue with `FixedLenStringArray` is that it is unable to avoid copies.
Essentially, this class acts as a means to create a copy of the data in a
format suitable for writing fixed-length strings; and the class acts as a tag
for HighFive to overload on. The support of `std::string` in HighFive. Since
2.8.0 we can write/read `std::string` to fixed or variable length HDF5 strings.

Therefore, this class serves no purpose anymore. Any occurrence of it can be
replaced with an `std::vector<std::string>` (for example).

If desired one can silence warnings by replacing `FixedLenStringArray` with
`deprecated::FixedLenStringArray`.
