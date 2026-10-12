# Geometry and measurement conventions

All lengths below are millimetres. The diagrams explain the construction; they are not full-size print templates or tool-clearance simulations.

## Router circle

Let H be the finished hole radius, G the circular guide radius and C the cutting radius. The cutter centre runs on a circle of radius H - C. The guide touches the inside of the template at radius:

```
Template radius = H + G - C
Template diameter = hole diameter + guide diameter - cutter diameter
```

A 75 mm hole, 162 mm guide and 8 mm cutting diameter therefore require a template opening of R114.5 / diameter 229 mm. R32 with the same guide and cutter requires R109. The result is the template's inside edge, not the stock board's outside edge or the cutter path used to manufacture that template.

The hole cannot be smaller than the cutter, and the guide must exceed the cutting diameter. For a straight cutter, plunge depth does not enter this top-view equation. The cutting diameter is not automatically the shank diameter. The guide must be concentric and circular at the actual template-contact height; an irregular base is not compensated.

## Straight cylindrical stave shell

Inputs are finished outside radius R, finished radial wall thickness W, N main staves, outside milling allowance Po and inside allowance Pi. Then:

```
finished inside radius Ri = R - W
alpha = pi / N                       (radians)
division = 360 / N                    (degrees)
edge bevel from square = 180 / N      (degrees)
A = R + Po                           (centre to outer flat)
B = (Ri - Pi) * cos(alpha)            (centre to inner flat)
rough stock thickness = A - B
plain outer width = 2 A tan(alpha)
plain inner width = 2 B tan(alpha)
```

The outer polygon surrounds the required outside circle. The inner polygon corners remain inside the intended bore with its allowance. Using `Ri - Pi` directly for the inner flat would leave too little material at the corners; the cosine is not optional decoration.

Angles describe ideal part geometry, not saw calibration. A cylindrical stave uses a bevel along its long edges; its axial ends are square. A segmented ring made with short pieces may use the same plan geometry with a different physical saw setup. Tapered or curved shells need a different model and are not handled by this tab.

## Parallel spacer strips

A strip is rectangular in plan, with thickness s measured normal to its two parallel glue faces. It is centred on the nominal radial joint. Its long glue edges have no taper. The main staves retain the same angular pitch and bevel, but their edge lines move inward to reserve the strip's width.

For a main stave with left and right strip thicknesses L and Q (each either zero or s):

```
width deduction = (L + Q) / (2 cos(alpha))
outer width = 2 A tan(alpha) - width deduction
inner width = 2 B tan(alpha) - width deduction
face midpoint offset = (L - Q) / (4 cos(alpha))
```

Offsets use the stave's local tangential axis: positive is counterclockwise when looking toward -Z. The local radial axis points from the shell centre through that stave's nominal centre direction. With a strip on only one side, the actual face midpoint is no longer on that radial axis. The exported vertices preserve this asymmetry.

The rectangular strip spans radial positions:

```
inner end = B / cos(alpha) - s tan(alpha) / 2
outer end = max(A, A / cos(alpha) - s tan(alpha) / 2)
```

The maximum prevents a wide strip from falling inside the required outside circle. Protruding stock is intentional and is milled away. Strips that consume a stave's inner face are rejected.

N counts main staves, not main staves plus strips. Cadence inserts after Nth joints, starting at the cadence value. A non-dividing cadence gives a different wrap-around interval, which is reported. An explicit comma-separated list overrides cadence. Joints are 1-based; joint N joins the last stave back to stave 1. Glue-film thickness and compression are not additional parameters: use the intended assembled strip thickness and verify a dry fit.

## Depth, mounting and reach

Z=0 is the mounting plate's top face; +Z runs along the shell into the blank. The plate lies below zero and is not part of a stave's cut length.

```
finished bottom Z = measured router end gap + bottom trim
finished top Z = finished bottom Z + finished depth
stave/strip cut length = finished top Z + top trim
assembly height = cut length + base plate thickness
reachable axial stock length = cut length - measured router end gap
```

The interval from zero to the end gap remains inaccessible in this setup. Extra length keeps it outside the intended finished shell, so it can later be removed or finished from another setup. Plate thickness adds mounting height; adding it again to cut length would double-count the fixture.

The end gap is measured for the actual tool, holder, machine and fixture. It is not calculated from the cutter diameter or router base diameter. No plate diameter, robot reach envelope, collision model, feed, spindle speed or clamping solution is implied.

Stock estimates use one separating end-cut kerf per piece after an already squared stock end. The finished piece dimensions exclude kerf. Estimates do not nest parts in boards or include ripping, initial squaring, defects and offcuts.

## Export contract

Geometry JSON uses schema 1, explicit millimetres/degrees, right-handed coordinates and counterclockwise XY loops. Parts have stable position-based labels (`S001` for a main stave, `P001` for a strip after stave 1), plus a numbered assembly sequence. Axial intervals, stock widths and offsets remain numeric rather than formatted strings. The CSV is a human cut list with up to five decimal places; neither display precision nor extra decimal places imply achievable machining tolerance.

`machineReady: false` is mandatory. Consumers must validate schema, units, coordinate frame, geometry and their complete machining setup before developing any machine instructions. The current export is a modelling input, not a control file.

The original comparison reference was the Woodturners Resource segment calculator: https://www.woodturnersresource.com/extras/projects/segmentcalc/index.html . Parallel-strip compensation, independent allowances, saved designs and mounting-end conventions are defined by this application's model, not inferred from that reference.
