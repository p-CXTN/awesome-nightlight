import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('release', Path(__file__).resolve().parents[1] / 'package_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class PackagingTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.build = self.root / 'build'
        self.images = self.root / 'images'
        self.build.mkdir()
        self.make_build(1)

    def make_build(self, revision):
        app = bytearray(256)
        app[0] = 0xe9
        app[32:36] = bytes.fromhex('3254cdab')
        app[100] = revision
        usb = bytearray(b'\xff' * 0x400000)
        usb[0x10000:0x10100] = app
        (self.build / 'NightlightRevK.ino.bin').write_bytes(app)
        (self.build / 'NightlightRevK.ino.merged.bin').write_bytes(usb)

    def test_versions_archive_and_repeat(self):
        latest, first = release.package(self.build, self.images)
        self.assertEqual(first['version'], '1.0.0')
        self.assertEqual(latest, self.images)
        self.assertFalse((self.images / 'latest').exists())
        for entry in first['files'].values():
            self.assertRegex(entry['name'], r'NightlightRevK-v1\.0\.0-\d{8}T\d{6}Z-ESP32C3-(OTA|USB)\.bin')
        self.assertEqual(release.package(self.build, self.images)[1], first)
        self.make_build(2)
        _, second = release.package(self.build, self.images)
        self.assertEqual(second['version'], '1.0.1')
        self.assertEqual(release.read_release(self.images / 'archive/v1.0.0'), first)
        self.assertEqual(release.read_release(latest), second)
        self.assertEqual(len(list(self.images.glob('*.bin'))), 2)
        with self.assertRaises(ValueError):
            release.package(self.build, self.images, '1.0.0')

    def test_bad_pair_leaves_current_untouched(self):
        latest, first = release.package(self.build, self.images)
        self.make_build(2)
        (self.build / 'NightlightRevK.ino.merged.bin').write_bytes(b'bad')
        with self.assertRaises(ValueError):
            release.package(self.build, self.images)
        self.assertEqual(release.read_release(latest), first)

    def test_legacy_is_preserved(self):
        self.images.mkdir()
        (self.images / 'ota.bin').write_bytes((self.build / 'NightlightRevK.ino.bin').read_bytes())
        (self.images / 'NightlightRevK-USB.bin').write_bytes((self.build / 'NightlightRevK.ino.merged.bin').read_bytes())
        (self.images / 'SHA256SUMS').write_text('legacy checksums\n')
        release.package(self.build, self.images)
        legacy = release.read_release(self.images / 'archive/v0.0.0-legacy')
        self.assertEqual(legacy['version'], '0.0.0-legacy')
        self.assertFalse((self.images / 'ota.bin').exists())

    def test_previous_layout_migrates_without_new_version(self):
        latest, first = release.package(self.build, self.images)
        previous_layout = self.images / 'latest'
        release.move_release(latest, previous_layout)
        root, migrated = release.package(self.build, self.images)
        self.assertEqual(root, self.images)
        self.assertEqual(migrated, first)
        self.assertFalse(previous_layout.exists())
        self.assertEqual(release.read_release(root), first)


if __name__ == '__main__':
    unittest.main()
