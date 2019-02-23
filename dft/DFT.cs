using System;
using System.Numerics;

namespace Fourier
{
    public class DFT
    {
        private uint freq_max;
        private const double PI2 = 2.0 * Math.PI;

        public DFT(uint freq_sampling)
        {
            freq_max = freq_sampling / 2;
        }

        public Complex[] transform(short[] sample, int len)
        {
            Complex[] spectre = new Complex[len];

            for (int i = 0; i < len; i++)
            {
                double re = 0.0;
                double im = 0.0;
                //double tmp = PI2 * i / freq;
                double tmp = PI2 * i / len;

                for (int j = 0; j < len; j++)
                {
                    re += sample[j] * Math.Cos(tmp * j);
                    im += sample[j] * Math.Sin(tmp * j);
                }
                spectre[i] = new Complex(re, im);
            }

            return spectre;
        }

    }
}
