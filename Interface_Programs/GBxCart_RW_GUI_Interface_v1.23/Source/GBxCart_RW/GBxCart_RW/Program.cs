using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices; // DLL support

namespace GBxCart_RW
{
    static class Program {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        ///

        const string dllLocation = "GBxCart_RW_COMLib.dll";
        //const string dllLocation = "C:\\Users\\Alex\\Documents\\Visual Studio 2015\\Projects\\GBxCart_RW_COMLib\\Release\\GBxCart_RW_COMLib.dll";

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern int RS232_OpenComport(int comport_number, int baudrate, string mode);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_cartridge_mode();

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void RS232_CloseComport(int comport_number);
        
        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern int check_if_file_exists();

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr read_gb_header(ref Int32 length);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr read_gba_header(ref Int32 length);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void read_rom(int cartMode, ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void read_ram(bool saveAsNewFile, ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void write_ram(string writeSaveFileName, ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void set_mode(char command);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_config(int type);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void update_config(int comport, Int32 baudrate, int alwaysAddDateTimeToSave, int promptForRestoreSaveFile);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void gb_specify_rom_size(int size);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void gb_specify_mbc_type(int mbcType);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void gba_specify_rom_size(int size);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void gb_specify_ram_size(int size);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern void gba_specify_ram_size(int memoryType, int flashType, int size);

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_firmware_version();

        [DllImport(dllLocation, CallingConvention = CallingConvention.Cdecl)]
        public static extern int request_value(char command);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void write_rom(string fileName, int flashCartType, UInt32 fileSize, ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void erase_rom(int cartType, ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void erase_ram(ref UInt32 length, ref int cancelOperation);

        [DllImport(dllLocation, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void update_current_folder(string folderName);

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
