import torch

def print_cuda_version():
    if torch.cuda.is_available():
        print(f"CUDA Version: {torch.version.cuda}")
    else:
        print("CUDA is not available on this system.")

if __name__ == "__main__":
    print_cuda_version()