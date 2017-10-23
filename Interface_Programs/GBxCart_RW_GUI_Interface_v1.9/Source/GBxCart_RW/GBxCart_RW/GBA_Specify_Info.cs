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
    }
}
