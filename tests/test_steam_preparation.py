"""Image-preparation and non-destructive entry checks with synthetic data."""
import sys,unittest,tempfile,struct
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from PIL import Image
from prepare_steam_data import prepare_images,prepare

class SteamPreparationTests(unittest.TestCase):
    def test_images_keep_indices_and_map_alpha(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);gfx=root/'assets/gfx';gfx.mkdir(parents=True)
            ref=Image.new('P',(2,1));ref.putpalette([x for n in range(256) for x in (n,n,n)]);ref.putdata([1,2]);ref.save(gfx/'gfx.png')
            rgba=Image.new('RGBA',(2,1));rgba.putdata([(120,120,120,255),(255,255,255,0)]);rgba.save(gfx/'rgba.png')
            self.assertEqual(len(prepare_images(root)),2)
            raw=(gfx/'gfx.sgimg').read_bytes();self.assertEqual(raw[:12],b'SGI1'+struct.pack('<II',2,1));self.assertEqual(raw[1036:],bytes([1,2]))
            self.assertEqual((gfx/'rgba.sgimg').read_bytes()[1036:],bytes([120,0]))

    def test_existing_destination_is_rejected_before_reading(self):
        with tempfile.TemporaryDirectory() as temp:
            with self.assertRaises(FileExistsError):prepare(Path('missing.sgr'),Path(temp),Path('missing-ffmpeg'))

    def test_missing_ffmpeg_does_not_create_output(self):
        with tempfile.TemporaryDirectory() as temp:
            out=Path(temp)/'new'
            with self.assertRaises(ValueError):prepare(Path('missing.sgr'),out,Path('missing-ffmpeg'))
            self.assertFalse(out.exists())

if __name__=='__main__':unittest.main(verbosity=2)
