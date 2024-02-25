import sys
import json


# 读取JSON文件
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
        return data


def gen_code(task):
    return

if __name__ == "__main__":
    cfg = read_json_file("./gen_config")
    
    tasks = cfg.get("tasks")
    for t in range(tasks):
        gen_task(t)

