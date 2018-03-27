namespace GBxCart_RW {
    partial class SaveOptions {
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
            this.label2 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.AddDateTimeButton = new System.Windows.Forms.Button();
            this.OverwriteButton = new System.Windows.Forms.Button();
            this.CancelButton1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(155, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(136, 15);
            this.label2.TabIndex = 93;
            this.label2.Text = "Existing Save Detected";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.label11.Location = new System.Drawing.Point(12, 33);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(416, 90);
            this.label11.TabIndex = 100;
            this.label11.Text = "Existing Save Game detected on your PC.\r\n\r\nWould you like to:\r\n- Add the date/tim" +
    "e to the save file name, e.g F1RACE_2018.02.25-15.33.12.sav\r\n- Overwrite the exi" +
    "sting file\r\n- Cancel";
            // 
            // AddDateTimeButton
            // 
            this.AddDateTimeButton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.AddDateTimeButton.Location = new System.Drawing.Point(15, 140);
            this.AddDateTimeButton.Name = "AddDateTimeButton";
            this.AddDateTimeButton.Size = new System.Drawing.Size(143, 22);
            this.AddDateTimeButton.TabIndex = 101;
            this.AddDateTimeButton.Text = "Add Date/Time to Save";
            this.AddDateTimeButton.UseVisualStyleBackColor = true;
            this.AddDateTimeButton.Click += new System.EventHandler(this.AddDateTimeButton_Click);
            // 
            // OverwriteButton
            // 
            this.OverwriteButton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.OverwriteButton.Location = new System.Drawing.Point(212, 140);
            this.OverwriteButton.Name = "OverwriteButton";
            this.OverwriteButton.Size = new System.Drawing.Size(67, 22);
            this.OverwriteButton.TabIndex = 102;
            this.OverwriteButton.Text = "Overwrite";
            this.OverwriteButton.UseVisualStyleBackColor = true;
            this.OverwriteButton.Click += new System.EventHandler(this.OverwriteButton_Click_1);
            // 
            // CancelButton1
            // 
            this.CancelButton1.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.CancelButton1.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.CancelButton1.Location = new System.Drawing.Point(337, 140);
            this.CancelButton1.Name = "CancelButton1";
            this.CancelButton1.Size = new System.Drawing.Size(67, 22);
            this.CancelButton1.TabIndex = 103;
            this.CancelButton1.Text = "Cancel";
            this.CancelButton1.UseVisualStyleBackColor = true;
            this.CancelButton1.Click += new System.EventHandler(this.CancelButton1_Click);
            // 
            // SaveOptions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(458, 174);
            this.ControlBox = false;
            this.Controls.Add(this.CancelButton1);
            this.Controls.Add(this.OverwriteButton);
            this.Controls.Add(this.AddDateTimeButton);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.label2);
            this.Name = "SaveOptions";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Save as New File/Overwrite/Cancel";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Button AddDateTimeButton;
        private System.Windows.Forms.Button OverwriteButton;
        private System.Windows.Forms.Button CancelButton1;
    }
}