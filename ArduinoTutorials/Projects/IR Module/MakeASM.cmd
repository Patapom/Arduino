@REM @ECHO %PATH%
@REM @SET PATH=%PATH%;"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin"
@REM @ECHO %PATH%

@REM ECHO PROUT
@REM @ECHO %GCC_EXEC_PREFIX%
@REM avr-gcc.exe -gstabs -Wa,-ahlmsd=output.lst -dp -fverbose-asm -O2 "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\pulseTrainInLOW.cpp"

@REM avr-gcc.exe -gstabs -Wa,-ahlmsd=output.lst -dp -fverbose-asm -O2 pulseTrainInLOW.cpp -o pulseTrainInLOW.a -S
@REM avr-gcc.exe -S -O2 "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\pulseTrainInLOW.cpp" -o "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\pulseTrainInLOW.a"


avr-gcc.exe -S -gstabs -Wa,-ahlmsd=output.lst -dp -fverbose-asm -O2 "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\pulseTrainInLOW.cpp" -o "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\pulseTrainInLOW.a"
avr-gcc.exe -S -gstabs -Wa,-ahlmsd=output2.lst -dp -fverbose-asm -O2 "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\englumoule.cpp" -o "D:\Workspaces\Electro\ArduinoTutorials\Projects\IR Module\englumoule.a"
