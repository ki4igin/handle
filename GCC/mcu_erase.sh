utility="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

if "$utility" -c port=SWD -e all; then
    exit 0
elif "$utility" -c port=USB1 -e all; then
    exit 0
else
    exit 1
fi
