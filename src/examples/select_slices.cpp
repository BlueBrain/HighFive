#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <highfive/highfive.hpp>

void print_mask(const std::vector<std::vector<double>>& values,
                const std::vector<std::vector<double>>& selected);

void print_values(const std::vector<std::vector<double>>& values);

void pretty_print(const std::vector<std::vector<double>>& values,
                  const std::vector<std::vector<double>>& selected);

int main(void) {
    using namespace HighFive;
    using container_type = std::vector<std::vector<double>>;

    const std::string file_name("select_slices.h5");
    const std::string dataset_name("dset");

    // Create a new file using the default property lists.
    File file(file_name, File::Truncate);

    // we have some example values in a 2D vector 2x5
    // clang-format off
    container_type values = {
      {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7},
      {2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7},
      {3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7},
      {4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7},
      {5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7},
      {6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7},
      {7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7},
      {8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7},
      {9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7}
    };
    // clang-format on

    auto dset = file.createDataSet(dataset_name, values);

    // Let's start with the selection `values[1:4, 2:4]`.
    {
        auto xslice = std::array<size_t, 2>{2, 4};
        auto yslice = std::array<size_t, 2>{1, 4};

        auto selected = dset.select(ProductSet(yslice, xslice)).read<container_type>();
        std::cout << " -- values[1:4, 2:4] ------------ \n";
        pretty_print(values, selected);
    }

    // Now we'd like the selection `values[[1,2,8], 2:4]`.
    {
        auto xslice = std::array<size_t, 2>{2, 4};
        auto yslice = std::vector<size_t>{1, 2, 8};

        auto selected = dset.select(ProductSet(yslice, xslice)).read<container_type>();
        std::cout << "\n -- values[[1,2,8], 2:4] -------- \n";
        pretty_print(values, selected);
    }

    // ... or the union of multiple slices.
    {
        auto xslice = std::array<size_t, 2>{2, 4};
        auto yslice = std::vector<std::array<size_t, 2>>{{0, 2}, {5, 9}};

        auto selected = dset.select(ProductSet(yslice, xslice)).read<container_type>();
        std::cout << "\n -- values[[0:2, 5:10], 2:4] -------- \n";
        pretty_print(values, selected);
    }

    // Similar for the union of multiple slices in both x- and y-direction.
    {
        auto xslice = std::vector<std::array<size_t, 2>>{{0, 1}, {2, 4}, {6, 7}};
        auto yslice = std::vector<std::array<size_t, 2>>{{0, 2}, {5, 9}};

        auto selected = dset.select(ProductSet(yslice, xslice)).read<container_type>();
        std::cout << "\n -- values[[0:2, 5:10], [0:1, 2:4, 6:7]] -------- \n";
        pretty_print(values, selected);
    }

    // If only a single row/column is need use the following. However,
    // selecting elements one-by-one in a loop can be a serious performance
    // issue.
    {
        auto xslice = std::vector<std::array<size_t, 2>>{{0, 1}, {2, 4}, {6, 7}};
        auto row_id = 3;

        auto selected = dset.select(ProductSet(row_id, xslice)).read<container_type>();
        std::cout << "\n -- values[3, [0:1, 2:4, 6:7]] -------- \n";
        pretty_print(values, selected);
    }

    return 0;
}

void print_mask(const std::vector<std::vector<double>>& values,
                const std::vector<std::vector<double>>& selected) {
    std::vector<double> flat_selection;
    for (const auto& row: selected) {
        for (const auto& x: row) {
            flat_selection.push_back(x);
        }
    }

    for (const auto& row: values) {
        for (const auto& x: row) {
            auto found = std::find(flat_selection.begin(), flat_selection.end(), x) !=
                         flat_selection.end();
            std::cout << (found ? "x" : ".") << "  ";
        }
        std::cout << "\n";
    }
}

void print_values(const std::vector<std::vector<double>>& values) {
    for (const auto& row: values) {
        for (const auto& x: row) {
            std::cout << x << "  ";
        }
        std::cout << "\n";
    }
}

void pretty_print(const std::vector<std::vector<double>>& values,
                  const std::vector<std::vector<double>>& selected) {
    print_mask(values, selected);
    std::cout << "\n";
    print_values(selected);
}
