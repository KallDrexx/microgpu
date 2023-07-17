pub mod displays;

/// An RGB565 encoded buffer containing a frame that can be sent
/// to a display.
pub struct FrameBuffer {
    pixels: Vec<u16>,
    width: usize,
    height: usize,
}

impl FrameBuffer {
    /// Creates a new frame buffer with the given dimensions.
    pub fn new(width: usize, height: usize) -> Self {
        Self {
            pixels: vec![0; width * height],
            width,
            height,
        }
    }

    pub fn width(&self) -> usize {
        self.width
    }

    pub fn height(&self) -> usize {
        self.height
    }

    pub fn pixels(&self) -> &[u16] {
        &self.pixels
    }

    pub fn mut_pixels(&mut self) -> &mut [u16] {
        &mut self.pixels
    }
}
