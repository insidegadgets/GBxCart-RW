using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GBxCart_RW {
    public partial class GBA_Specify_Info : Form {

        public GBA_Specify_Info() {
            InitializeComponent();
        }

        private void gbaapplybutton_Click(object sender, EventArgs e) {
            if (gbaCartTypeBox.Text == "16M (MSPLV128) GBA Flash Cart") {
                int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                int gbxcartFirmwareVersion = Program.read_firmware_version();
                if (gbxcartPcbVersion == Form1.PCB_1_0) {
                    System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                }
                else {
                    if (gbxcartFirmwareVersion <= 4) {
                        System.Windows.Forms.MessageBox.Show("Please update to Firmware R5 or higher to support this function.");
                    }
                    else {
                        Form1.writeRomCartType = 100;
                        Form1.writeRomCartSize = 0x1000000;
                        Form1.headerTokens[0] = gbaCartTypeBox.Text;
                        Form1.headerTokens[1] = "ROM size: 16 MByte";
                        Form1.headerTokens[2] = "SRAM size: 512 Kbytes";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                }
            }

            if (Form1.writeRomCartType == 0) {
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
                    Form1.headerTokens[1] = "ROM size: " + gbaromsizebox.Text;
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
                    if (memory_type == 1) {
                        Form1.headerTokens[2] = "SRAM size: " + gbaramsizebox.Text;
                        Form1.headerTokens[3] = "EEPROM: None";
                    }
                    else if (memory_type == 2) {
                        Form1.headerTokens[2] = "Flash size: " + gbaramsizebox.Text;
                        Form1.headerTokens[3] = "EEPROM: None";
                    }
                    else if (memory_type == 3) {
                        Form1.headerTokens[2] = "SRAM/Flash size: None";
                        Form1.headerTokens[3] = "EEPROM: " + eepromsizebox.Text;
                    }

                    Program.gba_specify_ram_size(memory_type, flash_type, memory_size);
                }
            }
            this.Close();
        }

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

        private void gbaCartTypeBox_SelectedIndexChanged(object sender, EventArgs e) {
            if (gbaCartTypeBox.Text == "16M (MSPLV128) GBA Flash Cart") {
                gbaromsizebox.Text = "16 Mbyte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512 KBytes";
            }
            else {
                gbaromsizebox.Text = "";
                memorytypebox.Text = "";
            }
        }
    }
}
