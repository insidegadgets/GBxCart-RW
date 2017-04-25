namespace GBxCart_RW
{
    partial class Form1
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
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.closeportbutton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.readromlabel = new System.Windows.Forms.Label();
            this.saveramlabel = new System.Windows.Forms.Label();
            this.writeramlabel = new System.Windows.Forms.Label();
            this.cartinfobutton = new System.Windows.Forms.Button();
            this.gbacartinfopanel = new System.Windows.Forms.Panel();
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
            this.gbcartinfopanel = new System.Windows.Forms.Panel();
            this.cartinfocancelbutton = new System.Windows.Forms.Button();
            this.cartinfoapplybutton = new System.Windows.Forms.Button();
            this.ramsizebox = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.romsizebox = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.gbacartinfopanel.SuspendLayout();
            this.gbcartinfopanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // stopbutton
            // 
            this.stopbutton.Location = new System.Drawing.Point(286, 182);
            this.stopbutton.Name = "stopbutton";
            this.stopbutton.Size = new System.Drawing.Size(75, 21);
            this.stopbutton.TabIndex = 23;
            this.stopbutton.Text = "Stop";
            this.stopbutton.UseVisualStyleBackColor = true;
            this.stopbutton.Click += new System.EventHandler(this.stopbutton_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(5, 182);
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
            this.readheaderbutton.Location = new System.Drawing.Point(321, 65);
            this.readheaderbutton.Name = "readheaderbutton";
            this.readheaderbutton.Size = new System.Drawing.Size(90, 21);
            this.readheaderbutton.TabIndex = 30;
            this.readheaderbutton.Text = "Read Header";
            this.readheaderbutton.UseVisualStyleBackColor = true;
            this.readheaderbutton.Click += new System.EventHandler(this.readheaderbutton_Click);
            // 
            // saverambutton
            // 
            this.saverambutton.Location = new System.Drawing.Point(286, 119);
            this.saverambutton.Name = "saverambutton";
            this.saverambutton.Size = new System.Drawing.Size(75, 21);
            this.saverambutton.TabIndex = 31;
            this.saverambutton.Text = "Save RAM";
            this.saverambutton.UseVisualStyleBackColor = true;
            this.saverambutton.Click += new System.EventHandler(this.saverambutton_Click);
            // 
            // writerambutton
            // 
            this.writerambutton.Location = new System.Drawing.Point(367, 119);
            this.writerambutton.Name = "writerambutton";
            this.writerambutton.Size = new System.Drawing.Size(75, 21);
            this.writerambutton.TabIndex = 32;
            this.writerambutton.Text = "Write RAM";
            this.writerambutton.UseVisualStyleBackColor = true;
            this.writerambutton.Click += new System.EventHandler(this.writerambutton_Click);
            // 
            // readrombutton
            // 
            this.readrombutton.Location = new System.Drawing.Point(321, 92);
            this.readrombutton.Name = "readrombutton";
            this.readrombutton.Size = new System.Drawing.Size(90, 21);
            this.readrombutton.TabIndex = 33;
            this.readrombutton.Text = "Read ROM";
            this.readrombutton.UseVisualStyleBackColor = true;
            this.readrombutton.Click += new System.EventHandler(this.readrombutton_Click);
            // 
            // openportbutton
            // 
            this.openportbutton.Location = new System.Drawing.Point(245, 10);
            this.openportbutton.Name = "openportbutton";
            this.openportbutton.Size = new System.Drawing.Size(70, 21);
            this.openportbutton.TabIndex = 42;
            this.openportbutton.Text = "Open Port";
            this.openportbutton.UseVisualStyleBackColor = true;
            this.openportbutton.Click += new System.EventHandler(this.openportbutton_Click);
            // 
            // baudtextBox
            // 
            this.baudtextBox.Location = new System.Drawing.Point(162, 12);
            this.baudtextBox.Name = "baudtextBox";
            this.baudtextBox.Size = new System.Drawing.Size(57, 20);
            this.baudtextBox.TabIndex = 43;
            this.baudtextBox.Text = "1000000";
            // 
            // comPortTextBox
            // 
            this.comPortTextBox.BackColor = System.Drawing.SystemColors.Window;
            this.comPortTextBox.Location = new System.Drawing.Point(69, 11);
            this.comPortTextBox.Name = "comPortTextBox";
            this.comPortTextBox.Size = new System.Drawing.Size(32, 20);
            this.comPortTextBox.TabIndex = 44;
            this.comPortTextBox.Text = "1";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(2, 15);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(61, 13);
            this.label6.TabIndex = 45;
            this.label6.Text = "COM Port";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(120, 15);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(36, 13);
            this.label7.TabIndex = 46;
            this.label7.Text = "Baud";
            // 
            // richTextBox1
            // 
            this.richTextBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.richTextBox1.Location = new System.Drawing.Point(5, 64);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.Size = new System.Drawing.Size(275, 86);
            this.richTextBox1.TabIndex = 47;
            this.richTextBox1.Text = "";
            // 
            // closeportbutton
            // 
            this.closeportbutton.Location = new System.Drawing.Point(321, 10);
            this.closeportbutton.Name = "closeportbutton";
            this.closeportbutton.Size = new System.Drawing.Size(69, 21);
            this.closeportbutton.TabIndex = 48;
            this.closeportbutton.Text = "Close Port";
            this.closeportbutton.UseVisualStyleBackColor = true;
            this.closeportbutton.Click += new System.EventHandler(this.closeportbutton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(120, 48);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 13);
            this.label1.TabIndex = 49;
            this.label1.Text = "Header";
            // 
            // readromlabel
            // 
            this.readromlabel.AutoSize = true;
            this.readromlabel.BackColor = System.Drawing.Color.Transparent;
            this.readromlabel.Location = new System.Drawing.Point(92, 166);
            this.readromlabel.Name = "readromlabel";
            this.readromlabel.Size = new System.Drawing.Size(84, 13);
            this.readromlabel.TabIndex = 51;
            this.readromlabel.Text = "Reading ROM...";
            this.readromlabel.Visible = false;
            // 
            // saveramlabel
            // 
            this.saveramlabel.AutoSize = true;
            this.saveramlabel.BackColor = System.Drawing.Color.Transparent;
            this.saveramlabel.Location = new System.Drawing.Point(92, 166);
            this.saveramlabel.Name = "saveramlabel";
            this.saveramlabel.Size = new System.Drawing.Size(76, 13);
            this.saveramlabel.TabIndex = 52;
            this.saveramlabel.Text = "Saving RAM...";
            this.saveramlabel.Visible = false;
            // 
            // writeramlabel
            // 
            this.writeramlabel.AutoSize = true;
            this.writeramlabel.BackColor = System.Drawing.Color.Transparent;
            this.writeramlabel.Location = new System.Drawing.Point(92, 166);
            this.writeramlabel.Name = "writeramlabel";
            this.writeramlabel.Size = new System.Drawing.Size(76, 13);
            this.writeramlabel.TabIndex = 53;
            this.writeramlabel.Text = "Writing RAM...";
            this.writeramlabel.Visible = false;
            // 
            // cartinfobutton
            // 
            this.cartinfobutton.Location = new System.Drawing.Point(315, 146);
            this.cartinfobutton.Name = "cartinfobutton";
            this.cartinfobutton.Size = new System.Drawing.Size(101, 21);
            this.cartinfobutton.TabIndex = 54;
            this.cartinfobutton.Text = "Specify Cart Info";
            this.cartinfobutton.UseVisualStyleBackColor = true;
            this.cartinfobutton.Click += new System.EventHandler(this.cartinfobutton_Click);
            // 
            // gbacartinfopanel
            // 
            this.gbacartinfopanel.Controls.Add(this.gbaflashtypebox);
            this.gbacartinfopanel.Controls.Add(this.flashtypetext);
            this.gbacartinfopanel.Controls.Add(this.eepromsizebox);
            this.gbacartinfopanel.Controls.Add(this.label10);
            this.gbacartinfopanel.Controls.Add(this.memorytypebox);
            this.gbacartinfopanel.Controls.Add(this.gbacancelbutton);
            this.gbacartinfopanel.Controls.Add(this.gbaapplybutton);
            this.gbacartinfopanel.Controls.Add(this.gbaramsizebox);
            this.gbacartinfopanel.Controls.Add(this.label5);
            this.gbacartinfopanel.Controls.Add(this.label8);
            this.gbacartinfopanel.Controls.Add(this.gbaromsizebox);
            this.gbacartinfopanel.Controls.Add(this.label9);
            this.gbacartinfopanel.Location = new System.Drawing.Point(0, 17);
            this.gbacartinfopanel.Name = "gbacartinfopanel";
            this.gbacartinfopanel.Size = new System.Drawing.Size(437, 155);
            this.gbacartinfopanel.TabIndex = 56;
            this.gbacartinfopanel.Visible = false;
            // 
            // gbaflashtypebox
            // 
            this.gbaflashtypebox.FormattingEnabled = true;
            this.gbaflashtypebox.Items.AddRange(new object[] {
            "Atmel",
            "Non-Atmel"});
            this.gbaflashtypebox.Location = new System.Drawing.Point(362, 65);
            this.gbaflashtypebox.Name = "gbaflashtypebox";
            this.gbaflashtypebox.Size = new System.Drawing.Size(70, 21);
            this.gbaflashtypebox.TabIndex = 67;
            this.gbaflashtypebox.Visible = false;
            // 
            // flashtypetext
            // 
            this.flashtypetext.AutoSize = true;
            this.flashtypetext.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.flashtypetext.Location = new System.Drawing.Point(297, 68);
            this.flashtypetext.Name = "flashtypetext";
            this.flashtypetext.Size = new System.Drawing.Size(59, 13);
            this.flashtypetext.TabIndex = 66;
            this.flashtypetext.Text = "Flash Type";
            this.flashtypetext.Visible = false;
            // 
            // eepromsizebox
            // 
            this.eepromsizebox.FormattingEnabled = true;
            this.eepromsizebox.Items.AddRange(new object[] {
            "4Kbit",
            "64Kbit"});
            this.eepromsizebox.Location = new System.Drawing.Point(192, 95);
            this.eepromsizebox.Name = "eepromsizebox";
            this.eepromsizebox.Size = new System.Drawing.Size(90, 21);
            this.eepromsizebox.TabIndex = 65;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label10.Location = new System.Drawing.Point(115, 68);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(71, 13);
            this.label10.TabIndex = 64;
            this.label10.Text = "Memory Type";
            // 
            // memorytypebox
            // 
            this.memorytypebox.FormattingEnabled = true;
            this.memorytypebox.Items.AddRange(new object[] {
            "SRAM",
            "Flash",
            "EEPROM"});
            this.memorytypebox.Location = new System.Drawing.Point(192, 65);
            this.memorytypebox.Name = "memorytypebox";
            this.memorytypebox.Size = new System.Drawing.Size(90, 21);
            this.memorytypebox.TabIndex = 63;
            this.memorytypebox.SelectedIndexChanged += new System.EventHandler(this.memorytypebox_SelectedIndexChanged);
            // 
            // gbacancelbutton
            // 
            this.gbacancelbutton.Location = new System.Drawing.Point(240, 130);
            this.gbacancelbutton.Name = "gbacancelbutton";
            this.gbacancelbutton.Size = new System.Drawing.Size(67, 23);
            this.gbacancelbutton.TabIndex = 62;
            this.gbacancelbutton.Text = "Cancel";
            this.gbacancelbutton.UseVisualStyleBackColor = true;
            this.gbacancelbutton.Click += new System.EventHandler(this.gbacancelbutton_Click);
            // 
            // gbaapplybutton
            // 
            this.gbaapplybutton.Location = new System.Drawing.Point(118, 129);
            this.gbaapplybutton.Name = "gbaapplybutton";
            this.gbaapplybutton.Size = new System.Drawing.Size(67, 23);
            this.gbaapplybutton.TabIndex = 61;
            this.gbaapplybutton.Text = "Apply";
            this.gbaapplybutton.UseVisualStyleBackColor = true;
            this.gbaapplybutton.Click += new System.EventHandler(this.gbaapplybutton_Click);
            // 
            // gbaramsizebox
            // 
            this.gbaramsizebox.FormattingEnabled = true;
            this.gbaramsizebox.Items.AddRange(new object[] {
            "256Kbit",
            "512Kbit",
            "1Mbit"});
            this.gbaramsizebox.Location = new System.Drawing.Point(192, 95);
            this.gbaramsizebox.Name = "gbaramsizebox";
            this.gbaramsizebox.Size = new System.Drawing.Size(90, 21);
            this.gbaramsizebox.TabIndex = 60;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(131, 98);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(54, 13);
            this.label5.TabIndex = 59;
            this.label5.Text = "RAM Size";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(130, 37);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(55, 13);
            this.label8.TabIndex = 58;
            this.label8.Text = "ROM Size";
            // 
            // gbaromsizebox
            // 
            this.gbaromsizebox.FormattingEnabled = true;
            this.gbaromsizebox.Items.AddRange(new object[] {
            "4 Mbyte",
            "8 Mbyte",
            "16 Mbyte",
            "32 Mbyte"});
            this.gbaromsizebox.Location = new System.Drawing.Point(191, 34);
            this.gbaromsizebox.Name = "gbaromsizebox";
            this.gbaromsizebox.Size = new System.Drawing.Size(90, 21);
            this.gbaromsizebox.TabIndex = 57;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(161, 7);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(131, 13);
            this.label9.TabIndex = 56;
            this.label9.Text = "Specify GBA Cart Info";
            // 
            // gbcartinfopanel
            // 
            this.gbcartinfopanel.Controls.Add(this.cartinfocancelbutton);
            this.gbcartinfopanel.Controls.Add(this.gbacartinfopanel);
            this.gbcartinfopanel.Controls.Add(this.cartinfoapplybutton);
            this.gbcartinfopanel.Controls.Add(this.ramsizebox);
            this.gbcartinfopanel.Controls.Add(this.label4);
            this.gbcartinfopanel.Controls.Add(this.label3);
            this.gbcartinfopanel.Controls.Add(this.romsizebox);
            this.gbcartinfopanel.Controls.Add(this.label2);
            this.gbcartinfopanel.Location = new System.Drawing.Point(5, 31);
            this.gbcartinfopanel.Name = "gbcartinfopanel";
            this.gbcartinfopanel.Size = new System.Drawing.Size(437, 172);
            this.gbcartinfopanel.TabIndex = 66;
            this.gbcartinfopanel.Visible = false;
            // 
            // cartinfocancelbutton
            // 
            this.cartinfocancelbutton.Location = new System.Drawing.Point(244, 133);
            this.cartinfocancelbutton.Name = "cartinfocancelbutton";
            this.cartinfocancelbutton.Size = new System.Drawing.Size(67, 23);
            this.cartinfocancelbutton.TabIndex = 69;
            this.cartinfocancelbutton.Text = "Cancel";
            this.cartinfocancelbutton.UseVisualStyleBackColor = true;
            this.cartinfocancelbutton.Click += new System.EventHandler(this.cartinfocancelbutton_Click);
            // 
            // cartinfoapplybutton
            // 
            this.cartinfoapplybutton.Location = new System.Drawing.Point(122, 132);
            this.cartinfoapplybutton.Name = "cartinfoapplybutton";
            this.cartinfoapplybutton.Size = new System.Drawing.Size(67, 23);
            this.cartinfoapplybutton.TabIndex = 68;
            this.cartinfoapplybutton.Text = "Apply";
            this.cartinfoapplybutton.UseVisualStyleBackColor = true;
            this.cartinfoapplybutton.Click += new System.EventHandler(this.cartinfoapplybutton_Click);
            // 
            // ramsizebox
            // 
            this.ramsizebox.FormattingEnabled = true;
            this.ramsizebox.Items.AddRange(new object[] {
            "None",
            "512bytes (nibbles)",
            "2 KBytes",
            "8 Kbytes",
            "32 KBytes (4 banks of 8KBytes each)",
            "64 KBytes (8 banks of 8KBytes each)",
            "128 KBytes (16 banks of 8KBytes each)"});
            this.ramsizebox.Location = new System.Drawing.Point(154, 95);
            this.ramsizebox.Name = "ramsizebox";
            this.ramsizebox.Size = new System.Drawing.Size(202, 21);
            this.ramsizebox.TabIndex = 67;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(93, 98);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(54, 13);
            this.label4.TabIndex = 66;
            this.label4.Text = "RAM Size";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(93, 64);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 65;
            this.label3.Text = "ROM Size";
            // 
            // romsizebox
            // 
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
            this.romsizebox.Location = new System.Drawing.Point(154, 61);
            this.romsizebox.Name = "romsizebox";
            this.romsizebox.Size = new System.Drawing.Size(153, 21);
            this.romsizebox.TabIndex = 64;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(159, 29);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(123, 13);
            this.label2.TabIndex = 63;
            this.label2.Text = "Specify GB Cart Info";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(449, 213);
            this.Controls.Add(this.gbcartinfopanel);
            this.Controls.Add(this.cartinfobutton);
            this.Controls.Add(this.writeramlabel);
            this.Controls.Add(this.saveramlabel);
            this.Controls.Add(this.readromlabel);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.closeportbutton);
            this.Controls.Add(this.richTextBox1);
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
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "GBxCart RW v1.1 by insideGadgets";
            this.gbacartinfopanel.ResumeLayout(false);
            this.gbacartinfopanel.PerformLayout();
            this.gbcartinfopanel.ResumeLayout(false);
            this.gbcartinfopanel.PerformLayout();
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
        private System.Windows.Forms.RichTextBox richTextBox1;
        private System.Windows.Forms.Button closeportbutton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label readromlabel;
        private System.Windows.Forms.Label saveramlabel;
        private System.Windows.Forms.Label writeramlabel;
        private System.Windows.Forms.Button cartinfobutton;
        private System.Windows.Forms.Panel gbacartinfopanel;
        private System.Windows.Forms.Button gbacancelbutton;
        private System.Windows.Forms.Button gbaapplybutton;
        private System.Windows.Forms.ComboBox gbaramsizebox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox gbaromsizebox;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.ComboBox memorytypebox;
        private System.Windows.Forms.ComboBox eepromsizebox;
        private System.Windows.Forms.Panel gbcartinfopanel;
        private System.Windows.Forms.Button cartinfocancelbutton;
        private System.Windows.Forms.Button cartinfoapplybutton;
        private System.Windows.Forms.ComboBox ramsizebox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox romsizebox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox gbaflashtypebox;
        private System.Windows.Forms.Label flashtypetext;
    }
}

