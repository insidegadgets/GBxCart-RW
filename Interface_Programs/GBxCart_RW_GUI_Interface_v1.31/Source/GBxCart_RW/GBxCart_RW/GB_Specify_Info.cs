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
                    else if (gbflashchipbox.Text == "iG 32KB Cart") {
                        Form1.writeRomCartType = 6;
                        Form1.writeRomCartSize = 0x8000;
                        Form1.headerTokens[0] = "iG 32KB GB Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
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
                        int canWrite = 0;
                        int cartVoltage = Program.read_cartridge_mode();

                        if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GB_CART) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GBA_CART) {
                            canWrite = 1;
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_3) {
                            canWrite = 1;
                            Program.set_mode(Form1.VOLTAGE_3_3V);
                        }
                        else if (gbxcartPcbVersion == Form1.MINI_PCB_1_0) {
                            canWrite = 1;
                        }

                        if (canWrite == 1) {
                            Form1.writeRomCartType = 4;
                            Form1.writeRomCartSize = 0x100000;
                            Form1.headerTokens[0] = "ES29LV160 GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "2MB AM29LV160DB" || gbflashchipbox.Text == "2MB 29LV160CTTC" || gbflashchipbox.Text == "2MB 29LV160TE") {
                        int canWrite = 0;
                        int cartVoltage = Program.read_cartridge_mode();

                        if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GB_CART) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GBA_CART) {
                            canWrite = 1;
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_3) {
                            canWrite = 1;
                            Program.set_mode(Form1.VOLTAGE_3_3V);
                        }
                        else if (gbxcartPcbVersion == Form1.MINI_PCB_1_0) {
                            canWrite = 1;
                        }

                        if (canWrite == 1) {
                            Form1.writeRomCartType = 5;
                            Form1.writeRomCartSize = 0x200000;
                            if (gbflashchipbox.Text == "2MB AM29LV160DB") {
                                Form1.headerTokens[0] = "AM29LV160DB GB Flash Cart";
                            }
                            else if (gbflashchipbox.Text == "2MB 29LV160TE") {
                                Form1.headerTokens[0] = "29LV160TE GB Flash Cart";
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
                    else if (gbflashchipbox.Text == "2MB GB Smart 16M") {
                        Form1.writeRomCartType = 18;
                        Form1.writeRomCartSize = 0x200000;
                        Form1.headerTokens[0] = "2MB GB Smart 16M Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                    else if (gbflashchipbox.Text == "iG 2MB 128KB SRAM Cart") {
                        Form1.writeRomCartType = 8;
                        Form1.writeRomCartSize = 0x200000;
                        Form1.headerTokens[0] = "iG 2MB 128KB SRAM GB Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                    else if (gbflashchipbox.Text == "iG 2MB 32KB FRAM Cart") {
                        Form1.writeRomCartType = 8;
                        Form1.writeRomCartSize = 0x200000;
                        Form1.headerTokens[0] = "iG 2MB 32KB FRAM GB Flash Cart";
                        Form1.headerTokens[3] = "Choose a ROM file to write";
                        Form1.headerTokens[4] = "";
                    }
                    else if (gbflashchipbox.Text == "32MB 256M29" || gbflashchipbox.Text == "32MB MX29GL256") {
                        if (gbxcartFirmwareVersion <= 10) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R11 or higher to support this function.");
                        }
                        else {
                            int canWrite = 0;
                            int cartVoltage = Program.read_cartridge_mode();

                            if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GB_CART) {
                                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                                System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                            }
                            else if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GBA_CART) {
                                canWrite = 1;
                            }
                            else if (gbxcartPcbVersion == Form1.PCB_1_3) {
                                canWrite = 1;
                                Program.set_mode(Form1.VOLTAGE_3_3V);
                            }
                            else if (gbxcartPcbVersion == Form1.MINI_PCB_1_0) {
                                canWrite = 1;
                            }

                            if (canWrite == 1) {
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
                                if (gbflashchipbox.Text == "32MB 256M29") {
                                   
                                }
                                else {
                                    Form1.headerTokens[0] = "MX29GL256 GB Flash Cart";
                                }
                                Form1.headerTokens[3] = "Choose a ROM file to write";
                                Form1.headerTokens[4] = "";
                            }
                        }
                    }

                    else if (gbflashchipbox.Text == "32MB M29W256") {
                        if (gbxcartFirmwareVersion <= 10) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R11 or higher to support this function.");
                        }
                        else {
                            int canWrite = 0;
                            int cartVoltage = Program.read_cartridge_mode();

                            if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GB_CART) {
                                Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                                System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                            }
                            else if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GBA_CART) {
                                canWrite = 1;
                            }
                            else if (gbxcartPcbVersion == Form1.PCB_1_3) {
                                canWrite = 1;
                                Program.set_mode(Form1.VOLTAGE_3_3V);
                            }
                            else if (gbxcartPcbVersion == Form1.MINI_PCB_1_0) {
                                canWrite = 1;
                            }

                            if (canWrite == 1) {
                                if (flashBankSelectBox.Text == "Bank 1") {
                                    Form1.writeRomCartType = 19;
                                }
                                else if (flashBankSelectBox.Text == "Bank 2") {
                                    Form1.writeRomCartType = 20;
                                }
                                else if (flashBankSelectBox.Text == "Bank 3") {
                                    Form1.writeRomCartType = 21;
                                }
                                else if (flashBankSelectBox.Text == "Bank 4") {
                                    Form1.writeRomCartType = 22;
                                }

                                Form1.writeRomCartSize = 0x800000;
                                Form1.headerTokens[0] = "M29W256 GB Flash Cart";
                                Form1.headerTokens[3] = "Choose a ROM file to write";
                                Form1.headerTokens[4] = "";
                            }
                        }
                    }
                    
                    else if (gbflashchipbox.Text == "4MB M29W640" || gbflashchipbox.Text == "4MB 29DL32BF") {
                        int canWrite = 0;
                        int cartVoltage = Program.read_cartridge_mode();

                        if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GB_CART) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("You must use 3.3V to program this cart. Please unplug the device, switch to 3.3V, reconnect the device, press Open Port and then use the drop down Mode selection to manually change it to GB/GBC.");
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_1 && cartVoltage == Form1.GBA_CART) {
                            canWrite = 1;
                        }
                        else if (gbxcartPcbVersion == Form1.PCB_1_3) {
                            canWrite = 1;
                            Program.set_mode(Form1.VOLTAGE_3_3V);
                        }
                        else if (gbxcartPcbVersion == Form1.MINI_PCB_1_0) {
                            canWrite = 1;
                        }

                        if (canWrite == 1) {
                            Form1.writeRomCartType = 14;
                            Form1.writeRomCartSize = 0x400000;
                            if (gbflashchipbox.Text == "4MB M29W640") {
                                Form1.headerTokens[0] = "M29W640 GB Flash Cart";
                            }
                            else {
                                Form1.headerTokens[0] = "29DL32BF GB Flash Cart";
                            }
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "64MB Mighty Flash Cart") {
                        if (gbxcartFirmwareVersion <= 11) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R12 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 15;
                            Form1.writeRomCartSize = 0x4000000;
                            Form1.headerTokens[0] = "GB 64MB Mighty Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "64MB Mighty Flash Cart (2)") {
                        if (gbxcartFirmwareVersion <= 11) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R12 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 16;
                            Form1.writeRomCartSize = 0x4000000;
                            Form1.headerTokens[0] = "GB 64MB Mighty Flash Cart (2)";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "4MB MX29LV320") {
                        if (gbxcartFirmwareVersion <= 11) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R12 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 17;
                            Form1.writeRomCartSize = 0x4000000;
                            Form1.headerTokens[0] = "MX29LV320 GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "4MB S29GL032") {
                        if (gbxcartFirmwareVersion <= 11) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R12 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 17;
                            Form1.writeRomCartSize = 0x4000000;
                            Form1.headerTokens[0] = "S29GL032 GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                    else if (gbflashchipbox.Text == "iG 4MB 128KB SRAM Cart") {
                        if (gbxcartFirmwareVersion <= 11) {
                            Functions.MessageBoxHelper.PrepToCenterMessageBoxOnForm(this);
                            System.Windows.Forms.MessageBox.Show("Please update to Firmware R12 or higher to support this function.");
                        }
                        else {
                            Form1.writeRomCartType = 17;
                            Form1.writeRomCartSize = 0x4000000;
                            Form1.headerTokens[0] = "iG 4MB 128KB SRAM GB Flash Cart";
                            Form1.headerTokens[3] = "Choose a ROM file to write";
                            Form1.headerTokens[4] = "";
                        }
                    }
                }
            }

            // MBC Type
            int mbcType = 5;
            if (cartTypeBox.Text == "MBC1") {
                mbcType = 1;
            }
            else if (cartTypeBox.Text == "MBC2") {
                mbcType = 5;
            }
            else if (cartTypeBox.Text == "MBC3") {
                mbcType = 19;
            }
            else if (cartTypeBox.Text == "MBC5") {
                mbcType = 27;
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

                if (romsizebox.Text != "") {
                    Form1.headerTokens[2] = "ROM size: " + romsizebox.Text;
                    Program.gb_specify_rom_size(rom_size);
                }
                if (cartTypeBox.Text != "") {
                    Form1.headerTokens[1] = "MBC type: " + cartTypeBox.Text;
                    Program.gb_specify_mbc_type(mbcType);
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
                gbflashchipbox.Text = "iG 32KB Cart";
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
            if (gbflashchipbox.Text == "32KB AM29F010B" || gbflashchipbox.Text == "iG 32KB Cart") {
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
            else if (gbflashchipbox.Text == "2MB 29LV160TE") {
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
            else if (gbflashchipbox.Text == "2MB AM29F016B" || gbflashchipbox.Text == "iG 2MB 32KB FRAM Cart") {
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
            else if (gbflashchipbox.Text == "2MB GB Smart 16M") {
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
            else if (gbflashchipbox.Text == "iG 2MB 128KB SRAM Cart") {
                romsizebox.Text = "2MByte (128 banks)";
                ramsizebox.Text = "128 KBytes";
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
            else if (gbflashchipbox.Text == "32MB M29W256") {
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
            else if (gbflashchipbox.Text == "32MB MX29GL256") {
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
            else if (gbflashchipbox.Text == "4MB M29W640") {
                romsizebox.Text = "4MByte (256 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "SD007_48BALL-64M";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "4MB 29DL32BF") {
                romsizebox.Text = "4MByte (256 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "29DL32BF-70";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }            
            else if (gbflashchipbox.Text == "64MB Mighty Flash Cart" || gbflashchipbox.Text == "64MB Mighty Flash Cart (2)") {
                romsizebox.Text = "64MByte (8x 512 banks)";
                ramsizebox.Text = "8 KBytes";
                gbflashpcbbox.Text = "insideGadgets";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = false;
                chipEraseCheckbox.Checked = false;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "4MB S29GL032" || gbflashchipbox.Text == "4MB MX29LV320" || gbflashchipbox.Text == "iG 4MB 128KB SRAM Cart") {
                romsizebox.Text = "4MByte (256 banks)";
                ramsizebox.Text = "128 KBytes";
                gbflashpcbbox.Text = "insideGadgets";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
            }
            else if (gbflashchipbox.Text == "4MB S29GL032") {
                romsizebox.Text = "4MByte (256 banks)";
                ramsizebox.Text = "128 KBytes";
                gbflashpcbbox.Text = "insideGadgets";
                gbflashwelabel.Visible = false;
                gbflashwebox.Visible = false;
                chipEraseCheckbox.Visible = true;
                chipEraseCheckbox.Checked = true;
                flashBankSelectBox.Visible = false;
                flashBankLabel.Visible = false;
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
                chipEraseCheckbox.Visible = false;
                chipEraseCheckbox.Checked = false;
                System.Windows.Forms.MessageBox.Show("Please make sure you have power cycled GBxCart RW before proceeding with Banks 2, 3 & 4.");
            }
        }

        private void GB_Specify_Info_Load(object sender, EventArgs e) {

        }
    }
}
