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
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using FILETIME = System.Runtime.InteropServices.ComTypes.FILETIME;

namespace MemoryManagement
{
    /// <summary>
    /// Provides utility methods for dealing with raw memory, for marshalling objects and arrays of objects to and from raw memory,
    /// and for performing unusual type conversions where bit shifting is involved
    /// </summary>
    class MemoryUtility
    {
        /// <summary>
        /// Increment an IntPtr (a pointer) by a 32-bit offset
        /// </summary>
        /// <param name="src">The pointer to increment</param>
        /// <param name="offset">A 32-bit offset</param>
        /// <returns>The incremented pointer</returns>
        public static IntPtr Offset(IntPtr src, int offset)
        {
            switch (IntPtr.Size)
            {
                case 4:
                    return new IntPtr(src.ToInt32() + offset);
                case 8:
                    return new IntPtr(src.ToInt64() + offset);
                default:
                    throw new NotSupportedException("Max pointer size is 64 bits");
            }
        }

        /// <summary>
        /// Increment an IntPtr (a pointer) by a 64-bit offset
        /// </summary>
        /// <param name="src">The pointer to increment</param>
        /// <param name="offset">A 64-bit offset</param>
        /// <returns>The incremented pointer</returns>
        public static IntPtr Offset(IntPtr src, long offset)
        {
            switch (IntPtr.Size)
            {
                case 4: // errors can result!
                    return new IntPtr(src.ToInt32() + offset);
                case 8:
                    return new IntPtr(src.ToInt64() + offset);
                default:
                    throw new NotSupportedException("Max pointer size is 64 bits");
            }
        }

        [DllImport("kernel32", SetLastError = true, EntryPoint = "RtlZeroMemory")]
        static extern void SecureZeroMemory(IntPtr ptr, int cnt);

        /// <summary>
        /// Allocate some memory on the process' global heap (a special heap provided by Windows to every
        /// process, which is managed by special functions - not to be confused with compiler specific 
        /// heaps used by new() etc). The memory is zeroed
        /// </summary>
        /// <param name="size">The size of the memory block requested on the global heap</param>
        /// <returns>A pointer to the allocated memory</returns>
        public static IntPtr NewZeroedHGlobal(int size)
        {
            IntPtr buffer = Marshal.AllocHGlobal(size);
            SecureZeroMemory(buffer, size);
            return buffer;
        }

        /// <summary>
        /// Free some memory on the global heap, and set the pointer to that memory to NULL
        /// </summary>
        /// <param name="pBuffer">The pointer to the memory on the global heap</param>
        public static void FreeAndNullHGlobal(ref IntPtr pBuffer)
        {
            if (pBuffer == IntPtr.Zero)
                return;

            Marshal.FreeHGlobal(pBuffer);

            pBuffer = IntPtr.Zero;
        }

        /// <summary>
        /// Allocate memory on the global heap and copy/marshal an object to it
        /// </summary>
        /// <param name="data">The object to marshal to the raw memory</param>
        /// <returns>A pointer to the memory holding the marshalled binary representation of the object</returns>
        public static IntPtr MarshalToMemory(object data)
        {
            IntPtr buf = Marshal.AllocHGlobal(Marshal.SizeOf(data));
            Marshal.StructureToPtr(data, buf, false);
            return buf;
        }

        /// <summary>
        /// Automatically marshal a raw memory representation of type T generated by .NET marshalling
        /// into an instance of that type
        /// </summary>
        /// <typeparam name="T">The type of the object to marshal</typeparam>
        /// <param name="buf">The raw memory representation of the instance</param>
        /// <returns>An allocated and initialized instance of T</returns>
        public static T MarshalToType<T>(IntPtr buf)
        {
            return (T)Marshal.PtrToStructure(buf, typeof(T));
        }

        /// <summary>
        /// Create a byte array that represents any object using the .NET's built-in marshalling system
        /// </summary>
        /// <param name="anything">The object to marshall to an array of bytes</param>
        /// <returns>An array of bytes holding a binary representation of the object</returns>
        public static byte[] TypeToByteArray(object anything)
        {
            int rawsize = Marshal.SizeOf(anything);
            byte[] rawdata = new byte[rawsize];
            GCHandle handle = GCHandle.Alloc(rawdata, GCHandleType.Pinned); // don't let GC move it before we have copied data to it!
            Marshal.StructureToPtr(anything, handle.AddrOfPinnedObject(), false);
            handle.Free();
            return rawdata;
        }

        /// <summary>
        /// Create an instance of type T by reading a raw binary representation of the object that has
        /// been created using .NET marshalling from a stream
        /// </summary>
        /// <typeparam name="T">The type of the object to create</typeparam>
        /// <param name="fs">The stream to read the raw bytes from</param>
        /// <returns>An allocated and initialized instance of T</returns>
        public static T TypeFromStream<T>(FileStream fs)
        {
            byte[] buffer = new byte[Marshal.SizeOf(typeof(T))];
            fs.Read(buffer, 0, Marshal.SizeOf(typeof(T)));
            GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
            T temp = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            handle.Free();
            return temp;
        }

        /// <summary>
        /// Marshalls a null-terminated array of unicode strings into a CLR array of string
        /// objects
        /// </summary>
        /// <param name="pArray">Pointer to null-terminated array of strings</param>
        /// <param name="capacity">Initial capacity of array used in construction. Can be 0..n</param>
        /// <returns>An array of string objects</returns>
        /// <remarks>This function frees the unmanaged strings in the array, and also frees the
        /// unmanaged array itself</remarks>
        public static String[] MarshalToStringArrayCapacity(IntPtr pArray, int capacity)
        {
            List<String> arr = new List<String>(capacity);
            if (pArray != IntPtr.Zero)
            {
                IntPtr pEntry = pArray;

                IntPtr curr = Marshal.ReadIntPtr(pEntry);

                while (curr != IntPtr.Zero)
                {
                    arr.Add(Marshal.PtrToStringUni(curr));
                    pEntry = Offset(pEntry, IntPtr.Size);
                    curr = Marshal.ReadIntPtr(pEntry);
                }
            }
            return arr.ToArray();
        }

        public static int[] MarshalNegativeTerminatedIntArrayToIntArray(IntPtr pArray, int capacity)
        {
            List<int> arr = new List<int>(capacity);
            if (pArray != IntPtr.Zero)
            {
                IntPtr pEntry = pArray;

                int curr = Marshal.ReadInt32(pEntry);

                while (curr >= 0)
                {
                    arr.Add(curr);
                    pEntry = Offset(pEntry, Marshal.SizeOf(typeof(UInt32)));
                    curr = Marshal.ReadInt32(pEntry);
                }
            }
            return arr.ToArray();
        }

        public static T[] MarshalToTypeArray<T>(IntPtr pArray, uint size)
        {
            T[] array = new T[size];
            if (pArray != IntPtr.Zero)
            {
                IntPtr pEntry = pArray;
                int i = 0;
                while (size-- > 0)
                {
                    array[i] = MarshalToType<T>(pEntry);
                    pEntry = Offset(pEntry, Marshal.SizeOf(typeof(T)));
                    i++;
                }
            }
            return array;
        }

        /// <summary>
        /// Objects that support this interface can be initialized from data held in memory addressed by the supplied pointer
        /// </summary>
        public interface ILoadFromMemory
        {
            /// <summary>
            /// Initialize the object from data held in memory
            /// </summary>
            /// <param name="memPtr">The address of the data held in memory</param>
            IntPtr LoadFromMemory(IntPtr memPtr);
        }

        /// <summary>
        /// Allocate and initialize a complex object of type T, which supports ILoadFromMemory, from a raw memory location.
        /// This is to be used with types where the default .NET marshalling either cannot manage the complexity of the type
        /// or offers an unacceptably inefficient transformation process.
        /// </summary>
        /// <typeparam name="T">The type of the complex object to load, which must support the ILoadFromMemory interface</typeparam>
        /// <param name="pObject">A pointer to the raw memory from which a new complex object must initialize itself</param>
        /// <returns>An allocated and initialized complex object of type T</returns>
        public static T MarshalToComplexType<T>(IntPtr pObject) where T : ILoadFromMemory, new()
        {
            T t = new T();
            t.LoadFromMemory(pObject);

            return t;
        }

        /// <summary>
        /// Allocate and initialize an array of complex objects of type T, which supports ILoadFromMemory, from a raw memory location.
        /// This is to be used with types where the default .NET marshalling either cannot manage the complexity of the type
        /// or offers an unacceptably inefficient transformation process.
        /// </summary>
        /// <typeparam name="T">The type of the complex objects to load, which must support the ILoadFromMemory interface</typeparam>
        /// <param name="pArray">A pointer to the address of a null terminated array of pointers to raw memory representations of the complex objects</param>
        /// <param name="capacity">The initial capacity of the array to which the objects are added. Reduce array memory re-allocations by estimating likely max number of objects</param>
        /// <returns>An array of allocated and initialized complex objects of type T</returns>
        public static T[] MarshalToObjectArray<T>(IntPtr pObjArray, int capacity) where T : ILoadFromMemory, new()
        {
            List<T> arr = new List<T>(capacity);
            if (pObjArray != IntPtr.Zero)
            {
                IntPtr pEntry = pObjArray;

                IntPtr curr = Marshal.ReadIntPtr(pEntry);

                while (curr != IntPtr.Zero)
                {
                    T t = new T();
                    t.LoadFromMemory(curr);
                    arr.Add(t);
                    pEntry = Offset(pEntry, IntPtr.Size);
                    curr = Marshal.ReadIntPtr(pEntry);
                }
            }
            return arr.ToArray();
        }

        public static T[] MarshalToStructArray<T>(IntPtr pValArray, uint len) where T : ILoadFromMemory, new()
        {
            T[] structArray = new T[len];

            IntPtr curr = pValArray;

            for (int i = 0; i < len; i++)
            {
                curr = structArray[i].LoadFromMemory(curr);
            }

            return structArray;
        }

        /// <summary>
        /// Frees a null terminated unicode string array that has been allocated with GlobalAlloc.
        /// The contained strings are freed, and the array itself. Note, that the memory be freed must
        /// have been allocated on the global heap using GlobalAlloc, and not on another heap specific
        /// within the unmanaged code that is specific to the compiler - as would occur, for example, if the
        /// array had been allocated using new or malloc
        /// </summary>
        /// <param name="pArray">A pointer to the null terminated unicode string array to free</param>
        public static void FreeAndNullStringArray(ref IntPtr pArray)
        {
            if (pArray != IntPtr.Zero)
            {
                IntPtr pEntry = pArray;

                IntPtr curr = Marshal.ReadIntPtr(pEntry);

                while (curr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(curr);
                    pEntry = Offset(pEntry, IntPtr.Size);
                    curr = Marshal.ReadIntPtr(pEntry);
                }

                Marshal.FreeHGlobal(pArray);
            }
            pArray = IntPtr.Zero;
        }

        /// <summary>
        /// Converts a Win32 FILETIME structure representing UTC into a CLR DateTime
        /// representing UTC
        /// </summary>
        /// <param name="ft">The FILETIME structure to convert</param>
        /// <returns>The DateTime equivalent</returns>
        public static DateTime FileTimeToDateTime(FILETIME ft)
        {
            if (ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0)
            {
                return DateTime.MinValue;
            }
            // WARNING: the below code may look weird, but unfortunately .NET thinks that .dwHighDateTime and
            // .dwLowDateTime are of type int (as you know, they are double words)!!! This leads to the 32nd bit
            // being lost in some circumstances and about 7 minutes being lopped off the decoded FILETIME!!
            // The code on the Internet, supplied both by Microsoft and others, is wrong for that reason, so
            // there must be a lot of people out there suffering! Don't mess with the below code.

            long lft = ft.dwHighDateTime;
            lft <<= 32;
            lft |= (uint)ft.dwLowDateTime;

            return DateTime.FromFileTimeUtc(lft);
        }

        /// <summary>
        /// Converts a CLR DateTime object representing UTC into a Win32 FILETIME
        /// structure representing UTC
        /// </summary>
        /// <param name="dt">The DateTime object to convert</param>
        /// <returns>The FILETIME equivalent</returns>
        public static FILETIME DateTimeToFileTime(DateTime dt)
        {
            FILETIME ft;

            if (dt == DateTime.MinValue)
            {
                ft.dwLowDateTime = 0;
                ft.dwHighDateTime = 0;
            }
            else
            {
                long lft = dt.ToFileTimeUtc();
                ft.dwLowDateTime = (int)(lft & 0x00000000FFFFFFFF);
                ft.dwHighDateTime = (int)(lft >> 32);
            }
            return ft;
        }

        public static uint DateTimeToWykiTime(DateTime dt)
        {
            if (dt == DateTime.MinValue)
            {
                return 0;
            }

            long lft = dt.ToFileTimeUtc();
		    long intervals = lft - 116444736000000000; // since 00:00:00 1/1/1970
            long seconds = intervals / 10000000; // get seconds 00:00:00 1/1/1970

            return (uint)seconds;
        }

        public static DateTime WykiTimeToDateTime(ulong wt)
        {
            if (wt == 0)
            {
                return DateTime.MinValue;
            }

            long filetime = 116444736000000000; // add FILETIME range offset to 00:00:00 1/1/1970 when Wyki time starts
            filetime += (long)wt * 10000000;    // convert seconds to 100 nanosecond intervals used by FILETIME
            DateTime dt = DateTime.FromFileTimeUtc(filetime);
            return dt;
        }

        public static UInt64 SwapByteOrder(UInt64 value)
        {
            UInt64 swapped = 
                ((0x00000000000000FF) & (value >> 56)
                |(0x000000000000FF00) & (value >> 40)
                |(0x0000000000FF0000) & (value >> 24)
                |(0x00000000FF000000) & (value >> 8)
                |(0x000000FF00000000) & (value << 8)
                |(0x0000FF0000000000) & (value << 24)
                |(0x00FF000000000000) & (value << 40)
                |(0xFF00000000000000) & (value << 56));

            return swapped;
        }

        public static UInt32 SwapByteOrder(UInt32 value)
        {
            UInt32 swapped =
                ( (0x000000FF) & (value >> 24)
                | (0x0000FF00) & (value << 8)
                | (0x00FF0000) & (value << 8)
                | (0xFF000000) & (value << 24));

            return swapped;
        }
    }
}
