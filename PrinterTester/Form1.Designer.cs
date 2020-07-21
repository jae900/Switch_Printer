namespace PrinterTester
{
    partial class MainForm
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.AutoStartButton = new System.Windows.Forms.Button();
            this.InfoTextBox = new System.Windows.Forms.TextBox();
            this.Run = new System.Windows.Forms.CheckBox();
            this.textBoxPrinter = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.changeCutePDF = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.findPrinter = new System.Windows.Forms.Button();
            this.checkBoxSaveAs = new System.Windows.Forms.CheckBox();
            this.textBoxOutDir = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.buttonApplyOutputDir = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.textBoxFileNameFormat = new System.Windows.Forms.TextBox();
            this.buttonApplyFileNameFormat = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // AutoStartButton
            // 
            this.AutoStartButton.Location = new System.Drawing.Point(282, 6);
            this.AutoStartButton.Name = "AutoStartButton";
            this.AutoStartButton.Size = new System.Drawing.Size(75, 23);
            this.AutoStartButton.TabIndex = 0;
            this.AutoStartButton.Text = "자동";
            this.AutoStartButton.UseVisualStyleBackColor = true;
            this.AutoStartButton.Click += new System.EventHandler(this.AutoStartButton_Click);
            // 
            // InfoTextBox
            // 
            this.InfoTextBox.BackColor = System.Drawing.SystemColors.Desktop;
            this.InfoTextBox.ForeColor = System.Drawing.SystemColors.HighlightText;
            this.InfoTextBox.Location = new System.Drawing.Point(0, 172);
            this.InfoTextBox.Multiline = true;
            this.InfoTextBox.Name = "InfoTextBox";
            this.InfoTextBox.ReadOnly = true;
            this.InfoTextBox.Size = new System.Drawing.Size(432, 138);
            this.InfoTextBox.TabIndex = 1;
            // 
            // Run
            // 
            this.Run.AutoSize = true;
            this.Run.Location = new System.Drawing.Point(13, 13);
            this.Run.Name = "Run";
            this.Run.Size = new System.Drawing.Size(88, 16);
            this.Run.TabIndex = 2;
            this.Run.Text = "파일로 출력";
            this.Run.UseVisualStyleBackColor = true;
            this.Run.CheckedChanged += new System.EventHandler(this.Run_CheckedChanged);
            // 
            // textBoxPrinter
            // 
            this.textBoxPrinter.Location = new System.Drawing.Point(71, 36);
            this.textBoxPrinter.MaxLength = 64;
            this.textBoxPrinter.Name = "textBoxPrinter";
            this.textBoxPrinter.Size = new System.Drawing.Size(253, 21);
            this.textBoxPrinter.TabIndex = 3;
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(71, 64);
            this.textBoxPort.MaxLength = 32;
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(253, 21);
            this.textBoxPort.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 40);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(45, 12);
            this.label1.TabIndex = 5;
            this.label1.Text = "프린터:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 68);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(33, 12);
            this.label2.TabIndex = 6;
            this.label2.Text = "포트:";
            // 
            // changeCutePDF
            // 
            this.changeCutePDF.Location = new System.Drawing.Point(330, 36);
            this.changeCutePDF.Name = "changeCutePDF";
            this.changeCutePDF.Size = new System.Drawing.Size(102, 21);
            this.changeCutePDF.TabIndex = 7;
            this.changeCutePDF.Text = "CutePDF로 출력";
            this.changeCutePDF.UseVisualStyleBackColor = true;
            this.changeCutePDF.Click += new System.EventHandler(this.changeCutePDF_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(204, 11);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(69, 12);
            this.label3.TabIndex = 8;
            this.label3.Text = "인젝션 방식";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(360, 11);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(57, 12);
            this.label4.TabIndex = 9;
            this.label4.Text = "으로 설정";
            // 
            // findPrinter
            // 
            this.findPrinter.Location = new System.Drawing.Point(330, 62);
            this.findPrinter.Name = "findPrinter";
            this.findPrinter.Size = new System.Drawing.Size(102, 23);
            this.findPrinter.TabIndex = 10;
            this.findPrinter.Text = "프린터 검색";
            this.findPrinter.UseVisualStyleBackColor = true;
            this.findPrinter.Click += new System.EventHandler(this.findPrinter_Click);
            // 
            // checkBoxSaveAs
            // 
            this.checkBoxSaveAs.AutoSize = true;
            this.checkBoxSaveAs.Location = new System.Drawing.Point(71, 119);
            this.checkBoxSaveAs.Name = "checkBoxSaveAs";
            this.checkBoxSaveAs.Size = new System.Drawing.Size(242, 16);
            this.checkBoxSaveAs.TabIndex = 11;
            this.checkBoxSaveAs.Text = "다른 이름으로 저장(파일저장 창 보이기)";
            this.checkBoxSaveAs.UseVisualStyleBackColor = true;
            this.checkBoxSaveAs.CheckedChanged += new System.EventHandler(this.checkBoxSaveAs_CheckedChanged);
            // 
            // textBoxOutDir
            // 
            this.textBoxOutDir.Location = new System.Drawing.Point(71, 92);
            this.textBoxOutDir.MaxLength = 260;
            this.textBoxOutDir.Name = "textBoxOutDir";
            this.textBoxOutDir.Size = new System.Drawing.Size(280, 21);
            this.textBoxOutDir.TabIndex = 12;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(15, 95);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(57, 12);
            this.label5.TabIndex = 13;
            this.label5.Text = "출력폴더:";
            // 
            // buttonApplyOutputDir
            // 
            this.buttonApplyOutputDir.Location = new System.Drawing.Point(357, 90);
            this.buttonApplyOutputDir.Name = "buttonApplyOutputDir";
            this.buttonApplyOutputDir.Size = new System.Drawing.Size(75, 23);
            this.buttonApplyOutputDir.TabIndex = 14;
            this.buttonApplyOutputDir.Text = "적용";
            this.buttonApplyOutputDir.UseVisualStyleBackColor = true;
            this.buttonApplyOutputDir.Click += new System.EventHandler(this.buttonApplyOutputDir_Click);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(13, 145);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(57, 12);
            this.label6.TabIndex = 15;
            this.label6.Text = "파일이름:";
            // 
            // textBoxFileNameFormat
            // 
            this.textBoxFileNameFormat.Location = new System.Drawing.Point(71, 141);
            this.textBoxFileNameFormat.MaxLength = 32;
            this.textBoxFileNameFormat.Name = "textBoxFileNameFormat";
            this.textBoxFileNameFormat.Size = new System.Drawing.Size(280, 21);
            this.textBoxFileNameFormat.TabIndex = 16;
            // 
            // buttonApplyFileNameFormat
            // 
            this.buttonApplyFileNameFormat.Location = new System.Drawing.Point(357, 139);
            this.buttonApplyFileNameFormat.Name = "buttonApplyFileNameFormat";
            this.buttonApplyFileNameFormat.Size = new System.Drawing.Size(75, 23);
            this.buttonApplyFileNameFormat.TabIndex = 17;
            this.buttonApplyFileNameFormat.Text = "적용";
            this.buttonApplyFileNameFormat.UseVisualStyleBackColor = true;
            this.buttonApplyFileNameFormat.Click += new System.EventHandler(this.buttonApplyFileNameFormat_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(433, 310);
            this.Controls.Add(this.buttonApplyFileNameFormat);
            this.Controls.Add(this.textBoxFileNameFormat);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.buttonApplyOutputDir);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.textBoxOutDir);
            this.Controls.Add(this.checkBoxSaveAs);
            this.Controls.Add(this.findPrinter);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.changeCutePDF);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxPrinter);
            this.Controls.Add(this.Run);
            this.Controls.Add(this.InfoTextBox);
            this.Controls.Add(this.AutoStartButton);
            this.Name = "MainForm";
            this.Text = "프린터 후킹 모듈 테스트 프로그램";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button AutoStartButton;
        private System.Windows.Forms.TextBox InfoTextBox;
        private System.Windows.Forms.CheckBox Run;
        private System.Windows.Forms.TextBox textBoxPrinter;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button changeCutePDF;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button findPrinter;
        private System.Windows.Forms.CheckBox checkBoxSaveAs;
        private System.Windows.Forms.TextBox textBoxOutDir;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button buttonApplyOutputDir;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textBoxFileNameFormat;
        private System.Windows.Forms.Button buttonApplyFileNameFormat;
    }
}

