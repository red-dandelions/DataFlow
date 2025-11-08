import gzip

# 以文本模式读取 gzip 文件（自动解压缩）
with gzip.open('./text_sample.gz', 'rt', encoding='utf-8') as f:
    # 逐行读取解压缩后的内容并打印
    for line_num, line in enumerate(f, 1):
        # 去除末尾换行符（可选），并打印行号和内容
        print(f"line: {line.rstrip()}")