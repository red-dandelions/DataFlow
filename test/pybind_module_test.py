import unittest
import gc
import sys

import DataFlow
import DataFlow.csrc
import DataFlow.csrc.pybind_module as df_module

class TestModule(unittest.TestCase):
    def test_DataReader(self):
        def test_fuc():
            file_list = ["/home/ubuntu/DataFlow/test/utils/text_sample.gz"]

            d = df_module.DataReader(file_list, source=df_module.DataReader.Source.kFileList)
            d = df_module.DataDecompressor(d)
            d = df_module.DataTextParser(d, format="sample|group_id|dense|sparse|label|tmiestamp")
            cnt = 0
            for i in d:
                print("s: ", i)
                cnt += 1
                if cnt >= 5:
                    break
            print(d)
            print(sys.getrefcount(d))
            del d
            gc.collect()

        test_fuc()

if __name__ == "__main__":
    unittest.main()
