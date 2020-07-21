# Switch_Printer
사용자가 출력할 때 지정된 프린터로 변경하여 출력하도록 하는 모듈

#### 모듈 
##### 1. Injector.exe, Injector32.exe
  - 프로세스에 DLL 파일을 인젝션 시키는 프로세스  
  - 사용법 :  
  1) Injector.exe -i 2020 c:\Test\PrinterHelper.dll  
   : 프로세스 ID가 2020인 프로세스에 PrinterHelper.dll을 인젝션 시킨다.  
  2) Injector.exe -d 2020 PrinterHelper.dll  
   : 프로세스 ID가 2020인 프로세스의 PrinterHelper.dll을 FreeLibrary 시킨다.  
  3) Injector.exe -d 0 PrinterHelper.dll  
   : 모든 프로세스의 PrinterHelper.dll을 FreeLibrary 시킨다.  
##### 2. PrinterHelper.dll, PrinterHelper32.dll
   - 실제로 프로세스에 로딩되어 기능에 필요한 API를 후킹하는 라이브러리
##### 3. PrinterHelperCtrl.dll
   - Injector.exe의 실행 및 기능 설정을 하는 Control DLL
##### 4. PrinterTester.exe
   - 테스트 프로그램  
   - 사용법 및 설명 : [SPrinter.pptx](https://github.com/neoqmin/Switch_Printer/blob/master/Documents/SPrinter.pptx)
   
  
