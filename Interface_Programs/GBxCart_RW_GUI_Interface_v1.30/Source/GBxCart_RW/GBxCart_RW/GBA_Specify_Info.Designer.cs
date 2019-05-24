namespace GBxCart_RW {
    partial class GBA_Specify_Info {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.gbaflashtypebox = new System.Windows.Forms.ComboBox();
            this.flashtypetext = new System.Windows.Forms.Label();
            this.eepromsizebox = new System.Windows.Forms.ComboBox();
            this.label10 = new System.Windows.Forms.Label();
            this.memorytypebox = new System.Windows.Forms.ComboBox();
            this.gbacancelbutton = new System.Windows.Forms.Button();
            this.gbaapplybutton = new System.Windows.Forms.Button();
            this.gbaramsizebox = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.gbaromsizebox = new System.Windows.Forms.ComboBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.gbaCartTypeBox = new System.Windows.Forms.ComboBox();
            this.gbaflashchiplabel = new System.Windows.Forms.Label();
            this.gbaflashchipbox = new System.Windows.Forms.ComboBox();
            this.gbaflashpcbbox = new System.Windows.Forms.ComboBox();
            this.gbaflashpcblabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // gbaflashtypebox
            // 
            this.gbaflashtypebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaflashtypebox.FormattingEnabled = true;
            this.gbaflashtypebox.Items.AddRange(new object[] {
            "Atmel",
            "Non-Atmel"});
            this.gbaflashtypebox.Location = new System.Drawing.Point(266, 98);
            this.gbaflashtypebox.Name = "gbaflashtypebox";
            this.gbaflashtypebox.Size = new System.Drawing.Size(84, 23);
            this.gbaflashtypebox.TabIndex = 79;
            this.gbaflashtypebox.Visible = false;
            this.gbaflashtypebox.SelectedIndexChanged += new System.EventHandler(this.gbaflashtypebox_SelectedIndexChanged);
            // 
            // flashtypetext
            // 
            this.flashtypetext.AutoSize = true;
            this.flashtypetext.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.flashtypetext.Location = new System.Drawing.Point(201, 101);
            this.flashtypetext.Name = "flashtypetext";
            this.flashtypetext.Size = new System.Drawing.Size(63, 15);
            this.flashtypetext.TabIndex = 78;
            this.flashtypetext.Text = "Flash Type";
            this.flashtypetext.Visible = false;
            // 
            // eepromsizebox
            // 
            this.eepromsizebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.eepromsizebox.FormattingEnabled = true;
            this.eepromsizebox.Items.AddRange(new object[] {
            "4Kbit",
            "64Kbit"});
            this.eepromsizebox.Location = new System.Drawing.Point(96, 127);
            this.eepromsizebox.Name = "eepromsizebox";
            this.eepromsizebox.Size = new System.Drawing.Size(90, 23);
            this.eepromsizebox.TabIndex = 77;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label10.Location = new System.Drawing.Point(11, 101);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(81, 15);
            this.label10.TabIndex = 76;
            this.label10.Text = "Memory Type";
            // 
            // memorytypebox
            // 
            this.memorytypebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.memorytypebox.FormattingEnabled = true;
            this.memorytypebox.Items.AddRange(new object[] {
            "SRAM",
            "Flash",
            "EEPROM"});
            this.memorytypebox.Location = new System.Drawing.Point(96, 98);
            this.memorytypebox.Name = "memorytypebox";
            this.memorytypebox.Size = new System.Drawing.Size(90, 23);
            this.memorytypebox.TabIndex = 75;
            this.memorytypebox.SelectedIndexChanged += new System.EventHandler(this.memorytypebox_SelectedIndexChanged);
            // 
            // gbacancelbutton
            // 
            this.gbacancelbutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.gbacancelbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbacancelbutton.Location = new System.Drawing.Point(246, 167);
            this.gbacancelbutton.Name = "gbacancelbutton";
            this.gbacancelbutton.Size = new System.Drawing.Size(67, 23);
            this.gbacancelbutton.TabIndex = 74;
            this.gbacancelbutton.Text = "Cancel";
            this.gbacancelbutton.UseVisualStyleBackColor = true;
            // 
            // gbaapplybutton
            // 
            this.gbaapplybutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.gbaapplybutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaapplybutton.Location = new System.Drawing.Point(131, 167);
            this.gbaapplybutton.Name = "gbaapplybutton";
            this.gbaapplybutton.Size = new System.Drawing.Size(67, 23);
            this.gbaapplybutton.TabIndex = 73;
            this.gbaapplybutton.Text = "Apply";
            this.gbaapplybutton.UseVisualStyleBackColor = true;
            this.gbaapplybutton.Click += new System.EventHandler(this.gbaapplybutton_Click);
            // 
            // gbaramsizebox
            // 
            this.gbaramsizebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaramsizebox.FormattingEnabled = true;
            this.gbaramsizebox.Items.AddRange(new object[] {
            "256Kbit",
            "512Kbit",
            "1Mbit"});
            this.gbaramsizebox.Location = new System.Drawing.Point(96, 128);
            this.gbaramsizebox.Name = "gbaramsizebox";
            this.gbaramsizebox.Size = new System.Drawing.Size(90, 23);
            this.gbaramsizebox.TabIndex = 72;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label5.Location = new System.Drawing.Point(35, 131);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(56, 15);
            this.label5.TabIndex = 71;
            this.label5.Text = "RAM Size";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label8.Location = new System.Drawing.Point(34, 70);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(57, 15);
            this.label8.TabIndex = 70;
            this.label8.Text = "ROM Size";
            // 
            // gbaromsizebox
            // 
            this.gbaromsizebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaromsizebox.FormattingEnabled = true;
            this.gbaromsizebox.Items.AddRange(new object[] {
            "4 MByte",
            "8 MByte",
            "16 MByte",
            "32 MByte"});
            this.gbaromsizebox.Location = new System.Drawing.Point(95, 67);
            this.gbaromsizebox.Name = "gbaromsizebox";
            this.gbaromsizebox.Size = new System.Drawing.Size(90, 23);
            this.gbaromsizebox.TabIndex = 69;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(160, 9);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(128, 15);
            this.label9.TabIndex = 68;
            this.label9.Text = "Specify GBA Cart Info";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label11.Location = new System.Drawing.Point(7, 41);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(85, 15);
            this.label11.TabIndex = 82;
            this.label11.Text = "Cartridge Type";
            // 
            // gbaCartTypeBox
            // 
            this.gbaCartTypeBox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaCartTypeBox.FormattingEnabled = true;
            this.gbaCartTypeBox.Items.AddRange(new object[] {
            "Regular ROM Cart",
            "Flash Cart"});
            this.gbaCartTypeBox.Location = new System.Drawing.Point(96, 38);
            this.gbaCartTypeBox.Name = "gbaCartTypeBox";
            this.gbaCartTypeBox.Size = new System.Drawing.Size(121, 23);
            this.gbaCartTypeBox.TabIndex = 81;
            this.gbaCartTypeBox.SelectedIndexChanged += new System.EventHandler(this.gbaCartTypeBox_SelectedIndexChanged);
            // 
            // gbaflashchiplabel
            // 
            this.gbaflashchiplabel.AutoSize = true;
            this.gbaflashchiplabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaflashchiplabel.Location = new System.Drawing.Point(233, 41);
            this.gbaflashchiplabel.Name = "gbaflashchiplabel";
            this.gbaflashchiplabel.Size = new System.Drawing.Size(62, 15);
            this.gbaflashchiplabel.TabIndex = 84;
            this.gbaflashchiplabel.Text = "Flash Chip";
            // 
            // gbaflashchipbox
            // 
            this.gbaflashchipbox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaflashchipbox.FormattingEnabled = true;
            this.gbaflashchipbox.Items.AddRange(new object[] {
            "MSP55LV128",
            "MSP55LV128M",
            "29GL128EHMC",
            "29LV128DTMC",
            "MX29GL128ELT",
            "M29W128",
            "S29GL128",
            "256M29EWH",
            "M36L0R706",
            "M36L0R706 (2)",
            "256L30B",
            "256L30B (2)",
            "4455LLZBQO",
            "4455LLZBQO (2)",
            "4000L0YBQ0",
            "4000L0YBQ0 (2)",
            "MX29LV320"});
            this.gbaflashchipbox.Location = new System.Drawing.Point(298, 38);
            this.gbaflashchipbox.Name = "gbaflashchipbox";
            this.gbaflashchipbox.Size = new System.Drawing.Size(119, 23);
            this.gbaflashchipbox.TabIndex = 83;
            this.gbaflashchipbox.SelectedIndexChanged += new System.EventHandler(this.gbaflashchipbox_SelectedIndexChanged);
            // 
            // gbaflashpcbbox
            // 
            this.gbaflashpcbbox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaflashpcbbox.FormattingEnabled = true;
            this.gbaflashpcbbox.Location = new System.Drawing.Point(298, 67);
            this.gbaflashpcbbox.Name = "gbaflashpcbbox";
            this.gbaflashpcbbox.Size = new System.Drawing.Size(119, 23);
            this.gbaflashpcbbox.TabIndex = 86;
            this.gbaflashpcbbox.Visible = false;
            // 
            // gbaflashpcblabel
            // 
            this.gbaflashpcblabel.AutoSize = true;
            this.gbaflashpcblabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbaflashpcblabel.Location = new System.Drawing.Point(263, 70);
            this.gbaflashpcblabel.Name = "gbaflashpcblabel";
            this.gbaflashpcblabel.Size = new System.Drawing.Size(29, 15);
            this.gbaflashpcblabel.TabIndex = 85;
            this.gbaflashpcblabel.Text = "PCB";
            this.gbaflashpcblabel.Visible = false;
            // 
            // GBA_Specify_Info
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.gbacancelbutton;
            this.ClientSize = new System.Drawing.Size(439, 199);
            this.ControlBox = false;
            this.Controls.Add(this.gbaflashpcbbox);
            this.Controls.Add(this.gbaflashpcblabel);
            this.Controls.Add(this.gbaflashchiplabel);
            this.Controls.Add(this.gbaflashchipbox);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.gbaCartTypeBox);
            this.Controls.Add(this.gbaflashtypebox);
            this.Controls.Add(this.flashtypetext);
            this.Controls.Add(this.eepromsizebox);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.memorytypebox);
            this.Controls.Add(this.gbacancelbutton);
            this.Controls.Add(this.gbaapplybutton);
            this.Controls.Add(this.gbaramsizebox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.gbaromsizebox);
            this.Controls.Add(this.label9);
            this.Name = "GBA_Specify_Info";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = " ";
            this.Load += new System.EventHandler(this.GBA_Specify_Info_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox gbaflashtypebox;
        private System.Windows.Forms.Label flashtypetext;
        private System.Windows.Forms.ComboBox eepromsizebox;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.ComboBox memorytypebox;
        private System.Windows.Forms.Button gbacancelbutton;
        private System.Windows.Forms.Button gbaapplybutton;
        private System.Windows.Forms.ComboBox gbaramsizebox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox gbaromsizebox;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.ComboBox gbaCartTypeBox;
        private System.Windows.Forms.Label gbaflashchiplabel;
        private System.Windows.Forms.ComboBox gbaflashchipbox;
        private System.Windows.Forms.ComboBox gbaflashpcbbox;
        private System.Windows.Forms.Label gbaflashpcblabel;
    }
}