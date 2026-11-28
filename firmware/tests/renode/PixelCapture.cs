// SPDX-License-Identifier: MIT
using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals.SPI;

namespace Antmicro.Renode.Peripherals.Mocks
{
    public class PixelCapture : ISPIPeripheral
    {
        public PixelCapture(IMachine machine)
        {
            this.machine = machine;
            Reset();
        }

        public byte Transmit(byte value)
        {
            bytes[position++] = value;
            if(position == bytes.Length)
            {
                LastFrame = BitConverter.ToString(bytes).Replace("-", "");
                FrameTimeMs = (long)machine.ElapsedVirtualTime.TimeElapsed.TotalMilliseconds;
                FrameCount++;
                position = 0;
            }
            return 0;
        }

        public void FinishTransmission() { }

        public void Reset()
        {
            position = 0;
            FrameCount = 0;
            FrameTimeMs = 0;
            LastFrame = "";
        }

        public string LastFrame { get; private set; }
        public long FrameTimeMs { get; private set; }
        public int FrameCount { get; private set; }

        // Nine GRB pixels, eight bits per color, four SPI bits per LED bit.
        private readonly byte[] bytes = new byte[9 * 3 * 4];
        private readonly IMachine machine;
        private int position;
    }
}
