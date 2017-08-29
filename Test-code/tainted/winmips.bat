set SOURCE_BASE=C:\Users\NJU-SEG\huawei\llvm\llvm-3.6.0.src\tools\clang\tools\huawei-checker\test\tainted


md .\obj
copy .\winmips.mk .\obj
cd .\obj

REM 
make -f winmips.mk clean

make -f winmips.mk

:: move /Y D:\Tornado2PPC\host\x86-win32\bin\ccppc.exe D:\Tornado2PPC\host\x86-win32\bin\nouse.exe
:: copy /Y D:\Tornado2PPC\host\x86-win32\kw-bak\ccppc.exe D:\Tornado2PPC\host\x86-win32\bin\ccppc.exe
:: D:\Tornado2PPC\host\x86-win32\bin\ccppc.exe --help |find "Usage">nul||pause

del .\winmips.mk
cd ..

