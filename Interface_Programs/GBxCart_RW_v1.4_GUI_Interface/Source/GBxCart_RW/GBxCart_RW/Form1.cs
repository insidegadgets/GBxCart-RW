/*

GBxCart RW - GUI  Interface
Version : 1.1
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 4 / 04 / 2017

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
using System.Runtime.InteropServices;     // DLL support
using System.IO.Ports;

namespace GBxCart_RW
{
    public partial class Form1 : Form
    {
        const int GB_CART = 1;
        const int GBA_CART = 2;

        const int READROM = 1;
        const int SAVERAM = 2;
        const int WRITERAM = 3;
        const int READHEADER = 4;

        bool comConnected = false;
        //int checkComCounter = 0;
       // int comFailedCounter = 0;
        int commandReceived = 0;
        bool headerRead = false;
        int cancelOperation = 0;
        UInt32 progress = 0;

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
        void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            // The progress percentage is a property of e
            progressBar1.Value = e.ProgressPercentage;
        }
        
        // Perform the reading/writing/header read in the background
        void backgroundWorker2_DoWork(object sender, DoWorkEventArgs e)
        {
            while (true) {

                // Before doing any operations, check if we are still connected and read the mode we are in
                if (commandReceived != 0) {
                    int checkCom = Program.read_firmware_version();
                    int checkComCounter = 0;
                    Console.Write(checkCom);

                    // Read which mode we are in
                    if (checkCom >= 1) {
                        int cartMode = Program.read_cartridge_mode();
                        if (cartMode == GB_CART) {
                            modeText.Text = "GB/GBC";
                        }
                        else if (cartMode == GBA_CART) {
                            modeText.Text = "GBA";
                        }
                    }

                    while (checkCom == 0) { // Not connected, check it a few times
                        checkCom = Program.read_firmware_version();
                        System.Threading.Thread.Sleep(100);
                        checkComCounter++;
                        Console.Write(checkCom);

                        if (checkComCounter >= 3) { // Close the COM port
                            commandReceived = 0;

                            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
                            comPortInt--;

                            Program.RS232_CloseComport(comPortInt);
                            comConnected = false;

                            comPortTextBox.Invoke((MethodInvoker)(() => {
                                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                            }));
                            
                            readromlabel.Invoke((MethodInvoker)(() => {
                                readromlabel.Text = "Device disconnected";
                                readromlabel.Visible = true;
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
                        readromlabel.Invoke((MethodInvoker)(() => {
                            readromlabel.Text = "Reading ROM... Finished";
                        }));
                    }
                    else {
                        readromlabel.Invoke((MethodInvoker)(() => {
                            readromlabel.Text = "Reading ROM... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == SAVERAM) { // Save RAM
                    Program.read_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        saveramlabel.Invoke((MethodInvoker)(() => {
                            saveramlabel.Text = "Saving RAM...... Finished";
                        }));
                    }
                    else {
                        saveramlabel.Invoke((MethodInvoker)(() => {
                            saveramlabel.Text = "Saving RAM...... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == WRITERAM) { // Write RAM
                    Program.write_ram(ref progress, ref cancelOperation);
                    commandReceived = 0;
                    System.Threading.Thread.Sleep(500);

                    if (cancelOperation == 0) {
                        writeramlabel.Invoke((MethodInvoker)(() => {
                            writeramlabel.Text = "Writing RAM...... Finished";
                        }));
                    }
                    else {
                        writeramlabel.Invoke((MethodInvoker)(() => {
                            writeramlabel.Text = "Writing RAM...... Cancelled";
                        }));
                    }
                }
                else if (commandReceived == READHEADER) { // Read Header
                    headerRead = true;
                    Int32 textLength = 0;
                    IntPtr headerPointer;
                    string headerText = "";

                    int cartMode = Program.read_cartridge_mode();
                    if (cartMode == GB_CART) {
                        headerPointer = Program.read_gb_header(ref textLength);
                        headerText = Marshal.PtrToStringAnsi(headerPointer, textLength);
                    }
                    else if (cartMode == GBA_CART) {
                        headerPointer = Program.read_gba_header(ref textLength);
                        headerText = Marshal.PtrToStringAnsi(headerPointer, textLength);
                    }
                    
                    headerTextBox.Invoke((MethodInvoker)(() => {
                        headerTextBox.Text = headerText;
                    }));

                    commandReceived = 0;
                }

                cancelOperation = 0;
                System.Threading.Thread.Sleep(100);
            }
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
                readromlabel.Visible = false;
                saveramlabel.Visible = false;
                writeramlabel.Visible = false;
            }
        }
        
        // Open port button
        private void openportbutton_Click(object sender, EventArgs e) {
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            headerTextBox.Text = "";

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
                readromlabel.Visible = false;
                saveramlabel.Visible = false;
                writeramlabel.Visible = false;

                Program.update_config(comPortInt, baudInt);
                
                // Read device firmware
                int firmwareVersion = Program.read_firmware_version();
                if (firmwareVersion >= 1) {
                    firmwareText.Text = "R" + firmwareVersion;
                    firmwareText.Visible = true;

                    // Read which mode we are in
                    int cartMode = Program.read_cartridge_mode();
                    if (cartMode == GB_CART) {
                        modeText.Text = "GB/GBC";
                        modeText.Visible = true;
                    }
                    else if (cartMode == GBA_CART) {
                        modeText.Text = "GBA";
                        modeText.Visible = true;
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
                
                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }

            headerRead = false;
            progress = 0;
            backgroundWorker1.ReportProgress(0);
            readromlabel.Visible = false;
            saveramlabel.Visible = false;
            writeramlabel.Visible = false;
            modeText.Visible = false;
            firmwareText.Visible = false;
        }

        // Read rom button
        private void readrombutton_Click(object sender, EventArgs e)
        {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = READROM;
                readromlabel.Visible = true;
                readromlabel.Text = "Reading ROM...";
                saveramlabel.Visible = false;
                writeramlabel.Visible = false;
            }
        }

        // Save ram button 
        private void saverambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);
                
                if (Program.check_if_file_exists() == 1) {
                    DialogResult dialogResult = MessageBox.Show("*** This will erase the save game from your PC ***\nPress Yes to continue or No to abort.", "Confirm Save", MessageBoxButtons.YesNo);
                    if (dialogResult == DialogResult.Yes) {
                        commandReceived = SAVERAM;

                        saveramlabel.Visible = true;
                        saveramlabel.Text = "Saving RAM...";
                        readromlabel.Visible = false;
                        writeramlabel.Visible = false;
                    }
                    else if (dialogResult == DialogResult.No) {
                        saveramlabel.Visible = true;
                        saveramlabel.Text = "Saving RAM cancelled";
                        readromlabel.Visible = false;
                        writeramlabel.Visible = false;
                    }
                }
                else {
                    commandReceived = SAVERAM;

                    saveramlabel.Visible = true;
                    saveramlabel.Text = "Saving RAM...";
                    readromlabel.Visible = false;
                    writeramlabel.Visible = false;
                }
            }
        }

        // Write ram button
        private void writerambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true && commandReceived == 0) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                if (Program.check_if_file_exists() == 1) {
                    DialogResult dialogResult = MessageBox.Show("*** This will erase the save game from your Gameboy/Gameboy Advance Cartridge ***\nPress Yes to continue or No to abort.", "Confirm Write", MessageBoxButtons.YesNo);
                    if (dialogResult == DialogResult.Yes) {
                        commandReceived = WRITERAM;
                        writeramlabel.Visible = true;
                        writeramlabel.Text = "Writing RAM...";
                        readromlabel.Visible = false;
                        saveramlabel.Visible = false;
                    }
                    else if (dialogResult == DialogResult.No) {
                        writeramlabel.Text = "Write RAM cancelled";
                        writeramlabel.Visible = true;
                        readromlabel.Visible = false;
                        saveramlabel.Visible = false;
                    }
                }
                else {
                    writeramlabel.Text = "Save file not found.";
                    writeramlabel.Visible = true;
                    readromlabel.Visible = false;
                    saveramlabel.Visible = false;
                }
            }
        }
        

        // Gameboy cart info button
        private void cartinfobutton_Click(object sender, EventArgs e) {
            if (comConnected == true && Program.read_cartridge_mode() == GB_CART) {
                gbcartinfopanel.Visible = true;
            }
            else if (comConnected == true && Program.read_cartridge_mode() == GBA_CART) {
                gbacartinfopanel.Visible = true;
            }
        }

        // GBA cart info button
        private void gbaapplybutton_Click(object sender, EventArgs e) {
            gbacartinfopanel.Visible = false;

            // ROM size
            int rom_size = 0;
            if (gbaromsizebox.Text == "4 Mbyte") {
                rom_size = 1;
            }
            else if (gbaromsizebox.Text == "8 Mbyte") {
                rom_size = 2;
            }
            else if (gbaromsizebox.Text == "16 Mbyte") {
                rom_size = 3;
            }
            else if (gbaromsizebox.Text == "32 Mbyte") {
                rom_size = 4;
            }
            if (rom_size >= 1) {
                Program.gba_specify_rom_size(rom_size);
            }
            //Console.WriteLine(rom_size);

            // RAM type
            int memory_type = 0;
            if (memorytypebox.Text == "SRAM") {
                memory_type = 1;
            }
            else if (memorytypebox.Text == "Flash") {
                memory_type = 2;
            }
            else if (memorytypebox.Text == "EEPROM") {
                memory_type = 3;
            }

            // Flash vendor
            int flash_type = 0;
            if (gbaflashtypebox.Text == "Atmel") {
                flash_type = 1;
            }
            else if (gbaflashtypebox.Text == "Non-Atmel") {
                flash_type = 2;
            }

            // SRAM/Flash size
            int memory_size = 0;
            if (gbaramsizebox.Text == "256Kbit") {
                memory_size = 1;
            }
            else if (gbaramsizebox.Text == "512Kbit") {
                memory_size = 2;
            }
            else if (gbaramsizebox.Text == "1Mbit") {
                memory_size = 3;
            }

            // EEPROM size
            if (eepromsizebox.Text == "4Kbit") {
                memory_size = 1;
            }
            else if (eepromsizebox.Text == "64Kbit") {
                memory_size = 2;
            }

            if (memory_size >= 1) {
                Program.gba_specify_ram_size(memory_type, flash_type, memory_size);
            }
        }

        // GBA info cancel button
        private void gbacancelbutton_Click(object sender, EventArgs e) {
            gbacartinfopanel.Visible = false;
        }

        // Selection box changes
        private void memorytypebox_SelectedIndexChanged(object sender, EventArgs e) {
            if (memorytypebox.Text == "SRAM" || memorytypebox.Text == "Flash") {
                gbaramsizebox.Visible = true;
                eepromsizebox.Visible = false;
                if (memorytypebox.Text == "Flash") {
                    gbaflashtypebox.Visible = true;
                    flashtypetext.Visible = true;
                }
                else {
                    gbaflashtypebox.Visible = false;
                    flashtypetext.Visible = false;
                }
            }
            else if (memorytypebox.Text == "EEPROM") {
                gbaramsizebox.Visible = false;
                eepromsizebox.Visible = true;
                gbaflashtypebox.Visible = false;
                flashtypetext.Visible = false;
            }
        }

        // GB cart info cancel button
        private void cartinfocancelbutton_Click(object sender, EventArgs e) {
            gbcartinfopanel.Visible = false;
        }
        
        // GB cart info apply button
        private void cartinfoapplybutton_Click(object sender, EventArgs e) {
            gbcartinfopanel.Visible = false;

            int rom_size = 0;
            if (romsizebox.Text == "64KByte (4 banks)") {
                rom_size = 1;
            }
            else if (romsizebox.Text == "128KByte (8 banks)") {
                rom_size = 2;
            }
            else if (romsizebox.Text == "256KByte (16 banks)") {
                rom_size = 3;
            }
            else if (romsizebox.Text == "512KByte (32 banks)") {
                rom_size = 4;
            }
            else if (romsizebox.Text == "1MByte (64 banks)") {
                rom_size = 5;
            }
            else if (romsizebox.Text == "2MByte (128 banks)") {
                rom_size = 6;
            }
            else if (romsizebox.Text == "4MByte (256 banks)") {
                rom_size = 7;
            }
            else if (romsizebox.Text == "8MByte (512 banks)") {
                rom_size = 8;
            }
            if (rom_size >= 1) {
                Program.gb_specify_rom_size(rom_size);
            }


            int ram_size = 0;
            if (ramsizebox.Text == "2 KBytes") {
                ram_size = 1;
            }
            else if (ramsizebox.Text == "8 Kbytes") {
                ram_size = 2;
            }
            else if (ramsizebox.Text == "32 KBytes (4 banks of 8KBytes each)") {
                ram_size = 3;
            }
            else if (ramsizebox.Text == "128 KBytes (16 banks of 8KBytes each)") {
                ram_size = 4;
            }
            else if (ramsizebox.Text == "64 KBytes (8 banks of 8KBytes each)") {
                ram_size = 5;
            }
            else if (ramsizebox.Text == "512bytes (nibbles)") {
                ram_size = 6;
            }
            if (ram_size >= 1) {
                Program.gb_specify_ram_size(ram_size);
            }
        }

        private void Form1_Load(object sender, EventArgs e) {

        }

        private void label11_Click(object sender, EventArgs e) {

        }

        private void label12_Click(object sender, EventArgs e) {

        }

        private void label11_Click_1(object sender, EventArgs e) {

        }
    }
}
