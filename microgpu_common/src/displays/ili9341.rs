use display_interface::WriteOnlyDataCommand;
use ili9341::Ili9341;

use super::EmbeddedDisplay;

impl<IFACE: WriteOnlyDataCommand, RESET> EmbeddedDisplay for Ili9341<IFACE, RESET> {
    fn width(&self) -> usize {
        self.width()
    }

    fn height(&self) -> usize {
        self.height()
    }

    fn draw_frame(&mut self, frame: crate::FrameBuffer) -> Option<crate::FrameBuffer> {
        let x1 = u16::try_from(self.width()).unwrap();
        let y1 = u16::try_from(self.height()).unwrap();

        let iterator = frame.bytes()
            .chunks_exact(2)
            .map(|x| x[0] as u16 | ((x[1] as u16) << 8));

        self.draw_raw_iter(0, 0, x1, y1, iterator).unwrap();

        // Since the ili9341 has it's own frame buffer, we can return the one
        // that was passed in.
        Some(frame)
    }
}