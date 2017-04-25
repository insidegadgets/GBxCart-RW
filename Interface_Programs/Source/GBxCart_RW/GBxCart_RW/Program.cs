using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices;     // DLL support

namespace GBxCart_RW
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        ///
       
        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int RS232_OpenComport(int comport_number, int baudrate, string mode);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_cartridge_mode();

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void RS232_CloseComport(int comport_number);
        
        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern int check_if_file_exists();

        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr read_gb_header(ref Int32 length);

        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr read_gba_header(ref Int32 length);

        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void read_rom(ref UInt32 length, ref int cancelOperation);

        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void read_ram(ref UInt32 length, ref int cancelOperation);

        [DllImport("GBxCart_RW_COMLib.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void write_ram(ref UInt32 length, ref int cancelOperation);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void set_mode(char command);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_config(int type);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void update_config(int comport, Int32 baudrate);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void gb_specify_rom_size(int size);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void gba_specify_rom_size(int size);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void gb_specify_ram_size(int size);

        [DllImport("GBxCart_RW_COMLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void gba_specify_ram_size(int memoryType, int flashType, int size);
        

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
