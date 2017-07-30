using System;
using System.IO;
using System.Text;
using System.Numerics;
using System.Collections.Generic;

public class DFT
{
    private uint freq;

    public DFT(uint frequency)
    {
        freq = frequency;
    }

    static List<Complex> transform(List<UInt16> buffer)
    {
        List<Complex> v = new List<Complex>(buffer.Count);

        for (int i = 0; i < freq; i++) {
            for (int j = 0; j < freq; j++) {
                v[j] += new Complex(buffer[j], buffer[j]) * get_roter(j * partial);
            }
        }

        return v;
    }

    static Complex get_roter(int theta)
    {
        return new Complex(0, 0);
    }

}

public class WaveReader
{
    private uint fileSize;
    private ushort nChannels;
    private uint samplingRate;
    private ushort depth;
    private uint dataSize;
    private BinaryReader reader;

    public ushort NumChannels
    {
        get {
            return nChannels;
        }
    }

    public uint SamplingRate
    {
        get {
            return samplingRate;
        }
    }

    public ushort Depth
    {
        get {
            return depth;
        }
    }

    private void Load()
    {
        uint riffHeader = reader.ReadUInt32();
        fileSize = reader.ReadUInt32();

        uint riffType = reader.ReadUInt32();

        uint chunkID = reader.ReadUInt32();
        uint chunkSize = reader.ReadUInt32();

        ushort formatId = reader.ReadUInt16();
        nChannels = reader.ReadUInt16();
        samplingRate = reader.ReadUInt32();
        uint dataRate = reader.ReadUInt32();
        ushort blockSize = reader.ReadUInt16();
        depth = reader.ReadUInt16();

        /*
        if (dataRate != nChannels * blockSize) {
        }
        */

        if (chunkSize > 16) {
            ushort xSize = reader.ReadUInt16();
            reader.ReadBytes(xSize);
        }

        uint dataID = reader.ReadUInt32();
        dataSize = reader.ReadUInt32();
    }

    public WaveReader(Stream st) {
        reader = new BinaryReader(st);
        Load();
    }

    public void Close()
    {
        reader.Close();
    }

    public Int16 ReadInt16()
    {
        return reader.ReadInt16();
    }

    public UInt32 ReadUInt32()
    {
        return reader.ReadUInt32();
    }

    public Int16 ReadMonoral()
    {
        return reader.ReadInt16();
    }

    public Int16[] ReadStereo()
    {
        Int16[] data = new Int16[2];

        data[0] = reader.ReadInt16();
        data[1] = reader.ReadInt16();

        return data;
    }
}

class Test {
    static void Main(string[] args)
    {
        WaveReader reader = new WaveReader(File.OpenRead(args[0]));
        Console.WriteLine("Number of Channels: {0}", reader.NumChannels);
        Console.WriteLine("Sampling Rate: {0}", reader.SamplingRate);
        Console.WriteLine("Depth: {0}", reader.Depth);

        try {
            for (;;) {
                Console.WriteLine(reader.ReadInt16());
            }
        }
        catch (System.IO.IOException e) {
        }
        finally {
            reader.Close();
        }
    }
}
