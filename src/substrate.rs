// drawing primitives

const RGBA_PIXEL_DEPTH: u32 = 4;

pub enum DrawOp {
    Store,
    StoreInvert,
    Or,
    And,
    Xor,
    Clr,
}

#[derive(Copy, Clone)]
pub struct Point {
    x: u32,
    y: u32,
}

impl Point {
    pub fn new(x: u32, y: u32) -> Point {
        Point { x, y }
    }
}

#[derive(Copy, Clone)]
pub struct Rect {
    origin: Point,
    corner: Point,
}

impl Rect {
    pub fn new(origin: Point, corner: Point) -> Rect {
        Rect { origin, corner }
    }
}

#[derive(Copy, Clone)]
pub struct Color {
    r: u8,
    g: u8,
    b: u8,
    a: u8,
}

impl Color {
    pub fn new(rgba: u32) -> Color {
        let r: u8 = ((rgba >> 24) & 0xff) as u8;
        let g: u8 = ((rgba >> 16) & 0xff) as u8;
        let b: u8 = ((rgba >> 8) & 0xff) as u8;
        let a: u8 = (rgba & 0xff) as u8;
        Color { r, g, b, a }
    }
}

#[derive(Clone)]
pub struct Bitmap {
    x: u32,
    y: u32,
    w: u32,
    h: u32,
    d: u32,
    pixels: Vec<u8>,
}

impl Bitmap {
    pub fn new(x: u32, y: u32, width: u32, height: u32) -> Bitmap {
        Bitmap {
            x,
            y,
            w: width,
            h: height,
            d: RGBA_PIXEL_DEPTH,
            pixels: vec![0u8; (width * height * RGBA_PIXEL_DEPTH) as usize],
        }
    }

    pub fn new_at_origin(width: u32, height: u32) -> Bitmap {
        Bitmap::new(0, 0, width, height)
    }

    pub fn origin(self) -> Point {
        Point {
            x: self.x,
            y: self.y,
        }
    }

    pub fn corner(self) -> Point {
        Point {
            x: self.x + self.w,
            y: self.y + self.h,
        }
    }

    pub fn rect(self) -> Rect {
        Rect {
            origin: self.clone().origin(),
            corner: self.clone().corner(),
        }
    }

    pub fn get(self, x: u32, y: u32) -> Color {
        let pos = ((y * self.w + x) * self.d) as usize;
        let r = self.pixels[pos];
        let g = self.pixels[pos + 1];
        let b = self.pixels[pos + 2];
        let a = self.pixels[pos + 3];
        Color { r, g, b, a }
    }

    pub fn set(&mut self, x: u32, y: u32, color: Color) {
        let pos = ((y * self.w + x) * self.d) as usize;
        self.pixels[pos] = color.r;
        self.pixels[pos + 1] = color.g;
        self.pixels[pos + 2] = color.b;
        self.pixels[pos + 3] = color.a;
    }

    pub fn fill(&mut self, color: Color) {
        for y in 0..self.h {
            for x in 0..self.w {
                self.set(x, y, color)
            }
        }
    }

    pub fn pixels(&self) -> &[u8] {
        &self.pixels[..]
    }
}

pub fn bitblt(
    src: Bitmap,
    src_rect: &mut Rect,
    dst: &mut Bitmap,
    at: &mut Point,
    op: DrawOp,
    clip_rect: &mut Rect,
) {
    clip(src.clone(), src_rect, dst, at, clip_rect);
    copy_bits(src.clone(), src_rect.clone(), dst, at.clone(), op);
}

fn clip(src: Bitmap, src_rect: &mut Rect, dst: &mut Bitmap, at: &mut Point, clip_rect: &mut Rect) {}

fn copy_bits(src: Bitmap, src_rect: Rect, dst: &mut Bitmap, at: Point, op: DrawOp) {}

pub struct Mouse {}

pub struct Substrate {
    screen: Bitmap,
    mouse: Mouse,
}
