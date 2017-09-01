using System;
using System.Numerics;

namespace Fourier
{
    public class FFT
    {
        private uint freq;
        private const double PI2 = 2.0 * Math.PI;

        public FFT(uint frequency)
        {
            freq = frequency;
        }

        public Complex[] transform(short[] sample, int len)
        {
            int lengthOfTable = 1;
            int numOfStages = 0;
            while (lengthOfTable < len) {
                numOfStages++;
                lengthOfTable <<= 1;
            }

            Complex[] a = new Complex[lengthOfTable];
            for (int i = 0; i < sample.Length; i++)
            {
                a[i] = new Complex(sample[i], 0.0);
            }

            for (int i = sample.Length; i < lengthOfTable; i++)
            {
                a[i] = new Complex(0.0, 0.0);
            }

            for (int stage = 0; stage < numOfStages; stage++)
            {
                int numOfSteps = 1 << stage;
                int sizeOfModule = lengthOfTable >> numOfSteps;

                for (int start = 0; start < lengthOfTable;
                     start += sizeOfModule * 2)
                {
                    for (int i = 0; i < sizeOfModule; i++)
                    {
                        var x = start + i;
                        var y = x + sizeOfModule;
                        var z = 1 << i;
                        Complex left = a[x];
                        Complex right = a[y]; // XXX: Index out of bound

                        a[x] = left + right;
                        a[y] = (left - right) * GetRoter(z);
                    }
                }
            }

            return a;
        }

        private Complex GetRoter(int i)
        {
            double tmp = PI2 * i / freq;
            return new Complex(Math.Cos(tmp), Math.Sin(tmp));
        }

    }
}

