/*

GBxCart RW - GUI  Interface
Version : 1.13
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 17 / 01 / 2017

GBxCart RW allows you to dump your Gameboy / Gameboy Colour / Gameboy Advance games ROM, save the RAM and write to the RAM.

*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;  // DLL support
using System.IO.Ports;
using System.Net;
using System.Reflection;
using System.Diagnostics;

namespace GBxCart_RW
{
    partial class Form1 : Form
    {
        const int GB_CART = 1;
        const int GBA_CART = 2;

        const int READROM = 1;
        const int SAVERAM = 2;
        const int WRITERAM = 3;
        const int READHEADER = 4;
        const int WRITEROM = 5;
        const int ERASERAM = 6;
        const int ERASEROM = 7;

        public const int PCB_1_0 = 1;
        public const int PCB_1_1 = 2;
        public const char READ_PCB_VERSION = 'h';

        bool comConnected = false;
        int commandReceived = 0;
        bool headerRead = false;
        int cancelOperation = 0;
        UInt32 progress = 0;

        public static string[] headerTokens;
        string writeRomFileName;
        bool writeRomSelected = false;
        public static int writeRomCartType = 0;
        public static UInt32 writeRomCartSize = 0;
        public static UInt32 writeRomSize = 0;
        public static int cartMode = GB_CART;

        public Form1() {
            InitializeComponent();

            backgroundWorker1.WorkerReportsProgress = true;
            backgroundWorker1.WorkerSupportsCancellation = true;
            backgroundWorker1.DoWork += new DoWorkEventHandler(backgroundWorker1_DoWork);
            backgroundWorker1.ProgressChanged += new ProgressChangedEventHandler(backgroundWorker1_ProgressChanged);
            backgroundWorker1.RunWorkerAsync();

            backgroundWorker2.DoWork += new DoWorkEventHandler(backgroundWorker2_DoWork);
            backgroundWorker2.WorkerSupportsCancellation = true;
            backgroundWorker2.RunWorkerAsync();

            comPortTextBox.Text = Convert.ToString(Program.read_config(1));
            baudtextBox.Text = Convert.ToString(Program.read_config(2));
            statuslabel.Text = "";
        }

        private void Form1_Load(object sender, EventArgs e) {

        }

        // Progress bar
        void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e) {
            while (true) {                
                double progress_percent = progress;
                progress_percent = progress_percent;

                if (progress_percent >= 100) {
                    progress_percent = 100;
                }

                backgroundWorker1.ReportProgress(Convert.ToInt32(progress_percent));
                System.Threading.Thread.Sleep(100);
                //Console.WriteLine(progress);
            }
        }

        // Update the progress bar
        void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            // The progress percentage is a property of e
            progressBar1.Value = e.ProgressPercentage;
        }
        
        // Perform the reading/writing/header read in the background
        void backgroundWorker2_DoWork(object sender, DoWorkEventArgs e) {
            while (true) {

                // Before doing any operations, check if we are still connected and read the mode we are in
                if (commandReceived != 0) {
                    int checkCom = Program.read_firmware_version();
                    int checkComCounter = 0;
                    //Console.Write(checkCom);

                    // Read which mode we are in
                    if (checkCom >= 1) {
                        if (cartMode == GB_CART) {
                            modeTextBox.Invoke((MethodInvoker)(() => {
                                modeTextBox.Text = "GB/GBC";
                            }));
                        }
                        else if (cartMode == GBA_CART) {
                            modeTextBox.Invoke((MethodInvoker)(() => {
                                modeTextBox.Text = "GBA";
                            }));
                        }
                    }

                    while (checkCom == 0) { // Not connected, check it a few times
                        checkCom = Program.read_firmware_version();
                        System.Threading.Thread.Sleep(100);
                        checkComCounter++;
                        //Console.Write(checkCom);

                        if (checkComCounter >= 3) { // Close the COM port
                            commandReceived = 0;

                            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
                            comPortInt--;

                            Program.RS232_CloseComport(comPortInt);
                            comConnected = false;

                            comPortTextBox.Invoke((MethodInvoker)(() => {
                                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                            }));
                            
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Device disconnected"; 
                            }));

                            break;
                        }
                    }
                }

                if (commandReceived == READROM) { // Read ROM
                    Program.read_rom(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Reading ROM... Finished";
                        }));
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Reading ROM... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == SAVERAM) { // Save RAM
                    Program.read_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Backing up Save... Finished";
                        }));
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Backing up Save... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == WRITERAM) { // Write RAM
                    Program.write_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Restoring Save... Finished";
                        }));
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Restoring Save... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == WRITEROM) { // Write ROM
                    if (writeRomCartType == 2 || writeRomCartType == 4) { // First erase the flash chip if needed
                        Program.erase_rom(writeRomCartType, ref progress, ref cancelOperation);

                        if (cancelOperation == 1) {
                            commandReceived = 0;
                            System.Threading.Thread.Sleep(500);

                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Erasing Flash... Cancelled";
                            }));
                        }
                        else {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM...";
                            }));

                            progress = 0;
                            backgroundWorker1.ReportProgress(0);
                        }
                    }

                    // Program the flash chip
                    if (cancelOperation != 1) {
                        Program.write_rom(writeRomFileName, writeRomCartType, writeRomSize, ref progress, ref cancelOperation);
                        commandReceived = 0;
                        System.Threading.Thread.Sleep(500);

                        if (cancelOperation == 0) {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM... Finished";
                            }));
                        }
                        else {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM... Cancelled";
                            }));
                        }
                    }
                }
                else if (commandReceived == ERASERAM) { // Erase RAM
                    Program.erase_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Erasing RAM... Finished";
                        }));
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Erasing RAM... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == READHEADER) { // Read Header
                    headerRead = true;
                    Int32 textLength = 0;
                    IntPtr headerPointer;
                    string headerText = "";

                    if (cartMode == GB_CART) {
                        headerPointer = Program.read_gb_header(ref textLength);
                        headerText = Marshal.PtrToStringAnsi(headerPointer, textLength);
                        headerTokens = headerText.Split('\n');
                    }
                    else if (cartMode == GBA_CART) {
                        headerPointer = Program.read_gba_header(ref textLength);
                        headerText = Marshal.PtrToStringAnsi(headerPointer, textLength);
                        headerTokens = headerText.Split('\n');
                    }
                    headerTextBox.Invoke((MethodInvoker)(() => {
                        //headerTextBox.Text = headerText;
                        headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                    }));

                    commandReceived = 0;
                }

                cancelOperation = 0;
                System.Threading.Thread.Sleep(100);
            }
        }
     

        
        // Open port button
        private void openportbutton_Click(object sender, EventArgs e) {
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            headerTextBox.Text = "";
            statuslabel.Text = "";

            // If port open, close it, we will open it again below
            if (comConnected == true) {
                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }

            // Successful open
            if (Program.RS232_OpenComport(comPortInt, baudInt, "8N1") == 0) {
                comPortTextBox.BackColor = Color.FromArgb(192, 255, 192);
                comConnected = true;
                commandReceived = 0;

                headerRead = false;
                progress = 0;
                backgroundWorker1.ReportProgress(0);
                

                Program.update_config(comPortInt, baudInt);
                
                // Read device firmware
                int firmwareVersion = Program.read_firmware_version();
                if (firmwareVersion >= 1) {
                    firmwareText.Text = "R" + firmwareVersion;
                    firmwareText.Visible = true;

                    // Read which mode we are in
                    cartMode = Program.read_cartridge_mode();
                    if (cartMode == GB_CART) {
                        modeTextBox.Invoke((MethodInvoker)(() => {
                            modeTextBox.Text = "GB/GBC";
                        }));
                    }
                    else if (cartMode == GBA_CART) {
                        modeTextBox.Invoke((MethodInvoker)(() => {
                            modeTextBox.Text = "GBA";
                        }));
                    }
                }
            }
            else {
                comPortTextBox.BackColor = Color.FromArgb(255, 192, 192);
            }
        }

        // Close port button
        private void closeportbutton_Click(object sender, EventArgs e) {
            if (comConnected == true) {
                int comPortInt = Convert.ToInt32(comPortTextBox.Text);
                comPortInt--;
                headerTextBox.Text = "";
                statuslabel.Text = "";

                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }

            headerRead = false;
            progress = 0;
            backgroundWorker1.ReportProgress(0);
            
            firmwareText.Visible = false;
        }

        // Stop button
        private void stopbutton_Click(object sender, EventArgs e) {
            commandReceived = 0;
            cancelOperation = 1;

            System.Threading.Thread.Sleep(250);

            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            int baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;

            Program.RS232_CloseComport(comPortInt);
            Program.RS232_OpenComport(comPortInt, baudInt, "8N1");
        }


        // Read header button
        private void readheaderbutton_Click(object sender, EventArgs e) {
            if (comConnected == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = READHEADER;
                statuslabel.Text = "";
            }
        }

        // Read rom button
        private void readrombutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = READROM;
                statuslabel.Text = "Reading ROM...";
            }
        }

        // Save ram button 
        private void saverambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);
                
                if (Program.check_if_file_exists() == 1) {
                    DialogResult dialogResult = MessageBox.Show("This will erase the save game from your PC.\nPress Yes to continue or No to abort.", "Confirm Save", MessageBoxButtons.YesNo);
                    if (dialogResult == DialogResult.Yes) {
                        commandReceived = SAVERAM;
                        statuslabel.Text = "Backing up Save...";
                    }
                    else if (dialogResult == DialogResult.No) {
                        statuslabel.Text = "Backing up Save cancelled";
                    }
                }
                else {
                    commandReceived = SAVERAM;
                    statuslabel.Text = "Backing up Save...";
                }
            }
        }

        // Write ram button
        private void writerambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                if (Program.check_if_file_exists() == 1) {
                    DialogResult dialogResult = MessageBox.Show("This will erase the save game from your Gameboy / Gameboy Advance cart\nPress Yes to continue or No to abort.", "Confirm Write", MessageBoxButtons.YesNo);
                    if (dialogResult == DialogResult.Yes) {
                        commandReceived = WRITERAM;
                        statuslabel.Text = "Restoring Save...";
                    }
                    else if (dialogResult == DialogResult.No) {
                        statuslabel.Text = "Restoring Save cancelled";
                    }
                }
                else {
                    statuslabel.Text = "Save file not found.";
                }
            }
        }

        // Write ROM button
        private void writerombutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && writeRomSelected == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = WRITEROM;
                if (writeRomCartType == 1 || writeRomCartType == 3 || writeRomCartType == 100) {
                    statuslabel.Text = "Writing ROM...";
                }
                else {
                    statuslabel.Text = "Erasing Flash...";
                }
            }
        }
        


        // --------------------- CART MENU ------------------

        // GB/GBA cart info
        private void specifyCartInfoMenuItem_Click(object sender, EventArgs e) {
            // Read header if not read yet
            if (comConnected == true && headerRead == false) {
                if (commandReceived == 0) {
                    progress = 0;
                    backgroundWorker1.ReportProgress(0);

                    commandReceived = READHEADER;
                    headerRead = true;
                }
            }
            
            if (comConnected == true && headerRead == true && cartMode == GB_CART) {
                GB_Specify_Info gbSpecifyInfoForm = new GB_Specify_Info();
                gbSpecifyInfoForm.ShowDialog();

                // Update header
                headerTextBox.Invoke((MethodInvoker)(() => {
                    headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                }));
            }
            else if (comConnected == true && headerRead == true && cartMode == GBA_CART) {
                GBA_Specify_Info gbaSpecifyInfoForm = new GBA_Specify_Info();
                gbaSpecifyInfoForm.ShowDialog();

                // Update header
                headerTextBox.Invoke((MethodInvoker)(() => {
                    headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                }));
            }
        }

        // Select ROM file to Write
        private void selectRomFiletoWriteMenuItem_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && writeRomCartType >= 1) {
                int gbxcartPcbVersion = Program.request_value(READ_PCB_VERSION);
                if (gbxcartPcbVersion == PCB_1_0) {
                    System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                }
                else {
                    DialogResult result = openFileDialog1.ShowDialog();
                    if (result == DialogResult.OK) {

                        FileInfo fileSelected = new FileInfo(openFileDialog1.FileName);
                        writeRomSize = (UInt32) fileSelected.Length;

                        if (fileSelected.Length <= writeRomCartSize) {
                            writeRomFileName = openFileDialog1.FileName;
                            headerTokens[3] = "ROM File: " + writeRomFileName;
                            writeRomSelected = true;
                            writerombutton.ForeColor = Color.Black;

                            headerTextBox.Invoke((MethodInvoker)(() => {
                                headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                            }));
                        }
                        else {
                            System.Windows.Forms.MessageBox.Show("File size is too large for Flash cart.");
                        }
                    }
                }
            }
            else {
                System.Windows.Forms.MessageBox.Show("Please specify a Flash cart first in Specify Cart.");
            }
        }

        // Erase RAM button
        private void eraseRAMToolStripMenuItem_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                DialogResult dialogResult = MessageBox.Show("This will erase the save game from your Gameboy / Gameboy Advance cart.\nPress Yes to continue or No to abort.", "Confirm Save", MessageBoxButtons.YesNo);
                if (dialogResult == DialogResult.Yes) {
                    commandReceived = ERASERAM;
                    statuslabel.Text = "Erasing RAM...";
                }
            }
            else {
                System.Windows.Forms.MessageBox.Show("Please read the Header first.");
            }
        }

        // --------------------- HELP MENU ------------------

        // Check for updates
        private void checkForUpdatesToolStripMenuItem_Click(object sender, EventArgs e) {
            UpdateCheck updateCheckForm = new UpdateCheck();
            updateCheckForm.ShowDialog();
        }

        // Manual - open in browser
        private void manualToolStripMenuItem_Click(object sender, EventArgs e) {
            System.Diagnostics.Process.Start("http://www.insidegadgets.com/updates/GBxCart_RW_Manual.pdf");
        }
        
        private void modeTextBox_SelectedIndexChanged(object sender, EventArgs e) {
            if (modeTextBox.Text == "GB/GBC") {
                cartMode = GB_CART;
            }
            else if (modeTextBox.Text == "GBA") {
                cartMode = GBA_CART;
            }
        }
    }
}
