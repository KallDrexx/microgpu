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

/// A device that can be drawn to
pub trait Display {
    fn width(&self) -> usize;
    fn height(&self) -> usize;

    /// Draws the frame buffer to the display. The display takes ownership
    /// of the frame buffer in case the display does not contain it's own
    /// frame buffer and must be constantly fed pixels. This allows the 
    /// microcontroller to draw the next frame in a separate buffer.
    ///
    /// The display returns the previous frame buffer if it has one.
    fn draw_frame(&mut self, frame: FrameBuffer) -> Option<FrameBuffer>;
}
