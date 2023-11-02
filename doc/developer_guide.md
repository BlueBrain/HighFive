# Developer Guide
First clone the repository and remember the `--recursive`:
```bash
git clone --recursive git@github.com:BlueBrain/HighFive.git
```
The instructions to recover if you forgot are:
```bash
git submodule update --init --recursive
```

One remark on submodules: each HighFive commit expects that the submodules are
at a particular commit. The catch is that performing `git checkout` will not
update the submodules automatically. Hence, sometimes a `git submodule update
--recursive` might be needed to checkout the expected version of the
submodules.

## Compiling and Running the Tests
The instructions for compiling with examples and unit-tests are:

```bash
cmake -B build -DCMAKE_BUILD_TYPE={Debug,Release} .
cmake --build build --parallel
ctest --test-dir build
```

You might want to turn off Boost `-DHIGHFIVE_USE_BOOST=Off` or turn on other
optional dependencies.

## Contributing
There's numerous HDF5 features that haven't been wrapped yet. HighFive is a
collaborative effort to slowly cover ever larger parts of the HDF5 library.
The process of contributing is to fork the repository and then create a PR.
Please ensure that any new API is appropriately documented and covered with
tests.

### Code formatting
The project is formatted using clang-format version 12.0.1 and CI will complain
if a commit isn't formatted accordingly. The `.clang-format` is at the root of
the git repository. Conveniently, `clang-format` is available via `pip`:

```bash
python -m venv venv
source venv/bin/activate

pip install clang-format==12.0.1
```

The changed lines can be formatted with `git-clang-format`, e.g. to format all lines changed compared to master:

```bash
git-clang-format master
```
(add `-f` to allow formatting unstaged changes if you trust it to not destroy
your changes.)

## Releasing HighFive
Before releasing a new version perform the following:

* Update `CHANGELOG.md` and `AUTHORS.txt` as required.
* Update `CMakeLists.txt` and `include/highfive/H5Version.hpp`.
* Follow semantic versioning when deciding the next version number.
* Check that
  [HighFive-testing](https://github.com/BlueBrain/HighFive-testing/actions) ran
  recently.

At this point there should be a commit on master which will be the release
candidate. Don't tag it yet.

Next step is to update the [HighFive/spack](https://github.com/BlueBrain/spack)
recipe such that the proposed version points to the release candidate using the
SHA of that commit. The recipe will look something like this:

```python
    # ...

    version("2.8.0", commit="094400f22145bcdcd2726ce72888d9d1c21e7068")
    version("2.7.1", sha256="25b4c51a94d1e670dc93b9b73f51e79b65d8ff49bcd6e5d5582d5ecd2789a249")
    version("2.7.0", sha256="8e05672ddf81a59ce014b1d065bd9a8c5034dbd91a5c2578e805ef880afa5907")
    # ...
```

Push the changes to the BlueBrain spack repository. This will trigger building
all BBP dependencies of HighFive, i.e. another integration test. Don't actually
merge this commit yet.

Now that we know that the integration test ran, and all BBP software can be
built with the proposed version of HighFive, we can proceed and create the
release. Once this is done perform a final round of updates:

* Download the archive (`*.tar.gz`) and compute its SHA256.
* Update BlueBrain Spack recipe to use the archive and not the Git commit.
* Update the upstream Spack recipe.

