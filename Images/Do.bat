rem This batch file runs the .bmp files through a tool that creates a .h header file for the .bmp file passed in.
rem The .h file produced is 2 dimensional array of 32-bit integers for 24 bit colour, in the correct format for
rem the WiiU OSScreen pixel function. 
rem
rem see https://github.com/MartinButlerAAA/ProcessBmp for the ProcessBmp tool.
rem
rem This tool can only handle .bmp images. Any other images need to be converted to 24-bit colour .bmp files, which can
rem be done using Microsoft Paint.
rem 
cd .
processBmp Game.bmp
processBmp sight.bmp
processBmp Bullet.bmp

processBmp MyPlane1.bmp
processBmp MyPlane2.bmp
processBmp MyPlane3.bmp

processBmp Shot1.bmp
processBmp Shot2.bmp
processBmp Shot3.bmp
processBmp Shot4.bmp
processBmp Shot5.bmp
processBmp Shot6.bmp
processBmp Shot7.bmp
processBmp Shot8.bmp

processBmp Exp1.bmp
processBmp Exp2.bmp
processBmp Exp3.bmp
processBmp Exp4.bmp
processBmp Exp5.bmp
processBmp End.bmp

processBmp PressX.bmp

Pause