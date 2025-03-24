import yaml

def parse_config(file_path):
    with open(file_path, 'r') as file:
        config = yaml.safe_load(file)
    return config

if __name__ == "__main__":
    import sys
    config = parse_config(sys.argv[1])
    print(config)