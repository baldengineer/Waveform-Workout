"""Reproduce the connector waveform and line spectrum using NumPy and Pillow."""
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def connector_gain(frequency):
    s = 2j * np.pi * frequency
    winding = 129 + s * 820e-6
    tank = winding / (1 + s * 56.65e-12 * winding)
    branch = 1 + tank * s * 75e-12
    passive = 1 / (branch + 5600 * s * (22e-12 * branch + 75e-12))
    follower = 1 / (1 + s / (2 * np.pi * 1e6))
    load = 1 / (1 + 340 * (1e-6 + s * 100e-12))
    return passive * follower * load


def simulate():
    tone = 250e3
    lsb = 3.45 / 4095
    # A high-high-low-low DAC pattern has fundamental Vpp = 4 * step / pi.
    code_span = round(np.pi / (4 * abs(connector_gain(tone)) * lsb))
    low = round(0.75 / connector_gain(0).real / lsb - code_span / 2)
    high = low + code_span
    levels = np.array([high, high, low, low]) * lsb
    count = 65536
    harmonic = np.arange(count // 2 + 1)
    # Fourier series of the four code levels held for 1 us each.
    spectrum = np.fft.fft(levels)[harmonic % 4] / 4
    spectrum *= np.sinc(harmonic / 4) * np.exp(-1j * np.pi * harmonic / 4)
    spectrum *= connector_gain(tone * harmonic)
    voltage = np.fft.irfft(count * spectrum, n=count)
    assert abs(4 * abs(spectrum[1]) - 1) < .001
    assert np.allclose(np.fft.rfft(voltage)[:8] / count, spectrum[:8])
    return low, high, levels, voltage, spectrum


def draw_figure(levels, voltage, spectrum):
    scale = 2
    canvas = Image.new("RGB", (1020 * scale, 745 * scale), "white")
    pen = ImageDraw.Draw(canvas)
    dark, mute, blue, copper = "#153641", "#496169", "#007486", "#a75f2a"

    def text(x, y, words, size=16, color=dark, anchor="la", bold=False):
        fonts = (["segoeuib.ttf", "DejaVuSans-Bold.ttf"] if bold else
                 ["segoeui.ttf", "DejaVuSans.ttf"])
        for name in fonts:
            try:
                font = ImageFont.truetype(name, size * scale)
                break
            except OSError:
                continue
        else:
            font = ImageFont.load_default(size=size * scale)
        pen.text((x * scale, y * scale), words, font=font, fill=color, anchor=anchor)

    def line(points, color, width=1):
        coords = [(round(x * scale), round(y * scale)) for x, y in points]
        pen.line(coords, fill=color, width=round(width * scale))

    text(38, 20, "250 kHz at the trainer connector", 31, bold=True)
    text(40, 69, "1 MSample/s   ·   3.45 V DAC reference   ·   1 MΩ in parallel with 100 pF", 17, mute)
    left, right = 83, 955
    x = lambda t: left + (right-left) * t / 12
    y = lambda v: 337 - (v-.15) * 184 / 1.2
    for value in (.25,.5,.75,1,1.25):
        line([(left,y(value)),(right,y(value))],"#d4dfe1")
        text(left-10,y(value),f"{value:.2f}",14,mute,"rm")
    for t in range(13):
        line([(x(t),153),(x(t),337)],"#e4eaeb")
        text(x(t),345,str(t),14,mute,"ma")
    text(39,109,"Voltage (V)",17,bold=True)
    for sx, name, color in [(325,"DAC pin (ideal hold)",copper),(604,"Loaded connector",blue)]:
        line([(sx,120),(sx+30,120)],color,2.5)
        text(sx+38,120,name,15,color,"lm")
    t_edges=np.repeat(np.arange(13),2)[1:-1]
    line(zip(x(t_edges),y(np.repeat(levels[np.arange(12)%4],2))),copper,1.8)
    t=np.linspace(0,12,4000)
    v=np.interp(t%4,np.arange(len(voltage))*4/len(voltage),voltage,period=4)
    line(zip(x(t),y(v)),blue,2.8)
    text(right,370,"Time (µs)",15,mute,"ra")
    text(39,410,"Connector spectrum (dBc)",20,bold=True)
    x_f=lambda mhz: left+(right-left)*mhz/2
    y_db=lambda db: 485-db*2.95
    for db in (0,-10,-20,-30,-40,-50,-60):
        line([(left,y_db(db)),(right,y_db(db))],"#d4dfe1")
        text(left-10,y_db(db),str(db),14,mute,"rm")
    for mhz in np.arange(9)*.25:
        text(x_f(mhz),672,f"{mhz:g}",14,mute,"ma")
    for n in (1,3,5,7):
        db=0.0 if n == 1 else 20*np.log10(abs(spectrum[n]/spectrum[1]))
        xval=x_f(n*.25)
        line([(xval,y_db(-60)),(xval,y_db(db))],blue,3)
        a, b = xval*scale, y_db(db)*scale
        pen.ellipse((a-5*scale,b-5*scale,a+5*scale,b+5*scale),fill=blue)
        text(xval,y_db(db)-21,f"{n*.25:g} MHz",15,dark,"mb",True)
        text(xval,y_db(db)-4,f"{db:.1f} dBc",14,mute,"mb")
    text(right,698,"Frequency (MHz)",15,mute,"ra")
    text(40,717,"Periodic steady state. DC offset omitted from spectrum.",15,mute)
    canvas.resize((1530,1118),Image.Resampling.LANCZOS).save(Path(__file__).with_name("250khz-connector.png"))


if __name__ == "__main__":
    low, high, levels, wave, spec = simulate()
    print("Codes, low/high:", low, high, "DAC volts:", levels[2], levels[0])
    print("Mean, min, max, trace Vpp:", wave.mean(), wave.min(), wave.max(), np.ptp(wave))
    for h in (1,3,5,7):
        print(h, "Vpp", 4*abs(spec[h]), "dBc",20*np.log10(abs(spec[h]/spec[1])))
    draw_figure(levels, wave, spec)
