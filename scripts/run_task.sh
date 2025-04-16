#!/bin/bash

# 解析 YAML 配置文件
CONFIG_FILE="config.yaml"
CONFIG=$(python3 parse_config.py $CONFIG_FILE)
root=../assets/snapshots
core=0x4
# 提取任务列表
TASKS=$(echo "$CONFIG" | grep -oP 'tasks: \K.*')
MEMORY=$(echo "$CONFIG" | grep -oP 'memory: \K.*')
REALM_MEMORY=$(echo "$MEMORY" | grep -oP 'realm_memory: \K\S+')

# 创建 Realm

echo "Creating Realm with memory $REALM_MEMORY..."
nice -n -20 taskset $core ./$root/lkvm run --realm --disable-sve -c 1 --memory $REALM_MEMORY \
-k $root/Image-cca -i $root/rootfs.realm.cpio --9p "/,host0" -p "fvp_escape_loop fvp_escape_off ip=off" --name my_realm


# 遍历每个任务
echo "$TASKS" | while read -r TASK; do
    # 提取任务参数
    TASK_ID=$(echo "$TASK" | grep -oP 'task_id: \K\d+')
    GPU_TASK_FILE=$(echo "$TASK" | grep -oP 'gpu_task_file: \K\S+')
    INPUT_DATA_FILE=$(echo "$TASK" | grep -oP 'input_data_file: \K\S+')
    OUTPUT_DATA_FILE=$(echo "$TASK" | grep -oP 'output_data_file: \K\S+')
    MODEL_WEIGHT=$(echo "$TASK" | grep -oP 'model_weight: \K\S+')

    echo "Processing Task $TASK_ID..."

    # 执行GPU任务（模拟）
    echo "Loading GPU task file ($GPU_TASK_FILE) to kernel..."
    cd /mnt/host/mnt/host/src/tasks
    $GPU_TASK_FILE input_data_file=$INPUT_DATA_FILE model_weight=$MODEL_WEIGHT output_data_file=$OUTPUT_DATA_FILE

    # 销毁 Realm
    

    echo "Task $TASK_ID completed!"
done


echo "Destroying Realm..."
lkvm stop my_realm
