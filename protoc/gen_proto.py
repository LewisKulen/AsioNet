import subprocess
import json

# 读取JSON文件
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
        return data
    
class ProtoTask:
    def __init__(self,root,cfg):
        self.out =  cfg.get("out")
        self.input_path = root + "/" + cfg.get("input")
        self.output_path = root + "/" + cfg.get("output")
        self.files = cfg.get("files")
        

# protoc --proto_path=src --cpp_out=build/gen src/foo.proto src/bar/baz.proto
        
class ProtoGenerator:
    def __init__(self,cfg):
        self.path = cfg.get("root") + "/" + cfg.get("generator").get("path")
        self.name = cfg.get("generator").get("name")

    
def prase_cfg(cfg):
    tasks = cfg.get("tasks")
    pbTasks = []
    root = cfg.get("root")
    for i in range(len(tasks)):
        pbTasks.append(ProtoTask(root,tasks[i]))
    return ProtoGenerator(cfg),pbTasks

def gen_code(gen,tasks):
    exe = gen.path + "/" + gen.name
    for i in range(len(tasks)):
        task = tasks[i]
        files = task.files
        if len(files) > 0:
            input = "--proto_path=" + task.input_path
            output = task.out + "=" + task.output_path
            cmd = [exe,input,output]
            cmd.extend(files)
            subprocess.call(cmd)

if __name__ == "__main__":
    cfg = read_json_file("./gen_config.json")
    gen,tasks = prase_cfg(cfg)
    gen_code(gen,tasks)


