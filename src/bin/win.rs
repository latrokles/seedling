use sdl2::event::Event;
use sdl2::pixels::PixelFormatEnum;

use seedling::substrate::{Bitmap, Color};

const WIDTH: u32 = 600;
const HEIGHT: u32 = 400;
const DEPTH: u32 = 4;

fn main() {
    let sdl_ctx = sdl2::init().expect("Unable to initialize SDL!");
    let video = sdl_ctx.video().expect("Unable to initialize Video");
    let window = video
        .window("Window", WIDTH, HEIGHT)
        .position_centered()
        .borderless()
        .build()
        .expect("Unable to open window!");

    let mut canvas = window
        .into_canvas()
        .accelerated()
        .build()
        .expect("Unable to open renderer!");

    let texture_creator = canvas.texture_creator();

    let mut texture = texture_creator
        .create_texture_streaming(PixelFormatEnum::ABGR8888, WIDTH, HEIGHT)
        .expect("Unable to create framebuffer!");

    let white = Color::new(0xffffffff);
    let red = Color::new(0xff0000ff);
    let green = Color::new(0x00ff00ff);

    let mut fb = Bitmap::new_at_origin(WIDTH.try_into().unwrap(), HEIGHT.try_into().unwrap());
    fb.fill(white);

    let mut event_pump = sdl_ctx.event_pump().expect("Unable to create event pump");
    'running: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::KeyDown {
                    timestamp,
                    window_id,
                    keycode,
                    scancode,
                    keymod,
                    repeat,
                } => fb.fill(green),
                Event::MouseButtonDown {
                    timestamp,
                    window_id,
                    which,
                    mouse_btn,
                    clicks,
                    x,
                    y,
                } => fb.fill(red),
                Event::Quit { .. } => break 'running,
                _ => {}
            }
        }

        texture
            .with_lock(None, |texture_buffer: &mut [u8], _pitch: usize| {
                texture_buffer.copy_from_slice(fb.pixels());
            })
            .unwrap();

        canvas.clear();
        canvas.copy(&texture, None, None).unwrap();
        canvas.present();
    }
}
