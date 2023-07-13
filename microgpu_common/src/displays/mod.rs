use crate::FrameBuffer;

pub mod ili9341;

/// A device that can be drawn to
pub trait EmbeddedDisplay {
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