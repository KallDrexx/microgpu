pub mod displays;

/// An RGB565 encoded buffer containing a frame that can be sent
/// to a display.
pub struct FrameBuffer {
    raw_bytes: Vec<u8>,
    width: usize,
    height: usize,
}

impl FrameBuffer {
    /// Creates a new frame buffer with the given dimensions.
    pub fn new(width: usize, height: usize) -> Self {
        Self {
            raw_bytes: vec![0; width * height * 2],
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

    pub fn bytes(&self) -> &[u8] {
        &self.raw_bytes
    }

    pub fn bytes_mut(&mut self) -> &mut [u8] {
        &mut self.raw_bytes
    }
}
