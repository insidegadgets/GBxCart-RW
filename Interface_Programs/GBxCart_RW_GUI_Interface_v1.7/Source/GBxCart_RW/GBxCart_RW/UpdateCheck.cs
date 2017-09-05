using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;
using System.Net;
using System.Reflection;
using System.Diagnostics;

namespace GBxCart_RW {
    public partial class UpdateCheck : Form {
        public UpdateCheck() {
            InitializeComponent();
        }

        private void cartinfoapplybutton_Click(object sender, EventArgs e) {

        }

        private void UpdateCheck_Load(object sender, EventArgs e) {
            
            string urlAddress = "https://www.insidegadgets.com/updates/gbxcartrw_version.txt";

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(urlAddress);
            HttpWebResponse response = (HttpWebResponse)request.GetResponse();

            if (response.StatusCode == HttpStatusCode.OK) {
                Stream receiveStream = response.GetResponseStream();
                StreamReader readStream = null;

                if (response.CharacterSet == null) {
                    readStream = new StreamReader(receiveStream);
                }
                else {
                    readStream = new StreamReader(receiveStream, Encoding.GetEncoding(response.CharacterSet));
                }

                string latestVersion = readStream.ReadToEnd();
                //Console.Write(data);

                response.Close();
                readStream.Close();

                // Get current version
                Assembly assembly = Assembly.GetExecutingAssembly();
                FileVersionInfo fileVersionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
                string currentVersion = fileVersionInfo.ProductVersion;
                //Console.Write(currentVersion);

                // Compare
                if (currentVersion.Contains(latestVersion)) {
                    //Console.Write("ok");
                    //System.Windows.Forms.MessageBox.Show("You are on the latest version " + currentVersion);
                    updatelabel.Text = "You are on the latest version " + currentVersion;
                }
                else {
                    //Console.Write("update available");
                    //System.Windows.Forms.MessageBox.Show("An update is available - v" + latestVersion);
                    updatelabel.Text = "An update is available - v" + latestVersion;
                    updateclick.Visible = true;
                }
            }
        }
        

        private void updateclick_Click(object sender, EventArgs e) {
            System.Diagnostics.Process.Start("https://www.insidegadgets.com/projects/gbxcart-rw/");
        }
    }
}
