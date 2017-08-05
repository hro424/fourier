using System;
using System.IO;

public class WaveReader
{
    private uint fileSize;
    private ushort nChannels;
    private uint samplingRate;
    private uint dataRate;
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

    public uint DataRate
    {
        get {
            return dataRate;
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
        dataRate = reader.ReadUInt32();
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

    public int Read(byte[] buffer, int index, int count)
    {
        return reader.Read(buffer, index, count);
    }
}


