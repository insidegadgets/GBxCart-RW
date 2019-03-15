namespace GBxCart_RW
{
    public partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.stopbutton = new System.Windows.Forms.Button();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.backgroundWorker2 = new System.ComponentModel.BackgroundWorker();
            this.readheaderbutton = new System.Windows.Forms.Button();
            this.saverambutton = new System.Windows.Forms.Button();
            this.writerambutton = new System.Windows.Forms.Button();
            this.readrombutton = new System.Windows.Forms.Button();
            this.openportbutton = new System.Windows.Forms.Button();
            this.baudtextBox = new System.Windows.Forms.TextBox();
            this.comPortTextBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.headerTextBox = new System.Windows.Forms.RichTextBox();
            this.closeportbutton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.statuslabel = new System.Windows.Forms.Label();
            this.modelabel = new System.Windows.Forms.Label();
            this.firmwareText = new System.Windows.Forms.Label();
            this.firmwarelabel = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SpecifyCartInfoMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectRomFiletoWriteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.eraseRAMMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.locationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.directoryNameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.persistanceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.promptForFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.manualMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.checkForUpdatesMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.writerombutton = new System.Windows.Forms.Button();
            this.openFileDialog2 = new System.Windows.Forms.OpenFileDialog();
            this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
            this.gbMode = new System.Windows.Forms.RadioButton();
            this.gbaMode = new System.Windows.Forms.RadioButton();
            this.usbStatusPictureOff = new System.Windows.Forms.PictureBox();
            this.usbStatusPictureOn = new System.Windows.Forms.PictureBox();
            this.alwaysRereadHeaToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.usbStatusPictureOff)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.usbStatusPictureOn)).BeginInit();
            this.SuspendLayout();
            // 
            // stopbutton
            // 
            this.stopbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.stopbutton.Location = new System.Drawing.Point(287, 190);
            this.stopbutton.Name = "stopbutton";
            this.stopbutton.Size = new System.Drawing.Size(75, 23);
            this.stopbutton.TabIndex = 23;
            this.stopbutton.Text = "Stop";
            this.stopbutton.UseVisualStyleBackColor = true;
            this.stopbutton.Click += new System.EventHandler(this.stopbutton_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(6, 191);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(275, 21);
            this.progressBar1.TabIndex = 24;
            // 
            // backgroundWorker1
            // 
            this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
            // 
            // backgroundWorker2
            // 
            this.backgroundWorker2.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker2_DoWork);
            // 
            // readheaderbutton
            // 
            this.readheaderbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.readheaderbutton.Location = new System.Drawing.Point(325, 75);
            this.readheaderbutton.Name = "readheaderbutton";
            this.readheaderbutton.Size = new System.Drawing.Size(102, 23);
            this.readheaderbutton.TabIndex = 30;
            this.readheaderbutton.Text = "Read Cart Info";
            this.readheaderbutton.UseVisualStyleBackColor = true;
            this.readheaderbutton.Click += new System.EventHandler(this.readheaderbutton_Click);
            // 
            // saverambutton
            // 
            this.saverambutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.saverambutton.Location = new System.Drawing.Point(287, 132);
            this.saverambutton.Name = "saverambutton";
            this.saverambutton.Size = new System.Drawing.Size(86, 23);
            this.saverambutton.TabIndex = 31;
            this.saverambutton.Text = "Backup Save";
            this.saverambutton.UseVisualStyleBackColor = true;
            this.saverambutton.Click += new System.EventHandler(this.saverambutton_Click);
            // 
            // writerambutton
            // 
            this.writerambutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.writerambutton.Location = new System.Drawing.Point(379, 132);
            this.writerambutton.Name = "writerambutton";
            this.writerambutton.Size = new System.Drawing.Size(86, 23);
            this.writerambutton.TabIndex = 32;
            this.writerambutton.Text = "Restore Save";
            this.writerambutton.UseVisualStyleBackColor = true;
            this.writerambutton.Click += new System.EventHandler(this.writerambutton_Click);
            // 
            // readrombutton
            // 
            this.readrombutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.readrombutton.Location = new System.Drawing.Point(287, 104);
            this.readrombutton.Name = "readrombutton";
            this.readrombutton.Size = new System.Drawing.Size(86, 23);
            this.readrombutton.TabIndex = 33;
            this.readrombutton.Text = "Read ROM";
            this.readrombutton.UseVisualStyleBackColor = true;
            this.readrombutton.Click += new System.EventHandler(this.readrombutton_Click);
            // 
            // openportbutton
            // 
            this.openportbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.openportbutton.Location = new System.Drawing.Point(341, 26);
            this.openportbutton.Name = "openportbutton";
            this.openportbutton.Size = new System.Drawing.Size(70, 22);
            this.openportbutton.TabIndex = 42;
            this.openportbutton.Text = "Connect";
            this.openportbutton.UseVisualStyleBackColor = true;
            this.openportbutton.Click += new System.EventHandler(this.openportbutton_Click);
            // 
            // baudtextBox
            // 
            this.baudtextBox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.baudtextBox.Location = new System.Drawing.Point(408, 163);
            this.baudtextBox.Name = "baudtextBox";
            this.baudtextBox.Size = new System.Drawing.Size(57, 23);
            this.baudtextBox.TabIndex = 43;
            this.baudtextBox.Text = "1000000";
            this.baudtextBox.Visible = false;
            // 
            // comPortTextBox
            // 
            this.comPortTextBox.BackColor = System.Drawing.SystemColors.Window;
            this.comPortTextBox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.comPortTextBox.Location = new System.Drawing.Point(287, 162);
            this.comPortTextBox.Name = "comPortTextBox";
            this.comPortTextBox.Size = new System.Drawing.Size(32, 23);
            this.comPortTextBox.TabIndex = 44;
            this.comPortTextBox.Text = "1";
            this.comPortTextBox.Visible = false;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(220, 165);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(61, 15);
            this.label6.TabIndex = 45;
            this.label6.Text = "COM Port";
            this.label6.Visible = false;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(366, 165);
            this.label7.Name = "label7";
            this.label7.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.label7.Size = new System.Drawing.Size(35, 15);
            this.label7.TabIndex = 46;
            this.label7.Text = "Baud";
            this.label7.Visible = false;
            // 
            // headerTextBox
            // 
            this.headerTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.headerTextBox.Location = new System.Drawing.Point(6, 73);
            this.headerTextBox.Name = "headerTextBox";
            this.headerTextBox.Size = new System.Drawing.Size(275, 90);
            this.headerTextBox.TabIndex = 47;
            this.headerTextBox.Text = "";
            // 
            // closeportbutton
            // 
            this.closeportbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.closeportbutton.Location = new System.Drawing.Point(336, 26);
            this.closeportbutton.Name = "closeportbutton";
            this.closeportbutton.Size = new System.Drawing.Size(80, 23);
            this.closeportbutton.TabIndex = 48;
            this.closeportbutton.Text = "Disconnect";
            this.closeportbutton.UseVisualStyleBackColor = true;
            this.closeportbutton.Visible = false;
            this.closeportbutton.Click += new System.EventHandler(this.closeportbutton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(103, 55);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(85, 15);
            this.label1.TabIndex = 49;
            this.label1.Text = "Cartridge Info";
            // 
            // statuslabel
            // 
            this.statuslabel.AutoSize = true;
            this.statuslabel.BackColor = System.Drawing.Color.Transparent;
            this.statuslabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.statuslabel.Location = new System.Drawing.Point(93, 170);
            this.statuslabel.Name = "statuslabel";
            this.statuslabel.Size = new System.Drawing.Size(65, 15);
            this.statuslabel.TabIndex = 53;
            this.statuslabel.Text = "Status here";
            // 
            // modelabel
            // 
            this.modelabel.AutoSize = true;
            this.modelabel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.modelabel.Location = new System.Drawing.Point(3, 30);
            this.modelabel.Name = "modelabel";
            this.modelabel.Size = new System.Drawing.Size(42, 15);
            this.modelabel.TabIndex = 67;
            this.modelabel.Text = "Mode:";
            // 
            // firmwareText
            // 
            this.firmwareText.AutoSize = true;
            this.firmwareText.BackColor = System.Drawing.Color.Transparent;
            this.firmwareText.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.firmwareText.Location = new System.Drawing.Point(211, 30);
            this.firmwareText.Name = "firmwareText";
            this.firmwareText.Size = new System.Drawing.Size(20, 15);
            this.firmwareText.TabIndex = 70;
            this.firmwareText.Text = "R1";
            this.firmwareText.Visible = false;
            // 
            // firmwarelabel
            // 
            this.firmwarelabel.AutoSize = true;
            this.firmwarelabel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.firmwarelabel.Location = new System.Drawing.Point(149, 30);
            this.firmwarelabel.Name = "firmwarelabel";
            this.firmwarelabel.Size = new System.Drawing.Size(63, 15);
            this.firmwarelabel.TabIndex = 69;
            this.firmwarelabel.Text = "Firmware:";
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.SystemColors.Menu;
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.locationToolStripMenuItem,
            this.optionsToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(470, 24);
            this.menuStrip1.TabIndex = 71;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.SpecifyCartInfoMenuItem,
            this.selectRomFiletoWriteMenuItem,
            this.eraseRAMMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
            this.fileToolStripMenuItem.Text = "Cart";
            // 
            // SpecifyCartInfoMenuItem
            // 
            this.SpecifyCartInfoMenuItem.Name = "SpecifyCartInfoMenuItem";
            this.SpecifyCartInfoMenuItem.Size = new System.Drawing.Size(201, 22);
            this.SpecifyCartInfoMenuItem.Text = "Specify Cart Info";
            this.SpecifyCartInfoMenuItem.Click += new System.EventHandler(this.specifyCartInfoMenuItem_Click);
            // 
            // selectRomFiletoWriteMenuItem
            // 
            this.selectRomFiletoWriteMenuItem.Name = "selectRomFiletoWriteMenuItem";
            this.selectRomFiletoWriteMenuItem.Size = new System.Drawing.Size(201, 22);
            this.selectRomFiletoWriteMenuItem.Text = "Select ROM File to Write";
            this.selectRomFiletoWriteMenuItem.Click += new System.EventHandler(this.selectRomFiletoWriteMenuItem_Click);
            // 
            // eraseRAMMenuItem
            // 
            this.eraseRAMMenuItem.Name = "eraseRAMMenuItem";
            this.eraseRAMMenuItem.Size = new System.Drawing.Size(201, 22);
            this.eraseRAMMenuItem.Text = "Erase Save";
            this.eraseRAMMenuItem.Click += new System.EventHandler(this.eraseRAMToolStripMenuItem_Click);
            // 
            // locationToolStripMenuItem
            // 
            this.locationToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.directoryNameToolStripMenuItem,
            this.persistanceToolStripMenuItem});
            this.locationToolStripMenuItem.Name = "locationToolStripMenuItem";
            this.locationToolStripMenuItem.Size = new System.Drawing.Size(67, 20);
            this.locationToolStripMenuItem.Text = "Directory";
            this.locationToolStripMenuItem.Click += new System.EventHandler(this.locationToolStripMenuItem_Click);
            // 
            // directoryNameToolStripMenuItem
            // 
            this.directoryNameToolStripMenuItem.Name = "directoryNameToolStripMenuItem";
            this.directoryNameToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.directoryNameToolStripMenuItem.Text = "C:\\";
            this.directoryNameToolStripMenuItem.Click += new System.EventHandler(this.directoryNameToolStripMenuItem_Click);
            // 
            // persistanceToolStripMenuItem
            // 
            this.persistanceToolStripMenuItem.Name = "persistanceToolStripMenuItem";
            this.persistanceToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.persistanceToolStripMenuItem.Text = "Remember: No";
            this.persistanceToolStripMenuItem.Click += new System.EventHandler(this.persistanceToolStripMenuItem_Click);
            // 
            // optionsToolStripMenuItem
            // 
            this.optionsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem,
            this.promptForFileToolStripMenuItem,
            this.alwaysRereadHeaToolStripMenuItem});
            this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
            this.optionsToolStripMenuItem.Size = new System.Drawing.Size(61, 20);
            this.optionsToolStripMenuItem.Text = "Options";
            this.optionsToolStripMenuItem.Click += new System.EventHandler(this.optionsToolStripMenuItem_Click);
            // 
            // alwaysAddDatetimeToSaveGamesYesToolStripMenuItem
            // 
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Name = "alwaysAddDatetimeToSaveGamesYesToolStripMenuItem";
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Size = new System.Drawing.Size(368, 22);
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Text = "Always add date/time to backed up Save Game files: No";
            this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Click += new System.EventHandler(this.alwaysAddDatetimeToSaveGamesYesToolStripMenuItem_Click);
            // 
            // promptForFileToolStripMenuItem
            // 
            this.promptForFileToolStripMenuItem.Name = "promptForFileToolStripMenuItem";
            this.promptForFileToolStripMenuItem.Size = new System.Drawing.Size(368, 22);
            this.promptForFileToolStripMenuItem.Text = "Always prompt for Save Game file when restoring: No";
            this.promptForFileToolStripMenuItem.Click += new System.EventHandler(this.promptForFileToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.manualMenuItem,
            this.checkForUpdatesMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // manualMenuItem
            // 
            this.manualMenuItem.Name = "manualMenuItem";
            this.manualMenuItem.Size = new System.Drawing.Size(171, 22);
            this.manualMenuItem.Text = "View Manual";
            this.manualMenuItem.Click += new System.EventHandler(this.manualToolStripMenuItem_Click);
            // 
            // checkForUpdatesMenuItem
            // 
            this.checkForUpdatesMenuItem.Name = "checkForUpdatesMenuItem";
            this.checkForUpdatesMenuItem.Size = new System.Drawing.Size(171, 22);
            this.checkForUpdatesMenuItem.Text = "Check for Updates";
            this.checkForUpdatesMenuItem.Click += new System.EventHandler(this.checkForUpdatesToolStripMenuItem_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "*.gb";
            // 
            // writerombutton
            // 
            this.writerombutton.BackColor = System.Drawing.SystemColors.Control;
            this.writerombutton.Cursor = System.Windows.Forms.Cursors.Default;
            this.writerombutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.writerombutton.ForeColor = System.Drawing.SystemColors.GrayText;
            this.writerombutton.Location = new System.Drawing.Point(379, 104);
            this.writerombutton.Name = "writerombutton";
            this.writerombutton.Size = new System.Drawing.Size(86, 23);
            this.writerombutton.TabIndex = 72;
            this.writerombutton.Text = "Write ROM";
            this.writerombutton.UseVisualStyleBackColor = true;
            this.writerombutton.Click += new System.EventHandler(this.writerombutton_Click);
            // 
            // openFileDialog2
            // 
            this.openFileDialog2.FileName = "*.sav";
            // 
            // notifyIcon1
            // 
            this.notifyIcon1.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon1.Icon")));
            this.notifyIcon1.Text = "GBxCart RW";
            this.notifyIcon1.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.notifyIcon1_MouseDoubleClick);
            // 
            // gbMode
            // 
            this.gbMode.AutoSize = true;
            this.gbMode.Location = new System.Drawing.Point(51, 29);
            this.gbMode.Name = "gbMode";
            this.gbMode.Size = new System.Drawing.Size(40, 17);
            this.gbMode.TabIndex = 74;
            this.gbMode.Text = "GB";
            this.gbMode.UseVisualStyleBackColor = true;
            this.gbMode.CheckedChanged += new System.EventHandler(this.radioButton1_CheckedChanged);
            // 
            // gbaMode
            // 
            this.gbaMode.AutoSize = true;
            this.gbaMode.Checked = true;
            this.gbaMode.Location = new System.Drawing.Point(94, 29);
            this.gbaMode.Name = "gbaMode";
            this.gbaMode.Size = new System.Drawing.Size(47, 17);
            this.gbaMode.TabIndex = 75;
            this.gbaMode.TabStop = true;
            this.gbaMode.Text = "GBA";
            this.gbaMode.UseVisualStyleBackColor = true;
            this.gbaMode.CheckedChanged += new System.EventHandler(this.gbaMode_CheckedChanged);
            // 
            // usbStatusPictureOff
            // 
            this.usbStatusPictureOff.Image = global::GBxCart_RW.Properties.Resources.usb_off;
            this.usbStatusPictureOff.Location = new System.Drawing.Point(319, 29);
            this.usbStatusPictureOff.Name = "usbStatusPictureOff";
            this.usbStatusPictureOff.Size = new System.Drawing.Size(16, 16);
            this.usbStatusPictureOff.TabIndex = 76;
            this.usbStatusPictureOff.TabStop = false;
            this.usbStatusPictureOff.Click += new System.EventHandler(this.pictureBox1_Click);
            // 
            // usbStatusPictureOn
            // 
            this.usbStatusPictureOn.Image = global::GBxCart_RW.Properties.Resources.usb_on_16;
            this.usbStatusPictureOn.Location = new System.Drawing.Point(319, 29);
            this.usbStatusPictureOn.Name = "usbStatusPictureOn";
            this.usbStatusPictureOn.Size = new System.Drawing.Size(16, 16);
            this.usbStatusPictureOn.TabIndex = 77;
            this.usbStatusPictureOn.TabStop = false;
            this.usbStatusPictureOn.Visible = false;
            this.usbStatusPictureOn.Click += new System.EventHandler(this.pictureBox1_Click_1);
            // 
            // alwaysRereadHeaToolStripMenuItem
            // 
            this.alwaysRereadHeaToolStripMenuItem.Name = "alwaysRereadHeaToolStripMenuItem";
            this.alwaysRereadHeaToolStripMenuItem.Size = new System.Drawing.Size(368, 22);
            this.alwaysRereadHeaToolStripMenuItem.Text = "Always re-read cart info when backing up save: No";
            this.alwaysRereadHeaToolStripMenuItem.Click += new System.EventHandler(this.alwaysRereadHeaToolStripMenuItem_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(470, 223);
            this.Controls.Add(this.usbStatusPictureOn);
            this.Controls.Add(this.usbStatusPictureOff);
            this.Controls.Add(this.gbaMode);
            this.Controls.Add(this.gbMode);
            this.Controls.Add(this.writerombutton);
            this.Controls.Add(this.firmwareText);
            this.Controls.Add(this.firmwarelabel);
            this.Controls.Add(this.modelabel);
            this.Controls.Add(this.statuslabel);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.closeportbutton);
            this.Controls.Add(this.headerTextBox);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.comPortTextBox);
            this.Controls.Add(this.baudtextBox);
            this.Controls.Add(this.openportbutton);
            this.Controls.Add(this.readrombutton);
            this.Controls.Add(this.writerambutton);
            this.Controls.Add(this.saverambutton);
            this.Controls.Add(this.readheaderbutton);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.stopbutton);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "GBxCart RW v1.27 by insideGadgets";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.usbStatusPictureOff)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.usbStatusPictureOn)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button stopbutton;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.ComponentModel.BackgroundWorker backgroundWorker2;
        private System.Windows.Forms.Button readheaderbutton;
        private System.Windows.Forms.Button saverambutton;
        private System.Windows.Forms.Button writerambutton;
        private System.Windows.Forms.Button readrombutton;
        private System.Windows.Forms.Button openportbutton;
        private System.Windows.Forms.TextBox baudtextBox;
        private System.Windows.Forms.TextBox comPortTextBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.RichTextBox headerTextBox;
        private System.Windows.Forms.Button closeportbutton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label statuslabel;
        private System.Windows.Forms.Label modelabel;
        private System.Windows.Forms.Label firmwareText;
        private System.Windows.Forms.Label firmwarelabel;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem checkForUpdatesMenuItem;
        private System.Windows.Forms.ToolStripMenuItem SpecifyCartInfoMenuItem;
        private System.Windows.Forms.ToolStripMenuItem selectRomFiletoWriteMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Button writerombutton;
        private System.Windows.Forms.ToolStripMenuItem manualMenuItem;
        private System.Windows.Forms.ToolStripMenuItem eraseRAMMenuItem;
        private System.Windows.Forms.ToolStripMenuItem locationToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem directoryNameToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem persistanceToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog2;
        private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem alwaysAddDatetimeToSaveGamesYesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem promptForFileToolStripMenuItem;
        private System.Windows.Forms.NotifyIcon notifyIcon1;
        private System.Windows.Forms.RadioButton gbMode;
        private System.Windows.Forms.RadioButton gbaMode;
        private System.Windows.Forms.PictureBox usbStatusPictureOff;
        private System.Windows.Forms.PictureBox usbStatusPictureOn;
        private System.Windows.Forms.ToolStripMenuItem alwaysRereadHeaToolStripMenuItem;
    }
}

