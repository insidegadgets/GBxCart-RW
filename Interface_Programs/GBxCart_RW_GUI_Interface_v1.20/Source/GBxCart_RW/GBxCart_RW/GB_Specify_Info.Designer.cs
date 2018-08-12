namespace GBxCart_RW {
    partial class GB_Specify_Info {
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
            this.label11 = new System.Windows.Forms.Label();
            this.cartTypeBox = new System.Windows.Forms.ComboBox();
            this.cartinfocancelbutton = new System.Windows.Forms.Button();
            this.cartinfoapplybutton = new System.Windows.Forms.Button();
            this.ramsizebox = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.romsizebox = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.gbflashpcbbox = new System.Windows.Forms.ComboBox();
            this.gbflashpcblabel = new System.Windows.Forms.Label();
            this.gbflashchiplabel = new System.Windows.Forms.Label();
            this.gbflashchipbox = new System.Windows.Forms.ComboBox();
            this.gbflashwebox = new System.Windows.Forms.ComboBox();
            this.gbflashwelabel = new System.Windows.Forms.Label();
            this.chipEraseCheckbox = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label11.Location = new System.Drawing.Point(12, 41);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(85, 15);
            this.label11.TabIndex = 80;
            this.label11.Text = "Cartridge Type";
            // 
            // cartTypeBox
            // 
            this.cartTypeBox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.cartTypeBox.FormattingEnabled = true;
            this.cartTypeBox.Items.AddRange(new object[] {
            "Regular ROM Cart",
            "Flash Cart"});
            this.cartTypeBox.Location = new System.Drawing.Point(101, 38);
            this.cartTypeBox.Name = "cartTypeBox";
            this.cartTypeBox.Size = new System.Drawing.Size(134, 23);
            this.cartTypeBox.TabIndex = 79;
            this.cartTypeBox.SelectedIndexChanged += new System.EventHandler(this.cartTypeBox_SelectedIndexChanged);
            // 
            // cartinfocancelbutton
            // 
            this.cartinfocancelbutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cartinfocancelbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.cartinfocancelbutton.Location = new System.Drawing.Point(262, 156);
            this.cartinfocancelbutton.Name = "cartinfocancelbutton";
            this.cartinfocancelbutton.Size = new System.Drawing.Size(67, 22);
            this.cartinfocancelbutton.TabIndex = 78;
            this.cartinfocancelbutton.Text = "Cancel";
            this.cartinfocancelbutton.UseVisualStyleBackColor = true;
            this.cartinfocancelbutton.Click += new System.EventHandler(this.cartinfocancelbutton_Click);
            // 
            // cartinfoapplybutton
            // 
            this.cartinfoapplybutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.cartinfoapplybutton.Location = new System.Drawing.Point(155, 156);
            this.cartinfoapplybutton.Name = "cartinfoapplybutton";
            this.cartinfoapplybutton.Size = new System.Drawing.Size(67, 22);
            this.cartinfoapplybutton.TabIndex = 77;
            this.cartinfoapplybutton.Text = "Apply";
            this.cartinfoapplybutton.UseVisualStyleBackColor = true;
            this.cartinfoapplybutton.Click += new System.EventHandler(this.cartinfoapplybutton_Click);
            // 
            // ramsizebox
            // 
            this.ramsizebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.ramsizebox.FormattingEnabled = true;
            this.ramsizebox.Items.AddRange(new object[] {
            "None",
            "512 bytes (nibbles)",
            "2 KBytes",
            "8 KBytes",
            "32 KBytes (4 banks x 8KBytes)",
            "64 KBytes (8 banks x 8KBytes)",
            "128 KBytes (16 banks x 8KBytes)"});
            this.ramsizebox.Location = new System.Drawing.Point(101, 106);
            this.ramsizebox.Name = "ramsizebox";
            this.ramsizebox.Size = new System.Drawing.Size(198, 23);
            this.ramsizebox.TabIndex = 76;
            this.ramsizebox.SelectedIndexChanged += new System.EventHandler(this.ramsizebox_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label4.Location = new System.Drawing.Point(40, 109);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(56, 15);
            this.label4.TabIndex = 75;
            this.label4.Text = "RAM Size";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label3.Location = new System.Drawing.Point(40, 75);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(57, 15);
            this.label3.TabIndex = 74;
            this.label3.Text = "ROM Size";
            // 
            // romsizebox
            // 
            this.romsizebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.romsizebox.FormattingEnabled = true;
            this.romsizebox.Items.AddRange(new object[] {
            "32KByte (no ROM banking)",
            "64KByte (4 banks)",
            "128KByte (8 banks)",
            "256KByte (16 banks)",
            "512KByte (32 banks)",
            "1MByte (64 banks)",
            "2MByte (128 banks)",
            "4MByte (256 banks)",
            "8MByte (512 banks)"});
            this.romsizebox.Location = new System.Drawing.Point(101, 72);
            this.romsizebox.Name = "romsizebox";
            this.romsizebox.Size = new System.Drawing.Size(178, 23);
            this.romsizebox.TabIndex = 73;
            this.romsizebox.SelectedIndexChanged += new System.EventHandler(this.romsizebox_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(181, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(120, 15);
            this.label2.TabIndex = 72;
            this.label2.Text = "Specify GB Cart Info";
            // 
            // gbflashpcbbox
            // 
            this.gbflashpcbbox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashpcbbox.FormattingEnabled = true;
            this.gbflashpcbbox.Location = new System.Drawing.Point(349, 72);
            this.gbflashpcbbox.Name = "gbflashpcbbox";
            this.gbflashpcbbox.Size = new System.Drawing.Size(126, 23);
            this.gbflashpcbbox.TabIndex = 90;
            this.gbflashpcbbox.Visible = false;
            this.gbflashpcbbox.SelectedIndexChanged += new System.EventHandler(this.gbflashpcbbox_SelectedIndexChanged);
            // 
            // gbflashpcblabel
            // 
            this.gbflashpcblabel.AutoSize = true;
            this.gbflashpcblabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashpcblabel.Location = new System.Drawing.Point(314, 75);
            this.gbflashpcblabel.Name = "gbflashpcblabel";
            this.gbflashpcblabel.Size = new System.Drawing.Size(29, 15);
            this.gbflashpcblabel.TabIndex = 89;
            this.gbflashpcblabel.Text = "PCB";
            this.gbflashpcblabel.Visible = false;
            // 
            // gbflashchiplabel
            // 
            this.gbflashchiplabel.AutoSize = true;
            this.gbflashchiplabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashchiplabel.Location = new System.Drawing.Point(250, 41);
            this.gbflashchiplabel.Name = "gbflashchiplabel";
            this.gbflashchiplabel.Size = new System.Drawing.Size(93, 15);
            this.gbflashchiplabel.TabIndex = 88;
            this.gbflashchiplabel.Text = "Flash Chip/Type";
            this.gbflashchiplabel.Visible = false;
            // 
            // gbflashchipbox
            // 
            this.gbflashchipbox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashchipbox.FormattingEnabled = true;
            this.gbflashchipbox.Items.AddRange(new object[] {
            "32KB AM29F010B",
            "32KB SST39SF010A",
            "32KB AT49F040",
            "2MB BV5",
            "1MB ES29LV160",
            "2MB AM29LV160DB",
            "2MB 29LV160CTTC",
            "2MB AM29F016B"});
            this.gbflashchipbox.Location = new System.Drawing.Point(349, 38);
            this.gbflashchipbox.Name = "gbflashchipbox";
            this.gbflashchipbox.Size = new System.Drawing.Size(126, 23);
            this.gbflashchipbox.TabIndex = 87;
            this.gbflashchipbox.Visible = false;
            this.gbflashchipbox.SelectedIndexChanged += new System.EventHandler(this.gbflashchipbox_SelectedIndexChanged);
            // 
            // gbflashwebox
            // 
            this.gbflashwebox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashwebox.FormattingEnabled = true;
            this.gbflashwebox.Items.AddRange(new object[] {
            "Audio",
            "WR"});
            this.gbflashwebox.Location = new System.Drawing.Point(372, 106);
            this.gbflashwebox.Name = "gbflashwebox";
            this.gbflashwebox.Size = new System.Drawing.Size(103, 23);
            this.gbflashwebox.TabIndex = 92;
            this.gbflashwebox.Visible = false;
            this.gbflashwebox.SelectedIndexChanged += new System.EventHandler(this.gbflashwebox_SelectedIndexChanged);
            // 
            // gbflashwelabel
            // 
            this.gbflashwelabel.AutoSize = true;
            this.gbflashwelabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.gbflashwelabel.Location = new System.Drawing.Point(342, 109);
            this.gbflashwelabel.Name = "gbflashwelabel";
            this.gbflashwelabel.Size = new System.Drawing.Size(24, 15);
            this.gbflashwelabel.TabIndex = 91;
            this.gbflashwelabel.Text = "WE";
            this.gbflashwelabel.Visible = false;
            // 
            // chipEraseCheckbox
            // 
            this.chipEraseCheckbox.AutoSize = true;
            this.chipEraseCheckbox.Checked = true;
            this.chipEraseCheckbox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chipEraseCheckbox.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.chipEraseCheckbox.Location = new System.Drawing.Point(394, 135);
            this.chipEraseCheckbox.Name = "chipEraseCheckbox";
            this.chipEraseCheckbox.Size = new System.Drawing.Size(81, 19);
            this.chipEraseCheckbox.TabIndex = 94;
            this.chipEraseCheckbox.Text = "Chip Erase";
            this.chipEraseCheckbox.UseVisualStyleBackColor = true;
            this.chipEraseCheckbox.Visible = false;
            this.chipEraseCheckbox.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // GB_Specify_Info
            // 
            this.AcceptButton = this.cartinfoapplybutton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cartinfocancelbutton;
            this.ClientSize = new System.Drawing.Size(487, 186);
            this.ControlBox = false;
            this.Controls.Add(this.chipEraseCheckbox);
            this.Controls.Add(this.gbflashwebox);
            this.Controls.Add(this.gbflashwelabel);
            this.Controls.Add(this.gbflashpcbbox);
            this.Controls.Add(this.gbflashpcblabel);
            this.Controls.Add(this.gbflashchiplabel);
            this.Controls.Add(this.gbflashchipbox);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.cartTypeBox);
            this.Controls.Add(this.cartinfocancelbutton);
            this.Controls.Add(this.cartinfoapplybutton);
            this.Controls.Add(this.ramsizebox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.romsizebox);
            this.Controls.Add(this.label2);
            this.Name = "GB_Specify_Info";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = " ";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.ComboBox cartTypeBox;
        private System.Windows.Forms.Button cartinfocancelbutton;
        private System.Windows.Forms.Button cartinfoapplybutton;
        private System.Windows.Forms.ComboBox ramsizebox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox romsizebox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox gbflashpcbbox;
        private System.Windows.Forms.Label gbflashpcblabel;
        private System.Windows.Forms.Label gbflashchiplabel;
        private System.Windows.Forms.ComboBox gbflashchipbox;
        private System.Windows.Forms.ComboBox gbflashwebox;
        private System.Windows.Forms.Label gbflashwelabel;
        private System.Windows.Forms.CheckBox chipEraseCheckbox;
    }
}