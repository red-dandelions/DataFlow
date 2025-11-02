#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
@file: prepare_text_sample_tool.py

@author: Jasmine (1011694931@qq.com)
@time: 2025-11-01
@description: 生成文本格式样本的工具

Copyright (c) 2024 Jasmine. All rights reserved.
"""

import random
import gzip
import time
from typing import List, Tuple


sample_num = 1000
dense_slots_num = 30
sparse_slots_num = 20
max_dense_size = 13
output_file = 'text_sample.gz'

dense_slots = {random.randint(1, 100): (random.randint(0, max_dense_size),) for _ in range(dense_slots_num)}
sparse_slots = [random.randint(1000, 2000) for _ in range(sparse_slots_num)]

# -------------------
# 样本生成
# -------------------
def generate_dense_values(shape: Tuple[int]) -> List[float]:
    size = shape[0]
    return [round(random.uniform(0,1), 6) for _ in range(size)]

def generate_sparse_values() -> str:
    id = (hash(str(random.randint(1, 10000))) & 0xFFFFFFFF)
    weight = round(random.uniform(0,1), 6)
    return f"{id}:{weight}"


def generate_text_sample():
    generated_info = []
    with gzip.open(output_file, 'wt') as f:
        for sample_id in range(sample_num):
            group_id = random.randint(1, 1000)
            
            sparse_fileds = []
            sparse_info = {}
            for sparse_slot in sparse_slots:
                sparse_value = generate_sparse_values()
                sparse_fileds.append(f"{sparse_slot}@{sparse_value}")
                sparse_info[sparse_slot] = sparse_value

            dense_fileds = []
            dense_info = {}
            for slot, shape in dense_slots.items():
                dense_values = generate_dense_values(shape)
                dense_value_str = ','.join(map(str, dense_values))
                dense_fileds.append(f"{slot}@{dense_value_str}")
                dense_info[slot] = dense_values

            label = random.randint(0, 1)
            timestamp = int(time.time())

            line = "|".join([
                str(sample_id),
                str(group_id),
                ";".join(sparse_fileds),
                ";".join(dense_fileds),
                str(label),
                str(timestamp)
            ]) + "\n"
            f.write(line)
            generated_info.append({
                "sample_id": sample_id,
                "group_id": group_id,
                "sparse_info": sparse_info,
                "dense_info": dense_info,
                "label": label,
                "timestamp": timestamp
            })
    return generated_info


if __name__ == "__main__":
    infos = generate_text_sample()
    #for info in infos[:2]:
    #    print(info)