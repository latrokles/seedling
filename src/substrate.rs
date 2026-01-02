// drawing primitives

const RGBA_PIXEL_DEPTH: isize = 4;

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
    x: isize,
    y: isize,
}

impl Point {
    pub fn new(x: isize, y: isize) -> Point {
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

    pub fn width(self) -> isize {
        self.corner.x - self.origin.x
    }

    pub fn height(self) -> isize {
        self.corner.y - self.origin.y
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
    x: isize,
    y: isize,
    w: isize,
    h: isize,
    d: isize,
    pixels: Vec<u8>,
}

impl Bitmap {
    pub fn new(x: isize, y: isize, width: isize, height: isize) -> Bitmap {
        Bitmap {
            x,
            y,
            w: width,
            h: height,
            d: RGBA_PIXEL_DEPTH,
            pixels: vec![0u8; (width * height * RGBA_PIXEL_DEPTH) as usize],
        }
    }

    pub fn new_at_origin(width: isize, height: isize) -> Bitmap {
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

    pub fn get(self, x: isize, y: isize) -> Color {
        let pos = ((y * self.w + x) * self.d) as usize;
        let r = self.pixels[pos];
        let g = self.pixels[pos + 1];
        let b = self.pixels[pos + 2];
        let a = self.pixels[pos + 3];
        Color { r, g, b, a }
    }

    pub fn set(&mut self, x: isize, y: isize, color: Color) {
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

fn clip(src: Bitmap, src_rect: &mut Rect, dst: &mut Bitmap, at: &mut Point, clip_rect: &mut Rect) {
    // if clip_rect is outside Bitmap dst, we discard the region that is out of bounds
    if clip_rect.origin.x < 0 {
        clip_rect.origin.x = 0
    }

    if clip_rect.origin.y < 0 {
        clip_rect.origin.y = 0
    }

    if clip_rect.corner.x > dst.w {
        clip_rect.corner.x = dst.w
    }

    if clip_rect.corner.y > dst.h {
        clip_rect.corner.y = dst.h
    }

    // clip and adjust src_rect according to clip_rect
    // along x axis
    if at.x <= clip_rect.origin.x {
        // the src_rect's left side is outside the clip_rect's left side
        // so we must adjust the src_rect's left side so we only copy the
        // src pixels that overlap with the clip_rect.
        // 1. move src_rect left side by the difference between at.x and clip_rect.origin.x
        // 2. adjust at.x to clip_rect.origin.x so we copy bits right at the clip_rect's origin.
        src_rect.origin.x += (clip_rect.origin.x - at.x);
        at.x = clip_rect.origin.x;
    }

    if (at.x + src_rect.width()) > clip_rect.corner.x {
        // the src_rect's right side is outside the clip_rect's right side
        // so we must adjust the src_rect's right side so we only copy the
        // src pixels that will fit into the clip_Rect.
        // 1. get the width of the clip_rect
        // 2. get the width of the src_rect (which has been adjusted above somewhat)
        // 3. if src_rect width is < clip_rect width do nothing
        // 4. otherwise get the difference and adjust src_rect left side by this amount.
        let diff = (at.x + src_rect.width()) - clip_rect.width();
        if diff > 0 {
            let delta_x = if diff > src_rect.width() {
                src_rect.width()
            } else {
                diff
            };
            src_rect.corner.x -= delta_x;
        }
    }

    // along y axis (we do the same but for y and height)
    if at.y <= clip_rect.origin.y {
        src_rect.origin.y += (clip_rect.origin.y - at.y);
        at.y = clip_rect.origin.y;
    }

    if (at.y + src_rect.height()) > clip_rect.corner.y {
        let diff = (at.y + src_rect.height()) - clip_rect.height();
        if diff > 0 {
            let delta_y = if diff > src_rect.height() {
                src_rect.height()
            } else {
                diff
            };
            src_rect.corner.y -= delta_y;
        }
    }

    if src_rect.origin.x < 0 {
        at.x = src_rect.origin.x;
        src_rect.origin.x = 0;
    }

    if src_rect.corner.x > src.w {
        src_rect.corner.x = src.w;
    }

    if src_rect.origin.y < 0 {
        at.y = src_rect.origin.y;
        src_rect.origin.y = 0;
    }

    if src_rect.corner.y > src.h {
        src_rect.corner.y = src.h;
    }
}

/*
 * For every row in src, merge the row's pixels with the pixels in
 * the corresponding row in dst.
 */
fn copy_bits(src: Bitmap, src_rect: Rect, dst: &mut Bitmap, at: Point, op: DrawOp) {
    let dst_y = at.y;

    'merge: for src_y in src_rect.origin.y..src_rect.corner.y {
        merge(
            src,
            src_rect.origin.x,
            src_y,
            dst,
            at.x,
            dst_y,
            src_rect.width(),
            op,
        );
    }
}

fn merge(
    src: Bitmap,
    src_x: isize,
    src_y: isize,
    dst: &mut Bitmap,
    dst_x: isize,
    dst_y: isize,
    n: isize,
    op: DrawOp,
) {
}

pub struct Mouse {}

pub struct Substrate {
    screen: Bitmap,
    mouse: Mouse,
}
