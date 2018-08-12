namespace GBxCart_RW {
    partial class UpdateCheck {
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
            this.updatelabel = new System.Windows.Forms.Label();
            this.updatecancelbutton = new System.Windows.Forms.Button();
            this.updateclick = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // updatelabel
            // 
            this.updatelabel.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.updatelabel.Location = new System.Drawing.Point(9, 9);
            this.updatelabel.Margin = new System.Windows.Forms.Padding(0);
            this.updatelabel.Name = "updatelabel";
            this.updatelabel.Size = new System.Drawing.Size(203, 19);
            this.updatelabel.TabIndex = 79;
            this.updatelabel.Text = "Checking for updates...";
            this.updatelabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // updatecancelbutton
            // 
            this.updatecancelbutton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.updatecancelbutton.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.updatecancelbutton.Location = new System.Drawing.Point(79, 54);
            this.updatecancelbutton.Name = "updatecancelbutton";
            this.updatecancelbutton.Size = new System.Drawing.Size(67, 23);
            this.updatecancelbutton.TabIndex = 80;
            this.updatecancelbutton.Text = "Ok";
            this.updatecancelbutton.UseVisualStyleBackColor = true;
            // 
            // updateclick
            // 
            this.updateclick.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.updateclick.ForeColor = System.Drawing.SystemColors.HotTrack;
            this.updateclick.Location = new System.Drawing.Point(9, 28);
            this.updateclick.Margin = new System.Windows.Forms.Padding(0);
            this.updateclick.Name = "updateclick";
            this.updateclick.Size = new System.Drawing.Size(203, 19);
            this.updateclick.TabIndex = 81;
            this.updateclick.Text = "Click here to view the update";
            this.updateclick.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            this.updateclick.Visible = false;
            this.updateclick.Click += new System.EventHandler(this.updateclick_Click);
            // 
            // UpdateCheck
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.updatecancelbutton;
            this.ClientSize = new System.Drawing.Size(221, 82);
            this.ControlBox = false;
            this.Controls.Add(this.updateclick);
            this.Controls.Add(this.updatecancelbutton);
            this.Controls.Add(this.updatelabel);
            this.Name = "UpdateCheck";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Update Check";
            this.Load += new System.EventHandler(this.UpdateCheck_Load);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Label updatelabel;
        private System.Windows.Forms.Button updatecancelbutton;
        private System.Windows.Forms.Label updateclick;
    }
}