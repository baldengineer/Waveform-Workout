"""Plot the enabled PCB layers as a single, bookmarked PDF with KiCad 10."""

import argparse
from io import BytesIO
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory

import pcbnew
from pypdf import PdfReader, PdfWriter
from reportlab.pdfgen.canvas import Canvas


def layer_views(board_path):
    board = pcbnew.LoadBoard(str(board_path))
    enabled = board.GetEnabledLayers()
    layers = [pcbnew.LayerName(layer) for layer in enabled.SeqStackupForPlotting()]
    layers = [name for name in layers if name not in ("F.Fab", "B.Fab")]
    yield "All layers except fabrication", layers, False
    yield "Front layers", [name for name in layers if name.startswith("F.")], False
    yield "Back layers (solder-side view)", [name for name in layers if name.startswith("B.")], True
    for layer in enabled.CuStack():
        name = pcbnew.LayerName(layer)
        yield name, [name], False


def add_heading(page, title, page_number):
    width, height = float(page.mediabox.width), float(page.mediabox.height)
    buffer = BytesIO()
    header = Canvas(buffer, pagesize=(width, height), invariant=True)
    header.setFont("Helvetica-Bold", 15)
    header.drawString(24, height - 26, title)
    header.setFont("Helvetica", 10)
    header.drawRightString(width - 24, height - 25, f"Waveform Workout PCB   /   {page_number}")
    header.save()
    page.merge_page(PdfReader(buffer).pages[0])


def plot(board_path, output):
    result = PdfWriter()
    result.add_metadata({"/Title": "Waveform Workout PCB"})
    with TemporaryDirectory() as directory:
        for index, (title, layers, mirror) in enumerate(layer_views(board_path)):
            path = Path(directory) / f"{index}.pdf"
            print(f"{title}: {', '.join(layers)}", flush=True)
            command = ["kicad-cli", "pcb", "export", "pdf", "--mode-single",
                       "--no-property-popups", "--scale", "0", "--layers", ",".join(layers)]
            if mirror:
                command.append("--mirror")
            subprocess.run([*command, "--output", str(path), str(board_path)], check=True)
            source = PdfReader(path)
            if len(source.pages) != 1:
                raise ValueError(f"Expected a single page for {title}")
            page = result.add_page(source.pages[0])
            add_heading(page, title, index + 1)
            page.compress_content_streams()
            result.add_outline_item(title, index)
        result.write(str(output))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("board", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    plot(args.board, args.output)
