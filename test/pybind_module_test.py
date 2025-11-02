import unittest

import DataFlow
import DataFlow.csrc
import DataFlow.csrc.pybind_module as df_module

print(dir(DataFlow))
print(dir(DataFlow.csrc))
print(dir(df_module))

print(df_module.ByteStreamMeta)
print(df_module.ByteStream)
print(df_module.DataReader)
print(df_module.DataReader.FileSource)

class TestModule(unittest.TestCase):
    def test_DataReader(self):
        file_list = ["/data/develop/DataFlow/test/utils/text_sample.gz"]

        d = df_module.DataReader(file_list, file_source=df_module.DataReader.FileSource.kFileList)
        d = df_module.DataDecompressor(d)
        cnt = 0
        for i in d:
            print("s: ", i)
            cnt += 1
            if cnt >= 5:
                break
        print(d)


if __name__ == "__main__":
    unittest.main()
