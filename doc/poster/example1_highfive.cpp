#include <HighFive>

using namespace HighFive;

void write_data() {
  std::vector<int> d1(50, 1);

  File file("tmp.h5", File::ReadWrite | File::Truncate);

  // Create DataSet and write data (short form)
  file.createDataSet("/group/dset1", d1);

  // Reading
  std::vector<int> d1_read;
  file.getDataSet("/group/dset1").read(d1_read);
}
