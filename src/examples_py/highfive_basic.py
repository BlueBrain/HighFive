import h5py
import HighFivePY as hf
import numpy as np

a = np.arange(15, dtype=np.int).reshape(3, 5)
# a = np.arange(15)
# b = np.arange(15).reshape(15, 1, order='F')
# np.vectorize(b)

#print(help(hf))
hf_file = hf.File("myFile.h5", hf.OpenFlag.Overwrite)
dset = hf_file.createDataSet("/path/to/data", hf.DataSpace(np.shape(a)), hf.AtomicInt())
dset.write(a)
b = dset.read()
# dset.read(b)
c = hf.VectorInt([1, 2, 3])
dset.foo(c)

d = np.array([[1.0, 2.0], [3.0, 4.0]], order='F')

orig = np.array([[1.0, 2, 3], [4, 5, 6], [7, 8, 9]])
zr = np.array(orig)
zc = np.array(orig, order="F")

hf.add_any(zr, 1, 0, 20)
assert np.all(zr == np.array([[1.0, 2, 3], [124, 5, 6], [7, 8, 9]]))
hf.add_any(zc, 1, 0, 10)
assert np.all(zc == np.array([[1.0, 2, 3], [214, 5, 6], [7, 8, 9]]))

hf_file_id = hf_file.getId(False)
hf_group = hf_file.createGroup("myGroup")
#hf_group = hf_file.createGroup("myGroup", hf.LinkCreateProps(), hf.GroupCreateProps(), hf.GroupAccessProps())
hf_group_id = hf_group.getId(False)  # 144115188075855872

h5py_file = h5py.File('myFile_py.h5','w')
h5py_file_id = h5py_file._id
h5py_group = h5py_file.create_group("myGroup")
h5py_group_id = h5py_group._id  # 144115188075855873

hf_group_id_h5py = h5py.h5g.GroupID(hf_group_id)

h5py_group = h5py.Group(hf_group_id_h5py)  # Exception has occurred: ValueError. 144115188075855872 is not a GroupID
h5py_group.create_group("wow")
