# Migration Guide
A collection of tips for migrating away from deprecated features.

## Deprecation of `FixedLenStringArray`.
The issue with `FixedLenStringArray` is that it is unable to avoid copies.
Essentially, this class acts as a means to create a copy of the data in a
format suitable for writing fixed-length strings. Additionally, the class acts
as a tag for HighFive to overload on. The support of `std::string` in HighFive
has improved considerable. Since 2.8.0 we can write/read `std::string` to fixed
or variable length HDF5 strings.

Therefore, this class serves no purpose anymore. Any occurrence of it can be
replaced with an `std::vector<std::string>` (for example).

If desired one can silence warnings by replacing `FixedLenStringArray` with
`deprecated::FixedLenStringArray`.

## Deprecation of `read(T*, ...)`.
A "raw read" is when the user allocates sufficient bytes and provides HighFive
with the pointer to the first byte. "Regular reads" take a detour via the
inspector and might resize the container, etc.

The issue is that HighFive `v2` had the following two overloads:
```
template<class T>
DataSet::read(T& x, /* skipped */);

template<class T>
DataSet::read(T* x, /* skipped */);
```
and the analogous for `Attribute`.

The issue is that the second overload will also match things like `T**` and
`T[][]`. For example the following code used the removed overload:
```
double x[2][3];
dset.read(x);
```
which is fine because is a contiguous sequence of doubles. It's equivalent to
following `v3` code:
```
double x[2][3];
dset.read_raw((double*) x);
```

### Accidental Raw Read
We consider the example above to be accidentally using a raw read, when it
could be performing a regular read. We suggest to not change the above, i.e.
```
double x[2][3];
dset.read(x);
```
continues to be correct in `v3` and can check that the dimensions match. The
inspector recognizes `double[2][3]` as a contiguous array of doubles.
Therefore, it'll use the shallow-copy buffer and avoid the any additional
allocations or copies.

### Intentional Raw Read
When genuinely performing a "raw read", one must replace `read` with
`read_raw`. For example:

```
double x = malloc(2*3 * sizeof(double));
dset.read_raw(x);
```
is correct in `v3`.
