using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Reflection;        // 외부 DLL 로딩을 위해 Reflection 사용
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace PrinterTester
{
    public partial class MainForm : Form
    {
        bool bManual = true;
        [MarshalAs(UnmanagedType.LPWStr)] StringBuilder pPrinterName = new StringBuilder(64);
        [MarshalAs(UnmanagedType.LPWStr)] StringBuilder pPort = new StringBuilder(32);
        [MarshalAs(UnmanagedType.LPWStr)] StringBuilder pOutputDir = new StringBuilder(260);
        [MarshalAs(UnmanagedType.LPWStr)] StringBuilder pFileNameFormat = new StringBuilder(32); 

        public MainForm()
        {
            uint value = 0;

            InitializeComponent();

            // 예외 등록 for Windows 10 x64
            // 이벤트 뷰어/Windows 로그/응용 프로그램에서 Application Error를 확인한다.
            Program.PrinterHelperCtrl.InsertExceptedProcess("C:\\Windows\\Explorer.exe");
            
            // PrinterHelperCtrl프로젝트의 CheckAndAttachModule 루틴에 기본 예외 정책이 정의되어 있음
            // 해당 부분에 추가하여 프로그램적으로 조치 가능(예, 폴더 기반 예외)
            // C:\Windows\\System32

            //Program.PrinterHelperCtrl.InsertExceptedProcess("C:\\Windows\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe");
            //Program.PrinterHelperCtrl.InsertExceptedProcess("C:\\Windows\\SystemApps\\Microsoft.Windows.Cortana_cw5n1h2txyewy\\SearchUI.exe");
            
            // 현재의 설정에서 파일로 저장 정책 확인
            value = Program.PrinterHelperCtrl.GetConfig(pOutputDir, pFileNameFormat);
            if (value > 0)
            {
                Run.Checked = true;
            } else
            {
                Run.Checked = false;
            }
            // 출력 폴더 확인
            if(pOutputDir.ToString().Length > 0)
            {
                textBoxOutDir.Text = pOutputDir.ToString();
                checkBoxSaveAs.Checked = false;
            } else
            {
                checkBoxSaveAs.Checked = true;
            }
            // 파일 이름 포멧 확인
            if(pFileNameFormat.ToString().Length > 0)
            {
                textBoxFileNameFormat.Text = pFileNameFormat.ToString();
            }
            // 현재 설정된 파일 프린터 확인
            value = Program.PrinterHelperCtrl.GetSelectedFilePrinter(pPrinterName, pPort, 0);
            if (value != 0)
            {
                Debug.WriteLine("Failed to get selected file printer.");
            } else
            {
                textBoxPrinter.Text = pPrinterName.ToString();
                textBoxPort.Text = pPort.ToString();
            }
        }

        private void AutoStartButton_Click(object sender, EventArgs e)
        {
            // 안내문 처리
            InfoTextBox.AppendText("프로세스 ");
            InfoTextBox.AppendText(AutoStartButton.Text);
            InfoTextBox.AppendText(" 인젝션 시작 중...\r\n");

            // 시작
            if (bManual)
                Program.PrinterHelperCtrl.AutoAttach(1);
            else
                Program.PrinterHelperCtrl.AutoAttach(0);

            // 안내문 처리
            InfoTextBox.AppendText("프로세스 ");
            InfoTextBox.AppendText(AutoStartButton.Text);
            InfoTextBox.AppendText(" 인젝션 시작\r\n");


            // 자동설정
            if (bManual)
                AutoStartButton.Text = "수동";
            else
                AutoStartButton.Text = "자동";
            bManual = !bManual;
        }

        private void Run_CheckedChanged(object sender, EventArgs e)
        {
            if (Run.Checked) // 동작 중이 아님
            {
                // 2번째 파라메터의 값이 없을때는 다른이름으로 저장
                Program.PrinterHelperCtrl.SetConfig(1, pOutputDir.ToString());
                //   Program.PrinterHelperCtrl.SetConfig(1, "D:\\Users\\neoqmin\\Documents");
                InfoTextBox.AppendText("파일로 출력하도록 설정한다.\r\n");
            } else
            {
                Program.PrinterHelperCtrl.SetConfig(0, pOutputDir.ToString());
                InfoTextBox.AppendText("선택된 실제 프린터로 출력하도록 설정한다.\r\n");
            }
        }
        // CutePDF로 시작
        private void changeCutePDF_Click(object sender, EventArgs e)
        {
            pPrinterName.Clear();
            pPrinterName.Append("CutePDF Writer");

            pPort.Clear();
            pPort.Append("CPW4:");

            textBoxPrinter.Text = pPrinterName.ToString();
            textBoxPort.Text = pPort.ToString();

            Program.PrinterHelperCtrl.PrinterRedirection(pPrinterName.ToString(), pPort.ToString());

            InfoTextBox.AppendText("CutePDF 프린터를 기본 프린터로 설정한다. 이미 다른 PDF 파일 프린터가 존재한다면, 해당 드라이버의 설치는 따로 해야한다.\r\n");
            InfoTextBox.AppendText("단, PDF 파일 프린터가 없을 경우. GetSelectedFilePrinter 함수 호출만으로 자동으로 비동기 설치된다.\r\n");
        }

        private void findPrinter_Click(object sender, EventArgs e)
        {
            uint value;

            InfoTextBox.AppendText("설치된 프린터를 검색해서 PDF 파일로 출력 하는 파일 프린터를 기본으로 설정한다.\r\n");
            InfoTextBox.AppendText("프린터가 표시되지 않으면, 설치 중이므로 일정시간(10초) 정도 대기 후 다시 확인한다.");
            value = Program.PrinterHelperCtrl.GetSelectedFilePrinter(pPrinterName, pPort, 1);
            if (value != 0)
            {
                Debug.WriteLine("Failed to get selected file printer({0}).", value);
            }
            else
            {
                textBoxPrinter.Text = pPrinterName.ToString();
                textBoxPort.Text = pPort.ToString();
            }
        }

        private void checkBoxSaveAs_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxSaveAs.Checked)
            {
                textBoxOutDir.Text = "";
            }
        }

        private void buttonApplyOutputDir_Click(object sender, EventArgs e)
        {
            int nRun = 1;

            if(!Run.Checked)
            {
                nRun = 0;
            }

            if (textBoxOutDir.Text.Length > 0)
            {
                Program.PrinterHelperCtrl.SetConfig(nRun, textBoxOutDir.Text);
                checkBoxSaveAs.Checked = false;
            } else
            {
                Program.PrinterHelperCtrl.SetConfig(nRun, null);
                checkBoxSaveAs.Checked = true;
            }
        }

        private void buttonApplyFileNameFormat_Click(object sender, EventArgs e)
        {
            if(textBoxFileNameFormat.Text.Length > 0)
            {
                Program.PrinterHelperCtrl.SetFileNameFormat(textBoxFileNameFormat.Text);
            }
        }
    }
}
