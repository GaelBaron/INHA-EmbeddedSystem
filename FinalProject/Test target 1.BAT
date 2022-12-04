SET PATH=C:\Keil_v5\ARM\ARMCLANG\Bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files\TortoiseSVN\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files\TortoiseSVN\bin;C:\Python33\Scripts;C:\Users\baron\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\baron\AppData\Local\Microsoft\WindowsApps;
SET CPU_TYPE=ARMCM4_FP
SET CPU_VENDOR=ARM
SET UV2_TARGET=Test target 1
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\start._ac"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmLink" --Via ".\Objects\Midterms.lnp"
