using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace PrinterTester
{
    static class Program
    {
        public class PrinterHelperCtrl
        {
            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint GetSelectedFilePrinter([MarshalAs(UnmanagedType.LPWStr)] StringBuilder wszPrinterName, [MarshalAs(UnmanagedType.LPWStr)] StringBuilder wszPort, int bInit);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void PrinterRedirection([MarshalAs(UnmanagedType.LPWStr)] String wszPrinterName, [MarshalAs(UnmanagedType.LPWStr)] String wszPort);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint SetConfig(int bRun, [MarshalAs(UnmanagedType.LPWStr)] String wszOutputDir);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint GetConfig([MarshalAs(UnmanagedType.LPWStr)] StringBuilder wszOutputDir, [MarshalAs(UnmanagedType.LPWStr)] StringBuilder wszFileNameFormat);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetFileNameFormat([MarshalAs(UnmanagedType.LPWStr)] String wszFileNameFormat);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint ManualAttach(uint dwProcessId);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint AutoAttach(int bAuto);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void InsertExceptedProcess([MarshalAs(UnmanagedType.LPWStr)] String lpProcessPath);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void RemoveExceptedProcess([MarshalAs(UnmanagedType.LPWStr)] String lpProcessPath);

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void CleanupExceptedProcesses();

            [DllImport("PrinterHelperCtrl.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void InstallFilePrinter();
        }

        /// <summary>
        /// 해당 응용 프로그램의 주 진입점입니다.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());

            Debug.WriteLine("EndProgram");

            
        }

        
    }
}
