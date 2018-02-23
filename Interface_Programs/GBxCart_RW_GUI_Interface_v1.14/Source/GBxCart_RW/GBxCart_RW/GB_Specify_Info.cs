using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GBxCart_RW {
    public partial class GB_Specify_Info : Form {

        public GB_Specify_Info() {
            InitializeComponent();
        }

        private void cartinfoapplybutton_Click(object sender, EventArgs e) {
            Form1.writeRomCartType = 0;

            // Check for Flash cart option
            if (cartTypeBox.Text == "Flash Cart") {
                int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                int gbxcartFirmwareVersion = Program.read_firmware_version();

                if (gbflashchipbox.Text == "32K") {
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        if (gbflashwebox.Text == "WR") {
                            if (gbxcartFirmwareVersion <= 3) {
                                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                                System.Windows.Forms.MessageBox.Show("Please update to Firmware R4 or higher to support this function.");
                            }
                            else {
                                Form1.writeRomCartType = 1;
                                Form1.writeRomCartSize = 0x8000;
                                Form1.headerTokens[0] = "32K GB Flash Cart (WR as WE)";
                                //Form1.headerTokens[1] = "ROM size: 32KByte (no ROM banking)";
                                //Form1.headerTokens[2] = "RAM size: None";
                                Form1.headerTokens[3] = "Choose a ROM file to write";
                                Form1.headerTokens[4] = "";
                            }
                        }
                        else { // Audio pin as WE
                            if (gbxcartFirmwareVersion <= 3) {
                                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                                System.Windows.Forms.MessageBox.Show("Please update to Firmware R4 or higher to support this function.");
                            }
                            else {
                                Form1.writeRomCartType = 3;
                                Form1.writeRomCartSize = 0x8000;

                                Form1.headerTokens[0] = "32K GB Flash Cart (Audio as WE)";
                                //Form1.headerTokens[1] = "ROM size: 32KByte (no ROM banking)";
                                //Form1.headerTokens[2] = "RAM size: None";
                                Form1.headerTokens[3] = "Choose a ROM file to write";
                                Form1.headerTokens[4] = "";
                            }
                        }
                    }
                }
                else if (gbflashchipbox.Text == "BV5") {
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        if (gbxcartFirmwareVersion <= 3) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R4 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 2;
                            Form1.writeRomCartSize = 0x200000;
                            Form1.headerTokens[0] = "BV5 GB Flash Cart";
                            //Form1.headerTokens[1] = "ROM size: 2MByte (128 banks)";
                           // Form1.headerTokens[2] = "RAM size: 8 Kbytes";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                }
                else if (gbflashchipbox.Text == "ES29LV160") {
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        int cartVoltage = Program.read_cartridge_mode();
                        if (gbxcartFirmwareVersion <= 6) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R7 or higher to support this function.");
                        }
                        else if (cartVoltage == 1) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GBA.");
                        }
                        else {
                            Form1.writeRomCartType = 4;
                            Form1.writeRomCartSize = 0x100000;
                            Form1.headerTokens[0] = "ES29LV160 GB Flash Cart";
                           // Form1.headerTokens[1] = "ROM size: 1MByte (64 banks)";
                            //Form1.headerTokens[2] = "RAM size: None";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                }
                else if (gbflashchipbox.Text == "AM29LV160DB" || gbflashchipbox.Text == "29LV160CTTC") {
                    if (gbxcartPcbVersion == Form1.PCB_1_0) {
                        Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                        System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                    }
                    else {
                        int cartVoltage = Program.read_cartridge_mode();
                        if (gbxcartFirmwareVersion <= 7) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R8 or higher to support this function.");
                        }
                        else if (cartVoltage == 1) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GBA.");
                        }
                        else {
                            Form1.writeRomCartType = 5;
                            Form1.writeRomCartSize = 0x200000;
                            if (gbflashchipbox.Text == "AM29LV160DB") {
                                Form1.headerTokens[0] = "AM29LV160DB GB Flash Cart";
                            }
                            else {
                                Form1.headerTokens[0] = "29LV160CTTC GB Flash Cart";
                            }
                                
                            // Form1.headerTokens[1] = "ROM size: 1MByte (64 banks)";
                            //Form1.headerTokens[2] = "RAM size: None";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                }
            }
            

            //if (Form1.writeRomCartType == 0) {
                // ROM Size
                int rom_size = 0;
                if (romsizebox.Text == "32KByte (no ROM banking)") {
                    rom_size = 0;
                }
                else if (romsizebox.Text == "64KByte (4 banks)") {
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

                if (cartTypeBox.Text == "Flash Cart") {
                    Form1.headerTokens[1] = "ROM size: " + romsizebox.Text;
                    Program.gb_specify_rom_size(rom_size);
                }
                else {
                    if (romsizebox.Text != "") { // Header text
                        Form1.headerTokens[2] = "ROM size: " + romsizebox.Text;
                        Program.gb_specify_rom_size(rom_size);
                    }
                }

                // Ram Size
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
                else if (ramsizebox.Text == "512 bytes (nibbles)") {
                    ram_size = 6;
                }
                if (cartTypeBox.Text == "Flash Cart") {
                    Form1.headerTokens[2] = "RAM size: " + ramsizebox.Text;
                    Program.gb_specify_ram_size(ram_size);
                }
                else {
                    if (ram_size >= 1) {
                        Form1.headerTokens[3] = "RAM size: " + ramsizebox.Text;
                        Program.gb_specify_ram_size(ram_size);
                    }
                }
            //}

            this.Close();
        }

        private void cartTypeBox_SelectedIndexChanged(object sender, EventArgs e) {
            if (cartTypeBox.Text == "Flash Cart") {
                gbflashchiplabel.Visible = true;
                gbflashchipbox.Visible = true;
                gbflashpcblabel.Visible = true;
                gbflashpcbbox.Visible = true;
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;

                // Default
                gbflashchipbox.Text = "32K";
                romsizebox.Text = "32KByte (no ROM banking)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwebox.Text = "Audio";
            }
            else {
                gbflashchiplabel.Visible = false;
                gbflashchipbox.Visible = false;
                gbflashpcblabel.Visible = false;
                gbflashpcbbox.Visible = false;
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
            }
        }

        private void romsizebox_SelectedIndexChanged(object sender, EventArgs e) {

        }

        private void gbflashchipbox_SelectedIndexChanged(object sender, EventArgs e) {
            if (gbflashchipbox.Text == "32K") {
                romsizebox.Text = "32KByte (no ROM banking)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;
                gbflashwebox.Text = "Audio";
            }
            else if (gbflashchipbox.Text == "BV5") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "BV5";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
            }
            else if (gbflashchipbox.Text == "ES29LV160") {
                romsizebox.Text = "1MByte (64 banks)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "0883_DRV";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
            }
            else if (gbflashchipbox.Text == "AM29LV160DB") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "LJ3843-2";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
            }
            else if (gbflashchipbox.Text == "29LV160CTTC") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
            }
        }

        private void gbflashwebox_SelectedIndexChanged(object sender, EventArgs e) {

        }
    }
}
