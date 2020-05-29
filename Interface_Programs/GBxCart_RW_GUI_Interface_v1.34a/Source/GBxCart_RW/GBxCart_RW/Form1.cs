/*

GBxCart RW - GUI  Interface
Version : 1.34
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 29 / 05 / 2020

GBxCart RW allows you to dump your Gameboy / Gameboy Colour / Gameboy Advance games ROM, save the RAM and write to the RAM.

*/

#define WINDOWS_7_BUILD

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
        public const int GB_CART = 1;
        public const int GBA_CART = 2;
        const char CART_MODE = 'C';

        public const char VOLTAGE_3_3V = '3';
        public const char VOLTAGE_5V = '5';

        const int READROM = 1;
        public const int SAVERAM = 2;
        const int WRITERAM = 3;
        const int READHEADER = 4;
        const int WRITEROM = 5;
        const int ERASERAM = 6;
        const int ERASEROM = 7;

        public const int PCB_1_0 = 1;
        public const int PCB_1_1 = 2;
        public const int PCB_1_2 = 3;
        public const int PCB_1_3 = 4;
        public const int GBXMAS = 90;
        public const int MINI_PCB_1_0 = 100;
        public const char READ_PCB_VERSION = 'h';

        bool formMinimised = false;
        bool comConnected = false;
        public static int commandReceived = 0;
        bool headerRead = false;
        int cancelOperation = 0;
        UInt32 progress = 0;
        UInt32 progressPrevious = 0;
        int progressPreviousCounter = 0;
        int progressStalled = 0;

        public static string[] headerTokens = {"", "", "", "", ""};
        string writeRomFileName;
        bool writeRomSelected = false;
        public static bool saveAsNewFile = false;
        string writeSaveFileName;
        int alwaysAddDateTimeToSave = 0;
        int promptForRestoreSaveFile = 0;
        int reReadCartHeader = 0;
        public static int writeRomCartType = 0;
        public static UInt32 writeRomCartSize = 0;
        public static UInt32 writeRomSize = 0;
        public static int cartMode = GBA_CART;
        public static int gbxcartPcbVersion = 0;
        public static int chipEraseSelected = 1;

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

            statuslabel.Text = "";

            comPortTextBox.Text = Convert.ToString(Program.read_config(1));
            baudtextBox.Text = Convert.ToString(Program.read_config(2));
            alwaysAddDateTimeToSave = Program.read_config(3);
            promptForRestoreSaveFile = Program.read_config(4);
            reReadCartHeader = Program.read_config(5);

            // Update menu text if set
            if (alwaysAddDateTimeToSave == 1) {
                alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Text = "Always add date/time to backed up Save Game files: Yes";
            }
            if (promptForRestoreSaveFile == 1) {
                promptForFileToolStripMenuItem.Text = "Always prompt for Save Game file when restoring: Yes";
            }
            if (reReadCartHeader == 1) {
                alwaysRereadHeaToolStripMenuItem.Text = "Always re-read cart info when backing up save: Yes";
            }

            // Load default directory
            this.directoryNameToolStripMenuItem.Text = @AppDomain.CurrentDomain.BaseDirectory;
            Program.update_current_folder(@AppDomain.CurrentDomain.BaseDirectory);

            // Check if directory file exists
            if (File.Exists(@"config-folder.ini")) {
                using (StreamReader configFolderFile = new StreamReader(@"config-folder.ini")) {
                    while (configFolderFile.Peek() >= 0) {
                        this.directoryNameToolStripMenuItem.Text = configFolderFile.ReadLine();
                        Program.update_current_folder(this.directoryNameToolStripMenuItem.Text);
                        persistanceToolStripMenuItem.Text = "Remember: Yes";
                    }
                }
            }
        }

        private void Form1_Load(object sender, EventArgs e) {

        }
        
        // If minimised and performing a task, notify user once complete
        protected override void WndProc(ref Message m) {
            if (m.Msg == 0x0112) { // WM_SYSCOMMAND
                if (m.WParam == new IntPtr(0xF020)) { // SC_MINIMIZE
                    formMinimised = true;
                    notifyIcon1.Visible = true;
                }
                else if (m.WParam == new IntPtr(0xF120)) { // SC_RESTORE
                    formMinimised = false;
                    notifyIcon1.Visible = false;
                }
                m.Result = new IntPtr(0);
            }
            base.WndProc(ref m);
        }


        // Progress bar
        void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e) {
            while (true) {                
                double progress_percent = progress;

                if (progress_percent >= 100) {
                    progress_percent = 100;
                }

                backgroundWorker1.ReportProgress(Convert.ToInt32(progress_percent));
                System.Threading.Thread.Sleep(100);
                //Console.WriteLine(progress);

                // Check if progress has stalled after 30 seconds
                if (commandReceived != 0 && comConnected == true) {
                    if (progressPreviousCounter >= 300) {
                        if (progress != progressPrevious) {
                            progressPrevious = progress;
                            progressPreviousCounter = 0;
                        }
                        else if (progressStalled == 0) {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = statuslabel.Text + " Stalled";
                            }));
                            progressStalled = 1;

                            if (formMinimised == true) {
                                notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                                notifyIcon1.BalloonTipText = "Stalled";
                                notifyIcon1.BalloonTipTitle = "GBxCart RW";
                                notifyIcon1.ShowBalloonTip(4000);
                            }
                            //Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            //TopMost = true;
                            //System.Windows.Forms.MessageBox.Show("Progress has stalled. Please unplug GBxCart RW, re-seat your game cartridge and try again.");

                            TopMostMessageBox.Show("Progress has stalled. Please unplug GBxCart RW, re-seat your game cartridge and try again. This program will close when you press Ok.", "GBxCart RW Stalled", MessageBoxButtons.OK);
                            //MessageBox.Show("Progress has stalled. Please unplug GBxCart RW, re-seat your game cartridge and try again.", "GBxCart RW Stalled", MessageBoxButtons.OK, MessageBoxIcon.None,
                            // MessageBoxDefaultButton.Button1, (MessageBoxOptions)0x40000);  // MB_TOPMOST
                            Application.Exit();
                        }
                    }
                    progressPreviousCounter++;
                }
                else {
                    progressPrevious = 0;
                    progressPreviousCounter = 0;
                    progressStalled = 0;
                }
            }
        }

        // Update the progress bar
        void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            // The progress percentage is a property of e
            progressBar1.Value = e.ProgressPercentage;

#if WINDOWS_7_BUILD
            try {
                TaskbarProgress.SetValue(this.Handle, e.ProgressPercentage, 100);

                if (progressStalled == 1) {
                    TaskbarProgress.SetState(this.Handle, TaskbarProgress.TaskbarStates.Error);
                }
                else {
                    TaskbarProgress.SetState(this.Handle, TaskbarProgress.TaskbarStates.Normal);
                }
            }
            catch (ObjectDisposedException a) {
                Console.WriteLine("Caught: {0}", a.Message);
            }
#endif
        }

        // Perform the reading/writing/header read in the background
        void backgroundWorker2_DoWork(object sender, DoWorkEventArgs e) {
            while (true) {

                // Before doing any operations, check if we are still connected and read the mode we are in
                if (commandReceived != 0) {
                    int checkCom = Program.read_firmware_version();
                    int checkComCounter = 0;
                    progressStalled = 0;
                    //Console.Write(checkCom);

                    // Read which mode we are in
                    /*if (checkCom >= 1) {
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
                    }*/

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

                            // Change icon/buttons
                            usbStatusPictureOff.Invoke((MethodInvoker)(() => {
                                usbStatusPictureOff.Visible = true;
                            }));
                            usbStatusPictureOn.Invoke((MethodInvoker)(() => {
                                usbStatusPictureOn.Visible = false;
                            }));
                            openportbutton.Invoke((MethodInvoker)(() => {
                                openportbutton.Visible = true;
                            }));
                            closeportbutton.Invoke((MethodInvoker)(() => {
                                closeportbutton.Visible = false;
                            }));

                            break;
                        }
                    }
                }

                if (commandReceived == READROM) { // Read ROM
                    Program.read_rom(cartMode, ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Reading ROM... Finished";
                        }));

                        if (formMinimised == true) {
                            notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                            notifyIcon1.BalloonTipText = "Reading ROM Finished";
                            notifyIcon1.BalloonTipTitle = "GBxCart RW";
                            notifyIcon1.ShowBalloonTip(4000);
                        }
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Reading ROM... Cancelled";
                        }));
                    }   
                }
                else if (commandReceived == SAVERAM) { // Save RAM
                    Program.read_ram(saveAsNewFile, ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Backing up Save... Finished";
                        }));

                        if (formMinimised == true) {
                            notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                            notifyIcon1.BalloonTipText = "Backing up Save Finished";
                            notifyIcon1.BalloonTipTitle = "GBxCart RW";
                            notifyIcon1.ShowBalloonTip(4000);
                        }
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Backing up Save... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == WRITERAM) { // Write RAM
                    Program.write_ram(writeSaveFileName, ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Restoring Save... Finished";
                        }));

                        if (formMinimised == true) {
                            notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                            notifyIcon1.BalloonTipText = "Restoring Save Finished";
                            notifyIcon1.BalloonTipTitle = "GBxCart RW";
                            notifyIcon1.ShowBalloonTip(4000);
                        }
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Restoring Save... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == WRITEROM) { // Write ROM
                   
                    // First erase the flash chip if needed or if selected
                    if (chipEraseSelected == 1) {
                        if (writeRomCartType == 2 || writeRomCartType == 4 || writeRomCartType == 5 || writeRomCartType == 6 || writeRomCartType == 7 || writeRomCartType == 8 || writeRomCartType == 9 || writeRomCartType == 10 || writeRomCartType == 14 || writeRomCartType == 17 || writeRomCartType == 19) {
                            Program.erase_rom(writeRomCartType, ref progress, ref cancelOperation);
                            Console.WriteLine("ROm");
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
                    }

                    // Program the flash chip
                    if (cancelOperation != 1) {
                        progressPrevious = 0;
                        progressPreviousCounter = 0;

                        Program.write_rom(writeRomFileName, writeRomCartType, writeRomSize, ref progress, ref cancelOperation);
                        commandReceived = 0;
                        System.Threading.Thread.Sleep(500);

                        if (cancelOperation == 0) {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM... Finished";
                            }));

                            if (formMinimised == true) {
                                notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                                notifyIcon1.BalloonTipText = "Writing ROM Finished";
                                notifyIcon1.BalloonTipTitle = "GBxCart RW";
                                notifyIcon1.ShowBalloonTip(4000);
                            }
                        }
                        else if (cancelOperation == 1) {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM... Cancelled";
                            }));
                        }
                        else if (cancelOperation == 2) {
                            statuslabel.Invoke((MethodInvoker)(() => {
                                statuslabel.Text = "Writing ROM... Stalled";
                            }));
                            progressStalled = 1;

                            if (formMinimised == true) {
                                notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                                notifyIcon1.BalloonTipText = "Writing ROM Stalled";
                                notifyIcon1.BalloonTipTitle = "GBxCart RW";
                                notifyIcon1.ShowBalloonTip(4000);
                            }
                        }
                    }
                }
                else if (commandReceived == ERASERAM) { // Erase RAM
                    Program.erase_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Erasing Save... Finished";
                        }));

                        if (formMinimised == true) {
                            notifyIcon1.BalloonTipIcon = ToolTipIcon.Info;
                            notifyIcon1.BalloonTipText = "Erasing Save Finished";
                            notifyIcon1.BalloonTipTitle = "GBxCart RW";
                            notifyIcon1.ShowBalloonTip(4000);
                        }
                    }
                    else {
                        statuslabel.Invoke((MethodInvoker)(() => {
                            statuslabel.Text = "Erasing Save... Cancelled";
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
                    commandReceived = 0;

                    headerTextBox.Invoke((MethodInvoker)(() => {
                        //headerTextBox.Text = headerText;
                        headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                    }));
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
            writeRomSelected = false;
            writerombutton.ForeColor = Color.Gray;

            // If port open, close it, we will open it again below
            if (comConnected == true) {
                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }
            else {
                // Try opening the port
                if (Program.RS232_OpenComport(comPortInt, baudInt, "8N1") == 0) { // Port opened
                    Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);

                    // See if device responds correctly
                    Program.set_mode('0');
                    int cartridgeMode = Program.request_value(CART_MODE);

                    // Responded correctly
                    if (cartridgeMode == GB_CART || cartridgeMode == GBA_CART) {
                        comConnected = true;
                    }
                    Program.RS232_CloseComport(comPortInt);
                }

                if (comConnected == false) {
                    for (int x = 0; x < 16; x++) {
                        if (Program.RS232_OpenComport(x, baudInt, "8N1") == 0) { // Port opened
                            comPortInt = x;
                            Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);

                            // See if device responds correctly
                            Program.set_mode('0');
                            int cartridgeMode = Program.request_value(CART_MODE);
                            Program.RS232_CloseComport(comPortInt);

                            if (cartridgeMode == GB_CART || cartridgeMode == GBA_CART) {
                                comPortTextBox.Text = Convert.ToString(comPortInt + 1);
                                break;
                            }
                        }
                    }
                }
            }
            
            // Successful open
            if (Program.RS232_OpenComport(comPortInt, baudInt, "8N1") == 0) {
                comPortTextBox.BackColor = Color.FromArgb(192, 255, 192);
                comConnected = true;
                commandReceived = 0;

                headerRead = false;
                progress = 0;
                backgroundWorker1.ReportProgress(0);
                
                // Update config
                Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);

                // Read PCB version
                gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);

                // If v1.3, set the voltage
                if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
                    if (cartMode == GB_CART) {
                        Program.set_mode(VOLTAGE_5V);
                    }
                    else {
                        Program.set_mode(VOLTAGE_3_3V);
                    }
                }

                // Read which mode we are in (for v1.2 PCB and below)
                if (gbxcartPcbVersion <= PCB_1_2) {
                    cartMode = Program.read_cartridge_mode();
                    if (cartMode == GB_CART) {
                        gbMode.Invoke((MethodInvoker)(() => {
                            gbMode.Checked = true;
                        }));
                    }
                    else if (cartMode == GBA_CART) {
                        gbaMode.Invoke((MethodInvoker)(() => {
                            gbaMode.Checked = true;
                        }));
                    }
                }

                // If GBxCart Mini, switch to GB mode if in GBA mode
                if (gbxcartPcbVersion == MINI_PCB_1_0) {
                    gbMode.Invoke((MethodInvoker)(() => {
                        gbMode.Checked = true;
                    }));
                    gbaMode.Invoke((MethodInvoker)(() => {
                        gbaMode.Checked = false;
                    }));
                }

                // Read device firmware
                int firmwareVersion = Program.read_firmware_version();
                if (firmwareVersion >= 1) {
                    firmwareText.Text = "R" + firmwareVersion;
                    firmwareText.Visible = true;
                }

                // Change icon/buttons
                usbStatusPictureOff.Visible = false;
                usbStatusPictureOn.Visible = true;
                openportbutton.Visible = false;
                closeportbutton.Visible = true;
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

            // Change icon/buttons
            usbStatusPictureOff.Visible = true;
            usbStatusPictureOn.Visible = false;
            openportbutton.Visible = true;
            closeportbutton.Visible = false;

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

                if (cartMode == GB_CART && headerTokens[4] != "Header Checksum: OK") {
                    System.Windows.Forms.MessageBox.Show("Header Checksum has failed. Can't proceed as corruption could occur.");
                }
                else {
                    commandReceived = READROM;
                    statuslabel.Text = "Reading ROM...";
                }    
            }
        }

        // Save ram button 
        private void saverambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && commandReceived == 0 && reReadCartHeader == 1) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = READHEADER;
                statuslabel.Text = "";

                while (commandReceived == READHEADER) {
                    System.Threading.Thread.Sleep(200);
                }
                headerRead = true;
            }
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                if (cartMode == GB_CART && headerTokens[4] != "Header Checksum: OK") {
                    System.Windows.Forms.MessageBox.Show("Header Checksum has failed. Can't proceed as corruption could occur.");
                }
                else {
                    if (alwaysAddDateTimeToSave == 1) {
                        saveAsNewFile = true;
                        commandReceived = SAVERAM;
                        statuslabel.Text = "Backing up Save...";
                    }
                    else {
                        saveAsNewFile = false;
                        if (Program.check_if_file_exists() == 1) {
                            SaveOptions SaveOptionsForm = new SaveOptions();
                            SaveOptionsForm.saveNameExisting.Text = headerTokens[0];
                            SaveOptionsForm.ShowDialog();

                            if (commandReceived == SAVERAM) {
                                statuslabel.Text = "Backing up Save...";
                            }
                            else {
                                statuslabel.Text = "Backing up Save cancelled";
                            }
                        }
                        else {
                            commandReceived = SAVERAM;
                            statuslabel.Text = "Backing up Save...";
                        }
                    }
                }              
            }
        }

        // Write ram button
        private void writerambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                if (cartMode == GB_CART && headerTokens[4] != "Header Checksum: OK") {
                    System.Windows.Forms.MessageBox.Show("Header Checksum has failed. Can't proceed as corruption could occur.");
                }
                else {
                    bool promptToRestore = false;
                    if (promptForRestoreSaveFile == 1) { // Select a save file
                        openFileDialog2.InitialDirectory = this.directoryNameToolStripMenuItem.Text;
                        DialogResult result = openFileDialog2.ShowDialog();
                        if (result == DialogResult.OK) {
                            FileInfo fileSelected = new FileInfo(openFileDialog2.FileName);
                            writeSaveFileName = openFileDialog2.FileName;
                            promptToRestore = true;
                        }
                    }
                    else { // Use the default save file name
                        writeSaveFileName = "";
                        if (Program.check_if_file_exists() == 1) {
                            promptToRestore = true;
                        }
                        else {
                            statuslabel.Text = "Save file not found.";
                        }
                    }

                    // Prompt to overwrite save
                    if (promptToRestore == true) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        DialogResult dialogResult = MessageBox.Show("This will erase the save game from your Gameboy / Gameboy Advance cart\nPress Yes to continue or No to abort.", "Confirm Write", MessageBoxButtons.YesNo);
                        if (dialogResult == DialogResult.Yes) {
                            commandReceived = WRITERAM;
                            statuslabel.Text = "Restoring Save...";
                        }
                        else if (dialogResult == DialogResult.No) {
                            statuslabel.Text = "Restoring Save cancelled";
                        }
                    }
                }
            }
        }

        // Write ROM button
        private void writerombutton_Click(object sender, EventArgs e) {
            if (comConnected == true && writeRomSelected == true && commandReceived == 0) {

                int gbxcartPcbVersion = Program.request_value(READ_PCB_VERSION);
               
                // Do another check for 3.3V Flash carts if with GBxCart RW (not Mini)
                int writingROM = 1;
                if ((writeRomCartType == 4 || writeRomCartType == 5 || writeRomCartType == 10 || writeRomCartType == 11 || writeRomCartType == 12 || writeRomCartType == 13) && gbxcartPcbVersion != Form1.MINI_PCB_1_0) {
                    int cartVoltage = Program.read_cartridge_mode();
                    if (cartVoltage == 1) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GBA.");
                        writingROM = 0;
                    }
                }

                // Write the ROM
                if (writingROM == 1) {
                    progress = 0;
                    backgroundWorker1.ReportProgress(0);
                    commandReceived = WRITEROM;

                    // Check if file size is more than 16MB for iG 32MB cart, if so, chip erase
                    if (writeRomCartType == 105 && writeRomSize > 0x1000000) {
                        writeRomCartType = 106;
                    }

                    // Depending on the cart, erase whole flash first or start writing (and erase sector by sector)
                    if (writeRomCartType == 100 || writeRomCartType == 101 || writeRomCartType == 102 || writeRomCartType == 103 || chipEraseSelected == 0 || writeRomCartType == 18) {
                        statuslabel.Text = "Writing ROM...";
                    }
                    else {
                        statuslabel.Text = "Erasing Flash...";
                    }
                }
            }
        }



        // --------------------- CART MENU ------------------

        // GB/GBA cart info
        private void specifyCartInfoMenuItem_Click(object sender, EventArgs e) {
            // Read header if not read yet
            /*if (comConnected == true) {// && headerRead == false) {
                if (commandReceived == 0) {
                    progress = 0;
                    backgroundWorker1.ReportProgress(0);

                    commandReceived = READHEADER;
                    headerRead = true;
                }
                headerRead = true;
            }
            else {
                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                MessageBox.Show("Please open the COM port.");
            }*/

            if (comConnected != true) {
                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                MessageBox.Show("Please open the COM port.");
            }

            //if (comConnected == true && headerRead == true && cartMode == GB_CART) {
            if (comConnected == true && cartMode == GB_CART) {
                GB_Specify_Info gbSpecifyInfoForm = new GB_Specify_Info();
                gbSpecifyInfoForm.ShowDialog();

                // Update header
                headerTextBox.Invoke((MethodInvoker)(() => {
                    headerTextBox.Text = headerTokens[0] + "\n" + headerTokens[1] + "\n" + headerTokens[2] + "\n" + headerTokens[3] + "\n" + headerTokens[4];
                }));
            }
            else if (comConnected == true && cartMode == GBA_CART) {
            //else if (comConnected == true && headerRead == true && cartMode == GBA_CART) {
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
            if (comConnected == true && writeRomCartType >= 1) {
            //if (comConnected == true && headerRead == true && writeRomCartType >= 1) {
                int gbxcartPcbVersion = Program.request_value(READ_PCB_VERSION);
                if (gbxcartPcbVersion == PCB_1_0) {
                    Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                    System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                }
                else {
                    openFileDialog1.InitialDirectory = this.directoryNameToolStripMenuItem.Text;
                    openFileDialog1.FileName = "*.gb";

                    DialogResult result = openFileDialog1.ShowDialog();
                    if (result == DialogResult.OK) {

                        FileInfo fileSelected = new FileInfo(openFileDialog1.FileName);
                        writeRomSize = (UInt32) fileSelected.Length;

                        // Store the last rom directory opened
                        this.directoryNameToolStripMenuItem.Text = Path.GetDirectoryName(openFileDialog1.FileName);
                        
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
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            MessageBox.Show("ROM File is too large for Flash cart.");
                        }
                    }
                }
            }
            else {
                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                MessageBox.Show("Please specify a Flash cart first in Cart > Specify Cart Info.");
            }
        }

        // Erase RAM button
        private void eraseRAMToolStripMenuItem_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                DialogResult dialogResult = MessageBox.Show("This will erase the save game from your Gameboy / Gameboy Advance cart.\nPress Yes to continue or No to abort.", "Confirm Erase", MessageBoxButtons.YesNo);
                if (dialogResult == DialogResult.Yes) {
                    commandReceived = ERASERAM;
                    statuslabel.Text = "Erasing RAM...";
                }
            }
            else {
                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                MessageBox.Show("Please read the Header first.");
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
            /*if (modeTextBox.Text == "GB/GBC") {
                cartMode = GB_CART;
            }
            else if (modeTextBox.Text == "GBA") {
                cartMode = GBA_CART;
            }*/
        }
        
        private void label2_Click(object sender, EventArgs e) {

        }
        

        private void locationToolStripMenuItem_Click(object sender, EventArgs e) {

        }

        private void directoryNameToolStripMenuItem_Click(object sender, EventArgs e) {
            var folderSelect = new Functions.FolderSelectDialog();
            folderSelect.Title = "Select a folder";
            folderSelect.InitialDirectory = @AppDomain.CurrentDomain.BaseDirectory;
            if (folderSelect.ShowDialog(IntPtr.Zero)) {
                //Console.WriteLine(folderSelect.FileName);
                Program.update_current_folder(folderSelect.FileName);
                this.directoryNameToolStripMenuItem.Text = folderSelect.FileName;

                // If remembered, save the new folder
                if (persistanceToolStripMenuItem.Text == "Remember: Yes") {
                    using (StreamWriter configFolderFile = new StreamWriter(@"config-folder.ini")) {
                        configFolderFile.WriteLine(this.directoryNameToolStripMenuItem.Text);
                    }
                }
            }
        }

        private void persistanceToolStripMenuItem_Click(object sender, EventArgs e) {
            if (persistanceToolStripMenuItem.Text == "Remember: Yes") {
                persistanceToolStripMenuItem.Text = "Remember: No";
                if (File.Exists(@"config-folder.ini")) { // Delete file if exists
                    File.Delete(@"config-folder.ini");
                }
            }
            else {
                persistanceToolStripMenuItem.Text = "Remember: Yes";
                using (StreamWriter configFolderFile = new StreamWriter(@"config-folder.ini")) {
                    configFolderFile.WriteLine(this.directoryNameToolStripMenuItem.Text);
                }
            }
        }

        private void alwaysAddDatetimeToSaveGamesYesToolStripMenuItem_Click(object sender, EventArgs e) {
            if (alwaysAddDateTimeToSave == 0) {
                alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Text = "Always add date/time to backed up Save Game files: Yes";
                alwaysAddDateTimeToSave = 1;
            }
            else {
                alwaysAddDatetimeToSaveGamesYesToolStripMenuItem.Text = "Always add date/ time to backed up Save Game files: No";
                alwaysAddDateTimeToSave = 0;
            }

            // Update config
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);
        }

        private void promptForFileToolStripMenuItem_Click(object sender, EventArgs e) {
            if (promptForRestoreSaveFile == 0) {
                promptForFileToolStripMenuItem.Text = "Always prompt for Save Game file when restoring: Yes";
                promptForRestoreSaveFile = 1;
            }
            else {
                promptForFileToolStripMenuItem.Text = "Always prompt for Save Game file when restoring: No";
                promptForRestoreSaveFile = 0;
            }

            // Update config
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);
        }

        private void optionsToolStripMenuItem_Click(object sender, EventArgs e) {

        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e) {

        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e) {
            cartMode = GB_CART;
        }

        private void gbaMode_CheckedChanged(object sender, EventArgs e) {
            cartMode = GBA_CART;
        }

        private void pictureBox1_Click(object sender, EventArgs e) {

        }

        private void pictureBox1_Click_1(object sender, EventArgs e) {

        }

        private void alwaysRereadHeaToolStripMenuItem_Click(object sender, EventArgs e) {
            if (reReadCartHeader == 0) {
                alwaysRereadHeaToolStripMenuItem.Text = "Always re-read cart info when backing up save: Yes";
                reReadCartHeader = 1;
            }
            else {
                alwaysRereadHeaToolStripMenuItem.Text = "Always re-read cart info when backing up save: No";
                reReadCartHeader = 0;
            }

            // Update config
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            Program.update_config(comPortInt, baudInt, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e) {

        }
    }

    // Code from: https://www.codeproject.com/Articles/18612/TopMost-MessageBox
    static public class TopMostMessageBox {
        static public DialogResult Show(string message) {
            return Show(message, string.Empty, MessageBoxButtons.OK);
        }

        static public DialogResult Show(string message, string title) {
            return Show(message, title, MessageBoxButtons.OK);
        }

        static public DialogResult Show(string message, string title,
            MessageBoxButtons buttons) {
            // Create a host form that is a TopMost window which will be the 
            // parent of the MessageBox.
            Form topmostForm = new Form();
            // We do not want anyone to see this window so position it off the 
            // visible screen and make it as small as possible
            topmostForm.Size = new System.Drawing.Size(1, 1);
            topmostForm.StartPosition = FormStartPosition.Manual;
            System.Drawing.Rectangle rect = SystemInformation.VirtualScreen;
            topmostForm.Location = new System.Drawing.Point(rect.Bottom + 10,
                rect.Right + 10);
            topmostForm.Show();
            // Make this form the active form and make it TopMost
            topmostForm.Focus();
            topmostForm.BringToFront();
            topmostForm.TopMost = true;
            // Finally show the MessageBox with the form just created as its owner
            DialogResult result = MessageBox.Show(topmostForm, message, title,
                buttons);
            topmostForm.Dispose(); // clean it up all the way

            return result;
        }
    }

#if WINDOWS_7_BUILD
    // Code from: https://stackoverflow.com/questions/1295890/windows-7-progress-bar-in-taskbar-in-c
    public static class TaskbarProgress {
        public enum TaskbarStates {
            NoProgress = 0,
            Indeterminate = 0x1,
            Normal = 0x2,
            Error = 0x4,
            Paused = 0x8
        }

        [ComImport()]
        [Guid("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")]
        [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        private interface ITaskbarList3 {
            // ITaskbarList
            [PreserveSig]
            void HrInit();
            [PreserveSig]
            void AddTab(IntPtr hwnd);
            [PreserveSig]
            void DeleteTab(IntPtr hwnd);
            [PreserveSig]
            void ActivateTab(IntPtr hwnd);
            [PreserveSig]
            void SetActiveAlt(IntPtr hwnd);

            // ITaskbarList2
            [PreserveSig]
            void MarkFullscreenWindow(IntPtr hwnd, [MarshalAs(UnmanagedType.Bool)] bool fFullscreen);

            // ITaskbarList3
            [PreserveSig]
            void SetProgressValue(IntPtr hwnd, UInt64 ullCompleted, UInt64 ullTotal);
            [PreserveSig]
            void SetProgressState(IntPtr hwnd, TaskbarStates state);
        }

        [ComImport()]
        [Guid("56fdf344-fd6d-11d0-958a-006097c9a090")]
        [ClassInterface(ClassInterfaceType.None)]
        private class TaskbarInstance {
        }

        private static ITaskbarList3 taskbarInstance = (ITaskbarList3)new TaskbarInstance();
        private static bool taskbarSupported = Environment.OSVersion.Version >= new Version(6, 1);

        public static void SetState(IntPtr windowHandle, TaskbarStates taskbarState) {
            if (taskbarSupported) taskbarInstance.SetProgressState(windowHandle, taskbarState);
        }

        public static void SetValue(IntPtr windowHandle, double progressValue, double progressMax) {
            if (taskbarSupported) taskbarInstance.SetProgressValue(windowHandle, (ulong)progressValue, (ulong)progressMax);
        }
    }
#endif

}
