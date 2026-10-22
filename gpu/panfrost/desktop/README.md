# Desktop renders

These 512x384 images are produced by the onetest panfrost host model through
the family AXI4 DPI bridge. They are validation evidence for the three
supported profiles:

| file | profile | result |
|---|---|---|
| `g52_512x384.png` | Mali-G52 (Bifrost) | native 512x384 render, desktop composition PASS |
| `g76_512x384.png` | Mali-G76 (Bifrost) | native 512x384 render, desktop composition PASS |
| `g78_512x384.png` | Mali-G78AE (Valhall) | native 512x384 render, **byte-identical to the source desktop texture** |

Notes:

* The G78 image was rendered with the official G78AE testbench flow; the
  render target matches the source texture byte-for-byte
  (`byte diffs = 0`, `pixel diffs = 0`).
* The desktop artwork intentionally contains CJK labels; that is image
  content, not source text. The model delivery itself is all-English.
* No vendor testbench, DDK, or other proprietary file is shipped in this
  directory -- only the rendered PNG output.
