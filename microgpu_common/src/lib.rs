#![no_std]

extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;
use embedded_graphics_core::pixelcolor::Rgb565;

pub mod displays;

pub enum Commands {
    Initialize {
        render_scale: u8,
    },

    DisplayFrame,

    ShowFps(bool),

    SetTexture {
        width: u8,
        height: u8,
        colors: Vec<Rgb565>,
    },

    ClearSprites,

    DefineSprite {
        id: u8,
        x: u8,
        y: u8,
        width: u8,
        height: u8,
    },

    DrawRectangle {
        x: u8,
        y: u8,
        width: u8,
        height: u8,
        fill: bool,
        color: Rgb565,
    },

    DrawCircle {
        center_x: u8,
        center_y: u8,
        radius: u8,
        fill: bool,
        color: Rgb565,
    },

    DrawTriangle {
        x0: u8,
        y0: u8,
        x1: u8,
        y1: u8,
        x2: u8,
        y2: u8,
        fill: bool,
        color: Rgb565,
    },

    DrawText {
        font_id: u8,
        color: Rgb565,
        text: String,
    }
}

pub enum Responses {
    Status(Status),
}

pub struct Status {
    pub display_width: u16,
    pub display_height: u16,
    pub render_width: u16,
    pub render_height: u16,
}
