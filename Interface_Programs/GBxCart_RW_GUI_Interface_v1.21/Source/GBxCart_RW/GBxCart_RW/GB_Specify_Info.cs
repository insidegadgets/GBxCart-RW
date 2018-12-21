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

            if (chipEraseCheckbox.Visible == true && chipEraseCheckbox.Checked) {
                Form1.chipEraseSelected = 1;
            }
            else {
                Form1.chipEraseSelected = 0;
            }

            // Check for Flash cart option
            if (cartTypeBox.Text == "Flash Cart") {
                int gbxcartPcbVersion = Program.request_value(Form1.READ_PCB_VERSION);
                int gbxcartFirmwareVersion = Program.read_firmware_version();
                bool flashCartSupported = false;

                // Firmware check
                if (gbxcartFirmwareVersion <= 8) {
                    Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                    System.Windows.Forms.MessageBox.Show("Please update to Firmware R9 or higher to support this function.");
                }
                else {
                    flashCartSupported = true;
                }
                
                // PCB check
                if (gbxcartPcbVersion == Form1.PCB_1_0) {
                    Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                    System.Windows.Forms.MessageBox.Show("PCB v1.0 is not supported for this function.");
                }
                else {
                    flashCartSupported = true;
                }

                if (flashCartSupported == true) {
                    if (gbflashchipbox.Text == "32KB AM29F010B" || gbflashchipbox.Text == "32KB SST39SF010A" || gbflashchipbox.Text == "32KB AT49F040") {
                        if (gbflashwebox.Text == "WR") {
                            if (gbflashchipbox.Text == "32KB AM29F010B") {
                                Form1.writeRomCartType = 3;
                            }
                            else {
                                Form1.writeRomCartType = 6;
                            }
                                
                            Form1.writeRomCartSize = 0x8000;
                            Form1.headerTokens[0] = gbflashchipbox.Text + " Flash Cart (WR as WE)";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                        else { // Audio pin as WE
                            if (gbflashchipbox.Text == "32KB AM29F010B") {
                                Form1.writeRomCartType = 1;
                            }
                            else {
                                Form1.writeRomCartType = 7;
                            }
                            
                            Form1.writeRomCartSize = 0x8000;
                            Form1.headerTokens[0] = gbflashchipbox.Text + " Flash Cart (Audio as WE)";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "512KB SST39SF040") {
                        if (gbxcartFirmwareVersion <= 10) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R11 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 9;
                            Form1.writeRomCartSize = 0x80000;
                            Form1.headerTokens[0] = "SST39SF040 GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "2MB BV5") {
                        Form1.writeRomCartType = 2;
                        Form1.writeRomCartSize = 0x200000;
                        Form1.headerTokens[0] = "BV5 GB Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                    else if (gbflashchipbox.Text == "1MB ES29LV160") {
                        int cartVoltage = Program.read_cartridge_mode();
                        if (cartVoltage == 1) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                        }
                        else {
                            Form1.writeRomCartType = 4;
                            Form1.writeRomCartSize = 0x100000;
                            Form1.headerTokens[0] = "ES29LV160 GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "2MB AM29LV160DB" || gbflashchipbox.Text == "2MB 29LV160CTTC") {
                        int cartVoltage = Program.read_cartridge_mode();
                        if (cartVoltage == 1) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                        }
                        else {
                            Form1.writeRomCartType = 5;
                            Form1.writeRomCartSize = 0x200000;
                            if (gbflashchipbox.Text == "2MB AM29LV160DB") {
                                Form1.headerTokens[0] = "AM29LV160DB GB Flash Cart";
                            }
                            else {
                                Form1.headerTokens[0] = "29LV160CTTC GB Flash Cart";
                            }

                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "2MB AM29F016B") {
                        Form1.writeRomCartType = 8;
                        Form1.writeRomCartSize = 0x200000;
                        Form1.headerTokens[0] = "AM29F016B GB Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                    else if (gbflashchipbox.Text == "32MB 256M29") {
                        if (gbxcartFirmwareVersion <= 10) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R11 or higher to support this function.");
                        }
                        else {
                            int cartVoltage = Program.read_cartridge_mode();
                            if (cartVoltage == 1) {
                                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                                System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                            }
                            else {
                                if (flashBankSelectBox.Text == "Bank 1") {
                                    Form1.writeRomCartType = 10;
                                }
                                else if(flashBankSelectBox.Text == "Bank 2") {
                                    Form1.writeRomCartType = 11;
                                }
                                else if (flashBankSelectBox.Text == "Bank 3") {
                                    Form1.writeRomCartType = 12;
                                }
                                else if (flashBankSelectBox.Text == "Bank 4") {
                                    Form1.writeRomCartType = 13;
                                }

                                Form1.writeRomCartSize = 0x800000;
                                Form1.headerTokens[0] = "256M29 GB Flash Cart";
                                Form1.headerTokens[3] = "Choose a ROM file to write";
                                Form1.headerTokens[4] = "";
                            }
                        }
                    }
                }
            }
            
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
            else if (ramsizebox.Text == "8 KBytes") {
                ram_size = 2;
            }
            else if (ramsizebox.Text == "32 KBytes (4 banks x 8KBytes)") {
                ram_size = 3;
            }
            else if (ramsizebox.Text == "128 KBytes (16 banks x 8KBytes)") {
                ram_size = 4;
            }
            else if (ramsizebox.Text == "64 KBytes (8 banks x 8KBytes)") {
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
                else {
                    Form1.headerTokens[3] = "RAM size: None";
                    Program.gb_specify_ram_size(ram_size);
                }
            }

            //Console.WriteLine(Form1.headerTokens[3]);
            //Console.WriteLine(ramsizebox.Text);
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
                gbflashchipbox.Text = "32KB AM29F010B";
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
            if (gbflashchipbox.Text == "32KB AM29F010B") {
                romsizebox.Text = "32KByte (no ROM banking)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;
                gbflashwebox.Text = "Audio";
                chipEraseCheckbox.Visible = false;
                chipEraseCheckbox.Checked = false;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "32KB SST39SF010A") {
                romsizebox.Text = "32KByte (no ROM banking)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;
                gbflashwebox.Text = "Audio";
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "512KB SST39SF040") {
                romsizebox.Text = "512KByte (32 banks)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;
                gbflashwebox.Text = "Audio";
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "32KB AT49F040") {
                romsizebox.Text = "32KByte (no ROM banking)";
                ramsizebox.Text = "None";
                gbflashpcbbox.Text = "N/A";
                gbflashwelabel.Visible = true;
                gbflashwebox.Visible = true;
                gbflashwebox.Text = "Audio";
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "2MB BV5") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "BV5";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "1MB ES29LV160") {
                romsizebox.Text = "1MByte (64 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "0883_DRV";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "2MB AM29LV160DB") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "LJ3843-2";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "2MB 29LV160CTTC") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "2MB AM29F016B") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "32MB 256M29") {
                romsizebox.Text = "8MByte (512 banks)";
                ramsizebox.Text = "32 KBytes";
                gbflashpcbbox.Text = "";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = true;
                flashBankSelectBox.Text = "Bank 1";
                flashBankLabel.Visible = true;
            }
        }

        private void gbflashwebox_SelectedIndexChanged(object sender, EventArgs e) {

        }

        private void cartinfocancelbutton_Click(object sender, EventArgs e) {

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e) {

        }

        private void ramsizebox_SelectedIndexChanged(object sender, EventArgs e) {

        }

        private void gbflashpcbbox_SelectedIndexChanged(object sender, EventArgs e) {

        }

        private void flashBankSelectBox_SelectedIndexChanged(object sender, EventArgs e) {
            if (flashBankSelectBox.Text == "Bank 1") {
                chipEraseCheckbox.Checked = true;
            }
            else {
                chipEraseCheckbox.Checked = false;
            }
        }
    }
}
