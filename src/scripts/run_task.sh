#!/bin/bash

# 解析 YAML 配置文件
CONFIG_FILE="config.yaml"
CONFIG=$(python3 parse_config.py $CONFIG_FILE)

# 提取任务列表
TASKS=$(echo "$CONFIG" | grep -oP 'tasks: \K.*')

# 遍历每个任务
echo "$TASKS" | while read -r TASK; do
    # 提取任务参数
    TASK_ID=$(echo "$TASK" | grep -oP 'task_id: \K\d+')
    GPU_TASK_FILE=$(echo "$TASK" | grep -oP 'gpu_task_file: \K\S+')
    INPUT_DATA_FILE=$(echo "$TASK" | grep -oP 'input_data_file: \K\S+')
    REALM_MEMORY=$(echo "$TASK" | grep -oP 'realm_memory: \K\S+')

    echo "Processing Task $TASK_ID..."

    # 创建 Realm
    echo "Creating Realm with memory $REALM_MEMORY..."
    lkvm run --memory $REALM_MEMORY --name my_realm_$TASK_ID &

    # 等待 Realm 启动
    sleep 5

    # 复制输入数据和模型参数到 Realm 内存
    echo "Copying input data and model parameters to Realm..."
    lkvm cp $INPUT_DATA_FILE my_realm_$TASK_ID:/root/input.txt

    # 加载 GPU 任务文件到内核
    echo "Loading GPU task file ($GPU_TASK_FILE) to kernel..."
    sudo insmod $GPU_TASK_FILE

    # 启动任务执行
    echo "Starting task execution..."
    lkvm exec my_realm_$TASK_ID -- /root/run_task.sh

    # 模拟生成结果文件
    echo "Generating result file..."
    lkvm exec my_realm_$TASK_ID -- /root/generate_result.sh

    # 将结果文件传回 Normal World
    echo "Copying result file to Normal World..."
    lkvm cp my_realm_$TASK_ID:/root/result.txt ./result_$TASK_ID.txt

    # 销毁 Realm
    echo "Destroying Realm..."
    lkvm stop my_realm_$TASK_ID

    echo "Task $TASK_ID completed!"
done