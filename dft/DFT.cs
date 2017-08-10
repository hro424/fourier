using System;
using System.IO;
using System.Text;

public class DFT
{
    private uint freq;
    private const double PI2 = 2.0 * Math.PI;

    public DFT(uint frequency)
    {
        freq = frequency;
    }

    public Tuple<double[], double[]> transform(short[] sample, int len)
    {
        double[] re = new double[len];
        double[] im = new double[len];

        for (int i = 0; i < len; i++) {
            double tmp = PI2 * i / freq;
            for (int j = 0; j < len; j++) {
                re[i] += sample[j] * Math.Cos(tmp * j);
                im[i] += sample[j] * Math.Sin(tmp * j);
            }
        }

        return new Tuple<double[], double[]>(re, im);
    }
}

class Test
{
    static void Main(string[] args)
    {
        WaveReader reader = new WaveReader(File.OpenRead(args[0]));
        DFT dft = new DFT(reader.SamplingRate);
        StreamWriter writer = File.CreateText(args[1]);

        Console.WriteLine("Number of Channels: {0}", reader.NumChannels);
        Console.WriteLine("Sampling Rate: {0}", reader.SamplingRate);
        Console.WriteLine("Depth: {0}", reader.Depth);

        try {
            short[] sample = new short[reader.SamplingRate];
            uint length = reader.SamplingRate / 2;

            for (uint i = 0; i < reader.SamplingRate; i++) {
                sample[i] = reader.ReadInt16();
            }

            Tuple<double[], double[]> result = dft.transform(sample, (int)length);
            for (uint i = 0; i < length; i++) {
                writer.WriteLine("{0} {1}", i, (int)result.Item1[i]);
            }
        }
        catch (System.IO.IOException e) {
        }
        finally {
            writer.Close();
            reader.Close();
        }
    }
}
