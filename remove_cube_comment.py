import os
import re
import subprocess

def process_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    patterns_dotall = [
        r'\/\*.*?STMicroelectronics.*?\*\/',
        r'#ifdef __cplusplus.*?#endif',
    ]
    patterns = [
        r'\/\*.*[i,I]ncludes.*\*\/',
        r'\/\*.*[t,T]ypes.*\*\/',
        r'\/\*.*[c,C]onstants.*\*\/',
        r'\/\*.*[m,M]acro.*\*\/',
        r'\/\*.*[d,D]efines.*\*\/',
        r'\/\*.*Define to prevent.*\*\/',
        r'\/\*.*Private.*\*\/',
        r'\/\* USER CODE.*',
        r'^\s*\n',
    ]
    content = re.sub('|'.join(patterns_dotall), '', content, flags=re.DOTALL)
    content = re.sub('|'.join(patterns), '', content)
    # print(content)

    with open(file_path, 'w') as file:
        file.write(content)

    subprocess.run(['clang-format', '-i', file_path])


def process_files_in_directory(directory_path):
    # Получаем список всех файлов в указанном каталоге
    files = [f for f in os.listdir(directory_path) if os.path.isfile(os.path.join(directory_path, f))]

    # Перебираем каждый файл и обрабатываем его
    for file_name in files:
        file_path = os.path.join(directory_path, file_name)
        process_file(file_path)

if __name__ == "__main__":    
    directory_path = 'code/Src'
    print(f"Cleaning files in {directory_path}")
    process_files_in_directory(directory_path)
    print("Complete")
