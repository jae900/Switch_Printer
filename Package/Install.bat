@echo off
pushd "%~dp0"

if exist %windir%\SysWOW64 (
  echo 64비트 설치
  xcopy /y .\x64\PrinterHelperCtrl.dll .
  xcopy /y .\x64\PrinterTester.exe .

  xcopy /y .\x64\vcruntime140.dll C:\Windows\System32\.
  xcopy /y .\x64\vcruntime140d.dll C:\Windows\System32\.
  xcopy /y .\x64\ucrtbase.dll C:\Windows\System32\.
  xcopy /y .\x64\ucrtbased.dll C:\Windows\System32\.

  set destPath=C:\Windows\SysWOW64\
  goto end
)

echo "32비트"

set destPath=C:\Windows\System32\
xcopy /y .\Win32\PrinterHelperCtrl.dll .
xcopy /y .\Win32\PrinterTester.exe .

:end
  echo %destPath%

  xcopy /y .\win32\vcruntime140.dll %destPath%.
  xcopy /y .\win32\vcruntime140d.dll %destPath%.
  xcopy /y .\win32\ucrtbase.dll %destPath%.
  xcopy /y .\win32\ucrtbased.dll %destPath%.

popd
pause
echo on
