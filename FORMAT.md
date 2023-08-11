---
version: 0.1.0
---

# Focal Curve Encoding (FCE)

FCE is a simple encoding scheme with influences from the [Tiny Well-known Binary](https://github.com/TWKB/Specification) specification.

## Structure

### Header

Offset | Bits | Name        | Purpose
------ | ---- | ----------- | --------
0      | 3    | Type Number | Describes the OGC geometry type.
3      | 1    | Z Flag      | If set, coordinates contain a Z dimension.
4      | 6    | Depth       | Describes the level of indexing used.
10     | 8    | Bounds Size | 
18     | ?    | Bounds      | Bounding box, see below.
144    | ---  | ---         | ---

The **type number** is a value in the range [0, 7], where,

- 0 Empty
- 1 Point
- 2 Linestring
- 3 Polygon
- 4 MultiPoint
- 5 MultiLinestring
- 6 MultiPolygon
- 7 GeometryCollection

The **bounding box** is a delta-list of coordinates represented by varints, such that:

- [`xmin`, `Δx`, `ymin`, `Δy`]

Where `Δx`, for example, represents the relative maximum compared to `xmin` (i.e. `xmax` - `xmin`).

If the **Z flag** is on, then the bounding box takes the form:

- [`xmin`, `Δx`, `ymin`, `Δy`, `zmin`, `Δz`]

The geometry data immediately follows the header.

## Types

### Index Array

Every object is composed of index arrays. This is similar to TWKB's `PointArrays`, but these are made up of
1D-projected integers derived from coordinates. Particularly, the steps for generating an index array are as follows:

- Take lists of coordinates (XY or XYZ)
- Index using a [**space-filling curve**](https://en.wikipedia.org/wiki/Space-filling_curve) or indexing technique,
  such that the 2D/3D coordinates are projected down to 1D, **within the bounding box given in the header**.
- Create an ordering index on the curve indices.
- Sort the ordering indices based on the curve indices, and the curve indices themselves, in descending order.
- Compute the deltas between curve indices
- Varint encode the list of deltas and the ordering indices

### Geometry

Each geometry is stored in the same form:

```
count            unsigned varint
curve indices    unsigned varint[]
order indices    unsigned varint[]
```

The exception to this are Geometry within Geometry Collections, which also include
a geometry type indicator before the count:

```
type             3-bit int
count            unsigned varint
curve indices    unsigned varint[]
order indices    unsigned varint[]
```
