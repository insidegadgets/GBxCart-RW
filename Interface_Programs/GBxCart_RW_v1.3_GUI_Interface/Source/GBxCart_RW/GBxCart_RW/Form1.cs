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

        bool comConnected = false;
        int commandReceived = 0;
        bool headerRead = false;
        int cancelOperation = 0;
        UInt32 progress = 0;

        public Form1()
        {
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
        

        // Stop button
        private void stopbutton_Click(object sender, EventArgs e)
        {
            commandReceived = 0;
            cancelOperation = 1;

            System.Threading.Thread.Sleep(250);

            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            int baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;

            Program.RS232_CloseComport(comPortInt);
            Program.RS232_OpenComport(comPortInt, baudInt, "8N1");
        }

        // Progress bar
        void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
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
        
        // Perform the reading/writing in the background
        void backgroundWorker2_DoWork(object sender, DoWorkEventArgs e)
        {
            while (true) {
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

                cancelOperation = 0;
                System.Threading.Thread.Sleep(100);
            }
        }

        private void readheaderbutton_Click(object sender, EventArgs e)
        {
            if (comConnected == true) {
                headerRead = true;
                Int32 textLength = 0;

                IntPtr headerPointer;
                string headerText;
                if (Program.read_cartridge_mode() == 1) { 
                    headerPointer = Program.read_gb_header(ref textLength);
                }
                else {
                    headerPointer = Program.read_gba_header(ref textLength);
                }
                headerText = Marshal.PtrToStringAnsi(headerPointer, textLength);

                //Console.WriteLine(headerText);
                richTextBox1.Text = headerText;
                
                readromlabel.Visible = false;
                saveramlabel.Visible = false;
                writeramlabel.Visible = false;

                progress = 0;
                backgroundWorker1.ReportProgress(0);
            }
        }
        
        private void openportbutton_Click(object sender, EventArgs e) {
            int comPortInt = Convert.ToInt32(comPortTextBox.Text);
            Int32 baudInt = Convert.ToInt32(baudtextBox.Text);
            comPortInt--;
            richTextBox1.Text = "";

            if (comConnected == true) {
                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }


            if (Program.RS232_OpenComport(comPortInt, baudInt, "8N1") == 0)
            {
                comPortTextBox.BackColor = Color.FromArgb(192, 255, 192);
                comConnected = true;

                Program.update_config(comPortInt, baudInt);
            }
            else
            {
                comPortTextBox.BackColor = Color.FromArgb(255, 192, 192);
            }
        }

        private void closeportbutton_Click(object sender, EventArgs e) {
            if (comConnected == true) {
                int comPortInt = Convert.ToInt32(comPortTextBox.Text);
                comPortInt--;
                richTextBox1.Text = "";

                Program.RS232_CloseComport(comPortInt);
                comPortTextBox.BackColor = Color.FromArgb(255, 255, 255);
                comConnected = false;
            }
        }

        private void readrombutton_Click(object sender, EventArgs e)
        {
            if (comConnected == true && headerRead == true) {
                progress = 0;
                backgroundWorker1.ReportProgress(0);

                commandReceived = READROM;
                readromlabel.Visible = true;
                readromlabel.Text = "Reading ROM...";
                saveramlabel.Visible = false;
                writeramlabel.Visible = false;
            }
        }

        private void saverambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true) {
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

        private void writerambutton_Click(object sender, EventArgs e) {
            if (comConnected == true && headerRead == true) {
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
        
        // Gameboy cart info
        private void cartinfobutton_Click(object sender, EventArgs e) {
            if (comConnected == true && Program.read_cartridge_mode() == GB_CART) {
                gbcartinfopanel.Visible = true;
            }
            else if (comConnected == true && Program.read_cartridge_mode() == GBA_CART) {
                gbacartinfopanel.Visible = true;
            }
        }

        // GBA cart info
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

        private void cartinfocancelbutton_Click(object sender, EventArgs e) {
            gbcartinfopanel.Visible = false;
        }

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
    }
}
