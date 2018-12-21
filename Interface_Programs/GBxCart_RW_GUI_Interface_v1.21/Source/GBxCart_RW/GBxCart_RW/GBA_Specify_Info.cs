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
            gbaflashchiplabel.Visible = false;
            gbaflashchipbox.Visible = false;
        }

        private void gbaapplybutton_Click(object sender, EventArgs e) {
            // Check for Flash cart option
            if (gbaCartTypeBox.Text == "Flash Cart") {
                if (gbaflashchipbox.Text == "MSP55LV128" || gbaflashchipbox.Text == "29LV128DTMC") {
                    int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                    int gbxcartFirmwareVersion = Program.read_firmware_version();
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        if (gbxcartFirmwareVersion <= 4) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R5 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 100;
                            Form1.headerTokens[0] = "MSP55LV128 GBA Flash Cart";
                            Form1.headerTokens[1] = "ROM size: 16 MByte";
                            Form1.headerTokens[2] = "SRAM size: 512Kbit";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                        if (gbaflashchipbox.Text == "29LV128DTMC") {
                            Form1.headerTokens[0] = "29LV128DTMC GBA Flash Cart";
                        }
                    }
                }
                else if (gbaflashchipbox.Text == "MSP55LV128M" || gbaflashchipbox.Text == "29GL128EHMC" || 
                         gbaflashchipbox.Text == "256M29EWH")  {
                    int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                    int gbxcartFirmwareVersion = Program.read_firmware_version();
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        if (gbxcartFirmwareVersion <= 4) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R5 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 101;

                            Form1.headerTokens[1] = "ROM size: 16 MByte";
                            Form1.headerTokens[2] = "SRAM size: 512Kbit";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";

                            if (gbaflashchipbox.Text == "MSP55LV128M") {
                                Form1.headerTokens[0] = "MSP55LV128M GBA Flash Cart";
                            }
                            else if (gbaflashchipbox.Text == "29GL128EHMC") {
                                Form1.headerTokens[0] = "29GL128EHMC GBA Flash Cart";
                            }
                            else if (gbaflashchipbox.Text == "256M29EWH") {
                                Form1.headerTokens[0] = "256M29EWH GBA Flash Cart";
                                Form1.headerTokens[1] = "ROM size: 32 MByte";
                            }
                        }
                    }
                }
                else if (gbaflashchipbox.Text == "M36L0R706" || gbaflashchipbox.Text == "256L30B") {
                    int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                    int gbxcartFirmwareVersion = Program.read_firmware_version();
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        if (gbxcartFirmwareVersion <= 9) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R10 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 102;

                            Form1.headerTokens[1] = "ROM size: 16 MByte";
                            Form1.headerTokens[2] = "SRAM size: 512Kbit";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";

                            if (gbaflashchipbox.Text == "M36L0R706") {
                                Form1.headerTokens[0] = "M36L0R706 GBA Flash Cart";
                            }
                            else if (gbaflashchipbox.Text == "256L30B") {
                                Form1.headerTokens[0] = "256L30B GBA Flash Cart";
                                Form1.headerTokens[1] = "ROM size: 32 MByte";
                            }
                        }
                    }
                }
            }
           
            // ROM size
            int rom_size = 0;
            if (gbaromsizebox.Text == "4 MByte") {
                rom_size = 1;
                Form1.writeRomCartSize = 0x400000;
            }
            else if (gbaromsizebox.Text == "8 MByte") {
                rom_size = 2;
                Form1.writeRomCartSize = 0x800000;
            }
            else if (gbaromsizebox.Text == "16 MByte") {
                rom_size = 3;
                Form1.writeRomCartSize = 0x1000000;
            }
            else if (gbaromsizebox.Text == "32 MByte") {
                rom_size = 4;
                Form1.writeRomCartSize = 0x2000000;
            }
            if (rom_size >= 1) {
                Form1.headerTokens[1] = "ROM size: " + gbaromsizebox.Text;
                Program.gba_specify_rom_size(rom_size);
            }
            //Console.WriteLine("size");
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
                if (gbaCartTypeBox.Text == "Flash Cart") {
                    if (memory_type == 1) {
                        Form1.headerTokens[2] = "SRAM size: " + gbaramsizebox.Text;
                    }
                    else if (memory_type == 2) {
                        Form1.headerTokens[2] = "Flash size: " + gbaramsizebox.Text;
                    }
                    else if (memory_type == 3) {
                        Form1.headerTokens[2] = "EEPROM: " + eepromsizebox.Text;
                    }
                }
                else {
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

                }
                Program.gba_specify_ram_size(memory_type, flash_type, memory_size);
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
            if (gbaCartTypeBox.Text == "Flash Cart") {
                gbaflashchiplabel.Visible = true;
                gbaflashchipbox.Visible = true;
                gbaflashpcblabel.Visible = true;
                gbaflashpcbbox.Visible = true;

                // Default
                gbaflashchipbox.Text = "MSP55LV128";
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
            }
            else {
                gbaflashchiplabel.Visible = false;
                gbaflashchipbox.Visible = false;
                gbaflashpcblabel.Visible = false;
                gbaflashpcbbox.Visible = false;
            }
        }

        private void gbaflashchipbox_SelectedIndexChanged(object sender, EventArgs e) {
            if (gbaflashchipbox.Text == "MSP55LV128") {
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "AGB-E05-01";
            }
            else if (gbaflashchipbox.Text == "MSP55LV128M") {
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "AGB-E05-01";
            }
            else if (gbaflashchipbox.Text == "29GL128EHMC") {
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "AGB-E08-09";
            }
            else if (gbaflashchipbox.Text == "29LV128DTMC") {
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "AGB-E08-09";
            }
            else if (gbaflashchipbox.Text == "256M29EWH") {
                gbaromsizebox.Text = "32 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "AGB-E05-01";
            }
            else if (gbaflashchipbox.Text == "M36L0R706") {
                gbaromsizebox.Text = "16 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "36LOR-DRV";
            }
            else if (gbaflashchipbox.Text == "256L30B") {
                gbaromsizebox.Text = "32 MByte";
                memorytypebox.Text = "SRAM";
                gbaramsizebox.Text = "512Kbit";
                gbaflashpcbbox.Text = "28F256L0RB-DRV";
            }
        }

        private void gbaflashtypebox_SelectedIndexChanged(object sender, EventArgs e) {

        }

        private void GBA_Specify_Info_Load(object sender, EventArgs e) {

        }
    }
}
