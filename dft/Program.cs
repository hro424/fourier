using System;
using System.IO;
using System.Text;
using System.Numerics;

namespace Fourier
{
    class Program
    {
        static void printHelp()
        {
            Console.WriteLine("usage: dft <wav file> <output>");
        }

        static void Main(string[] args)
        {
            WaveReader reader;
            DFT dft;
            StreamWriter writer;

            if (args.Length < 2)
            {
                printHelp();
                return;
            }

            reader = new WaveReader(File.OpenRead(args[0]));
            dft = new DFT(reader.SamplingRate);
            writer = File.CreateText(args[1]);

            Console.WriteLine("Number of Channels: {0}", reader.NumChannels);
            Console.WriteLine("Sampling Rate: {0}", reader.SamplingRate);
            Console.WriteLine("Depth: {0}", reader.Depth);

            try
            {
                short[] sample = new short[reader.SamplingRate];
                uint length = reader.SamplingRate / 2;

                for (uint i = 0; i < reader.SamplingRate; i++)
                {
                    sample[i] = reader.ReadInt16();
                }

                Complex[] result = dft.transform(sample, (int)length);
                for (uint i = 0; i < length; i++)
                {
                    writer.WriteLine("{0} {1}", i, (int)result[i].Real);
                }
            }
            catch (System.IO.IOException e)
            {
                Console.Write(e);
            }
            finally
            {
                writer.Close();
                reader.Close();
            }
        }
    }
}
