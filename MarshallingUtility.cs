// Version 01.03.03.09
//
// This source file has been released by Centre Technologies Ltd. under The BSD License. 
//
// The BSD License 
// Copyright (c) 2008-2009, Centre Technologies Ltd.
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met: 
//
// •Redistributions of source code must retain the above copyright notice, this list of conditions
//  and the following disclaimer.
//
// •Redistributions in binary form must reproduce the above copyright notice, this list of conditions
//  and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// •Neither the name of Centre Technologies Ltd, nor the names of its contributors may be used to
//  endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

using System;
using System.Text;
using System.Runtime.InteropServices;
using FILETIME = System.Runtime.InteropServices.ComTypes.FILETIME;
using MemoryManagement;


namespace Marshalling
{
    class MarshallingUtility
    {
        public static ulong ReadWykiGuid(ref IntPtr p)
        {
            return ReadAtom<UInt64>(ref p);
        }

        public static DateTime ReadWykiTime(ref IntPtr p)
        {
            uint seconds = ReadAtom<UInt32>(ref p);
            return MemoryUtility.WykiTimeToDateTime(seconds);
        }

        public static DateTime ReadFILETIME(ref IntPtr p)
        {
            FILETIME ft = MemoryUtility.MarshalToType<FILETIME>(p);
            DateTime dt = MemoryUtility.FileTimeToDateTime(ft);
            p = MemoryUtility.Offset(p, Marshal.SizeOf(typeof(FILETIME)));
            return dt;
        }

        public static string[] ReadNullTermStringArray(ref IntPtr p)
        {
            IntPtr pArray = Marshal.ReadIntPtr(p);
            string[] array = MemoryUtility.MarshalToStringArrayCapacity(pArray, 32);
            p = MemoryUtility.Offset(p, IntPtr.Size);
            return array;
        }

        public static T[] ReadArray<T>(ref IntPtr p, uint size)
        {
            IntPtr pArray = Marshal.ReadIntPtr(p);
            T[] array = MemoryUtility.MarshalToTypeArray<T>(pArray, size);
            p = MemoryUtility.Offset(p, IntPtr.Size);
            return array;
        }

        public static string ReadString(ref IntPtr p)
        {
            IntPtr strp = Marshal.ReadIntPtr(p);
            String s = Marshal.PtrToStringUni(strp);
            p = MemoryUtility.Offset(p, IntPtr.Size);
            return s;
        }

        public static T ReadAtom<T>(ref IntPtr p)
        {
            T atom = MemoryUtility.MarshalToType<T>(p);
            p = MemoryUtility.Offset(p, Marshal.SizeOf(typeof(T)));
            return atom;
        }

        public static string SafeString(string s)
        {
            if (s == null)
            {
                return "";
            }

            return s;
        }
    }
}
