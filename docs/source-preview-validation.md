# Source preview validation

15 November 2026, isolated Debian 12 amd64 build directory.

- Runtime Java tools and kits bootstrapped from the included source snapshot.
- python3 build.py sedona --profile qemu: passed.
- Generated kits.scode: 80696 bytes, 23 kit manifests; application image generated.
- This check compiled/staged the Sedona side. ESP-IDF firmware linking, flashing,
  live protocol tests, OTA and electrical I/O were not rerun for this publication.
- Historical hardware reports in the source notes describe earlier bench work.
  They are not new electrical acceptance of the publication snapshot.

No running bench service or physical controller was changed. Site-specific IPs
were removed; P4 network configuration defaults to DHCP.
