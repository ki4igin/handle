fileName=${PWD##*/}
fileExt=".hex"
fileNameFull=$filePath$fileName$fileExt
echo "Download file: ${fileNameFull}"

utility="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
if "$utility" -c port=SWD -w $fileNameFull -s; then
    exit 0
elif "$utility" -c port=USB1 -w $fileNameFull -s; then
    exit 0
else
    exit 1
fi
