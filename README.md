# Sedona for ESP32-S3

An independent Sedona firmware development line for ESP32-S3, with SOX engineering
connectivity, platform services and host-side simulation tooling.

## Scope

ESP-IDF/FreeRTOS platform services, SOX, kit manifests, diagnostics,
persistent application data, Wi-Fi hardware and OpenCores Ethernet/QEMU profiles.
Host simulation and firmware have different module support; consult modules.py.

This is the preserved Bclimate-kit development line, distinct from the spro kits
in [SedonaPro](https://github.com/SedonaPro/sedonapro). Kit checksums and native
interfaces must match the selected firmware image.

## Build

Use Linux, Python 3, a JDK and GCC. Firmware builds additionally require an
initialized ESP-IDF 6.0.3 environment. A runtime source snapshot is included in
runtime/; no private sibling checkout is required. SEDONA_RUNTIME can select a
different compatible runtime deliberately.

```sh
git clone https://github.com/SedonaPro/sedona-esp32-s3.git
cd sedona-esp32-s3
python3 build.py sedona --profile qemu
python3 build.py fw --profile qemu
```

Host-side experimentation:

```sh
python3 build.py sim
python3 tests/sim_test.py
```

These commands describe the development workflow, not electrical emulation or a
claim of complete firmware acceptance. See build.py --help, modules.py,
[source notes](README_RU.md) and [publication scope](PUBLICATION.md).

## Hardware and compatibility

Configure the exact board, allowed pins and network locally. Firmware uses
32-bit references; host simulation uses 64-bit references. S3 and P4 have separate
source and release tracks. Shared ancestry does not make images interchangeable.
Generated I/O lists and software tests do not establish electrical correctness.

This is a source preview, not a signed production firmware release. Follow-up
board reports should state exact revision, wiring, toolchain and test scope.

## Community and licence

See [Contributing](CONTRIBUTING.md) and
[Discussions](https://github.com/SedonaPro/.github/discussions).
Our port code uses Apache-2.0. Framework/runtime and third-party components retain
their licences and attribution: [LICENSE](LICENSE), [NOTICE](NOTICE.md).
