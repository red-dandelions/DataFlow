import unittest
import numpy as np
import DataFlow


class TestModule(unittest.TestCase):
    def test_Columns(self):
        dense_col = DataFlow.DenseColumn(name="dense_feature", shape=(10,), dtype=np.float32)
        sparse_col = DataFlow.SparseColumn(name="sparse_feature", shape=(100,), dtype=np.int64)

        self.assertEqual(dense_col.name, "dense_feature")
        self.assertEqual(dense_col.shape, (10,))
        #self.assertEqual(dense_col.dtype, np.float32)
        print("dense_col:", dense_col)

        self.assertEqual(sparse_col.name, "sparse_feature")
        self.assertEqual(sparse_col.shape, (100,))
        #self.assertEqual(sparse_col.dtype, np.int64)
        print("sparse_col:", sparse_col)

        columns = [dense_col, sparse_col]
        batch_row_meta = DataFlow.BatchRowMeta(columns)
        batch_row = DataFlow.BatchRow(batch_row_meta)
        print(batch_row_meta)
        #print("dense column from meta:", batch_row_meta.get_column_by_name("dense_feature"))
        #print("sparse column from meta:", batch_row_meta.get_column_by_index(1))



if __name__ == "__main__":
    unittest.main()
