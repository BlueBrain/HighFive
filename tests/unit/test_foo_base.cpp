int main() {
    std::vector<std::vector<std::vector>> data;
    std::vector label;
    std::string filename = "/path/to/data/h5_data/110020171219_h5/0.h5";
    File file(filename, File::ReadOnly);
    DataSet dataset1 = file.getDataSet("data");
    dataset1.read(data);
    DataSet dataset2 = file.getDataSet("label");
    dataset2.read(label);
    std::cout << label.size() << std::endl;
}

