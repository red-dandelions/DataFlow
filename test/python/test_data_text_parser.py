import unittest

from DataFlow import (
    DenseColumn,
    SparseColumn,
    DataReader,
    DataDecompressor,
    DataTextParser,
)
import DataFlow
import numpy as np

class TestModule(unittest.TestCase):
    def test_DataTextParser(self):
        file_list = ["/home/ubuntu/code/DataFlow/test/utils/text_sample_v2.gz"]
        # 生成文件时，从打印日志里复制过来
        columns = [
            DenseColumn(name="61", dtype=np.float32, shape=(11,)),
            DenseColumn(name="29", dtype=np.float32, shape=(6,)),
            DenseColumn(name="78", dtype=np.float32, shape=(1,)),
            DenseColumn(name="89", dtype=np.float32, shape=(12,)),
            DenseColumn(name="32", dtype=np.float32, shape=(13,)),
            DenseColumn(name="15", dtype=np.float32, shape=(10,)),
            DenseColumn(name="12", dtype=np.float32, shape=(2,)),
            DenseColumn(name="8", dtype=np.float32, shape=(1,)),
            DenseColumn(name="21", dtype=np.float32, shape=(13,)),
            DenseColumn(name="31", dtype=np.float32, shape=(12,)),
            DenseColumn(name="41", dtype=np.float32, shape=(1,)),
            DenseColumn(name="6", dtype=np.float32, shape=(3,)),
            DenseColumn(name="88", dtype=np.float32, shape=(10,)),
            DenseColumn(name="90", dtype=np.float32, shape=(11,)),
            DenseColumn(name="56", dtype=np.float32, shape=(9,)),
            DenseColumn(name="76", dtype=np.float32, shape=(4,)),
            DenseColumn(name="39", dtype=np.float32, shape=(1,)),
            DenseColumn(name="48", dtype=np.float32, shape=(6,)),
            DenseColumn(name="84", dtype=np.float32, shape=(8,)),
            DenseColumn(name="55", dtype=np.float32, shape=(1,)),
            DenseColumn(name="98", dtype=np.float32, shape=(11,)),
            DenseColumn(name="83", dtype=np.float32, shape=(10,)),
            DenseColumn(name="97", dtype=np.float32, shape=(1,)),
            DenseColumn(name="22", dtype=np.float32, shape=(10,)),
            DenseColumn(name="82", dtype=np.float32, shape=(3,)),
            DenseColumn(name="54", dtype=np.float32, shape=(12,)),
        ] + [
            SparseColumn(name="1399", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1041", dtype=np.int64, shape=(1,)),
            SparseColumn(name="1357", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1094", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1647", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1903", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1922", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1790", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1273", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1775", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1333", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1361", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1421", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1092", dtype=np.int64, shape=(2,)),
            SparseColumn(name="1591", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1726", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1985", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1111", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1263", dtype=np.int64, shape=(-1,)),
            SparseColumn(name="1457", dtype=np.int64, shape=(-1,)),
        ]
        d = DataReader(file_list, source=DataReader.Source.kFileList)
        d = DataDecompressor(d)
        d = DataTextParser(d, format="sample_id|group_id|sparse|dense|label|timestamp", columns=columns, external_data=["sample_id", "timestamp"])
        cnt = 0
        for i in d:
            print(f"batch_row {cnt}: ", i)
            cnt += 1
            if cnt >= 5:
                break
        print(d)


if __name__ == "__main__":
    #input()
    unittest.main()
