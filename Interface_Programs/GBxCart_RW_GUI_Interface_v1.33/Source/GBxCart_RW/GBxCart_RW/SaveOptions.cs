using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GBxCart_RW {
    public partial class SaveOptions : Form {
        public SaveOptions() {
            InitializeComponent();
            AcceptButton = AddDateTimeButton;
        }
        

        private void AddDateTimeButton_Click(object sender, EventArgs e) {
            Form1.saveAsNewFile = true;
            Form1.commandReceived = Form1.SAVERAM;
            this.Close();
        }
        
        private void OverwriteButton_Click_1(object sender, EventArgs e) {
            Form1.saveAsNewFile = false;
            Form1.commandReceived = Form1.SAVERAM;
            this.Close();
        }

        private void CancelButton1_Click(object sender, EventArgs e) {

        }

        private void label11_Click(object sender, EventArgs e) {

        }

        private void saveNameExisting_Click(object sender, EventArgs e) {

        }
    }
}
