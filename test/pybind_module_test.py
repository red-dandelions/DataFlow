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
print(df_module.DataReader.Source)

class TestModule(unittest.TestCase):
    def test_DataReader(self):
        file_list = ["/home/ubuntu/code/DataFlow/test/utils/text_sample.gz"]

        d = DataFlow.DataReader(file_list, source=df_module.DataReader.Source.kFileList)
        d = DataFlow.DataDecompressor(d)
        d = DataFlow.DataTextParser(d, format="sample_id|group_id|dense|sparse|label|timestamp")
        cnt = 0
        for i in d:
            print("s: ", i)
            cnt += 1
            #if cnt >= 5:
            #    break
        print(d)


if __name__ == "__main__":
    unittest.main()
