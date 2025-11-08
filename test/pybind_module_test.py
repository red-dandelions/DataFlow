import unittest

import DataFlow
import DataFlow.csrc
import DataFlow.csrc.pybind_module as df_module
import numpy as np

print(dir(DataFlow))
print(dir(DataFlow.csrc))
print(dir(df_module))

print(df_module.ByteStreamMeta)
print(df_module.ByteStream)
print(df_module.DataReader)
print(df_module.DataReader.Source)

class TestModule(unittest.TestCase):
    def test_DataReader(self):
        file_list = ["/home/ubuntu/code/DataFlow/test/utils/text_sample.gz"]

        """
        67@0.121579,0.662585,0.059782,0.950017,0.615238,0.117334,0.195037,0.150216,0.943708,0.391018,0.445609;
        56@0.262912,0.749598,0.467136,0.925941,0.94136,0.628454,0.27895,0.524211,0.556681,0.869557,0.693013,0.243626;
        73@0.05898,0.264405;
        38@0.795042,0.099788,0.501431,0.944934,0.024748,0.914699,0.026745,0.284343,0.625706,0.280179;
        1@

        {name:67,dtype:11,shape:[11],column_type:Dense}, data: 0.121579 0.662585 0.059782 0.950017 0.615238 0.117334 0.195037 0.150216 0.943708 0.391018 0.445609
        {name:56,dtype:11,shape:[12],column_type:Dense}, data: 0.262912 0.749598 0.467136 0.925941 0.94136 0.628454 0.27895 0.524211 0.556681 0.869557 0.693013 0.243626
        {name:73,dtype:11,shape:[2],column_type:Dense}, data: 0.05898 0.264405
        {name:38,dtype:11,shape:[9],column_type:Dense}, data: 0.795042 0.099788 0.501431 0.944934 0.024748 0.914699 0.026745 0.284343 0.625706
        {name:1,dtype:11,shape:[1],column_type:Dense}, data: 0
        """

        columns = [
            DataFlow.DenseColumn("67", dtype=np.float32, shape=(11,)),
            DataFlow.DenseColumn("56", dtype=np.float32, shape=(12,)),
            DataFlow.DenseColumn("73", dtype=np.float32, shape=(2,)),
            DataFlow.DenseColumn("38", dtype=np.float32, shape=(9,)),
            DataFlow.DenseColumn("1", dtype=np.float32, shape=(1,)),
        ]
        d = DataFlow.DataReader(file_list, source=df_module.DataReader.Source.kFileList)
        d = DataFlow.DataDecompressor(d)
        d = DataFlow.DataTextParser(d, format="sample_id|group_id|sparse|dense|label|timestamp", columns=columns)
        cnt = 0
        for i in d:
            print("s: ", i)
            cnt += 1
            #if cnt >= 5:
            #    break
        print(d)


if __name__ == "__main__":
    unittest.main()
